/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDNetwork/socket.h>
#include <KDNetwork/kdnetwork_export.h>
#include <KDNetwork/ip_address.h>

#include <KDUtils/bytearray.h>
#include <optional>

namespace KDNetwork {

class KDNETWORK_EXPORT TcpSocket : public Socket
{
public:
    KDBindings::Signal<> connected;
    KDBindings::Signal<> disconnected;
    KDBindings::Signal<std::int64_t> bytesReceived;
    KDBindings::Signal<std::int64_t> bytesWritten;

    explicit TcpSocket();

    // Used for creating a connected socket from an existing file descriptor
    // e.g., when accepting a connection on a server socket.
    explicit TcpSocket(int connectedFd, State initialState = State::Connected);

    ~TcpSocket() override;

    // Is not copyable
    TcpSocket(const TcpSocket &other) = delete;
    TcpSocket &operator=(const TcpSocket &other) = delete;

    // Is movable
    TcpSocket(TcpSocket &&other) noexcept = default;
    TcpSocket &operator=(TcpSocket &&other) noexcept = default;

    virtual bool connectToHost(const std::string &host, std::uint16_t port);
    virtual bool connectToHost(const IpAddress &address, std::uint16_t port);
    virtual void disconnectFromHost();

    std::int64_t write(const KDUtils::ByteArray &data);
    std::int64_t write(const std::uint8_t *data, std::int64_t size);
    KDUtils::ByteArray read(std::int64_t maxSize);
    KDUtils::ByteArray readAll();

    // The number of bytes currently available for reading in the internal buffer.
    std::int64_t bytesAvailable() const noexcept;

    // The number of bytes currently pending in the internal write buffer.
    std::int64_t bytesToWrite() const noexcept;

    IpAddress peerAddress() const noexcept;
    std::uint16_t peerPort() const noexcept;

protected:
    // Reads incoming data or handles EOF/errors. Also used to detect connection errors during the connecting phase.
    void onReadReady() override;

    // Used to detect connection success/failure during the connecting phase, or
    // to resume sending data from the write buffer when the socket becomes writable again.
    void onWriteReady() override;

private:
    // Called from onReadReady/onWriteReady when state is Connecting. Sets state, emits signals.
    void handleConnectionResult();

    // Called internally by write() and onWriteReady(). Handles partial sends and EWOULDBLOCK.
    void trySend();

    // Processes data received from the socket.
    // Appends data to the read buffer and emits bytesReceived signal.
    void processReceivedData(const std::uint8_t *buffer, int size);

    // Handle DNS lookup completion and initiate socket connection
    void handleDnsLookupCompleted(std::error_code ec, const std::vector<IpAddress> &addresses);

    KDUtils::ByteArray m_readBuffer; // Internal buffer for incoming data.
    KDUtils::ByteArray m_writeBuffer; // Internal buffer for outgoing data.

    // State for pending DNS lookup and connection
    struct PendingConnection {
        std::string hostname;
        std::uint16_t port;
    };
    std::optional<PendingConnection> m_pendingConnection;

    // Peer information
    IpAddress m_peerAddress;
    std::uint16_t m_peerPort{ 0 };
};

} // namespace KDNetwork
