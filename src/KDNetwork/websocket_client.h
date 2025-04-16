/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDNetwork/kdnetwork_export.h>
#include <KDNetwork/http_session.h>

#include <KDFoundation/timer.h>

#include <KDBindings/signal.h>

#include <KDUtils/bytearray.h>
#include <KDUtils/uri.h>

#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <string>

namespace KDNetwork {

// Forward declarations
class Socket;
class HttpClient;
class HttpRequest;
class HttpResponse;
class WebSocketFrame;

/**
 * @brief The WebSocketClient class provides a client for WebSocket connections
 *
 * This class implements the WebSocket protocol as defined in RFC 6455.
 * It allows for bidirectional communication with a WebSocket server.
 */
class KDNETWORK_EXPORT WebSocketClient : public std::enable_shared_from_this<WebSocketClient>
{
public:
    /**
     * @brief Connection states for the WebSocket client
     */
    enum class State {
        Closed, // Not connected
        Connecting, // HTTP upgrade in progress
        Connected, // Connection established
        Closing // Graceful shutdown in progress
    };

    /**
     * @brief Default constructor with optional session
     *
     * @param session Optional session to use. If nullptr, a new session is created.
     */
    explicit WebSocketClient(std::shared_ptr<HttpSession> session = nullptr);

    /**
     * @brief Destructor
     */
    ~WebSocketClient();

    /**
     * @brief Connect to a WebSocket server
     *
     * Initiates a WebSocket connection to the given URL. The URL must have a scheme
     * of "ws" or "wss" (for secure WebSocket connections).
     *
     * @param url The URL to connect to
     * @return A future that will be set to true if the connection was established successfully
     */
    std::future<bool> connectToUrl(const KDUtils::Uri &url);

    /**
     * @brief Disconnect from the WebSocket server
     *
     * Initiates a graceful disconnection from the server by sending a Close frame
     * and waiting for the server to respond with its own Close frame.
     *
     * @param code The WebSocket close code (default: 1000 - normal closure)
     * @param reason A human-readable reason for closing (default: "Normal closure")
     */
    void disconnect(uint16_t code = 1000, const std::string &reason = "Normal closure");

    /**
     * @brief Send a text message
     *
     * @param message The text message to send
     */
    void sendTextMessage(const std::string &message);

    /**
     * @brief Send a binary message
     *
     * @param message The binary message to send
     */
    void sendBinaryMessage(const KDUtils::ByteArray &message);

    /**
     * @brief Send a ping message
     *
     * @param payload Optional payload to include in the ping
     */
    void sendPing(const KDUtils::ByteArray &payload = {});

    /**
     * @brief Get the current state of the connection
     */
    State state() const;

    /**
     * @brief Check if the client is connected
     */
    bool isConnected() const;

    /**
     * @brief Set the maximum number of reconnection attempts
     *
     * @param maxAttempts Maximum number of attempts (0 = no limit)
     */
    void setMaxReconnectAttempts(int maxAttempts);

    /**
     * @brief Set the interval between reconnection attempts
     *
     * @param interval The interval between reconnection attempts
     */
    void setReconnectInterval(std::chrono::milliseconds interval);

    /**
     * @brief Enable or disable automatic reconnection
     *
     * @param enabled True to enable automatic reconnection, false to disable
     */
    void setAutoReconnect(bool enabled);

    /**
     * @brief Signal emitted when the connection is established
     */
    KDBindings::Signal<> connected;

    /**
     * @brief Signal emitted when the connection is closed
     *
     * The signal provides the WebSocket close code and reason.
     */
    KDBindings::Signal<uint16_t, const std::string &> disconnected;

    /**
     * @brief Signal emitted when a text message is received
     */
    KDBindings::Signal<const std::string &> textMessageReceived;

    /**
     * @brief Signal emitted when a binary message is received
     */
    KDBindings::Signal<const KDUtils::ByteArray &> binaryMessageReceived;

    /**
     * @brief Signal emitted when a pong message is received
     */
    KDBindings::Signal<const KDUtils::ByteArray &> pongReceived;

    /**
     * @brief Signal emitted when an error occurs
     */
    KDBindings::Signal<const std::string &> errorOccurred;

    /**
     * @brief Signal emitted just before a reconnection attempt
     */
    KDBindings::Signal<> aboutToReconnect;

private:
    // Setup socket after successful connection
    void setupSocket(std::shared_ptr<Socket> socket, const KDUtils::ByteArray &excessData = KDUtils::ByteArray());

    // Process incoming data from the socket
    void processIncomingData();

    // Handle a single WebSocket frame
    void handleFrame(const WebSocketFrame &frame);

    // Handle close frames
    void handleCloseFrame(const WebSocketFrame &frame);

    // Send a WebSocket frame
    void sendFrame(const WebSocketFrame &frame);

    // Send a pong response
    void sendPong(const KDUtils::ByteArray &payload);

    // Force close the connection
    void forceClose();

    // Handle socket errors
    void handleSocketError(std::error_code ec);

    // Start ping timer for keep-alive
    void startPingTimer();

    // Calculate the Accept key for WebSocket handshake
    std::string calculateAcceptKey(const std::string &key);

    // State
    State m_state = State::Closed;

    // Used for HTTP handshake
    std::shared_ptr<HttpClient> m_httpClient;

    // Connection socket
    std::shared_ptr<Socket> m_socket;

    // Original connection URL (for reconnect)
    KDUtils::Uri m_url;

    // Buffer for incoming data
    KDUtils::ByteArray m_receiveBuffer;

    // Reconnection settings
    bool m_autoReconnect = false;
    int m_maxReconnectAttempts = 3;
    int m_reconnectAttempts = 0;
    std::chrono::milliseconds m_reconnectInterval = std::chrono::seconds(3);

    // Timer for graceful closing
    std::unique_ptr<KDFoundation::Timer> m_closeTimer;

    // Timer for automatic reconnection
    std::unique_ptr<KDFoundation::Timer> m_reconnectTimer;

    // Timer for ping/pong keep-alive
    std::unique_ptr<KDFoundation::Timer> m_pingTimer;
    std::chrono::steady_clock::time_point m_lastPongReceived;
};

} // namespace KDNetwork
