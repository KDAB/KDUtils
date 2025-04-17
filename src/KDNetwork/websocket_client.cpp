/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/websocket_client.h>
#include <KDNetwork/websocket_frame.h>
#include <KDNetwork/websocket_common.h>
#include <KDNetwork/http_client.h>
#include <KDNetwork/http_request.h>
#include <KDNetwork/http_response.h>
#include <KDNetwork/socket.h>
#include <KDNetwork/tcp_socket.h>
#include <KDNetwork/ssl_socket.h>

#include <KDUtils/logging.h>

#include <openssl/sha.h>

#include <algorithm>
#include <iomanip>
#include <random>
#include <sstream>

namespace {
KDUtils::ByteArray sha1(const KDUtils::ByteArray &data)
{
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(data.data(), data.size(), hash);
    return KDUtils::ByteArray(hash, SHA_DIGEST_LENGTH);
}

KDUtils::ByteArray sha1(const std::string &data)
{
    return sha1(KDUtils::ByteArray(data.data(), data.size()));
}
} // namespace

namespace KDNetwork {

WebSocketClient::WebSocketClient(std::shared_ptr<HttpSession> session)
    : m_state(State::Closed)
    , m_httpClient(std::make_shared<HttpClient>(session))
    , m_lastPongReceived(std::chrono::steady_clock::now())
{
}

WebSocketClient::~WebSocketClient()
{
    forceClose();
}

std::future<bool> WebSocketClient::connectToUrl(const KDUtils::Uri &url)
{
    // Create promise and future for async result
    auto promise = std::make_shared<std::promise<bool>>();
    auto future = promise->get_future();

    // Save URL for reconnections
    m_url = url;

    // Reset reconnection attempts
    m_reconnectAttempts = 0;

    // Check if already connected
    if (m_state != State::Closed) {
        promise->set_value(false);
        return future;
    }

    // Validate URL scheme
    if (url.scheme() != "ws" && url.scheme() != "wss") {
        KDUtils::Logger::logger("WebsocketClient")->warn("Invalid URL scheme, must be ws:// or wss://");
        promise->set_value(false);
        return future;
    }

    // Set state to connecting
    m_state = State::Connecting;

    // Modify URL scheme for HTTP request (ws -> http, wss -> https)
    KDUtils::Uri httpUrl = url;
    if (url.scheme() == "ws") {
        httpUrl = httpUrl.withScheme("http");
    } else if (url.scheme() == "wss") {
        httpUrl = httpUrl.withScheme("https");
    }

    // Generate a random WebSocket key
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);

    KDUtils::ByteArray keyBytes(16);
    for (int i = 0; i < 16; ++i) {
        keyBytes[i] = static_cast<uint8_t>(distrib(gen));
    }
    const auto key = keyBytes.toBase64().toStdString();

    // Create HTTP request with WebSocket upgrade headers
    HttpRequest request(httpUrl);
    request.setHeader("Upgrade", "websocket");
    request.setHeader("Connection", "Upgrade");
    request.setHeader("Sec-WebSocket-Version", "13");
    request.setHeader("Sec-WebSocket-Key", key);

    // Send the request
    m_httpClient->send(request, [this, promise, key](const HttpResponse &response) {
        // Check if upgrade was successful
        if (response.statusCode() == 101 &&
            response.hasHeader("Upgrade") &&
            response.header("Upgrade").find("websocket") != std::string::npos &&
            response.hasHeader("Sec-WebSocket-Accept")) {

            // Verify the Sec-WebSocket-Accept header
            std::string expectedAccept = calculateAcceptKey(key);
            if (response.header("Sec-WebSocket-Accept") == expectedAccept) {
                // Take ownership of the socket from HttpClient
                auto socket = response.takeSocket();
                if (socket) {
                    // Check if there's any excess data from the HTTP response (might contain initial WebSocket frames)
                    KDUtils::ByteArray excessData = response.takeExcessData();

                    setupSocket(socket, excessData);
                    promise->set_value(true);
                    return;
                }
            } else {
                KDUtils::Logger::logger("WebsocketClient")->warn("Invalid Sec-WebSocket-Accept header");
            }
        } else {
            KDUtils::Logger::logger("WebsocketClient")->warn("Handshake failed, server returned status", response.statusCode());
        }

        // If we get here, the connection failed
        m_state = State::Closed;
        promise->set_value(false);
    });

    return future;
}

void WebSocketClient::disconnect(uint16_t code, const std::string &reason)
{
    if (m_state == State::Connected) {
        m_state = State::Closing;

        // Send close frame
        auto closeFrame = WebSocketFrame::createCloseFrame(code, reason);
        sendFrame(closeFrame);

        // Set up a timeout for clean disconnection
        // clang-format off
        m_closeTimer = KDFoundation::Timer::createTimeout([this]() {
            // Force close if no response from server
            forceClose();
        }, std::chrono::milliseconds(WebSocket::DEFAULT_CLOSE_TIMEOUT_MS));
        // clang-format on
    } else {
        forceClose();
    }
}

