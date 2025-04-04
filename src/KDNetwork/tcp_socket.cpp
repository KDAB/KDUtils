/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "tcp_socket.h"

#include <KDUtils/logging.h>

#if defined(KD_PLATFORM_WIN32)
#else
#include <errno.h>
#include <netdb.h> // For getaddrinfo (used as placeholder)
#include <sys/socket.h> // For getsockopt, SO_ERROR, send flags (MSG_NOSIGNAL)
#include <unistd.h> // For read/write/send/recv
#endif // KD_PLATFORM_WIN32

namespace KDNetwork {

TcpSocket::TcpSocket()
    : Socket(SocketType::Tcp)
{
}

TcpSocket::TcpSocket(int connectedFd, State initialState)
    : Socket(SocketType::Tcp)
{
    // This constructor takes an already connected FD (e.g., from accept)
#if defined(KD_PLATFORM_WIN32)
    if (connectedFd != INVALID_SOCKET) {
#else
    if (connectedFd >= 0) {
#endif
        m_socketFd = connectedFd; // Take ownership of the FD
        setState(initialState); // Usually State::Connected
        setError(SocketError::NoError); // Assume no error initially

        // Accepted sockets should generally be non-blocking for async use
        if (!setBlocking(false)) {
            // Failed to set non-blocking, this is problematic
            // Error already set by setBlocking
            cleanupSocket(); // Clean up failed setup
            // State remains Unconnected due to cleanupSocket
        } else {
            // Setup notifiers for the existing FD
            setupNotifiers();
            // Read notifier should be enabled by default for connected sockets
            setReadNotificationEnabled(true);
            // Write notifier disabled until needed
            setWriteNotificationEnabled(false);
        }
    } else {
        setError(SocketError::InvalidSocketError); // Invalid FD passed
        setState(State::Unconnected); // Ensure state is correct
    }
}

TcpSocket::~TcpSocket()
{
}

bool TcpSocket::connectToHost(const std::string &host, std::uint16_t port)
{
    if (state() != State::Unconnected) {
        setError(SocketError::InvalidSocketError); // Can only connect from Unconnected state
        return false;
    }

    // --- Asynchronous DNS Lookup Integration Point ---
    // This is where we would integrate an asynchronous DNS resolver such as c-ares.
    // TODO: Replace synchronous getaddrinfo with a call to an asynchronous DnsResolver class.
    // The DnsResolver would take the hostname/port and a callback/lambda.
    // When the lookup completes, the callback would execute the socket opening
    // and connection logic below. For now, we use synchronous getaddrinfo as a placeholder.
    // WARNING: Synchronous getaddrinfo will block the event loop!
    // ----------------------------------------------------
    addrinfo hints = {};
    hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP
    addrinfo *result = nullptr;
    std::string service = std::to_string(port);

    // *** START SYNC DNS LOOKUP (Replace with Async) ***
    int gaiError = ::getaddrinfo(host.c_str(), service.c_str(), &hints, &result);
    if (gaiError != 0) {
#if defined(KD_PLATFORM_WIN32)
        // Map gai_strerror equivalent if needed, or use generic error
        setError(SocketError::AddressResolutionError, gaiError); // Pass GAI error if possible
#else
        // Can potentially use gai_strerror(gaiError) for logging
        setError(SocketError::AddressResolutionError, gaiError);
#endif
        return false;
    }
    // Ensure result is freed even on exceptions / early returns
    std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> addrInfoPtr(result, freeaddrinfo);
    if (!addrInfoPtr) {
        setError(SocketError::AddressResolutionError); // No addresses found
        return false;
    }
    // *** END SYNC DNS LOOKUP ***
    // ----------------------------------------------------
    // --- End Asynchronous DNS Lookup Integration Point ---

    // Open the socket (base class handles making it non-blocking by default now)
    if (!open(addrInfoPtr->ai_family, addrInfoPtr->ai_socktype, addrInfoPtr->ai_protocol)) {
        // error set by open()
        return false;
    }

    // Socket opened successfully, now attempt non-blocking connect
    setState(State::Connecting); // Update state before connect call

    int ret = ::connect(m_socketFd, addrInfoPtr->ai_addr, static_cast<socklen_t>(addrInfoPtr->ai_addrlen));

    if (ret == 0) {
        // Connected immediately (likely localhost)
        setState(State::Connected);
        setError(SocketError::NoError);
        connected.emit();
        // Enable write notifier only if data is already waiting in the buffer
        setWriteNotificationEnabled(!m_writeBuffer.isEmpty());
        return true;
    } else { // ret < 0
#if defined(KD_PLATFORM_WIN32)
        int error_code = WSAGetLastError();
        // WSAEWOULDBLOCK is the typical code for non-blocking connect in progress
        if (error_code == WSAEWOULDBLOCK || error_code == WSAEINPROGRESS) { // WSAEINPROGRESS might also occur
            // Connection attempt is in progress asynchronously.
            // The outcome will be signaled via write readiness (success or SO_ERROR)
            // or read readiness (SO_ERROR). Enable the write notifier to detect this.
            setWriteNotificationEnabled(true);
            setError(SocketError::NoError); // Clear any potential error from open()
            return true; // Indicate connection process initiated
        } else {
            // Immediate connection error (e.g., network unreachable, connection refused)
            setError(SocketError::ConnectError, error_code);
            close(); // Cleanup the failed socket attempt
            // Do not emit disconnected here, as we never reached Connected state
            return false;
        }
#else
        if (errno == EINPROGRESS) {
            // Connection attempt is in progress asynchronously.
            // Enable write notifier to detect completion/failure.
            setWriteNotificationEnabled(true);
            setError(SocketError::NoError); // Clear any potential error from open()
            return true; // Indicate connection process initiated
        } else {
            // Immediate connection error
            setError(SocketError::ConnectError, errno);
            close(); // Cleanup the failed socket attempt
            // Do not emit disconnected here
            return false;
        }
#endif
    }
}

void TcpSocket::disconnectFromHost()
{
    if (!isValid() || state() == State::Unconnected)
        return;

    bool wasConnected = (state() == State::Connected || state() == State::Connecting);

    // Clear pending write data as we are closing abruptly
    m_writeBuffer.clear();

    // Clear pending read data
    m_readBuffer.clear();

    // Base class close() handles fd closure and sets state to Unconnected
    close();

    // Emit disconnected signal only if we were previously connected or connecting
    if (wasConnected)
        disconnected.emit();
}

std::int64_t TcpSocket::write(const KDUtils::ByteArray &data)
{
    return write(data.constData(), static_cast<std::int64_t>(data.size()));
}

std::int64_t TcpSocket::write(const std::uint8_t *data, std::int64_t size)
{
    // Can only write if connected
    if (state() != State::Connected) {
        setError(SocketError::WriteError);
        return -1;
    }

    if (!data || size <= 0) {
        return 0; // Nothing to write
    }

    // Append data to the internal write buffer
    m_writeBuffer.append(data, size);

    // Attempt to send the data immediately (if possible)
    // This might partially empty the buffer or trigger write notifications.
    trySend();

    // Return the number of bytes *accepted* into the buffer for writing.
    // This provides immediate feedback to the caller about acceptance,
    // but actual transmission is asynchronous.
    return size;
}

KDUtils::ByteArray TcpSocket::read(std::int64_t maxSize)
{
    if (m_readBuffer.isEmpty() || maxSize <= 0)
        return KDUtils::ByteArray(); // Nothing to read or invalid size

    // Determine actual number of bytes to read
    int64_t sizeToRead = std::min(maxSize, (int64_t)m_readBuffer.size());

    // Create a ByteArray with the data to return
    KDUtils::ByteArray chunk = m_readBuffer.left(sizeToRead); // Get copy of leftmost bytes

    // Remove the read data from the internal buffer
    m_readBuffer.remove(0, sizeToRead);

    return chunk;
}

KDUtils::ByteArray TcpSocket::readAll()
{
    // Return buffer content by moving it out - efficient and clears internal buffer
    return std::move(m_readBuffer);
}

std::int64_t TcpSocket::bytesAvailable() const noexcept
{
    return m_readBuffer.size();
}

std::int64_t TcpSocket::bytesToWrite() const noexcept
{
    return m_writeBuffer.size();
}

void TcpSocket::onReadReady()
{
    // First, check if we are in the Connecting state - read readiness might signal a connection error
    if (state() == State::Connecting) {
        handleConnectionResult();
        // If handleConnectionResult changed state to Connected, we might fall through
        // and try to read data immediately, which is usually fine.
        // If it resulted in an error/close, isValid() check below handles it.
    }

    // Ensure socket is valid and in a readable state
    if (!isValid() || (state() != State::Connected && state() != State::Closing)) {
        // TODO: Allow reading during graceful close? Maybe not needed if disconnect is abrupt.
        return;
    }

    // Read data in a loop as readiness notification is level-triggered
    constexpr int tempBufferSize = 4096; // Sensible chunk size
    std::uint8_t tempBuffer[tempBufferSize];
    int bytesRead = 0;

    while (isValid()) { // Loop while socket is valid
#if defined(KD_PLATFORM_WIN32)
        bytesRead = ::recv(m_socketFd, reinterpret_cast<char *>(tempBuffer), tempBufferSize, 0);
#else
        bytesRead = ::recv(m_socketFd, reinterpret_cast<char *>(tempBuffer), tempBufferSize, 0);
#endif

        if (bytesRead > 0) {
            // Successfully read some data
            processReceivedData(tempBuffer, bytesRead);
        } else if (bytesRead == 0) {
            // Peer has performed an orderly shutdown (EOF)
            setError(SocketError::NoError); // This is not an application error
            disconnectFromHost(); // Close our side and emit disconnected
            return; // Stop reading loop
        } else { // bytesRead < 0
            // An error occurred during recv
#if defined(KD_PLATFORM_WIN32)
            int error_code = WSAGetLastError();
            if (error_code == WSAEWOULDBLOCK) {
                // No more data available right now on the non-blocking socket
                return; // Finished reading for this notification cycle
            } else {
                // Other fatal read error (e.g., connection reset)
                setError(SocketError::ReadError, error_code);
                disconnectFromHost(); // Close connection on fatal read error
                return; // Stop reading loop
            }
#else
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No more data available right now
                return; // Finished reading for this notification cycle
            } else {
                // Other fatal read error
                setError(SocketError::ReadError, errno);
                disconnectFromHost(); // Close connection on fatal read error
                return; // Stop reading loop
            }
#endif
        }
    } // End while loop
}

void TcpSocket::onWriteReady()
{
    if (!isValid())
        return;

    if (state() == State::Connecting) {
        // Write readiness during connecting signals completion or error of the connect() call
        handleConnectionResult();
    } else if (state() == State::Connected) {
        // Socket is ready for writing more data, try sending pending data from buffer
        // Note: We don't need to disable the notifier here. trySend() will disable it
        // if the buffer becomes empty. If trySend() blocks again, the notifier
        // needs to remain enabled.
        trySend();
    }
}

void TcpSocket::handleConnectionResult()
{
    // This function is called when the socket becomes writable (or readable with error)
    // while in the Connecting state. It checks the actual result of the connect() call.
    if (state() != State::Connecting)
        return;

    int error = 0;
    socklen_t len = sizeof(error);

#if defined(KD_PLATFORM_WIN32)
    // On Windows, use getsockopt with SO_ERROR to check connection status
    if (::getsockopt(m_socketFd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&error), &len) < 0) {
        // Failed to get socket error status - treat as connection error
        int getsockoptError = WSAGetLastError();
        setError(SocketError::ConnectError, getsockoptError);
        disconnectFromHost(); // Close socket, emit disconnected
        return;
    }
#else
    // On POSIX, use getsockopt with SO_ERROR
    if (::getsockopt(m_socketFd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
        // Failed to get socket error status
        setError(SocketError::ConnectError, errno);
        disconnectFromHost(); // Close socket, emit disconnected
        return;
    }
#endif

    // Check the retrieved error code
    if (error == 0) {
        // Connection successful
        setState(State::Connected);
        setError(SocketError::NoError); // Clear any transient errors
        connected.emit(); // Notify user

        // Disable write notifier *unless* data was already added to the write buffer
        // between calling connectToHost() and the connection completing.
        setWriteNotificationEnabled(!m_writeBuffer.isEmpty());

        // If data is pending, try sending it immediately now that we are connected.
        if (!m_writeBuffer.isEmpty()) {
            trySend();
        }
    } else {
        // Connection failed
        // The error variable contains the errno/WSAError code for the failure
        setError(SocketError::ConnectError, error); // Set the specific error
        disconnectFromHost(); // Close socket, emit disconnected
    }
}

void TcpSocket::trySend()
{
    // Cannot send if buffer is empty, not connected, or socket is blocking
    if (m_writeBuffer.isEmpty() || state() != State::Connected || isBlocking()) {
        return;
    }

    int bytesSentTotal = 0; // Track bytes sent in this call

    while (!m_writeBuffer.isEmpty()) {
        int bytesSentNow = 0;
#if defined(KD_PLATFORM_WIN32)
        bytesSentNow = ::send(m_socketFd, reinterpret_cast<const char *>(m_writeBuffer.constData()),
                              static_cast<int>(std::min((int64_t)m_writeBuffer.size(), (int64_t)INT_MAX)), // Windows send takes int size
                              0);
#else
        // Use MSG_NOSIGNAL on Linux/macOS to prevent SIGPIPE if peer disconnects, handle EPIPE error instead
        bytesSentNow = ::send(m_socketFd, reinterpret_cast<const char *>(m_writeBuffer.constData()), m_writeBuffer.size(), MSG_NOSIGNAL);
#endif

        if (bytesSentNow > 0) {
            // Successfully sent some data
            m_writeBuffer.remove(0, bytesSentNow); // Remove sent data from buffer start
            bytesSentTotal += bytesSentNow;
        } else if (bytesSentNow == 0) {
            // According to man pages, send() should not return 0 for TCP unless size was 0.
            // Treat as unexpected, stop trying.
            KDUtils::Logger::logger("KDNetwork")->error("TcpSocket::trySend: send() returned 0 bytes unexpectedly");
            setError(SocketError::WriteError); // Set error for unexpected 0 bytes sent
            break;
        } else { // bytesSentNow < 0
            // An error occurred during send
#if defined(KD_PLATFORM_WIN32)
            int error_code = WSAGetLastError();
            if (error_code == WSAEWOULDBLOCK) {
                // Socket buffer is full, cannot send more now. Need to wait for readyWrite.
                setWriteNotificationEnabled(true); // Ensure notifier is active
                return; // Stop trying for now, will resume in onWriteReady
            } else {
                // Other fatal send error (e.g., connection reset)
                setError(SocketError::WriteError, error_code);
                disconnectFromHost(); // Close connection on fatal write error
                return; // Stop trying
            }
#else
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Socket buffer is full, need to wait for readyWrite
                setWriteNotificationEnabled(true); // Ensure notifier is active
                return; // Stop trying for now, will resume in onWriteReady
            } else if (errno == EPIPE) {
                // Peer closed connection (broken pipe) - Write error
                setError(SocketError::WriteError, errno);
                disconnectFromHost(); // Close connection
                return; // Stop trying
            } else {
                // Other fatal send error
                setError(SocketError::WriteError, errno);
                disconnectFromHost(); // Close connection on fatal write error
                return; // Stop trying
            }
#endif
        }
    } // End while loop

    // If we reached here, the write buffer is empty.
    bytesWritten.emit(bytesSentTotal); // Emit signal with total bytes sent

    // If we sent all data, disable the write notifier to avoid spurious events
    if (m_writeBuffer.isEmpty())
        setWriteNotificationEnabled(false); // Disable notifier, no more data to write
}

void TcpSocket::processReceivedData(const std::uint8_t *buffer, int size)
{
    if (size <= 0)
        return;

    // Append the received chunk to the internal read buffer
    // Assuming ByteArray::append is efficient
    m_readBuffer.append(buffer, size);

    // Emit the bytesReceived signal with the size of the chunk of received data.
    // The user can then call bytesAvailable() to check how much data is available
    // in total. Or they can call read() or readAll() to consume the data.
    bytesReceived.emit(size);
}

} // namespace KDNetwork
