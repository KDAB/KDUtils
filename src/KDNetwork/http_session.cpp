/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/http_session.h>
#include <KDNetwork/socket.h>
#include <KDNetwork/tcp_socket.h>
#include <KDNetwork/ssl_socket.h>

#include <algorithm>
#include <chrono>

namespace KDNetwork {

bool HttpSession::ConnectionKey::operator<(const ConnectionKey &other) const
{
    if (host != other.host) {
        return host < other.host;
    }
    if (port != other.port) {
        return port < other.port;
    }
    return secure < other.secure;
}

HttpSession::HttpSession()
{
    // Set default User-Agent header
    setDefaultHeader("User-Agent", "KDNetwork HttpClient/1.0");
    setDefaultHeader("Accept", "*/*");
}

HttpSession::~HttpSession()
{
    // Close all connections
    m_connectionPool.clear();
}

HttpCookieJar &HttpSession::cookieJar()
{
    return m_cookieJar;
}

const HttpCookieJar &HttpSession::cookieJar() const
{
    return m_cookieJar;
}

void HttpSession::setCookieJar(const HttpCookieJar &jar)
{
    m_cookieJar = jar;
}

std::string HttpSession::defaultHeader(const std::string &name) const
{
    const std::string normalized = normalizeHeaderName(name);
    auto it = m_defaultHeaders.find(normalized);
    return (it != m_defaultHeaders.end()) ? it->second : std::string();
}

void HttpSession::setDefaultHeader(const std::string &name, const std::string &value)
{
    const std::string normalized = normalizeHeaderName(name);
    m_defaultHeaders[normalized] = value;
}

void HttpSession::removeDefaultHeader(const std::string &name)
{
    const std::string normalized = normalizeHeaderName(name);
    m_defaultHeaders.erase(normalized);
}

std::map<std::string, std::string> HttpSession::defaultHeaders() const
{
    return m_defaultHeaders;
}

void HttpSession::applyDefaultHeaders(HttpRequest &request) const
{
    for (const auto &[name, value] : m_defaultHeaders) {
        // Only apply default header if not set in the request
        if (request.header(name).empty()) {
            request.setHeader(name, value);
        }
    }
}

void HttpSession::setUserAgent(const std::string &userAgent)
{
    setDefaultHeader("User-Agent", userAgent);
}

std::string HttpSession::userAgent() const
{
    return defaultHeader("User-Agent");
}

void HttpSession::setConnectionTimeout(std::chrono::milliseconds timeout)
{
    m_connectionTimeout = timeout;
}

std::chrono::milliseconds HttpSession::connectionTimeout() const
{
    return m_connectionTimeout;
}

void HttpSession::setIdleConnectionTimeout(std::chrono::milliseconds timeout)
{
    m_idleConnectionTimeout = timeout;
}

std::chrono::milliseconds HttpSession::idleConnectionTimeout() const
{
    return m_idleConnectionTimeout;
}

void HttpSession::setMaxConnectionsPerHost(int count)
{
    m_maxConnectionsPerHost = count;
}

int HttpSession::maxConnectionsPerHost() const
{
    return m_maxConnectionsPerHost;
}

void HttpSession::setFollowRedirects(bool follow)
{
    m_followRedirects = follow;
}

bool HttpSession::followRedirects() const
{
    return m_followRedirects;
}

void HttpSession::setMaxRedirects(int count)
{
    m_maxRedirects = count;
}

int HttpSession::maxRedirects() const
{
    return m_maxRedirects;
}

std::shared_ptr<Socket> HttpSession::getConnection(const std::string &host, uint16_t port, bool secure)
{
    const ConnectionKey key{ host, port, secure };

    auto it = m_connectionPool.find(key);
    if (it == m_connectionPool.end() || it->second.empty()) {
        // No connections available for this host:port
        return nullptr;
    }

    // Get the most recent connection (most likely to still be valid)
    auto connection = std::move(it->second.back());
    it->second.pop_back();

    // If that was the last connection, remove the entry
    if (it->second.empty()) {
        m_connectionPool.erase(it);
    }

    // Check if the connection is still valid
    if (connection.socket->state() != Socket::State::Connected) {
        // Connection is not valid, don't return it
        return nullptr;
    }

    return connection.socket;
}

void HttpSession::returnConnection(const std::string &host, uint16_t port, bool secure, std::shared_ptr<Socket> socket)
{
    // Don't return invalid or disconnected sockets
    if (!socket || socket->state() != Socket::State::Connected) {
        return;
    }

    const ConnectionKey key{ host, port, secure };
    auto &connections = m_connectionPool[key];

    // Check if we already have enough connections for this host
    if (static_cast<int>(connections.size()) >= m_maxConnectionsPerHost) {
        // Too many connections, close this one
        socket->close();
        return;
    }

    // Add the connection to the pool
    ConnectionEntry entry;
    entry.socket = socket;
    entry.lastUsed = std::chrono::steady_clock::now();
    connections.push_back(std::move(entry));
}

void HttpSession::cleanupConnections()
{
    auto now = std::chrono::steady_clock::now();

    for (auto it = m_connectionPool.begin(); it != m_connectionPool.end();) {
        auto &connections = it->second;

        connections.erase(std::remove_if(connections.begin(), connections.end(),
                                         [&](const ConnectionEntry &entry) {
                                             // Remove if socket is invalid or connection timed out
                                             const bool invalid = !entry.socket ||
                                                     entry.socket->state() != Socket::State::Connected;

                                             const bool timedOut = (now - entry.lastUsed) > m_idleConnectionTimeout;

                                             if ((invalid || timedOut) && entry.socket) {
                                                 entry.socket->close();
                                             }

                                             return invalid || timedOut;
                                         }),
                          connections.end());

        if (connections.empty()) {
            it = m_connectionPool.erase(it);
        } else {
            ++it;
        }
    }
}

std::string HttpSession::normalizeHeaderName(const std::string &name)
{
    std::string result = name;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

} // namespace KDNetwork
