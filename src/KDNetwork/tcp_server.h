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
#include <KDNetwork/tcp_socket.h>

#include <KDFoundation/file_descriptor_notifier.h>

#include <KDBindings/signal.h>

#include <functional>
#include <memory>

namespace KDNetwork {

class KDNETWORK_EXPORT TcpServer
{
public:
    // The callback receives ownership of the new TcpSocket via std::unique_ptr.
    using NewConnectionCallback = std::function<void(std::unique_ptr<TcpSocket>)>;

    KDBindings::Signal<std::error_code> errorOccurred;

    explicit TcpServer();
    ~TcpServer();

    // Is not copyable
    TcpServer(const TcpServer &other) = delete;
    TcpServer &operator=(const TcpServer &other) = delete;

    // Is movable
    TcpServer(TcpServer &&other) noexcept = default;
    TcpServer &operator=(TcpServer &&other) noexcept = default;

    void setNewConnectionCallback(NewConnectionCallback callback);

    bool listen(const std::string &address, uint16_t port, int backlog = 128);

    // TODO: Add overload for IpAddress object
    // bool listen(const IpAddress& address, uint16_t port, int backlog = 128);

    void close();

    bool isListening() const noexcept { return m_isListening; }

    // TODO: Add methods to retrieve the actual server address and port if needed,
    // especially useful if listening on port 0 to get the assigned port.
    // IpAddress serverAddress() const;
    // uint16_t serverPort() const;

private:
    void onIncomingConnection();
    void setError(SocketError error, int sysErrno = 0);
    bool isValid() const noexcept;

    int m_listeningFd{ -1 }; // Native socket descriptor for the listening socket (-1 if not listening).
    std::unique_ptr<KDFoundation::FileDescriptorNotifier> m_listenNotifier; // Notifier monitoring m_listeningFd for readability.
    bool m_isListening{ false };
    SocketError m_lastError{ SocketError::NoError };
    std::error_code m_lastErrorCode{ KDNetwork::make_error_code(SocketError::NoError) };
    NewConnectionCallback m_newConnectionCallback{ nullptr }; // Callback function for new connections.
};

} // namespace KDNetwork
