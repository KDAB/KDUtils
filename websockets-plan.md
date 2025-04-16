# WebSocket Client Implementation Plan

The WebSocket protocol (RFC6455) is perfect to build on your existing networking infrastructure since it starts with an HTTP handshake and then upgrades to a WebSocket connection for bi-directional communication.

## 1. Core Components

Let's create the following key components:

1. **WebSocketClient** - Main user-facing class
2. **WebSocketFrame** - Internal representation of WebSocket frames
3. **WebSocketHandler** - Internal class handling frame parsing/generation

## 2. Implementation ArchitectureLooking at the available header files, I can see you have a strong networking foundation to build upon with components like:

- `tcp_socket.h` and ssl_socket.h for connection handling
- `http_client.h` for making the initial WebSocket handshake
- sse_client.h as another client implementation we can use as a reference

Let's continue with the WebSocket implementation plan:

## 2. Implementation Architecture

### `WebSocketClient` Class

The main user-facing class that will provide:

```cpp
class KDNETWORK_EXPORT WebSocketClient : public std::enable_shared_from_this<WebSocketClient>
{
public:
    // Connection states
    enum class State {
        Closed,         // Not connected
        Connecting,     // HTTP upgrade in progress
        Connected,      // Connection established
        Closing         // Graceful shutdown in progress
    };

    // Constructor with optional session for connection reuse
    explicit WebSocketClient(std::shared_ptr<HttpSession> session = nullptr);
    ~WebSocketClient();

    // Connection management
    std::future<bool> connectToUrl(const KDUtils::Uri &url);
    void disconnect(uint16_t code = 1000, const std::string &reason = "Normal closure");

    // Message sending
    void sendTextMessage(const std::string &message);
    void sendBinaryMessage(const KDUtils::ByteArray &message);
    void sendPing(const KDUtils::ByteArray &payload = {});

    // State access
    State state() const;
    bool isConnected() const;

    // Configuration
    void setMaxReconnectAttempts(int maxAttempts);
    void setReconnectInterval(std::chrono::milliseconds interval);
    void setAutoReconnect(bool enabled);

    // Signals
    KDBindings::Signal<> connected;
    KDBindings::Signal<uint16_t, const std::string &> disconnected; // code, reason
    KDBindings::Signal<const std::string &> textMessageReceived;
    KDBindings::Signal<const KDUtils::ByteArray &> binaryMessageReceived;
    KDBindings::Signal<const KDUtils::ByteArray &> pongReceived;
    KDBindings::Signal<const std::string &> errorOccurred;
    KDBindings::Signal<> aboutToReconnect;
};
```

### `WebSocketFrame` Class (Internal)

```cpp
// Internal class for WebSocket frame handling
class WebSocketFrame
{
public:
    enum class OpCode : uint8_t {
        Continuation = 0x0,
        Text = 0x1,
        Binary = 0x2,
        Close = 0x8,
        Ping = 0x9,
        Pong = 0xA
    };

    WebSocketFrame();

    // Frame construction helpers
    static WebSocketFrame createTextFrame(const std::string &text, bool isFinalFragment = true);
    static WebSocketFrame createBinaryFrame(const KDUtils::ByteArray &data, bool isFinalFragment = true);
    static WebSocketFrame createCloseFrame(uint16_t code, const std::string &reason);
    static WebSocketFrame createPingFrame(const KDUtils::ByteArray &payload);
    static WebSocketFrame createPongFrame(const KDUtils::ByteArray &payload);

    // Getters
    OpCode opCode() const;
    bool isFinal() const;
    KDUtils::ByteArray payload() const;
    KDUtils::ByteArray encode(bool maskFrame = true) const;

    // Decoding
    static std::optional<WebSocketFrame> decode(const KDUtils::ByteArray &data, size_t &bytesProcessed);
};
```

## 3. Implementation Details

### HTTP Upgrade Process

1. Use the existing `HttpClient` to perform the initial connection and WebSocket upgrade:

