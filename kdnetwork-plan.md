# KDFoundation Asynchronous Networking Plan

## Core Components & Design Philosophy

1.  **Leverage KDFoundation Primitives:** The core idea is to build directly on BSD sockets and integrate tightly with KDFoundation's event loop (`CoreApplication::processEvents`, etc.) and `FileDescriptorNotifier` (which exists according to the provided file list: `/src/KDFoundation/file_descriptor_notifier.h`). We will avoid external networking libraries like Boost.Asio or libuv for the core socket operations, as requested.
2.  **Asynchronous Operations:** All network operations (connect, accept, read, write, DNS lookup, TLS handshake) must be non-blocking. Completion, data availability, or errors should be signaled asynchronously, likely via signals/slots or callback mechanisms that integrate naturally with the KDFoundation event loop.
3.  **Modern C++:** Utilize features like RAII (for socket/resource management), `std::function` or similar for callbacks, `std::error_code` or custom error types, and potentially C++17/20 features where appropriate for cleaner and safer code.

## Proposed Implementation Plan

1.  **URI Handling (`KDUtils::Url`)**

    - **Status:** Already exists (`/src/KDUtils/url.h`).
    - **Action:** Use `KDUtils::Url` for parsing and constructing URIs needed for HTTP requests and potentially other protocols. Ensure it provides easy access to scheme, host, port, path, etc., as needed by the networking components.

2.  **Asynchronous Socket Base (`KDFoundation::Net::Socket` or similar)**

    - **Responsibility:** Manage the raw BSD socket file descriptor and its integration with the event loop.
    - **Features:**
      - Methods: `open()`, `close()`, `bind()`, `setBlocking(false)`.
      - RAII: Constructor acquires the socket FD, destructor closes it. Consider move semantics.
      - **Event Loop Integration:** Internally create and manage `KDFoundation::FileDescriptorNotifier` instances to monitor the socket FD for readability and writability. When the notifier signals, emit corresponding signals/callbacks (e.g., `readyRead()`, `bytesWritten()`, `errorOccurred(ErrorCode)`).
    - **Error Handling:** Define and use a consistent error reporting mechanism (e.g., `std::error_code`, custom enum/class) that can represent socket-level errors (`errno`) and potentially higher-level issues.

3.  **TCP Client (`KDFoundation::Net::TcpSocket`)**

    - **Responsibility:** Handle client-side TCP connections. Could inherit from or compose the base `Socket`.
    - **Features:**
      - `connectToHost(host, port)`: Initiates a non-blocking connection. Monitor the socket FD for writability (signals connection success) or error using the `FileDescriptorNotifier`.
      - Signals/Callbacks: `connected()`, `disconnected()`, `errorOccurred(ErrorCode)`, `readyRead()`, `bytesWritten(count)`. These are triggered by the underlying event loop integration.
      - `read(buffer, maxSize)` / `readAll()`: Reads available data _after_ the `readyRead()` signal is emitted. Returns data read or indicates errors. Non-blocking.
      - `write(data)`: Attempts to write data. Returns bytes written immediately or 0/-1 if the socket isn't ready (EWOULDBLOCK/EAGAIN). Use the `bytesWritten` signal (triggered when the notifier signals writability) to manage buffering and report completion. Non-blocking.

4.  **TCP Server (`KDFoundation::Net::TcpServer`)**

    - **Responsibility:** Listen for and accept incoming TCP connections.
    - **Features:**
      - Uses a base `Socket` internally for the listening socket.
      - `listen(address, port)`: Binds and starts listening. Uses `FileDescriptorNotifier` on the listening socket, monitoring for readability.
      - Signal/Callback: `newConnection(std::unique_ptr<TcpSocket>)`. Triggered when the `FileDescriptorNotifier` indicates the listening socket is readable (signaling a pending connection).
      - Internal Logic: When notified, call `accept()` to get the new client socket FD, create a new `Net::TcpSocket` instance (transferring FD ownership, perhaps via move semantics), and emit the `newConnection` signal with the new socket object.

