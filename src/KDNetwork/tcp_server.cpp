/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/tcp_server.h>
#include <KDNetwork/dns_resolver.h>
#include <KDNetwork/socket_error.h>

#include <KDUtils/logging.h>

#include <type_traits>

#if defined(KD_PLATFORM_WIN32)
#include <WinSock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#endif

namespace KDNetwork {

TcpServer::TcpServer()
{
}

TcpServer::~TcpServer()
{
    close();
}

void TcpServer::setNewConnectionCallback(NewConnectionCallback callback)
{
    m_newConnectionCallback = std::move(callback);
}

bool TcpServer::listen(const std::string &host, uint16_t port, int backlog)
{
    // Only proceed if we're not already listening or resolving
    if (m_state != State::NotListening) {
        setError(SocketError::ServerIsAlreadyListening);
        return false;
    }

    // Set state to Resolving
    setState(State::Resolving);

    // Use the DnsResolver for async hostname resolution
    auto &resolver = DnsResolver::instance();

    // Start asynchronous DNS lookup for host
    const bool lookupStarted = resolver.lookup(host, [this, port, backlog](std::error_code ec, const DnsResolver::AddressInfoList &addresses) {
        this->handleDnsLookupCompleted(ec, addresses, port, backlog);
    });

    if (!lookupStarted) {
        // DNS lookup failed to start
        setState(State::NotListening);
        setError(SocketError::AddressResolutionError);
        return false;
    }

    return true; // Successfully initiated DNS resolution
}

bool TcpServer::listen(const IpAddress &address, uint16_t port, int backlog)
{
    // Only proceed if we're not already listening or resolving
    if (m_state != State::NotListening) {
        setError(SocketError::ServerIsAlreadyListening);
        return false;
    }

    // Directly call the common implementation with the provided IP address
    return listenOnAddress(address, port, backlog);
}

void TcpServer::handleDnsLookupCompleted(std::error_code ec, const std::vector<IpAddress> &addresses,
                                         uint16_t port, int backlog)
{
    // Check for DNS errors or no addresses resolved
    if (ec || addresses.empty()) {
        setError(SocketError::AddressResolutionError, ec.value());
        setState(State::NotListening);
        return;
    }

    // Use the first resolved address
    const IpAddress &address = addresses[0];

    // Call common implementation with the resolved address
    if (!listenOnAddress(address, port, backlog)) {
        // Error already set by listenOnAddress
        setState(State::NotListening);
    }
}

bool TcpServer::listenOnAddress(const IpAddress &address, uint16_t port, int backlog)
{
    // Validate address is not null
    if (address.isNull()) {
        setError(SocketError::AddressResolutionError);
        return false;
    }

    // Create a socket
    const int family = address.isIPv4() ? AF_INET : AF_INET6;
    m_listeningFd = static_cast<int>(socket(family, SOCK_STREAM, 0));

    if (m_listeningFd < 0) {
#if defined(KD_PLATFORM_WIN32)
        const int error = WSAGetLastError();
#else
        int error = errno;
#endif
        setError(SocketError::SocketCreationError, error);
        return false;
    }

    // Set socket to non-blocking mode
#if defined(KD_PLATFORM_WIN32)
    u_long mode = 1; // 1 = non-blocking
    if (ioctlsocket(m_listeningFd, FIONBIO, &mode) != 0) {
        const int error = WSAGetLastError();
        setError(SocketError::SocketConfigurationError, error);
        closesocket(m_listeningFd);
        m_listeningFd = -1;
        return false;
    }
#else
    int flags = fcntl(m_listeningFd, F_GETFL, 0);
    if (flags < 0 || fcntl(m_listeningFd, F_SETFL, flags | O_NONBLOCK) < 0) {
        setError(SocketError::SocketConfigurationError, errno);
        ::close(m_listeningFd);
        m_listeningFd = -1;
        return false;
    }
#endif

    // Set SO_REUSEADDR to allow binding to recently closed ports
    int reuseAddrOption = 1;
    if (setsockopt(m_listeningFd, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<const char *>(&reuseAddrOption), sizeof(reuseAddrOption)) != 0) {
#if defined(KD_PLATFORM_WIN32)
        const int error = WSAGetLastError();
#else
        int error = errno;
#endif
        setError(SocketError::SocketConfigurationError, error);
#if defined(KD_PLATFORM_WIN32)
        closesocket(m_listeningFd);
#else
        ::close(m_listeningFd);
#endif
        m_listeningFd = -1;
        return false;
    }

    // Create sockaddr structure for binding
    struct sockaddr_storage addr;
    socklen_t addrLen = sizeof(addr);

    // Use the IpAddress's toSockAddr method to fill in the sockaddr structure
    if (!address.toSockAddr(reinterpret_cast<struct sockaddr *>(&addr), addrLen, port)) {
        setError(SocketError::AddressResolutionError);
#if defined(KD_PLATFORM_WIN32)
        closesocket(m_listeningFd);
#else
        ::close(m_listeningFd);
#endif
        m_listeningFd = -1;
        return false;
    }

    // Bind socket to the specified address and port
    if (bind(m_listeningFd, reinterpret_cast<struct sockaddr *>(&addr), addrLen) != 0) {
#if defined(KD_PLATFORM_WIN32)
        const int error = WSAGetLastError();
#else
        int error = errno;
#endif
        setError(SocketError::BindError, error);
#if defined(KD_PLATFORM_WIN32)
        closesocket(m_listeningFd);
#else
        ::close(m_listeningFd);
#endif
        m_listeningFd = -1;
        return false;
    }

    // Start listening for connections
    if (::listen(m_listeningFd, backlog) != 0) {
#if defined(KD_PLATFORM_WIN32)
        const int error = WSAGetLastError();
#else
        int error = errno;
#endif
        setError(SocketError::ListenError, error);
#if defined(KD_PLATFORM_WIN32)
        closesocket(m_listeningFd);
#else
        ::close(m_listeningFd);
#endif
        m_listeningFd = -1;
        return false;
    }

    // If we provided port 0, we need to find out what port we got
    if (port == 0) {
        struct sockaddr_storage boundAddr;
        socklen_t boundAddrLen = sizeof(boundAddr);
        if (getsockname(m_listeningFd, reinterpret_cast<struct sockaddr *>(&boundAddr), &boundAddrLen) == 0) {
            if (boundAddr.ss_family == AF_INET) {
                port = ntohs(reinterpret_cast<struct sockaddr_in *>(&boundAddr)->sin_port);
            } else if (boundAddr.ss_family == AF_INET6) {
                port = ntohs(reinterpret_cast<struct sockaddr_in6 *>(&boundAddr)->sin6_port);
            }
        }
    }

    // Store server address and port
    m_serverAddress = address;
    m_serverPort = port;

    // Create a notifier to monitor the listening socket for incoming connections
    m_listenNotifier = std::make_unique<KDFoundation::FileDescriptorNotifier>(
            m_listeningFd, KDFoundation::FileDescriptorNotifier::NotificationType::Read);

    // Connect a handler to the notifier
    std::ignore = m_listenNotifier->triggered.connect([this]() {
        this->onIncomingConnection();
    });

    // Set state to Listening
    setState(State::Listening);

    // Enable the notifier
    m_listenNotifier->setEnabled(true);

    // Emit the listeningStarted signal
    listeningStarted.emit();

    return true;
}

void TcpServer::close()
{
    // Clean up the notifier first
    m_listenNotifier.reset();

    // Close the socket if it's open
    if (m_listeningFd >= 0) {
#if defined(KD_PLATFORM_WIN32)
        closesocket(m_listeningFd);
#else
        ::close(m_listeningFd);
#endif
        m_listeningFd = -1;
    }

    // Reset address and port
    m_serverAddress = IpAddress();
    m_serverPort = 0;

    // Update state
    setState(State::NotListening);
}

void TcpServer::setState(State newState)
{
    if (m_state != newState) {
        m_state = newState;
        stateChanged.emit(m_state);
    }
}

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
    return m_listeningFd >= 0;
}

void TcpServer::onIncomingConnection()
{
    if (!isValid() || m_state != State::Listening)
        return;

    struct sockaddr_storage clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Accept the connection
    const int clientFd = accept(m_listeningFd, reinterpret_cast<struct sockaddr *>(&clientAddr), &clientAddrLen);

    if (clientFd < 0) {
#if defined(KD_PLATFORM_WIN32)
        const int error = WSAGetLastError();
        // Connection aborted or would block is not a fatal error
        if (error != WSAEINTR && error != WSAEWOULDBLOCK && error != WSAECONNABORTED) {
            setError(SocketError::ServerAcceptError, error);
        }
#else
        // EAGAIN, EWOULDBLOCK, or EINTR means we should try again later, not a real error
        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
            setError(SocketError::ServerAcceptError, errno);
        }
#endif
        return; // Wait for next incoming connection
    }

    // Creating a socket for the new connection
    auto newSocket = std::make_unique<TcpSocket>(clientFd, Socket::State::Connected);

    // Call the callback with the new socket
    if (m_newConnectionCallback) {
        m_newConnectionCallback(std::move(newSocket));
    } else {
        // No callback registered, just close the socket
        newSocket->close();
    }
}

} // namespace KDNetwork