```cpp
std::future<bool> WebSocketClient::connectToUrl(const KDUtils::Uri &url) {
    // Create promise and future for async result
    auto promise = std::make_shared<std::promise<bool>>();
    auto future = promise->get_future();

    // Modify url scheme if needed (ws -> http, wss -> https)
    KDUtils::Uri httpUrl = url;
    if (url.scheme() == "ws") {
        httpUrl.setScheme("http");
    } else if (url.scheme() == "wss") {
        httpUrl.setScheme("https");
    } else {
        // Invalid scheme
        promise->set_value(false);
        return future;
    }

    // Create HTTP request with WebSocket upgrade headers
    HttpRequest request(httpUrl);
    request.setHeader("Upgrade", "websocket");
    request.setHeader("Connection", "Upgrade");
    request.setHeader("Sec-WebSocket-Version", "13");

    // Generate random WebSocket key
    KDUtils::ByteArray keyBytes(16);
    // Fill with random data...
    auto key = Base64::encode(keyBytes);
    request.setHeader("Sec-WebSocket-Key", key);

    // Send the request
    m_httpClient->send(request, [this, promise, key](const HttpResponse &response) {
        // Check if upgrade successful
        if (response.statusCode() == 101
                && response.hasHeader("Upgrade")
                && response.hasHeader("Sec-WebSocket-Accept")) {

            // Verify the Sec-WebSocket-Accept header
            std::string expectedAccept = calculateAcceptKey(key);
            if (response.header("Sec-WebSocket-Accept") == expectedAccept) {
                // Take ownership of the socket from HttpClient
                setupSocket(response.takeSocket());
                promise->set_value(true);
                return;
            }
        }

        promise->set_value(false);
    });

    return future;
}
```

### Socket Management

Once the HTTP upgrade is complete, we'll need to:

1. Take ownership of the socket from the `HttpClient`
2. Set up read/write handlers for WebSocket frame processing
3. Manage the WebSocket connection lifecycle

```cpp
void WebSocketClient::setupSocket(std::shared_ptr<Socket> socket) {
    m_socket = socket;
    m_state = State::Connected;

    // Set up read handler
    m_socket->readyRead.connect([this]() {
        processIncomingData();
    });

    m_socket->errorOccurred.connect([this](std::error_code ec) {
        handleSocketError(ec);
    });

    // Start keep-alive timer for ping/pong
    startPingTimer();

    // Emit connected signal
    connected.emit();
}
```

### Frame Processing

The WebSocket protocol requires proper frame parsing and generation:

```cpp
void WebSocketClient::processIncomingData() {
    // Read available data from socket
    auto data = m_tcpSocket->readAll();

    // Append to buffer
    m_receiveBuffer.append(data);

    // Process frames in buffer
    size_t bytesProcessed = 0;
    while (m_receiveBuffer.size() > 2) { // Minimum frame size
        auto frame = WebSocketFrame::decode(m_receiveBuffer, bytesProcessed);

        if (!frame.has_value()) {
            // Need more data
            break;
        }

        // Handle the frame based on opcode
        handleFrame(*frame);

        // Remove processed frame from buffer
        m_receiveBuffer = m_receiveBuffer.mid(bytesProcessed);
    }
}

void WebSocketClient::handleFrame(const WebSocketFrame &frame) {
    switch (frame.opCode()) {
        case WebSocketFrame::OpCode::Text:
            textMessageReceived.emit(frame.payload().toString());
            break;

        case WebSocketFrame::OpCode::Binary:
            binaryMessageReceived.emit(frame.payload());
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

        default:
            // Unhandled opcode
            break;
    }
}
```

### Reconnection Logic

To support automatic reconnection:

```cpp
void WebSocketClient::handleSocketError(std::error_code ec) {
    // Reset state
    m_state = State::Closed;

    // Emit error
    errorOccurred.emit("Socket error: " + ec.message());

    // Attempt reconnect if enabled
    if (m_autoReconnect && m_reconnectAttempts < m_maxReconnectAttempts) {
        m_reconnectAttempts++;
        aboutToReconnect.emit();

        // Schedule reconnect
        auto timer = std::make_shared<KDFoundation::Timer>();
        timer->setSingleShot(true);
        timer->setInterval(m_reconnectInterval);
        timer->timeout.connect([this, timer]() {
            if (m_url.isValid()) {
                connectToUrl(m_url);
            }
        });
        timer->start();
    }
}
```

