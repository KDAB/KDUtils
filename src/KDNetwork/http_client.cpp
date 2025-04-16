/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/http_client.h>
#include <KDNetwork/tcp_socket.h>
#include <KDNetwork/ssl_socket.h>
#include <KDNetwork/dns_resolver.h>
#include <KDNetwork/http_parser.h>
#include <KDNetwork/sse_client.h>

#include <KDFoundation/core_application.h>
#include <KDFoundation/timer.h>

#include <KDUtils/uri.h>

#include <chrono>
#include <future>
#include <sstream>
#include <algorithm>
#include <memory>
#include <system_error>

namespace KDNetwork {

// Internal request state class
class HttpClient::RequestState
{
public:
    RequestState(const HttpRequest &req, std::function<void(const HttpResponse &)> callback, std::promise<HttpResponse> promise)
        : request(req), userCallback(std::move(callback)), responsePromise(std::move(promise))
    {
        response = HttpResponse(req);
        startTime = std::chrono::steady_clock::now();
    }

    HttpRequest request;
    HttpResponse response;
    std::function<void(const HttpResponse &)> userCallback;
    std::promise<HttpResponse> responsePromise;

    std::shared_ptr<Socket> socket;
    std::string host;
    uint16_t port{ 0 };
    bool secure{ false };

    std::chrono::steady_clock::time_point startTime;
    std::unique_ptr<HttpParser> parser;
    KDUtils::ByteArray responseBuffer;
    KDUtils::ByteArray requestData;
    int64_t bytesSent{ 0 };
    int64_t expectedContentLength{ -1 };

    int redirectCount{ 0 };
    std::unique_ptr<KDFoundation::Timer> timeoutTimer;
    bool headersParsed{ false };
    bool completed{ false };

    // New flag for SSE/streaming mode
    bool streamingMode{ false };

    // Flag for Websocket upgrade
    bool websocketUpgrade{ false };

    // Reference to associated SSE client for streaming connections
    std::shared_ptr<SseClient> sseClient;

