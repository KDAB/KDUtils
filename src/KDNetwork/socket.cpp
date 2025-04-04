/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "socket.h"

#include <KDFoundation/file_descriptor_notifier.h>

#include <KDUtils/logging.h>

#include <utility>

#if defined(KD_PLATFORM_WIN32)
#else
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h> // For close()
#include <fcntl.h> // For fcntl() O_NONBLOCK
#include <errno.h> // For errno
#include <netinet/in.h> // For sockaddr_in etc. (needed for bind example)
#include <arpa/inet.h> // For inet_pton etc.
#endif // KD_PLATFORM_WIN32

using namespace KDFoundation;

namespace KDNetwork {

// Static initialization block to ensure WSAStartup is called prior to any socket operations
struct WSAInitializer {
    WSAInitializer()
    {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            KDUtils::Logger::logger("KDNetwork")->error("WSAStartup failed: {}", result);
        }
    }
    ~WSAInitializer()
    {
        WSACleanup();
    }
};

static WSAInitializer wsaInitializer;

Socket::Socket(SocketType type)
    : m_type(type)
{
}

Socket::~Socket()
{
    cleanupSocket();
}

bool Socket::isValid() const
{
#if defined(KD_PLATFORM_WIN32)
    return m_socketFd != INVALID_SOCKET && m_socketFd != -1;
#else
    return m_socketFd >= 0;
#endif
}

bool Socket::open(int domain, int type, int protocol)
{
    if (isValid()) {
        setError(SocketError::InvalidSocketError); // Already open
        return false;
    }

    cleanupSocket(); // Ensure clean state before opening

#if defined(KD_PLATFORM_WIN32)
    m_socketFd = static_cast<int>(::WSASocketW(domain, type, protocol, nullptr, 0, WSA_FLAG_OVERLAPPED));
    if (m_socketFd == INVALID_SOCKET) {
        setError(SocketError::SocketCreationError, WSAGetLastError());
        m_socketFd = -1;
        return false;
    }
#else
    m_socketFd = ::socket(domain, type, protocol);
    if (m_socketFd < 0) {
        setError(SocketError::SocketCreationError, errno);
        m_socketFd = -1;
        return false;
    }
#endif

    // Sockets start blocking by default on most platforms
    m_isBlocking = true;
    setError(SocketError::NoError); // Clear previous error
    setState(State::Opening); // Indicate socket is created but not ready

    // Setup notifiers for the new FD
    setupNotifiers();

    // Set to non-blocking by default for asynchronous use
    if (!setBlocking(false)) {
        // Error already set by setBlocking
        cleanupSocket(); // Clean up failed setup
        return false;
    }

    return true;
}

void Socket::close()
{
    cleanupSocket();
    setState(State::Unconnected);
}

bool Socket::bind(const sockaddr *addr, socklen_t addrlen)
{
    if (!isValid()) {
        setError(SocketError::InvalidSocketError);
        return false;
    }

    if (::bind(m_socketFd, addr, addrlen) < 0) {
#if defined(KD_PLATFORM_WIN32)
        setError(SocketError::BindError, WSAGetLastError());
#else
        setError(SocketError::BindError, errno);
#endif
        return false;
    }

    setError(SocketError::NoError);
    setState(State::Bound);
    return true;
}

bool Socket::setBlocking(bool enabled) noexcept
{
    if (!isValid()) {
        setError(SocketError::InvalidSocketError);
        return false;
    }

#if defined(KD_PLATFORM_WIN32)
    // Windows implementation using ioctlsocket
    u_long mode = enabled ? 0 : 1; // 0 for blocking, non-zero for non-blocking
    if (ioctlsocket(m_socketFd, FIONBIO, &mode) != 0) {
        setError(SocketError::SetNonBlockingError, WSAGetLastError());
        return false;
    }
#else
    // POSIX implementation using fcntl
    int flags = ::fcntl(m_socketFd, F_GETFL, 0);
    if (flags == -1) {
        setError(SocketError::SetNonBlockingError, errno);
        return false;
    }
    if (enabled) {
        flags &= ~O_NONBLOCK; // Clear non-blocking flag
    } else {
        flags |= O_NONBLOCK; // Set non-blocking flag
    }
    if (::fcntl(m_socketFd, F_SETFL, flags) == -1) {
        setError(SocketError::SetNonBlockingError, errno);
        return false;
    }
#endif

    m_isBlocking = enabled;
    setError(SocketError::NoError);
    return true;
}

void Socket::setError(SocketError error, int sysErrno)
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

// TODO: I think this can just be a read-only property
void Socket::setState(State newState)
{
    if (m_state != newState) {
        m_state = newState;
        stateChanged.emit(newState); // Emit the new state
    }
}

// TODO: Implement FileDescriptorNotifier::setEnabled
void Socket::setReadNotificationEnabled(bool enabled)
{
    if (m_readNotifier)
        m_readNotifier->setEnabled(enabled);
}

void Socket::setWriteNotificationEnabled(bool enabled)
{
    if (m_writeNotifier)
        m_writeNotifier->setEnabled(enabled);
}

void Socket::setupNotifiers()
{
    if (!isValid())
        return;

    // Read Notifier
    if (!m_readNotifier) {
        m_readNotifier = std::make_unique<FileDescriptorNotifier>(m_socketFd, FileDescriptorNotifier::NotificationType::Read);
        // Connect signal using lambda capturing this
        std::ignore = m_readNotifier->triggered.connect([this]() { this->onReadReady(); });
    }

    // Write Notifier
    if (!m_writeNotifier) {
        m_writeNotifier = std::make_unique<FileDescriptorNotifier>(m_socketFd, FileDescriptorNotifier::NotificationType::Write);
        std::ignore = m_writeNotifier->triggered.connect([this]() { this->onWriteReady(); });
    }
}

void Socket::cleanupSocket()
{
    if (isValid()) {
        // Disable notifiers *before* closing the FD to prevent spurious events
        // during cleanup. Reset them afterwards.
        m_readNotifier.reset();
        m_writeNotifier.reset();

#if defined(KD_PLATFORM_WIN32)
        ::closesocket(m_socketFd);
#else
        if (::close(m_socketFd) != 0) {
            KDUtils::Logger::logger("KDNetwork")->error("Failed to close socket: {}", errno);
        }
#endif
        m_socketFd = -1; // Mark as invalid
    }
    // Ensure notifiers are gone even if socket wasn't valid
    m_readNotifier.reset();
    m_writeNotifier.reset();

    // Reset state, but preserve last error if close() was called explicitly
    if (m_state != State::Unconnected) {
        setState(State::Unconnected);
    }
}

void Socket::onReadReady()
{
    // Basic implementation: just emit the public signal.
    // Derived classes might add logic here or connect directly to readyRead.
    // Note: For non-blocking sockets, readiness is level-triggered.
    // The notifier might fire again immediately if data isn't fully read.
    readyRead.emit();
}

void Socket::onWriteReady()
{
    // Basic implementation: emit signal.
    // Crucially, the code performing the write operation should typically
    // disable the write notifier after a successful write or when the
    // write buffer is empty, and re-enable it only if a subsequent write
    // call returns EWOULDBLOCK/EAGAIN.
    // setWriteNotificationEnabled(false); // DO NOT disable here automatically.
    readyWrite.emit();
}

} // namespace KDNetwork
