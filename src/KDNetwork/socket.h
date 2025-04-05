/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDNetwork/kdnetwork_export.h>
#include <KDNetwork/socket_error.h>

#include <KDFoundation/config.h>
#include <KDFoundation/file_descriptor_notifier.h>

#include <KDBindings/signal.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#if defined(KD_PLATFORM_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#define KDNETWORK_UNDEF_WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#define KDNETWORK_UNDEF_NOMINMAX
#endif // NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h> // For sockaddr_in, sockaddr_in6
#else
#include <sys/socket.h> // For sockaddr, socklen_t
#endif

namespace KDNetwork {

class KDNETWORK_EXPORT Socket
{
public:
    // High-level state of the socket
    enum class State {
        Unconnected, // Initial state, or after close()
        Opening, // Socket FD created but not yet configured/connected
        Bound, // Socket has been bound to an address/port
        Resolving, // Asynchronous DNS resolution in progress
        Connecting, // Client specific: connection attempt in progress
        Connected, // Client specific: connection established
        Listening, // Server specific: listening for incoming connections
        Closing // Socket is in the process of closing (optional state)
    };

    KDBindings::Signal<> connected;
    KDBindings::Signal<> disconnected;
    KDBindings::Signal<> readyRead;
    KDBindings::Signal<> readyWrite;
    KDBindings::Signal<std::int64_t> bytesWritten;
    KDBindings::Signal<std::error_code> errorOccurred;
    KDBindings::Signal<State> stateChanged;

    virtual ~Socket();

    // Is not copyable
    Socket(Socket const &other) = delete;
    Socket &operator=(Socket const &other) = delete;

    // Is movable
    Socket(Socket &&other) noexcept = default;
    Socket &operator=(Socket &&other) noexcept = default;

    bool isValid() const;
    inline State state() const noexcept { return m_state; }
    inline SocketError lastError() const noexcept { return m_lastError; }
    inline std::error_code lastErrorCode() const noexcept { return m_lastErrorCode; }

    inline bool isBlocking() const noexcept { return m_isBlocking; }
    bool setBlocking(bool enabled) noexcept;

    enum class SocketType : std::uint8_t {
        Tcp,
        Udp,
        SslTcp
    };
    SocketType type() const noexcept { return m_type; }
    int socketFileDescriptor() const noexcept { return m_socketFd; }

    virtual bool open(int domain, int type, int protocol);
    virtual void close();

    // TODO: Add convenience overloads for bind (e.g., taking IpAddress, port)
    virtual bool bind(const struct sockaddr *addr, socklen_t addrlen);

protected:
    explicit Socket(SocketType type);

    SocketType m_type;
    int m_socketFd{ -1 }; // File descriptor for the socket
    State m_state{ State::Unconnected };
    SocketError m_lastError{ SocketError::NoError };
    std::error_code m_lastErrorCode{ KDNetwork::make_error_code(SocketError::NoError) };
    bool m_isBlocking{ true }; // Default to blocking mode

    std::unique_ptr<KDFoundation::FileDescriptorNotifier> m_readNotifier; // Notifier for read events
    std::unique_ptr<KDFoundation::FileDescriptorNotifier> m_writeNotifier; // Notifier for write events

    void setError(SocketError error, int sysErrno = 0);
    void setState(State newState);
    void setReadNotificationEnabled(bool enabled);
    void setWriteNotificationEnabled(bool enabled);

    // Connected to the FileDescriptorNotifier's triggered signals.
    virtual void onReadReady();
    virtual void onWriteReady();

    // Sets up the read and write notifiers for the current m_socketFd.
    void setupNotifiers();

    // Cleans up socket resources (closes FD, resets notifiers). Called by close(), destructor, move ops.
    void cleanupSocket();
};

} // namespace KDNetwork

#ifdef KDNETWORK_UNDEF_WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#undef KDNETWORK_UNDEF_WIN32_LEAN_AND_MEAN
#endif // KDNETWORK_UNDEF_WIN32_LEAN_AND_MEAN

#ifdef KDNETWORK_UNDEF_NOMINMAX
#undef NOMINMAX
#undef KDNETWORK_UNDEF_NOMINMAX
#endif // KDNETWORK_UNDEF_NOMINMAX
