/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDNetwork/kdnetwork_export.h>
#include <KDNetwork/http_request.h>
#include <KDNetwork/http_response.h>
#include <KDNetwork/http_session.h>

#include <KDFoundation/timer.h>

#include <kdbindings/signal.h>

#include <chrono>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <queue>
#include <stdint.h>

namespace KDUtils {
class Uri;
}

namespace KDNetwork {

// Forward declarations
class Socket;
class TcpSocket;
class SslSocket;
class DnsResolver;
class SseClient;

/**
 * @brief The HttpClient class provides functionality for making HTTP requests
 *
 * This class handles HTTP/1.1 requests and responses, with support for:
 * - Asynchronous operation (signals + callbacks)
 * - Connection pooling (Keep-Alive)
 * - Cookie management
 * - Follow redirects
 * - Authentication
 * - Timeouts
 * - SSL/TLS
 * - Server-Sent Events (SSE)
 */
class KDNETWORK_EXPORT HttpClient : public std::enable_shared_from_this<HttpClient>
{
public:
    /**
     * @brief Default constructor with optional session
     *
     * @param session Optional session to use. If nullptr, a new session is created.
     */
    explicit HttpClient(std::shared_ptr<HttpSession> session = nullptr);

    /**
     * @brief Destructor
     */
    ~HttpClient();

    /**
     * @brief Send an HTTP request
     *
     * @param request The request to send
     * @param callback Optional callback function to call when the request completes
     * @return A future that will be set to the response when completed
     */
    std::future<HttpResponse> send(const HttpRequest &request,
                                   std::function<void(const HttpResponse &)> callback = nullptr);

    /**
     * @brief Convenience method to send a GET request
     *
     * @param url The URL to request
     * @param callback Optional callback function to call when the request completes
     * @return A future that will be set to the response when completed
     */
    std::future<HttpResponse> get(const KDUtils::Uri &url,
                                  std::function<void(const HttpResponse &)> callback = nullptr);

    /**
     * @brief Convenience method to send a HEAD request
     */
    std::future<HttpResponse> head(const KDUtils::Uri &url,
                                   std::function<void(const HttpResponse &)> callback = nullptr);

    /**
     * @brief Convenience method to send a POST request
     *
     * @param url The URL to request
     * @param data The data to send in the request body
     * @param contentType The content type of the data
     * @param callback Optional callback function to call when the request completes
     * @return A future that will be set to the response when completed
     */
    std::future<HttpResponse> post(const KDUtils::Uri &url,
                                   const KDUtils::ByteArray &data,
                                   const std::string &contentType = "application/x-www-form-urlencoded",
                                   std::function<void(const HttpResponse &)> callback = nullptr);

    /**
     * @brief Convenience method to send a POST request. Caller is responsible for setting the content type.
     *
     * @param url The URL to request
     * @param data The data to send in the request body
     * @param callback Optional callback function to call when the request completes
     * @return A future that will be set to the response when completed
     */
    std::future<HttpResponse> post(const KDUtils::Uri &url,
                                   const KDUtils::ByteArray &data,
                                   std::function<void(const HttpResponse &)> callback = nullptr);

    /**
     * @brief Convenience method to send a PUT request
     */
    std::future<HttpResponse> put(const KDUtils::Uri &url,
                                  const KDUtils::ByteArray &data,
                                  const std::string &contentType = "application/x-www-form-urlencoded",
                                  std::function<void(const HttpResponse &)> callback = nullptr);

    /**
     * @brief Convenience method to send a DELETE request
     */
    std::future<HttpResponse> deleteResource(const KDUtils::Uri &url,
                                             std::function<void(const HttpResponse &)> callback = nullptr);

    /**
     * @brief Convenience method to send a PATCH request
     */
    std::future<HttpResponse> patch(const KDUtils::Uri &url,
                                    const KDUtils::ByteArray &data,
                                    const std::string &contentType = "application/x-www-form-urlencoded",
                                    std::function<void(const HttpResponse &)> callback = nullptr);

    /**
     * @brief Convenience method to send a OPTIONS request
     */
    std::future<HttpResponse> options(const KDUtils::Uri &url,
                                      std::function<void(const HttpResponse &)> callback = nullptr);

    /**
     * @brief Cancel all pending requests
     */
    void cancelAll();

    /**
     * @brief Get the session
     */
    std::shared_ptr<HttpSession> session() const;

    /**
     * @brief Set a new session
     */
    void setSession(std::shared_ptr<HttpSession> session);

    /**
     * @brief Create a Server-Sent Events client
     *
     * Creates a new SseClient instance that uses this HttpClient for its connections.
     * The SSE client can be used to establish persistent connections to SSE endpoints.
     *
     * @return A new SseClient instance
     */
    std::shared_ptr<SseClient> createSseClient();

    /**
     * @brief Send an HTTP request with associated SSE client
     *
     * Internal method used by SseClient to associate itself with a request.
     * Not intended to be called directly by users.
     *
     * @param request The HTTP request to send
     * @param sseClient The SseClient that initiated this request
     * @param callback Optional callback function to call when the request completes
     * @return A future that will be set to the response when completed
     */
    std::future<HttpResponse> sendWithSseClient(const HttpRequest &request,
                                                std::shared_ptr<SseClient> sseClient,
                                                std::function<void(const HttpResponse &)> callback = nullptr);

    /**
     * @brief Signal emitted when a request is about to be sent
     */
    KDBindings::Signal<HttpRequest &> aboutToSendRequest;

    /**
     * @brief Signal emitted when a response is received
     */
    KDBindings::Signal<const HttpResponse &> responseReceived;

    /**
     * @brief Signal emitted when an error occurs
     */
    KDBindings::Signal<const HttpRequest &, const std::string &> error;

    /**
     * @brief Signal emitted when download progress is made
     */
    KDBindings::Signal<const HttpRequest &, std::int64_t, std::int64_t> downloadProgress;

    /**
     * @brief Signal emitted when upload progress is made
     */
    KDBindings::Signal<const HttpRequest &, std::int64_t, std::int64_t> uploadProgress;

private:
    // Internal request state
    class RequestState;
    std::shared_ptr<RequestState> createRequestState(const HttpRequest &request,
                                                     std::function<void(const HttpResponse &)> callback,
                                                     std::promise<HttpResponse> promise);

    void startRequest(std::shared_ptr<RequestState> state);
    void finishRequest(std::shared_ptr<RequestState> state);
    void failRequest(std::shared_ptr<RequestState> state, const std::string &errorString);
    void followRedirect(std::shared_ptr<RequestState> state);

    void setupSocketCallbacks(std::shared_ptr<RequestState> state);
    void onReadyRead(std::shared_ptr<RequestState> state);
    void onSocketConnected(std::shared_ptr<RequestState> state);
    void onSocketError(std::shared_ptr<RequestState> state, std::error_code ec);
    void onTimeout(std::shared_ptr<RequestState> state);

    void setupParserCallbacks(std::shared_ptr<RequestState> state);

    std::shared_ptr<KDNetwork::Socket> createSocket(bool secure, const std::string &host = {});

    std::shared_ptr<HttpSession> m_session;

    // Active requests
    std::map<std::shared_ptr<Socket>, std::shared_ptr<RequestState>> m_activeRequests;

    // Cleanup timer
    std::unique_ptr<KDFoundation::Timer> m_cleanupTimer;
};

} // namespace KDNetwork