    // Helper to generate request data
    void buildRequestData()
    {
        std::stringstream ss;

        // Request line
        ss << toStdString(request.method()) << " ";

        // Extract path from URL
        std::string urlStr = request.url().toString();
        std::string path = "/";
        size_t hostStart = urlStr.find("://");
        if (hostStart != std::string::npos) {
            std::string hostAndPath = urlStr.substr(hostStart + 3);
            size_t pathStart = hostAndPath.find('/');
            if (pathStart != std::string::npos) {
                path = hostAndPath.substr(pathStart);
            }
        }

        ss << path << " HTTP/1.1\r\n";

        // Headers
        for (const auto &header : request.allHeaders()) {
            ss << header.first << ": " << header.second << "\r\n";
        }

        // Add Host header if not present
        if (request.header("Host").empty()) {
            ss << "Host: " << host;
            if ((secure && port != 443) || (!secure && port != 80)) {
                ss << ":" << port;
            }
            ss << "\r\n";
        }

        // Add Content-Length header if not present and we have a body
        if (!request.body().empty() && request.header("Content-Length").empty()) {
            ss << "Content-Length: " << request.body().size() << "\r\n";
        }

        // End of headers
        ss << "\r\n";

        // Convert to ByteArray
        std::string headerString = ss.str();
        requestData = KDUtils::ByteArray(
                reinterpret_cast<const uint8_t *>(headerString.data()),
                headerString.size());

        // Append body if present
        if (!request.body().empty()) {
            requestData.append(request.body());
        }
    }
};

HttpClient::HttpClient(std::shared_ptr<HttpSession> session)
    : m_session(session ? session : std::make_shared<HttpSession>())
{
    // Set up cleanup timer
    m_cleanupTimer = std::make_unique<KDFoundation::Timer>();
    m_cleanupTimer->interval = std::chrono::seconds(30);
    std::ignore = m_cleanupTimer->timeout.connect([this]() {
        m_session->cleanupConnections();
    });
    m_cleanupTimer->running = true;
}

HttpClient::~HttpClient()
{
    cancelAll();
}

std::future<HttpResponse> HttpClient::send(const HttpRequest &request,
                                           std::function<void(const HttpResponse &)> callback)
{
    // Create promise for the future
    std::promise<HttpResponse> promise;
    std::future<HttpResponse> future = promise.get_future();

    // Create request state
    auto state = createRequestState(request, callback, std::move(promise));

    // Start the request (async)
    startRequest(state);

    return future;
}

std::future<HttpResponse> HttpClient::sendWithSseClient(
        const HttpRequest &request,
        std::shared_ptr<SseClient> sseClient,
        std::function<void(const HttpResponse &)> callback)
{
    // Create promise for the future
    std::promise<HttpResponse> promise;
    std::future<HttpResponse> future = promise.get_future();

    // Create request state
    auto state = createRequestState(request, callback, std::move(promise));

    // Store the reference to the SSE client
    state->sseClient = sseClient;

    // Start the request (async)
    startRequest(state);

    return future;
}

std::future<HttpResponse> HttpClient::get(const KDUtils::Uri &url,
                                          std::function<void(const HttpResponse &)> callback)
{
    HttpRequest request(url, HttpMethod::Get);
    return send(request, callback);
}

std::future<HttpResponse> HttpClient::head(const KDUtils::Uri &url,
                                           std::function<void(const HttpResponse &)> callback)
{
    HttpRequest request(url, HttpMethod::Head);
    return send(request, callback);
}

std::future<HttpResponse> HttpClient::post(const KDUtils::Uri &url,
                                           const KDUtils::ByteArray &data,
                                           const std::string &contentType,
                                           std::function<void(const HttpResponse &)> callback)
{
    HttpRequest request(url, HttpMethod::Post);
    request.setBody(data);
    request.setHeader("Content-Type", contentType);
    return send(request, callback);
}

std::future<HttpResponse> HttpClient::post(const KDUtils::Uri &url,
                                           const KDUtils::ByteArray &data,
                                           std::function<void(const HttpResponse &)> callback)
{
    HttpRequest request(url, HttpMethod::Post);
    request.setBody(data);
    return send(request, callback);
}

std::future<HttpResponse> HttpClient::put(const KDUtils::Uri &url,
                                          const KDUtils::ByteArray &data,
                                          const std::string &contentType,
                                          std::function<void(const HttpResponse &)> callback)
{
    HttpRequest request(url, HttpMethod::Put);
    request.setBody(data);
    request.setHeader("Content-Type", contentType);
    return send(request, callback);
}

std::future<HttpResponse> HttpClient::deleteResource(const KDUtils::Uri &url,
                                                     std::function<void(const HttpResponse &)> callback)
{
    HttpRequest request(url, HttpMethod::Delete);
    return send(request, callback);
}

std::future<HttpResponse> HttpClient::patch(const KDUtils::Uri &url,
                                            const KDUtils::ByteArray &data,
                                            const std::string &contentType,
                                            std::function<void(const HttpResponse &)> callback)
{
    HttpRequest request(url, HttpMethod::Patch);
    request.setBody(data);
    request.setHeader("Content-Type", contentType);
    return send(request, callback);
}

std::future<HttpResponse> HttpClient::options(const KDUtils::Uri &url,
                                              std::function<void(const HttpResponse &)> callback)
{
    HttpRequest request(url, HttpMethod::Options);
    return send(request, callback);
}

void HttpClient::cancelAll()
{
    // Make a copy of the active requests before iterating
    auto requests = m_activeRequests;
    for (const auto &[socket, state] : requests) {
        failRequest(state, "Request cancelled");
    }
}

std::shared_ptr<HttpSession> HttpClient::session() const
{
    return m_session;
}

void HttpClient::setSession(std::shared_ptr<HttpSession> session)
{
    m_session = session ? session : std::make_shared<HttpSession>();
}

std::shared_ptr<HttpClient::RequestState> HttpClient::createRequestState(
        const HttpRequest &request,
        std::function<void(const HttpResponse &)> callback,
        std::promise<HttpResponse> promise)
{
    auto state = std::make_shared<RequestState>(request, callback, std::move(promise));

    // Use KDUtils::Uri functionality to extract URL components
    const KDUtils::Uri &uri = request.url();

    // Get scheme and determine if connection is secure
    std::string scheme = uri.scheme();
    state->secure = (scheme == "https");

    // Get host
    state->host = uri.host();

    // Get port - use explicit port if available, otherwise use default for the scheme
    if (uri.hasExplicitPort()) {
        state->port = uri.port();
    } else {
        state->port = state->secure ? 443 : 80; // Default ports
    }

    // Set up the timeout timer
    state->timeoutTimer = std::make_unique<KDFoundation::Timer>();
    state->timeoutTimer->singleShot = true;
    state->timeoutTimer->interval = request.timeout();
    // TODO: Can we ignore this? Or should we put the connection handler in the state?
    std::ignore = state->timeoutTimer->timeout.connect([this, state]() {
        onTimeout(state);
    });

    return state;
}

void HttpClient::startRequest(std::shared_ptr<RequestState> state)
{
    // Apply session defaults
    auto modifiableRequest = state->request;
    if (modifiableRequest.autoAddCommonHeaders()) {
        m_session->applyDefaultHeaders(modifiableRequest);
    }

    // Add cookies if available
    std::string cookieHeader = m_session->cookieJar().cookieHeaderForUrl(modifiableRequest.url());
    if (!cookieHeader.empty() && modifiableRequest.header("Cookie").empty()) {
        modifiableRequest.setHeader("Cookie", cookieHeader);
    }

    // Allow signal handlers to modify the request
    aboutToSendRequest.emit(modifiableRequest);
    state->request = modifiableRequest;

    // Check if we have a pooled connection available
    state->socket = m_session->getConnection(state->host, state->port, state->secure);

    if (state->socket && state->socket->state() == Socket::State::Connected) {
        // Use existing connection
        setupSocketCallbacks(state);
        state->buildRequestData();
        // Start timeout
        state->timeoutTimer->running = true;

        // Send request
        if (state->socket->type() == Socket::SocketType::Tcp ||
            state->socket->type() == Socket::SocketType::SslTcp) {
            auto tcpSocket = std::dynamic_pointer_cast<TcpSocket>(state->socket);
            if (tcpSocket) {
                state->bytesSent = tcpSocket->write(state->requestData);
                if (state->bytesSent < 0) {
                    // Error sending data
                    failRequest(state, "Error sending request: socket write error");
                    return;
                }

                // Add to active requests map
                m_activeRequests[state->socket] = state;

                // Set up HTTP parser
                state->parser = std::make_unique<HttpParser>(HttpParser::Type::Response);
                setupParserCallbacks(state);
            } else {
                failRequest(state, "Error: invalid socket type");
            }
        } else {
            failRequest(state, "Error: invalid socket type");
        }
    } else {
        // Need to create a new connection
        // Resolve hostname first
        DnsResolver &resolver = DnsResolver::instance();
        resolver.lookup(state->host, [this, state](std::error_code ec, const DnsResolver::AddressInfoList &addresses) {
            if (ec) {
                // DNS resolution failed
                failRequest(state, "DNS resolution failed: " + ec.message());
                return;
            }

            if (addresses.empty()) {
                // No addresses found
                failRequest(state, "No addresses found for host: " + state->host);
                return;
            }

            // Create socket
            state->socket = createSocket(state->secure, state->host);
            if (!state->socket) {
                failRequest(state, "Failed to create socket");
                return;
            }

            // Set up socket callbacks
            setupSocketCallbacks(state);

            // Add to active requests map
            m_activeRequests[state->socket] = state;

            // Start timeout
            state->timeoutTimer->running = true;

            // Connect to the first address
            KDUtils::Logger::logger("KDNetwork")->debug("Connecting to host: {} at {}", state->host, addresses[0].toString());
            auto tcpSocket = std::dynamic_pointer_cast<TcpSocket>(state->socket);
            if (tcpSocket) {
                if (!tcpSocket->connectToHost(addresses[0], state->port)) {
                    failRequest(state, "Failed to connect to host");
                    return;
                }

                // Set up HTTP parser
                state->parser = std::make_unique<HttpParser>(HttpParser::Type::Response);
                setupParserCallbacks(state);

                // Build request data (will be sent when connected)
                state->buildRequestData();
            } else {
                failRequest(state, "Error: invalid socket type");
            }
        });
    }
}

void HttpClient::finishRequest(std::shared_ptr<RequestState> state)
{
    // Stop timeout timer
    if (state->timeoutTimer) {
        state->timeoutTimer->running = false;
    }

    // Set elapsed time
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - state->startTime);
    state->response.setElapsed(elapsed);