5.  **UDP Sockets (`KDFoundation::Net::UdpSocket`)**

    - **Responsibility:** Handle UDP datagram sending and receiving.
    - **Features:**
      - Uses a base `Socket` internally.
      - `bind(address, port)`: Binds the socket for receiving. Uses `FileDescriptorNotifier` for readability.
      - `writeDatagram(data, host, port)`: Sends a datagram using `sendto()`. Non-blocking.
      - `readDatagram(buffer, maxSize, &senderHost, &senderPort)`: Called after `readyRead()` signal to receive data using `recvfrom()`.
      - Signals/Callbacks: `readyRead(senderHost, senderPort)`, `errorOccurred(ErrorCode)`. `readyRead` should probably include sender info.

6.  **Asynchronous DNS Lookups (`KDFoundation::Net::DnsResolver`)**

    - **Challenge:** Standard `getaddrinfo` is blocking.
    - **Options:**
      - **Thread Pool:** Execute blocking `getaddrinfo` in a separate KDFoundation thread (if threading is available/suitable) and post the results back to the main event loop via signals/events. This is often the simplest cross-platform approach if a dedicated async library isn't used.
      - **c-ares Library:** Integrate the `c-ares` C library. It's specifically designed for asynchronous DNS and can integrate with external event loops by providing file descriptors and timeouts to monitor via `FileDescriptorNotifier` and KDFoundation timers. More complex but potentially higher performance.
    - **Interface:** Define a `Net::DnsResolver` class. A method like `lookup(hostname)` should return a future/promise or take a callback, which is invoked asynchronously from the event loop with the list of addresses or an error.

7.  **SSL/TLS Support (`KDFoundation::Net::SslSocket`, `KDFoundation::Net::SslServer`)**

    - **Dependency:** Link against libssl (OpenSSL).
    - **Approach:** Wrap OpenSSL's non-blocking BIO or socket-level API.
    - **Context:** Create `Net::SslContext` to manage `SSL_CTX` (loading certificates, keys, setting protocols/ciphers).
    - **`Net::SslSocket`:**
      - Wraps a `Net::TcpSocket`.
      - Manages an `SSL*` object associated with the socket FD.
      - Implements `connectToHostEncrypted()`, `readEncrypted()`, `writeEncrypted()`.
      - **Integration:** Perform OpenSSL operations (`SSL_connect`, `SSL_accept`, `SSL_read`, `SSL_write`). When they return `SSL_ERROR_WANT_READ` or `SSL_ERROR_WANT_WRITE`, pause the operation and rely on the underlying `TcpSocket`'s `FileDescriptorNotifier` integration to signal when the socket is ready. Resume the OpenSSL operation when notified.
      - Handles the asynchronous TLS handshake process using this mechanism. Signals like `encrypted()` can indicate handshake completion.
    - **`Net::SslServer`:** Similar to `TcpServer`, but after accepting a raw TCP connection, it initiates the server-side TLS handshake (`SSL_accept`) asynchronously using the mechanism above, ultimately providing `Net::SslSocket` instances via a signal like `newEncryptedConnection`.

8.  **HTTP Client (`KDFoundation::Http::Client`)**

    - **Responsibility:** High-level HTTP/1.1 request execution.
    - **Features:**
      - Uses `Net::TcpSocket` or `Net::SslSocket` (determined by the `KDUtils::Url` scheme) and `Net::DnsResolver`.
      - Input: Takes a `Http::Request` object (containing `KDUtils::Url`, method, headers, optional body).
      - Process: Performs DNS lookup, connects (TCP/TLS), sends the formatted HTTP request headers and body, reads and parses the HTTP response (status line, headers, body).
      - Output: Asynchronously returns a `Http::Response` object (or an error) via a future, promise, or callback.
      - Considerations: Handle redirects, connection reuse (Keep-Alive), chunked transfer encoding.

9.  **Websocket Support (Future)**
    - **Foundation:** The `Net::TcpSocket` and `Net::SslSocket` provide the necessary transport layer.
    - **Implementation:**
      - Perform the initial HTTP/S handshake using logic similar to the `Http::Client` to negotiate the protocol upgrade.
      - Once upgraded, use the underlying `TcpSocket`/`SslSocket` directly to send/receive raw websocket frames according to RFC 6455.
      - Implement websocket framing logic (parsing opcodes, masking client-to-server frames, handling fragmentation, pings/pongs) as a layer on top of the socket.

This structured approach should provide a solid foundation for adding powerful and convenient asynchronous networking capabilities to KDFoundation. Remember to prioritize clear error handling and reporting throughout the implementation.
