/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "tcp_server.h"

#include <KDUtils/logging.h>

using namespace KDFoundation;

namespace KDNetwork {

TcpServer::TcpServer()
{
}

TcpServer::~TcpServer()
{
    close();
}

/**
 * @brief Sets the callback function to be invoked when a new connection is accepted.
 *
 * The provided callback will receive ownership of the new TcpSocket. If no callback
 * is set, accepted sockets will be immediately closed and destroyed.
 *
 * @param callback The function or lambda to call with the new TcpSocket.
 */
void TcpServer::setNewConnectionCallback(NewConnectionCallback callback)
{
    m_newConnectionCallback = std::move(callback);
}

/**
 * @brief Starts listening for incoming connections on the specified address and port.
 *
 * If the address is empty or "0.0.0.0" (IPv4) / "::" (IPv6), the server listens on all available network interfaces.
 * Uses getaddrinfo for robust address resolution and socket creation.
 * Sets SO_REUSEADDR socket option.
 *
 * @param address The IP address string (e.g., "127.0.0.1", "0.0.0.0", "::1", "::") or hostname to listen on.
 * @param port The port number to listen on.
 * @param backlog The maximum length of the queue for pending connections (passed to ::listen).
 * @return True if listening started successfully, false otherwise (check errorOccurred signal).
 */
bool TcpServer::listen(const std::string &address, uint16_t port, int backlog)
{
    if (isListening()) {
        setError(SocketError::InvalidSocketError, 0); // Already listening
        return false;
    }

    // 1. Resolve Address and Setup Socket Address Structure using getaddrinfo
    addrinfo hints = {};
    hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE; // For binding to listen: fill IP automatically if address is empty/null
    addrinfo *result = nullptr;
    std::string service = std::to_string(port);

    // Use nullptr for address if string is empty, otherwise pass C string
    const char *nodeName = address.empty() ? nullptr : address.c_str();

    int gaiError = ::getaddrinfo(nodeName, service.c_str(), &hints, &result);
    if (gaiError != 0) {
        // Provide a more specific error if possible, or map gaiError
        setError(SocketError::AddressResolutionError, gaiError);
        return false;
    }
    // Use unique_ptr for automatic cleanup of getaddrinfo results
    std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> addrInfoPtr(result, freeaddrinfo);
    if (!addrInfoPtr) {
        // Should not happen if getaddrinfo returned 0, but check anyway
        setError(SocketError::AddressResolutionError, 0);
        return false;
    }

    // 2. Create Listening Socket based on resolved address info
    m_listeningFd = ::socket(addrInfoPtr->ai_family, addrInfoPtr->ai_socktype, addrInfoPtr->ai_protocol);
#if defined(KD_PLATFORM_WIN32)
    if (m_listeningFd == INVALID_SOCKET) {
        setError(SocketError::SocketCreationError, WSAGetLastError());
        m_listeningFd = -1;
        return false;
    }
#else
    if (m_listeningFd < 0) {
        setError(SocketError::SocketCreationError, errno);
        m_listeningFd = -1;
        return false;
    }
#endif

    // 3. Set SO_REUSEADDR Socket Option (Crucial for servers to quickly restart)
    int reuse = 1;
#if defined(KD_PLATFORM_WIN32)
    if (::setsockopt(m_listeningFd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&reuse), sizeof(reuse)) < 0) {
        int error_code = WSAGetLastError();
        setError(SocketError::UnsupportedOperationError, error_code);
        ::closesocket(m_listeningFd);
        m_listeningFd = -1;
        return false;
    }
    // Consider SO_EXCLUSIVEADDRUSE on Windows for stricter binding?
#else
    if (::setsockopt(m_listeningFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        setError(SocketError::UnsupportedOperationError, errno);
        ::close(m_listeningFd);
        m_listeningFd = -1;
        return false;
    }
    // SO_REUSEPORT might be useful on Linux/BSD for advanced scenarios (multiple servers on same port)
#endif

    // 4. Bind Socket to the Address
    if (::bind(m_listeningFd, addrInfoPtr->ai_addr, static_cast<socklen_t>(addrInfoPtr->ai_addrlen)) < 0) {
#if defined(KD_PLATFORM_WIN32)
        int error_code = WSAGetLastError();
        setError(SocketError::BindError, error_code);
        ::closesocket(m_listeningFd);
#else
        setError(SocketError::BindError, errno);
        ::close(m_listeningFd);
#endif
        m_listeningFd = -1;
        return false;
    }

    // 5. Start Listening for Incoming Connections
    if (::listen(m_listeningFd, backlog) < 0) {
#if defined(KD_PLATFORM_WIN32)
        int error_code = WSAGetLastError();
        setError(SocketError::ListenError, error_code);
        ::closesocket(m_listeningFd);
#else
        setError(SocketError::ListenError, errno);
        ::close(m_listeningFd);
#endif
        m_listeningFd = -1;
        return false;
    }

    // 6. Set Listening Socket to Non-Blocking Mode
    // This ensures that the accept() calls in onIncomingConnection() do not block.
#if defined(KD_PLATFORM_WIN32)
    u_long mode = 1; // 1 for non-blocking
    if (ioctlsocket(m_listeningFd, FIONBIO, &mode) != 0) {
        int error_code = WSAGetLastError();
        setError(SocketError::SetNonBlockingError, error_code);
        ::closesocket(m_listeningFd);
        m_listeningFd = -1;
        return false;
    }
#else
    int flags = ::fcntl(m_listeningFd, F_GETFL, 0);
    if (flags == -1 || ::fcntl(m_listeningFd, F_SETFL, flags | O_NONBLOCK) == -1) {
        setError(SocketError::SetNonBlockingError, errno);
        ::close(m_listeningFd);
        m_listeningFd = -1;
        return false;
    }
#endif

    // 7. Setup FileDescriptorNotifier for Incoming Connections
    try {
        // Read readiness == pending connection
        m_listenNotifier = std::make_unique<FileDescriptorNotifier>(m_listeningFd, FileDescriptorNotifier::NotificationType::Read);

        // Connect the notifier's signal to our internal slot
        std::ignore = m_listenNotifier->triggered.connect([this]() { this->onIncomingConnection(); });
    } catch (...) { // Catch potential exceptions from notifier creation/setup
        setError(SocketError::UnknownError, 0); // Or specific NotifierError?
#if defined(KD_PLATFORM_WIN32)
        ::closesocket(m_listeningFd);
#else
        ::close(m_listeningFd);
#endif
        m_listeningFd = -1;
        m_listenNotifier.reset();
        return false;
    }

    // Success!
    m_isListening = true;
    setError(SocketError::NoError); // Clear any previous error state

    // TODO: Optionally retrieve and store the actual bound address/port using getsockname()

    return true;
}

/**
 * @brief Stops the server from listening for new connections.
 * Closes the listening socket. Already accepted connections are not affected.
 */
void TcpServer::close()
{
    if (!m_isListening && m_listeningFd < 0) {
        return; // Already closed or never opened
    }

    m_isListening = false; // Mark as not listening immediately

    // 1. Disable and destroy the notifier *before* closing the FD
    m_listenNotifier.reset();

    // 2. Close the listening socket file descriptor
    if (isValid()) {
#if defined(KD_PLATFORM_WIN32)
        ::closesocket(m_listeningFd);
#else
        ::close(m_listeningFd);
#endif
        m_listeningFd = -1; // Mark FD as invalid
    }

    // Keep the last error if close() was called explicitly after an error.
}

/**
 * @brief Internal function connected to the listening socket's read notifier (m_listenNotifier).
 * This method is called by the event loop when there are pending incoming connections.
 * It calls ::accept() in a loop to handle all pending connections for this event cycle.
 */
void TcpServer::onIncomingConnection()
{
    // This slot is called when m_listenNotifier emits 'activated'.
    if (!m_isListening)
        return; // Should not happen if notifier is disabled on close, but check anyway

    // Loop to accept all pending connections for this event cycle
    while (m_isListening) {
        sockaddr_storage peerAddr; // Use sockaddr_storage to hold IPv4 or IPv6
        socklen_t peerAddrLen = sizeof(peerAddr);
        int clientFd = -1; // Accepted client socket descriptor

#if defined(KD_PLATFORM_WIN32)
        // Accept the connection
        clientFd = static_cast<int>(::accept(m_listeningFd, reinterpret_cast<sockaddr *>(&peerAddr), &peerAddrLen));

        if (clientFd == INVALID_SOCKET) {
            int error_code = WSAGetLastError();
            if (error_code == WSAEWOULDBLOCK) {
                // No more pending connections waiting to be accepted right now.
                return; // Exit the loop and wait for the next notification.
            } else {
                // An actual error occurred during accept()
                // (e.g., connection reset before accept, resource issue)
                // Log this error? Should we stop the server? Emit errorOccurred?
                KDUtils::Logger::logger("KDNetwork")->error("TcpServer::onIncomingConnection: accept() failed: {}", error_code);
                setError(SocketError::UnknownError, error_code); // Or a specific AcceptError enum value
                // For robustness, let's assume we can continue listening unless it's a fatal error.
                // Depending on the error, might need to close and restart listening.
                return; // Exit loop for now to avoid potential infinite error loop.
            }
        }
#else
        // Accept the connection
        clientFd = ::accept(m_listeningFd, reinterpret_cast<sockaddr *>(&peerAddr), &peerAddrLen);

        if (clientFd < 0) {
            // An error occurred during accept()
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No more pending connections waiting to be accepted right now.
                return; // Exit the loop and wait for the next notification.
            } else if (errno == ECONNABORTED) {
                // A connection was aborted by the client after it was queued but before accept()
                // This is common, just ignore it and try accepting the next one (if any).
                continue; // Go to the next iteration of the while loop.
            } else {
                // Another accept error (e.g., file descriptor limit, network error)
                setError(SocketError::UnknownError, errno); // Or a specific AcceptError enum value
                // Log error? Stop server?
                return; // Exit loop for now.
            }
        }
#endif

        // If we reach here, a connection was successfully accepted (clientFd is valid)

        // Create a new TcpSocket instance to manage this connection.
        // Pass the accepted clientFd and mark initial state as Connected.
        // The TcpSocket constructor should handle setting the new socket to non-blocking.
        std::unique_ptr<TcpSocket> newSocket = nullptr;
        try {
            newSocket = std::make_unique<TcpSocket>(clientFd, Socket::State::Connected);
        } catch (...) {
            // Handle potential exceptions during TcpSocket creation (e.g., memory allocation)
#if defined(KD_PLATFORM_WIN32)
            ::closesocket(clientFd); // Clean up the accepted FD if socket creation failed
#else
            ::close(clientFd);
#endif
            KDUtils::Logger::logger("KDNetwork")->error("TcpServer::onIncomingConnection: TcpSocket creation failed");
            setError(SocketError::UnknownError, 0); // Indicate resource issue?
            continue; // Try accepting next connection
        }

        if (!newSocket || !newSocket->isValid()) {
            // TcpSocket constructor failed internally (e.g., couldn't set non-blocking)
            // The TcpSocket should ideally log or signal its own construction failure.
            // We don't have the socket object if make_unique failed or it invalidated itself.
            // Ensure FD is closed if newSocket is null or invalid but FD was > -1 initially.
#if defined(KD_PLATFORM_WIN32)
            // No easy way to check if newSocket closed the FD, assume it didn't if invalid.
            // If newSocket is null, clientFd was never passed. If invalid, maybe it closed it? Risky.
            if (clientFd != INVALID_SOCKET && (!newSocket || !newSocket->isValid()))
                ::closesocket(clientFd);
#else
            if (clientFd >= 0 && (!newSocket || !newSocket->isValid()))
                ::close(clientFd);
#endif
            continue; // Try accepting next connection
        }

        // TODO: Extract peer IP address and port from peerAddr
        // and store it in the TcpSocket object or pass it with the signal.

        // Invoke the callback (if registered), transferring ownership of the unique_ptr
        if (m_newConnectionCallback) {
            try {
                // Move the socket ownership into the callback
                m_newConnectionCallback(std::move(newSocket));
            } catch (const std::exception &e) {
                // User callback threw an exception. Log it.
                KDUtils::Logger::logger("KDNetwork")->error("Exception in newConnection callback: {}", e.what());
                // The socket object's ownership was transferred, we can't do much here.
                // The unique_ptr might have been destroyed within the callback's scope if it didn't store it.
            } catch (...) {
                // User callback threw something else. Log it.
                KDUtils::Logger::logger("KDNetwork")->error("Unknown exception in newConnection callback.");
            }
        } else {
            // No callback registered. The accepted socket (newSocket) will be
            // automatically closed and destroyed when the unique_ptr goes out of scope here.
            // Log a warning
            KDUtils::Logger::logger("KDNetwork")->warn("TcpServer: Accepted connection but no callback registered. Connection closed.");
        }

        // Loop back to accept the next pending connection (if any)
    } // End while loop
}

/**
 * @brief Helper method to set the internal error state and emit the errorOccurred signal.
 * @param error The SocketError code.
 * @param sysErrno Optional system errno or WSAGetLastError() value for logging/debugging.
 */
void TcpServer::setError(SocketError error, int sysErrno)
{
    m_lastError = error;
    if (error == SocketError::NoError) {
        m_lastErrorCode = make_error_code(SocketError::NoError);
    } else {
        if (sysErrno != 0) {
            // Create system_error from errno/WSAGetLastError
            m_lastErrorCode = std::error_code(sysErrno, std::system_category());
        } else {
            // Use our custom category if no system error provided
            m_lastErrorCode = make_error_code(error);
        }
        // Emit error signal only if it's a real error state
        errorOccurred.emit(error);
    }
}

bool TcpServer::isValid() const noexcept
{
#if defined(KD_PLATFORM_WIN32)
    return m_listeningFd != INVALID_SOCKET && m_listeningFd != -1;
#else
    return m_listeningFd >= 0;
#endif
}

} // namespace KDNetwork