    // Set redirect count
    state->response.setRedirectCount(state->redirectCount);

    // Check if we should follow redirects
    if (state->response.isRedirect() &&
        state->redirectCount < state->request.maxRedirects() &&
        (m_session->followRedirects() ||
         state->request.redirectPolicy() != HttpRequest::RedirectPolicy::DontFollow)) {

        followRedirect(state);
        return;
    }

    // Return or reuse the connection if keep-alive
    if (state->socket && state->response.isKeepAlive() &&
        state->socket->state() == Socket::State::Connected) {
        m_session->returnConnection(state->host, state->port, state->secure, state->socket);
    } else if (state->socket) {
        state->socket->close();
    }

    // Remove from active requests
    m_activeRequests.erase(state->socket);

    // Emit signal
    responseReceived.emit(state->response);

    // Call user callback
    if (state->userCallback) {
        state->userCallback(state->response);
    }

    // Set promise value
    try {
        state->responsePromise.set_value(state->response);
    } catch (const std::future_error &) {
        // Promise might already be set (e.g., on timeout)
    }
}

void HttpClient::failRequest(std::shared_ptr<RequestState> state, const std::string &errorString)
{
    // Stop timeout timer
    if (state->timeoutTimer) {
        state->timeoutTimer->running = false;
    }

    // Set error on response
    state->response.setError(errorString);

    // Set elapsed time
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - state->startTime);
    state->response.setElapsed(elapsed);

    // Close socket
    if (state->socket) {
        state->socket->close();
    }

    // Remove from active requests
    auto it = m_activeRequests.find(state->socket);
    if (it != m_activeRequests.end()) {
        m_activeRequests.erase(it);
    }

    // Emit error signal
    error.emit(state->request, errorString);

    // Call user callback
    if (state->userCallback) {
        state->userCallback(state->response);
    }

    // Set promise value
    try {
        state->responsePromise.set_value(state->response);
    } catch (const std::future_error &) {
        // Promise might already be set (e.g., on timeout or cancellation)
    }
}