## 4. Graceful Connection Handling

To implement graceful disconnection:

```cpp
void WebSocketClient::disconnect(uint16_t code, const std::string &reason) {
    if (m_state == State::Connected) {
        m_state = State::Closing;

        // Send close frame
        auto closeFrame = WebSocketFrame::createCloseFrame(code, reason);
        sendFrame(closeFrame);

        // Set up a timeout for clean disconnection
        m_closeTimer = std::make_unique<KDFoundation::Timer>();
        m_closeTimer->setSingleShot(true);
        m_closeTimer->setInterval(std::chrono::seconds(5));
        m_closeTimer->timeout.connect([this]() {
            // Force close if no response
            forceClose();
        });
        m_closeTimer->start();
    } else {
        forceClose();
    }
}

void WebSocketClient::forceClose() {
    if (m_socket) {
        m_socket->close();
    }
    m_state = State::Closed;
    disconnected.emit(1006, "Connection closed abnormally");
}

void WebSocketClient::handleCloseFrame(const WebSocketFrame &frame) {
    uint16_t code = 1005; // Default: No status code
    std::string reason = "No reason specified";

    // Extract code and reason if available
    if (frame.payload().size() >= 2) {
        code = (static_cast<uint16_t>(frame.payload()[0]) << 8) | frame.payload()[1];
        if (frame.payload().size() > 2) {
            reason = frame.payload().mid(2).toString();
        }
    }

    // If we initiated the close, this is the confirmation
    if (m_state == State::Closing) {
        forceClose();
    } else {
        // Server initiated the close, send confirmation and close
        m_state = State::Closing;
        auto closeFrame = WebSocketFrame::createCloseFrame(code, "");
        sendFrame(closeFrame);
        forceClose();
    }

    disconnected.emit(code, reason);
}
```

## 5. Integration with Existing Library

The WebSocketClient implementation fits naturally alongside your existing HttpClient and SseClient implementations:

1. It leverages the same `Socket`/`TcpSocket`/`SslSocket` infrastructure
2. It uses `HttpClient` for the initial connection setup
3. It follows similar patterns as `SseClient` for event handling

## 6. Example Usage

Here's how a user would interact with the WebSocketClient API:

```cpp
// Create a WebSocket client
auto client = std::make_shared<KDNetwork::WebSocketClient>();

// Set up event handlers
client->connected.connect([]() {
    std::cout << "Connected to WebSocket server" << std::endl;

    // Authenticate after connection
    client->sendTextMessage("{\"action\":\"auth\",\"params\":\"API_KEY\"}");
});

client->textMessageReceived.connect([](const std::string &message) {
    std::cout << "Received: " << message << std::endl;
});

client->disconnected.connect([](uint16_t code, const std::string &reason) {
    std::cout << "Disconnected: " << code << " - " << reason << std::endl;
});

client->errorOccurred.connect([](const std::string &error) {
    std::cout << "Error: " << error << std::endl;
});

// Enable auto-reconnect
client->setAutoReconnect(true);
client->setMaxReconnectAttempts(5);
client->setReconnectInterval(std::chrono::seconds(3));

// Connect to a WebSocket server
KDUtils::Uri url("wss://socket.polygon.io/stocks");
client->connectToUrl(url).then([](bool success) {
    if (success) {
        std::cout << "Connection initiated successfully" << std::endl;
    } else {
        std::cout << "Failed to initiate connection" << std::endl;
    }
});

// Send a message
client->sendTextMessage("{\"action\":\"subscribe\",\"params\":\"T.AAPL\"}");

// Later, request a graceful disconnection
client->disconnect(1000, "Done with connection");
```

## 7. Implementation Files Structure

I would suggest organizing the WebSocket implementation with these files:

1. `websocket_client.h` and `websocket_client.cpp` - Main client API
2. `websocket_frame.h` and `websocket_frame.cpp` - Internal frame handling
3. `websocket_common.h` - Constants and utility functions

This implementation plan builds on your existing networking stack and follows the same design patterns seen in your other networking classes. Would you like me to elaborate on any specific aspect of this plan?