void WebSocketClient::sendTextMessage(const std::string &message)
{
    if (m_state == State::Connected) {
        auto frame = WebSocketFrame::createTextFrame(message);
        sendFrame(frame);
    }
}

void WebSocketClient::sendBinaryMessage(const KDUtils::ByteArray &message)
{
    if (m_state == State::Connected) {
        auto frame = WebSocketFrame::createBinaryFrame(message);
        sendFrame(frame);
    }
}

void WebSocketClient::sendPing(const KDUtils::ByteArray &payload)
{
    if (m_state == State::Connected) {
        auto frame = WebSocketFrame::createPingFrame(payload);
        sendFrame(frame);
    }
}

WebSocketClient::State WebSocketClient::state() const
{
    return m_state;
}

bool WebSocketClient::isConnected() const
{
    return m_state == State::Connected;
}

void WebSocketClient::setMaxReconnectAttempts(int maxAttempts)
{
    m_maxReconnectAttempts = maxAttempts;
}

void WebSocketClient::setReconnectInterval(std::chrono::milliseconds interval)
{
    m_reconnectInterval = interval;
}

void WebSocketClient::setAutoReconnect(bool enabled)
{
    m_autoReconnect = enabled;
}

void WebSocketClient::setupSocket(std::shared_ptr<Socket> socket, const KDUtils::ByteArray &excessData)
{
    auto tcpSocket = std::dynamic_pointer_cast<TcpSocket>(socket);
    if (!tcpSocket) {
        KDUtils::Logger::logger("WebsocketClient")->warn("Invalid socket type");
        m_state = State::Closed;
        return;
    }

    KDUtils::Logger::logger("WebsocketClient")->debug("Connected to WebSocket server");

    // Store the socket and update state
    m_socket = socket;
    m_state = State::Connected;

    // Reset buffer
    m_receiveBuffer.clear();

    // Append any excess data from the HTTP response
    if (!excessData.isEmpty()) {
        m_receiveBuffer.append(excessData);
    }

    // Emit the connected signal and process any initial data
    connected.emit();

    // Process any initial data in the buffer. This can happen if the server includes a message in the
    // same packet as the upgrade response.
    if (!m_receiveBuffer.isEmpty()) {
        processIncomingData();
    }

    // Set up read handler for future data
    std::ignore = tcpSocket->bytesReceived.connect([this]() {
        processIncomingData();
    });

    // Set up error handler
    std::ignore = m_socket->errorOccurred.connect([this](std::error_code ec) {
        handleSocketError(ec);
    });

    // Start ping timer for keep-alive
    startPingTimer();
}

void WebSocketClient::processIncomingData()
{
    if (!m_socket) {
        return;
    }

    // Get socket type
    TcpSocket *tcpSocket = nullptr;
    SslSocket *sslSocket = nullptr;

    if (m_socket->type() == Socket::SocketType::Tcp) {
        tcpSocket = static_cast<TcpSocket *>(m_socket.get());
    } else if (m_socket->type() == Socket::SocketType::SslTcp) {
        sslSocket = static_cast<SslSocket *>(m_socket.get());
    } else {
        KDUtils::Logger::logger("WebsocketClient")->warn("Unsupported socket type");
        return;
    }

    // Read available data from socket
    KDUtils::ByteArray data;
    if (tcpSocket) {
        data = tcpSocket->readAll();
    } else if (sslSocket) {
        data = sslSocket->readAll();
    }

    // Append to buffer
    if (!data.isEmpty()) {
        m_receiveBuffer.append(data);
    }

    // Process frames in buffer
    size_t bytesProcessed = 0;
    while (m_receiveBuffer.size() > 2) { // Minimum frame size is 2 bytes
        auto frame = WebSocketFrame::decode(m_receiveBuffer, bytesProcessed);

        if (!frame.has_value()) {
            // Need more data
            break;
        }

        // Handle the frame
        handleFrame(*frame);

        // Remove processed frame from buffer
        m_receiveBuffer = m_receiveBuffer.mid(bytesProcessed);
    }
}

void WebSocketClient::handleFrame(const WebSocketFrame &frame)
{
    switch (frame.opCode()) {
    case WebSocketFrame::OpCode::Text:
        if (frame.isFinal()) {
            // Convert payload to string
            const auto text = frame.payload().toStdString();
            textMessageReceived.emit(text);
        }
        break;

    case WebSocketFrame::OpCode::Binary:
        if (frame.isFinal()) {
            binaryMessageReceived.emit(frame.payload());
        }
        break;

    case WebSocketFrame::OpCode::Close:
        handleCloseFrame(frame);
        break;

    case WebSocketFrame::OpCode::Ping:
        // Automatically respond with pong
        sendPong(frame.payload());
        break;

    case WebSocketFrame::OpCode::Pong:
        m_lastPongReceived = std::chrono::steady_clock::now();
        pongReceived.emit(frame.payload());
        break;

    case WebSocketFrame::OpCode::Continuation:
        // We don't support fragmentation yet, would need to implement message reassembly
        KDUtils::Logger::logger("WebsocketClient")->warn("Received continuation frame, fragmentation not yet supported");
        break;

    default:
        KDUtils::Logger::logger("WebsocketClient")->warn("Received frame with unknown opcode:", static_cast<int>(frame.opCode()));
        break;
    }
}