void HttpClient::followRedirect(std::shared_ptr<RequestState> state)
{
    auto redirectUrl = state->response.redirectUrl();
    if (!redirectUrl) {
        failRequest(state, "Redirect response with no valid Location header");
        return;
    }

    // Check redirect policy
    switch (state->request.redirectPolicy()) {
    case HttpRequest::RedirectPolicy::DontFollow:
        // Don't follow redirects, just return the response
        finishRequest(state);
        return;

    case HttpRequest::RedirectPolicy::FollowSameHost: {
        // Check if redirect URL has the same host
        std::string originalHost = state->host;
        std::string redirectHost;

        std::string redirectUrlStr = redirectUrl->toString();
        size_t hostStart = redirectUrlStr.find("://");
        if (hostStart != std::string::npos) {
            std::string hostAndPath = redirectUrlStr.substr(hostStart + 3);
            size_t pathStart = hostAndPath.find('/');
            redirectHost = (pathStart != std::string::npos) ? hostAndPath.substr(0, pathStart) : hostAndPath;

            // Remove port if present
            size_t portPos = redirectHost.find(':');
            if (portPos != std::string::npos) {
                redirectHost = redirectHost.substr(0, portPos);
            }
        }

        if (originalHost != redirectHost) {
            // Different host, don't follow redirect
            finishRequest(state);
            return;
        }
        break;
    }

    case HttpRequest::RedirectPolicy::FollowSameHostAndProtocol: {
        // Check if redirect URL has the same host and protocol
        std::string originalScheme = state->secure ? "https" : "http";
        std::string redirectScheme;
        std::string redirectHost;

        std::string redirectUrlStr = redirectUrl->toString();
        size_t schemeEnd = redirectUrlStr.find("://");
        if (schemeEnd != std::string::npos) {
            redirectScheme = redirectUrlStr.substr(0, schemeEnd);
            std::transform(redirectScheme.begin(), redirectScheme.end(), redirectScheme.begin(),
                           [](unsigned char c) { return std::tolower(c); });

            std::string hostAndPath = redirectUrlStr.substr(schemeEnd + 3);
            size_t pathStart = hostAndPath.find('/');
            redirectHost = (pathStart != std::string::npos) ? hostAndPath.substr(0, pathStart) : hostAndPath;

            // Remove port if present
            size_t portPos = redirectHost.find(':');
            if (portPos != std::string::npos) {
                redirectHost = redirectHost.substr(0, portPos);
            }
        }

        if (originalScheme != redirectScheme || state->host != redirectHost) {
            // Different host or protocol, don't follow redirect
            finishRequest(state);
            return;
        }
        break;
    }

    case HttpRequest::RedirectPolicy::FollowAll:
        // Always follow redirects
        break;
    }

    // Close current connection unless we're redirecting to the same host
    bool needNewConnection = true;
    if (state->socket && state->socket->state() == Socket::State::Connected) {
        std::string redirectUrlStr = redirectUrl->toString();
        std::string redirectScheme;
        std::string redirectHost;
        uint16_t redirectPort = 0;

        size_t schemeEnd = redirectUrlStr.find("://");
        if (schemeEnd != std::string::npos) {
            redirectScheme = redirectUrlStr.substr(0, schemeEnd);
            std::transform(redirectScheme.begin(), redirectScheme.end(), redirectScheme.begin(),
                           [](unsigned char c) { return std::tolower(c); });

            bool redirectSecure = (redirectScheme == "https");

            std::string hostAndPath = redirectUrlStr.substr(schemeEnd + 3);
            size_t pathStart = hostAndPath.find('/');
            std::string hostAndPort = (pathStart != std::string::npos) ? hostAndPath.substr(0, pathStart) : hostAndPath;

            // Check for port in the host
            size_t portPos = hostAndPort.find(':');
            if (portPos != std::string::npos) {
                redirectHost = hostAndPort.substr(0, portPos);
                std::string portStr = hostAndPort.substr(portPos + 1);
                try {
                    redirectPort = std::stoi(portStr);
                } catch (...) {
                    redirectPort = redirectSecure ? 443 : 80; // Default ports
                }
            } else {
                redirectHost = hostAndPort;
                redirectPort = redirectSecure ? 443 : 80; // Default ports
            }

            if (redirectHost == state->host && redirectPort == state->port &&
                redirectSecure == state->secure) {
                // Same host, port, and scheme, can reuse connection
                needNewConnection = false;
            }
        }
    }

    // Create a new request with the redirect URL
    HttpRequest redirectRequest = state->request;
    redirectRequest.setUrl(*redirectUrl);

    // For POST redirects to GET (301, 302, 303)
    int statusCode = state->response.statusCode();
    if ((statusCode == 301 || statusCode == 302 || statusCode == 303) &&
        redirectRequest.method() == HttpMethod::Post) {
        redirectRequest.setMethod(HttpMethod::Get);
        redirectRequest.setBody(KDUtils::ByteArray());
        redirectRequest.removeHeader("Content-Type");
        redirectRequest.removeHeader("Content-Length");
    }

    // Remove from active requests
    auto it = m_activeRequests.find(state->socket);
    if (it != m_activeRequests.end()) {
        m_activeRequests.erase(it);
    }

    // Reset response
    state->response = HttpResponse(redirectRequest);
    state->redirectCount++;
    state->completed = false;
    state->headersParsed = false;
    state->responseBuffer.clear();

    // Use the existing socket if possible, otherwise close it
    if (needNewConnection && state->socket) {
        state->socket->close();
        state->socket = nullptr;
    }

    // Reset parser
    if (state->parser) {
        state->parser->reset();
        setupParserCallbacks(state);
    }

    // Update request
    state->request = redirectRequest;

    // Restart the request
    startRequest(state);
}

