/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDNetwork/kdnetwork_export.h>
#include <KDNetwork/http_cookie_jar.h>
#include <KDNetwork/http_request.h>

#include <memory>
#include <string>
#include <map>
#include <chrono>

namespace KDNetwork {

// Forward declarations
class TcpSocket;
class SslSocket;
class Socket;

/**
 * @brief The HttpSession class manages state across multiple HTTP requests
 *
 * This class handles:
 * - Cookie management
 * - Connection pooling (keep-alive)
 * - Default request headers
 * - Authentication credentials
 */
class KDNETWORK_EXPORT HttpSession
{
public:
    /**
     * @brief Default constructor
     */
    HttpSession();

    /**
     * @brief Destructor
     */
    ~HttpSession();

    /**
     * @brief Get the cookie jar
     */
    HttpCookieJar &cookieJar();

    /**
     * @brief Get the cookie jar (const version)
     */
    const HttpCookieJar &cookieJar() const;

    /**
     * @brief Set a new cookie jar
     *
     * @param jar The cookie jar to copy from
     */
    void setCookieJar(const HttpCookieJar &jar);

    /**
     * @brief Get a default header value
     */
    std::string defaultHeader(const std::string &name) const;

    /**
     * @brief Set a default header
     *
     * Default headers are applied to all requests if they don't already have
     * the header set.
     *
     * @param name The header name
     * @param value The header value
     */
    void setDefaultHeader(const std::string &name, const std::string &value);

    /**
     * @brief Remove a default header
     */
    void removeDefaultHeader(const std::string &name);

    /**
     * @brief Get all default headers
     */
    std::map<std::string, std::string> defaultHeaders() const;

    /**
     * @brief Apply default headers to a request
     *
     * Only applies headers that aren't already set in the request.
     *
     * @param request The request to apply headers to
     */
    void applyDefaultHeaders(HttpRequest &request) const;

    /**
     * @brief Set the user agent
     */
    void setUserAgent(const std::string &userAgent);

    /**
     * @brief Get the user agent
     */
    std::string userAgent() const;

    /**
     * @brief Set the connection timeout
     */
    void setConnectionTimeout(std::chrono::milliseconds timeout);

    /**
     * @brief Get the connection timeout
     */
    std::chrono::milliseconds connectionTimeout() const;

    /**
     * @brief Set the idle connection timeout (for keep-alive connections)
     */
    void setIdleConnectionTimeout(std::chrono::milliseconds timeout);

    /**
     * @brief Get the idle connection timeout
     */
    std::chrono::milliseconds idleConnectionTimeout() const;

    /**
     * @brief Set the maximum number of connections per host
     */
    void setMaxConnectionsPerHost(int count);

    /**
     * @brief Get the maximum number of connections per host
     */
    int maxConnectionsPerHost() const;

    /**
     * @brief Enable or disable automatic redirect following
     */
    void setFollowRedirects(bool follow);

    /**
     * @brief Check if automatic redirect following is enabled
     */
    bool followRedirects() const;

    /**
     * @brief Set the maximum number of redirects to follow
     */
    void setMaxRedirects(int count);

    /**
     * @brief Get the maximum number of redirects to follow
     */
    int maxRedirects() const;

    // These methods are intended to be used by HttpClient internally

    /**
     * @brief Get a pooled connection for the given host and port
     *
     * @param host The host to connect to
     * @param port The port to connect to
     * @param secure Whether to use SSL
     * @return A pooled connection, or nullptr if none available
     */
    std::shared_ptr<Socket> getConnection(const std::string &host, uint16_t port, bool secure);

    /**
     * @brief Return a connection to the pool
     *
     * @param host The host the connection is for
     * @param port The port the connection is for
     * @param secure Whether the connection uses SSL
     * @param socket The connection to return to the pool
     */
    void returnConnection(const std::string &host, uint16_t port, bool secure, std::shared_ptr<Socket> socket);

    /**
     * @brief Clean up idle connections
     */
    void cleanupConnections();

private:
    // Connection pool key (host, port, secure)
    struct ConnectionKey {
        std::string host;
        uint16_t port;
        bool secure;

        bool operator<(const ConnectionKey &other) const;
    };

    // Connection pool entry
    struct ConnectionEntry {
        std::shared_ptr<Socket> socket;
        std::chrono::steady_clock::time_point lastUsed;
    };

    HttpCookieJar m_cookieJar; // Cookie jar for managing cookies, now stored by value
    std::map<std::string, std::string> m_defaultHeaders;

    std::chrono::milliseconds m_connectionTimeout{ 30000 }; // 30 seconds
    std::chrono::milliseconds m_idleConnectionTimeout{ 60000 }; // 60 seconds
    int m_maxConnectionsPerHost{ 6 };
    bool m_followRedirects{ true };
    int m_maxRedirects{ 5 };

    // Connection pool
    std::map<ConnectionKey, std::vector<ConnectionEntry>> m_connectionPool;

    // Helper to normalize header names
    static std::string normalizeHeaderName(const std::string &name);
};

} // namespace KDNetwork