void WebSocketClient::handleCloseFrame(const WebSocketFrame &frame)
{
    uint16_t code = static_cast<uint16_t>(WebSocket::CloseCode::NoStatusReceived);
    std::string reason = "No reason provided";

    // Extract code and reason if available
    if (frame.payload().size() >= 2) {
        code = (static_cast<uint16_t>(frame.payload()[0]) << 8) | frame.payload()[1];

        if (frame.payload().size() > 2) {
            reason = frame.payload().mid(2).toStdString();
        }
    }

    if (m_state == State::Closing) {
        // We initiated the close, this is the server's response, so finalize closing
        forceClose();
    } else {
        // Server initiated the close, send a close frame back
        m_state = State::Closing;
        auto closeFrame = WebSocketFrame::createCloseFrame(code, "");
        sendFrame(closeFrame);
        forceClose();
    }

    // Emit the disconnected signal
    disconnected.emit(code, reason);
}

void WebSocketClient::sendFrame(const WebSocketFrame &frame)
{
    if (!m_socket || (m_state != State::Connected && m_state != State::Closing)) {
        return;
    }

    KDUtils::ByteArray encodedFrame = frame.encode();

    // Send the frame
    if (m_socket->type() == Socket::SocketType::Tcp) {
        static_cast<TcpSocket *>(m_socket.get())->write(encodedFrame);
    } else if (m_socket->type() == Socket::SocketType::SslTcp) {
        static_cast<SslSocket *>(m_socket.get())->write(encodedFrame);
    }
}

void WebSocketClient::sendPong(const KDUtils::ByteArray &payload)
{
    auto frame = WebSocketFrame::createPongFrame(payload);
    sendFrame(frame);
}

void WebSocketClient::forceClose()
{
    // Stop timers
    if (m_closeTimer) {
        m_closeTimer->running = false;
        m_closeTimer.reset();
    }

    if (m_pingTimer) {
        m_pingTimer->running = false;
        m_pingTimer.reset();
    }

    // Close socket
    if (m_socket) {
        m_socket->close();
        m_socket.reset();
    }

    // Update state
    m_state = State::Closed;
}

void WebSocketClient::handleSocketError(std::error_code ec)
{
    // Log the error
    KDUtils::Logger::logger("WebsocketClient")->warn("Socket error:", ec.message());

    // Emit the error signal
    errorOccurred.emit("Socket error: " + ec.message());

    // Reset state
    forceClose();

    // Attempt reconnect if enabled
    if (m_autoReconnect && m_reconnectAttempts < m_maxReconnectAttempts) {
        m_reconnectAttempts++;
        aboutToReconnect.emit();

        // Schedule reconnect
        // clang-format off
        m_reconnectTimer = KDFoundation::Timer::createTimeout([this]() {
            if (m_url.isValid())
                connectToUrl(m_url);
        }, m_reconnectInterval);
        // clang-format on
    }
}

void WebSocketClient::startPingTimer()
{
    // Initialize the last pong timestamp
    m_lastPongReceived = std::chrono::steady_clock::now();

    // Create ping timer
    m_pingTimer = std::make_unique<KDFoundation::Timer>();
    m_pingTimer->interval = std::chrono::milliseconds(WebSocket::DEFAULT_PING_INTERVAL_MS);
    std::ignore = m_pingTimer->timeout.connect([this]() {
        // Check if we've received a pong recently
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastPongReceived);

        if (elapsed.count() > WebSocket::DEFAULT_PING_INTERVAL_MS * 2.5) {
            // No pong received for a long time, connection might be dead
            KDUtils::Logger::logger("WebsocketClient")->warn("No pong response received, closing connection");
            handleSocketError(std::make_error_code(std::errc::connection_aborted));
            return;
        }

        // Send a ping
        sendPing(KDUtils::ByteArray("ping"));
    });
    m_pingTimer->running = true;
}

std::string WebSocketClient::calculateAcceptKey(const std::string &key)
{
    // Concatenate with the WebSocket GUID
    std::string concat = key + WebSocket::GUID;

    // Calculate SHA-1 hash
    KDUtils::ByteArray hash = sha1(concat);

    // Convert to base64
    return hash.toBase64().toStdString();
}

} // namespace KDNetwork