void HttpClient::onReadyRead(std::shared_ptr<RequestState> state)
{
    auto tcpSocket = std::dynamic_pointer_cast<TcpSocket>(state->socket);
    if (!tcpSocket) {
        failRequest(state, "Invalid socket type");
        return;
    }

    // Read available data
    KDUtils::ByteArray data = tcpSocket->readAll();
    if (data.empty()) {
        return;
    }

    // Process received data
    if (state->parser) {
        // Parse the data
        bool parseResult = state->parser->parse(data);

        if (!parseResult) {
            if (state->websocketUpgrade && state->parser->error() == HttpParser::ParserError::PausedUpgrade) {
                // Find where the parser paused and store the remaining data so that the WebSocket client can handle it
                KDUtils::ByteArray excessData = data.mid(state->parser->errorLocation());
                state->response.setExcessData(excessData);

                // Emit completion signals - but don't touch the socket
                if (state->userCallback) {
                    state->userCallback(state->response);
                }

                return;
            } else {
                // Parsing error occurred
                failRequest(state, "HTTP parsing error");
                return;
            }
        }

        // Update download progress
        state->responseBuffer.append(data);
        downloadProgress.emit(state->request, state->responseBuffer.size(), state->expectedContentLength);
    } else {
        failRequest(state, "Parser not initialized");
    }
}

