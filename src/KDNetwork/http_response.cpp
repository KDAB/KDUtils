/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/http_response.h>

#include <algorithm>
#include <cctype>
#include <charconv>
#include <sstream>
#include "http_response.h"

namespace KDNetwork {

namespace {

// Convert a string to lowercase for case-insensitive header comparison
std::string toLower(const std::string &str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

// Helper function for case-insensitive header name comparison
std::string normalizeHeaderName(const std::string &name)
{
    return toLower(name);
}

} // namespace

HttpResponse::HttpResponse()
{
}

HttpResponse::HttpResponse(const HttpRequest &request)
    : m_request(request)
{
}

int HttpResponse::statusCode() const
{
    return m_statusCode;
}

void HttpResponse::setStatusCode(int code)
{
    m_statusCode = code;
}

std::string HttpResponse::reasonPhrase() const
{
    return m_reasonPhrase;
}

void HttpResponse::setReasonPhrase(const std::string &phrase)
{
    m_reasonPhrase = phrase;
}

std::string HttpResponse::httpVersion() const
{
    return m_httpVersion;
}

void HttpResponse::setHttpVersion(const std::string &version)
{
    m_httpVersion = version;
}

bool HttpResponse::hasHeader(const std::string &name) const
{
    const std::string normalized = normalizeHeaderName(name);
    return m_headers.find(normalized) != m_headers.end();
}

bool HttpResponse::isSuccessful() const
{
    return m_statusCode >= 200 && m_statusCode < 300;
}

bool HttpResponse::isRedirect() const
{
    return m_statusCode >= 300 && m_statusCode < 400 &&
            (m_statusCode == 301 || m_statusCode == 302 ||
             m_statusCode == 303 || m_statusCode == 307 || m_statusCode == 308);
}

bool HttpResponse::isClientError() const
{
    return m_statusCode >= 400 && m_statusCode < 500;
}

bool HttpResponse::isServerError() const
{
    return m_statusCode >= 500 && m_statusCode < 600;
}

std::string HttpResponse::header(const std::string &name) const
{
    const std::string normalized = normalizeHeaderName(name);
    auto range = m_headers.equal_range(normalized);
    if (range.first != range.second) {
        return range.first->second;
    }
    return {};
}

std::vector<std::string> HttpResponse::headers(const std::string &name) const
{
    std::vector<std::string> values;
    const std::string normalized = normalizeHeaderName(name);
    auto range = m_headers.equal_range(normalized);
    for (auto it = range.first; it != range.second; ++it) {
        values.push_back(it->second);
    }
    return values;
}

std::multimap<std::string, std::string> HttpResponse::allHeaders() const
{
    return m_headers;
}

void HttpResponse::setHeader(const std::string &name, const std::string &value, bool overwrite)
{
    const std::string normalized = normalizeHeaderName(name);
    if (overwrite) {
        removeHeader(normalized);
    }
    m_headers.emplace(normalized, value);
}

void HttpResponse::addHeader(const std::string &name, const std::string &value)
{
    const std::string normalized = normalizeHeaderName(name);
    m_headers.emplace(normalized, value);
}

void HttpResponse::removeHeader(const std::string &name)
{
    const std::string normalized = normalizeHeaderName(name);
    m_headers.erase(normalized);
}

const KDUtils::ByteArray &HttpResponse::body() const
{
    return m_body;
}

void HttpResponse::setBody(const KDUtils::ByteArray &body)
{
    m_body = body;
}

std::string HttpResponse::bodyAsString() const
{
    return std::string(reinterpret_cast<const char *>(m_body.data()), m_body.size());
}

const HttpRequest &HttpResponse::request() const
{
    return m_request;
}

std::optional<KDUtils::Uri> HttpResponse::redirectUrl() const
{
    if (!isRedirect()) {
        return std::nullopt;
    }

    std::string location = header("Location");
    if (location.empty()) {
        return std::nullopt;
    }

    // Check if location is absolute or relative
    if (location.find("://") != std::string::npos) {
        // Absolute URL
        return KDUtils::Uri(location);
    } else {
        // Relative URL, need to resolve against original request URL
        KDUtils::Uri baseUrl = m_request.url();
        // TODO: Properly resolve relative URLs
        // For now, this is a simple implementation that only handles absolute paths
        if (location.front() == '/') {
            std::string scheme = baseUrl.scheme();
            std::string host = baseUrl.toString();

            // Extract host from base URL
            size_t schemeEnd = host.find("://");
            if (schemeEnd != std::string::npos) {
                host = host.substr(schemeEnd + 3);
                size_t pathStart = host.find('/');
                if (pathStart != std::string::npos) {
                    host = host.substr(0, pathStart);
                }
            }

            return KDUtils::Uri(scheme + "://" + host + location);
        }

        // If not starting with /, we'd need more complex URL resolution
        // This is a placeholder for full implementation
        return std::nullopt;
    }
}

std::string HttpResponse::contentType() const
{
    return header("Content-Type");
}

int64_t HttpResponse::contentLength() const
{
    std::string contentLenStr = header("Content-Length");
    if (contentLenStr.empty()) {
        return -1;
    }

    // Parse content length
    int64_t length = -1;
    try {
        length = std::stoll(contentLenStr);
    } catch (...) {
        return -1;
    }

    return length;
}

bool HttpResponse::isChunked() const
{
    std::string transferEncoding = header("Transfer-Encoding");
    return !transferEncoding.empty() && transferEncoding.find("chunked") != std::string::npos;
}

bool HttpResponse::isKeepAlive() const
{
    // HTTP/1.0 defaults to Connection: close
    // HTTP/1.1 defaults to Connection: keep-alive
    std::string connection = toLower(header("Connection"));

    if (m_httpVersion == "1.0") {
        // For HTTP/1.0, need explicit keep-alive
        return !connection.empty() && (connection.find("keep-alive") != std::string::npos);
    } else {
        // For HTTP/1.1+, keep-alive is default unless explicitly closed
        return connection.empty() || (connection.find("close") == std::string::npos);
    }
}

std::chrono::milliseconds HttpResponse::elapsed() const
{
    return m_elapsed;
}

void HttpResponse::setElapsed(std::chrono::milliseconds elapsed)
{
    m_elapsed = elapsed;
}

int HttpResponse::redirectCount() const
{
    return m_redirectCount;
}

void HttpResponse::setRedirectCount(int count)
{
    m_redirectCount = count;
}

bool HttpResponse::isError() const
{
    return m_isError;
}

std::string HttpResponse::error() const
{
    return m_errorString;
}

void HttpResponse::setError(const std::string &error)
{
    m_isError = true;
    m_errorString = error;
}

std::shared_ptr<Socket> HttpResponse::takeSocket() const
{
    auto socket = m_socket;
    m_socket.reset();
    return socket;
}

void HttpResponse::setSocket(std::shared_ptr<Socket> socket)
{
    m_socket = socket;
}

// Setting and retrieving excess data (data beyond HTTP response) for WebSocket upgrades
void HttpResponse::setExcessData(const KDUtils::ByteArray &data)
{
    if (!data.isEmpty()) {
        m_excessData = data;
    }
}

KDUtils::ByteArray HttpResponse::takeExcessData() const
{
    KDUtils::ByteArray data = std::move(m_excessData);
    m_excessData.clear();
    return data;
}

} // namespace KDNetwork
