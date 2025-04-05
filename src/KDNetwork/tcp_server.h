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
#include <KDNetwork/ip_address.h>

#include <KDFoundation/file_descriptor_notifier.h>

#include <KDBindings/signal.h>

#include <functional>
#include <memory>
#include <system_error>
#include <string>

namespace KDNetwork {

/**
 * @brief The TcpServer class provides a TCP server for accepting connections.
 *
 * TcpServer allows setting up a socket to listen for incoming TCP connections.
 * When a client connects, the server creates a new TcpSocket object for the connection
 * and passes it to the application via a callback or signal.
 */
class KDNETWORK_EXPORT TcpServer
{
public:
    /**
     * @brief Possible states for the TCP server
     */
    enum class State {
        NotListening, ///< Initial state, or after close()
        Resolving, ///< Asynchronous DNS resolution in progress (when starting to listen)
        Listening ///< Listening for incoming connections
    };

    // The callback receives ownership of the new TcpSocket via std::unique_ptr.
    using NewConnectionCallback = std::function<void(std::unique_ptr<TcpSocket>)>;

    KDBindings::Signal<std::error_code> errorOccurred;
    KDBindings::Signal<> listeningStarted;
    KDBindings::Signal<State> stateChanged;

    explicit TcpServer();
    ~TcpServer();

    // Is not copyable
    TcpServer(const TcpServer &other) = delete;
    TcpServer &operator=(const TcpServer &other) = delete;

    // Is movable
    TcpServer(TcpServer &&other) noexcept = default;
    TcpServer &operator=(TcpServer &&other) noexcept = default;

    void setNewConnectionCallback(NewConnectionCallback callback);

    /**
     * @brief Start listening for incoming connections on the specified hostname and port
     *
     * This variant performs an asynchronous DNS lookup on the hostname before
     * starting to listen on the resolved IP address.
     *
     * @param host The hostname or IP address to listen on (e.g., "localhost", "127.0.0.1")
     * @param port The port number to listen on (use 0 for system-assigned port)
     * @param backlog The maximum length of the pending connections queue
     * @return True if the listening process was initiated successfully
     */
    bool listen(const std::string &host, uint16_t port, int backlog = 128);

    /**
     * @brief Start listening for incoming connections on the specified IP address and port
     *
     * This variant uses the provided IpAddress directly without requiring DNS resolution.
     *
     * @param address The IP address to listen on
     * @param port The port number to listen on (use 0 for system-assigned port)
     * @param backlog The maximum length of the pending connections queue
     * @return True if the server is now listening
     */
    bool listen(const IpAddress &address, uint16_t port, int backlog = 128);

    /**
     * @brief Close the server socket and stop listening
     */
    void close();

    /**
     * @brief Returns whether the server is currently listening
     * @return True if the server is listening
     */
    bool isListening() const noexcept { return m_state == State::Listening; }

    /**
     * @brief Returns the current state of the server
     * @return Current state
     */
    State state() const noexcept { return m_state; }

    /**
     * @brief Returns the last error that occurred
     * @return Last error code
     */
    SocketError lastError() const noexcept { return m_lastError; }

    /**
     * @brief Returns the last error that occurred as a std::error_code
     * @return Last error code
     */
    std::error_code lastErrorCode() const noexcept { return m_lastErrorCode; }

    /**
     * @brief Returns the address the server is listening on
     * @return Server address, or null address if not listening
     */
    IpAddress serverAddress() const noexcept { return m_serverAddress; }

    /**
     * @brief Returns the port the server is listening on
     * @return Server port, or 0 if not listening
     */
    uint16_t serverPort() const noexcept { return m_serverPort; }

private:
    /**
     * @brief Internal implementation of listen() that is called after DNS resolution
     *
     * This method sets up the socket for listening on the provided address and port.
     *
     * @param address The IP address to listen on
     * @param port The port number to listen on
     * @param backlog The maximum length of the pending connections queue
     * @return True if the server is now listening
     */
    bool listenOnAddress(const IpAddress &address, uint16_t port, int backlog);

    /**
     * @brief Handler for DNS resolution completion
     *
     * This method is called when the asynchronous DNS lookup completes.
     *
     * @param ec Error code from DNS resolution
     * @param addresses List of resolved IP addresses
     * @param port Port to listen on
     * @param backlog Maximum length of the pending connections queue
     */
    void handleDnsLookupCompleted(std::error_code ec, const std::vector<IpAddress> &addresses,
                                  uint16_t port, int backlog);

    void onIncomingConnection();
    void setError(SocketError error, int sysErrno = 0);
    void setState(State newState);
    bool isValid() const noexcept;

    int m_listeningFd{ -1 }; // Native socket descriptor for the listening socket (-1 if not listening).
    std::unique_ptr<KDFoundation::FileDescriptorNotifier> m_listenNotifier; // Notifier monitoring m_listeningFd for readability.
    State m_state{ State::NotListening };
    SocketError m_lastError{ SocketError::NoError };
    std::error_code m_lastErrorCode{ KDNetwork::make_error_code(SocketError::NoError) };
    NewConnectionCallback m_newConnectionCallback{ nullptr }; // Callback function for new connections.

    IpAddress m_serverAddress; // The address the server is listening on
    uint16_t m_serverPort{ 0 }; // The port the server is listening on
};

} // namespace KDNetwork
