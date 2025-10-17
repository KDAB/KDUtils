seanharmer: Some HTTP based APIs such as the OpenAI responses end point (https://platform.openai.com/docs/api-reference/responses) can optionally make use of Server Sent Events (SSE) for a more dynamic integration and user experience.

When setting the option `"stream": true` in the request, then the OpenAI end point will emit SSE's as described at https://platform.openai.com/docs/api-reference/responses-streaming.

Please suggest an outline plan for how we could extend HttpClient and the KDNetwork library to support Server Sent Events.

GitHub Copilot: # Plan for Extending KDNetwork Library to Support Server Sent Events (SSE)

Server Sent Events (SSE) is a technology that allows a server to push data to a client over an HTTP connection after the initial request has been made. This is particularly useful for applications like the OpenAI API's streaming responses where results need to be delivered incrementally as they become available.

Let me outline a plan for extending the KDNetwork library to support SSE:

## 1. Create a Server Sent Events Client Class

First, we should create a specialized client class for handling SSE connections:Based on my analysis of the codebase, here's a comprehensive plan for extending the KDNetwork library to support Server Sent Events (SSE):

## 1. Create an SSE Client Class

First, we should create a dedicated class to handle SSE connections. This class would extend the existing HttpClient functionality:

```cpp
class SseClient {
public:
    // Constructor accepting an HttpClient
    explicit SseClient(std::shared_ptr<HttpClient> httpClient = nullptr);

    // Connect to an SSE endpoint
    void connect(const HttpRequest& request);

    // Disconnect from the current SSE stream
    void disconnect();

    // Signal emitted when a message is received
    KDBindings::Signal<const SseEvent&> messageReceived;

    // Signal emitted when the connection is established
    KDBindings::Signal<> connected;

    // Signal emitted when the connection is closed
    KDBindings::Signal<> disconnected;

    // Signal emitted when an error occurs
    KDBindings::Signal<const std::string&> error;
};
```

## 2. Define an SSE Event Class

Next, we need a class to represent SSE events:

```cpp
class SseEvent {
public:
    // The event ID (optional in SSE)
    std::string id;

    // The event type (defaults to "message" if not specified)
    std::string event;

    // The data payload
    std::string data;

    // Retry timeout in milliseconds
    std::optional<int> retry;
};
```

## 3. Extend HttpClient to Support SSE

The HttpClient class would need modifications to:

1. Support maintaining a persistent connection
2. Parse SSE-formatted responses
3. Emit events as they come in rather than waiting for the full response

```cpp
// Add to HttpClient class:
public:
    /**
     * @brief Create a Server-Sent Events client
     * @return A new SseClient instance using this HttpClient
     */
    std::shared_ptr<SseClient> createSseClient();

private:
    // Internal method for SSE connection handling
    void setupSseConnection(std::shared_ptr<RequestState> state);

    // Parse incoming SSE data
    void processSseData(std::shared_ptr<RequestState> state);
```

## 4. Add an SSE Parser

Implement a parser to handle the SSE format according to the specification:

```cpp
class SseParser {
public:
    // Process incoming data chunk and return any complete events
    std::vector<SseEvent> processData(const KDUtils::ByteArray& data);

private:
    // Buffer for incomplete events
    std::string m_buffer;

    // Parse a single event from text
    SseEvent parseEvent(const std::string& eventText);
};
```

## 5. Modify RequestState Class

The existing `RequestState` class needs to be extended to handle SSE connections:

```cpp
// Add to existing RequestState class:
bool sseMode = false;
SseParser sseParser;
std::function<void(const SseEvent&)> sseCallback;
```

## 6. Implement SSE-specific HTTP Request Handling

We need to implement special handling for SSE requests:

1. Set appropriate headers:
   - `Accept: text/event-stream`
   - `Cache-Control: no-cache`
2. Keep the connection open
3. Parse incoming chunks as they arrive

## 7. Implementation Flow

Here's how the SSE functionality would work:

1. User creates an `HttpRequest` object specifying the SSE endpoint
2. User connects to the SSE endpoint using either:
   - `SseClient::connect(request)`
   - `HttpClient::send(request)` with a special flag or header
3. The `HttpClient` keeps the connection open
4. As data arrives, it's parsed using the `SseParser`
5. Complete SSE events are emitted via signals
6. The connection remains open until explicitly closed or an error occurs

## 8. Example Usage

```cpp
// Create an HTTP client
auto httpClient = std::make_shared<KDNetwork::HttpClient>();

// Create an SSE client
auto sseClient = httpClient->createSseClient();

// Connect to SSE endpoint
KDNetwork::HttpRequest request("https://api.openai.com/v1/completions");
request.setMethod(KDNetwork::HttpMethod::Post);
request.setHeader("Content-Type", "application/json");
request.setHeader("Authorization", "Bearer YOUR_API_KEY");
request.setBody("{\"model\": \"gpt-3.5-turbo\", \"stream\": true, \"messages\": [{\"role\": \"user\", \"content\": \"Hello\"}]}");

// Handle events
std::ignore = sseClient->messageReceived.connect([](const KDNetwork::SseEvent& event) {
    // Process each chunk as it arrives
    std::cout << "Received: " << event.data << std::endl;
});

// Handle connection events
std::ignore = sseClient->connected.connect([]() {
    std::cout << "Connected to SSE stream" << std::endl;
});

std::ignore = sseClient->disconnected.connect([]() {
    std::cout << "Disconnected from SSE stream" << std::endl;
});

// Connect to the stream
sseClient->connect(request);
```

## 9. Technical Challenges and Solutions

1. **Connection Management**: Keep-alive connections need to be maintained indefinitely. The connection pooling and cleanup mechanisms in HttpClient may need adjustments.

2. **Chunked Transfer Encoding**: SSE typically uses chunked transfer encoding. The response parser needs to handle this correctly.

3. **Handling Reconnection**: If the connection is lost, SSE clients should automatically reconnect with the last received event ID.

4. **Event Parsing**: SSE has a specific format that needs careful parsing:

   ```
   event: event_type
   id: event_id
   data: event_data
   retry: reconnection_time

   ```

5. **Timeouts**: Need to handle keep-alive timeouts and implement reconnection logic.

## 10. Integration with Existing Architecture

The implementation should leverage the existing event system in KDFoundation:

1. Use `FileDescriptorNotifier` for socket readiness notifications
2. Leverage the existing event loop for async behavior
3. Utilize the signal mechanism from KDBindings for event notifications

This approach maintains consistency with the rest of the library while adding the specialized SSE functionality.

Would you like me to provide more details on any specific part of this implementation plan?