void HttpClient::onSocketConnected(std::shared_ptr<RequestState> state)
{
    auto tcpSocket = std::dynamic_pointer_cast<TcpSocket>(state->socket);
    if (!tcpSocket) {
        failRequest(state, "Invalid socket type");
        return;
    }

    // Send request
    state->bytesSent = tcpSocket->write(state->requestData);
    if (state->bytesSent < 0) {
        failRequest(state, "Error sending request: socket write error");
    } else {
        // Update upload progress
        uploadProgress.emit(state->request, state->bytesSent, state->requestData.size());
    }
}

void HttpClient::onSocketError(std::shared_ptr<RequestState> state, std::error_code ec)
{
    failRequest(state, "Socket error: " + ec.message());
}

void HttpClient::onTimeout(std::shared_ptr<RequestState> state)
{
    failRequest(state, "Request timeout");
}

std::shared_ptr<Socket> HttpClient::createSocket(bool secure, const std::string &host)
{
    if (secure) {
        auto sslSocket = std::make_shared<SslSocket>();

        // Set default verification mode
        sslSocket->setVerificationMode(SslSocket::VerificationMode::VerifyPeer);

        // Set the peer verification name
        if (!host.empty())
            sslSocket->setPeerVerifyName(host);

        // Connect signal for SSL handshake errors
        std::ignore = sslSocket->handshakeError.connect([this, &sslSocket](const std::string &error) {
            auto it = std::find_if(m_activeRequests.begin(), m_activeRequests.end(),
                                   [&sslSocket](const auto &pair) {
                                       return pair.first == sslSocket;
                                   });

            if (it != m_activeRequests.end()) {
                failRequest(it->second, "SSL handshake error: " + error);
            }
        });

        return sslSocket;
    } else {
        return std::make_shared<TcpSocket>();
    }
}

void HttpClient::setupSocketCallbacks(std::shared_ptr<RequestState> state)
{
    auto tcpSocket = std::dynamic_pointer_cast<TcpSocket>(state->socket);
    if (!tcpSocket) {
        return;
    }

    // Set up socket signal connections
    std::ignore = tcpSocket->bytesReceived.connect([this, state]() {
        onReadyRead(state);
    });

    std::ignore = tcpSocket->connected.connect([this, state]() {
        onSocketConnected(state);
    });

    std::ignore = tcpSocket->errorOccurred.connect([this, state](std::error_code ec) {
        onSocketError(state, ec);
    });
}

void HttpClient::setupParserCallbacks(std::shared_ptr<RequestState> state)
{
    // Header complete callback
    state->parser->setHeaderCompleteCallback([this, state](const std::string &firstLine,
                                                           const std::multimap<std::string, std::string> &headers) {
        state->headersParsed = true;

        // Parse status code and reason phrase from first line
        // Example: "HTTP/1.1 200 OK"
        std::istringstream iss(firstLine);
        std::string httpVersion, statusCodeStr, reasonPhrase;
        iss >> httpVersion >> statusCodeStr;

        // Extract reason phrase (rest of the line)
        std::getline(iss, reasonPhrase);
        if (!reasonPhrase.empty() && reasonPhrase[0] == ' ') {
            reasonPhrase = reasonPhrase.substr(1);
        }

        // Extract HTTP version (remove "HTTP/")
        if (httpVersion.size() > 5) {
            httpVersion = httpVersion.substr(5);
        }

        int statusCode = 0;
        try {
            statusCode = std::stoi(statusCodeStr);
        } catch (...) {
            failRequest(state, "Invalid status code");
            return;
        }

        // Update response
        state->response.setStatusCode(statusCode);
        state->response.setReasonPhrase(reasonPhrase);
        state->response.setHttpVersion(httpVersion);

        // Add headers
        for (const auto &[name, value] : headers) {
            state->response.addHeader(name, value);
        }

        // Get expected content length
        state->expectedContentLength = state->response.contentLength();

        // Check if this is a WebSocket upgrade
        if (state->response.statusCode() == 101 &&
            state->response.hasHeader("Upgrade") &&
            state->response.header("Upgrade").find("websocket") != std::string::npos) {
            // For WebSocket upgrades, we finish the request, but we don't close the socket
            // The socket ownership will be transferred to WebSocketClient via takeSocket()
            state->websocketUpgrade = true;
            state->response.setSocket(state->socket);

            // To prevent the finishRequest from reusing the socket, we'll remove it from our tracking
            auto it = m_activeRequests.find(state->socket);
            if (it != m_activeRequests.end()) {
                m_activeRequests.erase(it);
            }

            // Disable read/write notifications to prevent further processing
            if (state->socket) {
                // We don't want to close the socket, but we do want to stop listening to it
                // The socket callbacks will be set up by WebSocketClient after it takes ownership
                if (auto tcpSocket = std::dynamic_pointer_cast<TcpSocket>(state->socket)) {
                    tcpSocket->bytesReceived.disconnectAll();
                    tcpSocket->connected.disconnectAll();
                    tcpSocket->errorOccurred.disconnectAll();
                }
            }

            responseReceived.emit(state->response);

            // Set promise value
            try {
                state->responsePromise.set_value(state->response);
            } catch (const std::future_error &) {
                // Promise might already be set
            }

            // Stop the timer
            if (state->timeoutTimer) {
                state->timeoutTimer->running = false;
            }

            return;
        } else if (state->response.hasHeader("Content-Type") && // Check if this is an SSE stream by looking at the content type and set streaming mode flag
                   state->response.header("Content-Type").find("text/event-stream") != std::string::npos) {
            state->streamingMode = true;

            // For streaming responses with SSE client, call the client's callback immediately
            // with just the headers so it can emit the 'connected' signal
            if (state->sseClient && state->request.header("Accept").find("text/event-stream") != std::string::npos) {
                // Create a response with just the headers for the callback
                if (state->userCallback) {
                    state->userCallback(state->response);
                }
            }

            // For streaming responses, we want to emit the headers right away
            // so clients can start processing the content type and other headers
            responseReceived.emit(state->response);
        }
    });

    // Body data callback
    state->parser->setBodyDataCallback([this, state](const uint8_t *data, size_t length) {
        // Create a chunk from the incoming data
        KDUtils::ByteArray chunk(data, length);

        // Append body data to response
        KDUtils::ByteArray currentBody = state->response.body();
        currentBody.append(chunk);
        state->response.setBody(currentBody);

        // Update download progress
        downloadProgress.emit(state->request, currentBody.size(), state->expectedContentLength);

        // For SSE streaming mode, send only the new chunk to the associated SseClient
        if (state->streamingMode && state->headersParsed) {
            // If this request is from an SSE client, pass the chunk directly to it
            if (state->sseClient) {
                // Only send the new chunk, not the entire accumulated body
                state->sseClient->processDataChunk(chunk);
            } else {
                // For other streaming consumers, emit the response
                responseReceived.emit(state->response);
            }
        }
    });

    // Message complete callback
    state->parser->setMessageCompleteCallback([this, state]() {
        state->completed = true;

        if (state->websocketUpgrade) {
            // WebSocket upgrade completed, no need to finish the request
            return;
        }

        // Process cookies
        auto cookieHeaders = state->response.headers("Set-Cookie");
        if (!cookieHeaders.empty()) {
            std::vector<std::string> setCookieValues(cookieHeaders.begin(), cookieHeaders.end());
            m_session->cookieJar().parseCookies(state->request.url(), setCookieValues);
        }

        // For non-streaming responses, finish the request now
        else if (!state->streamingMode) {
            // Finish the request
            finishRequest(state);
        }
        // For streaming responses (like SSE), we don't finish the request
        // as the connection stays open
    });

    // Error callback
    state->parser->setErrorCallback([this, state](const std::string &error) {
        // Don't fail the request if we are performing a websocket upgrade
        if (state->websocketUpgrade && error == "Pause on CONNECT/Upgrade") {
            return;
        }
        failRequest(state, "HTTP parsing error: " + error);
    });
}

std::shared_ptr<SseClient> HttpClient::createSseClient()
{
    // Create a new SseClient instance using this HttpClient
    // Use the shared_from_this pattern to ensure HttpClient stays alive
    // as long as the SseClient needs it
    return std::shared_ptr<SseClient>(new SseClient(shared_from_this()));
}

} // namespace KDNetwork
