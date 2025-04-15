/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/http_request.h>

#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>
#include "http_request.h"

namespace KDNetwork {

std::string toStdString(HttpMethod method)
{
    switch (method) {
    case HttpMethod::Get:
        return "GET";
    case HttpMethod::Head:
        return "HEAD";
    case HttpMethod::Post:
        return "POST";
    case HttpMethod::Put:
        return "PUT";
    case HttpMethod::Delete:
        return "DELETE";
    case HttpMethod::Options:
        return "OPTIONS";
    case HttpMethod::Patch:
        return "PATCH";
    case HttpMethod::Connect:
        return "CONNECT";
    case HttpMethod::Trace:
        return "TRACE";
    default:
        return "GET";
    }
}

namespace {
// Helper function for case-insensitive header name comparison
std::string normalizeHeaderName(const std::string &name)
{
    std::string result = name;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}
} // namespace

HttpRequest::HttpRequest()
{
}

HttpRequest::HttpRequest(const KDUtils::Uri &url, HttpMethod method)
    : m_url(url)
    , m_method(method)
{
}

KDUtils::Uri HttpRequest::url() const
{
    return m_url;
}

void HttpRequest::setUrl(const KDUtils::Uri &url)
{
    m_url = url;
}

HttpMethod HttpRequest::method() const
{
    return m_method;
}

void HttpRequest::setMethod(HttpMethod method)
{
    m_method = method;
}

bool HttpRequest::hasHeader(const std::string &name) const
{
    const std::string normalized = normalizeHeaderName(name);
    return m_headers.find(normalized) != m_headers.end();
}

std::string HttpRequest::header(const std::string &name) const
{
    const std::string normalized = normalizeHeaderName(name);
    auto range = m_headers.equal_range(normalized);
    if (range.first != range.second) {
        return range.first->second;
    }
    return {};
}

std::vector<std::string> HttpRequest::headers(const std::string &name) const
{
    std::vector<std::string> values;
    const std::string normalized = normalizeHeaderName(name);
    auto range = m_headers.equal_range(normalized);
    for (auto it = range.first; it != range.second; ++it) {
        values.push_back(it->second);
    }
    return values;
}

std::multimap<std::string, std::string> HttpRequest::allHeaders() const
{
    return m_headers;
}

void HttpRequest::setHeader(const std::string &name, const std::string &value, bool overwrite)
{
    const std::string normalized = normalizeHeaderName(name);
    if (overwrite) {
        removeHeader(normalized);
    }
    m_headers.emplace(normalized, value);
}

void HttpRequest::addHeader(const std::string &name, const std::string &value)
{
    const std::string normalized = normalizeHeaderName(name);
    m_headers.emplace(normalized, value);
}

void HttpRequest::removeHeader(const std::string &name)
{
    const std::string normalized = normalizeHeaderName(name);
    m_headers.erase(normalized);
}

const KDUtils::ByteArray &HttpRequest::body() const
{
    return m_body;
}

void HttpRequest::setBody(const KDUtils::ByteArray &body)
{
    m_body = body;
}

void HttpRequest::setBody(const std::string &body)
{
    m_body = KDUtils::ByteArray(reinterpret_cast<const uint8_t *>(body.data()), body.size());
}

std::chrono::milliseconds HttpRequest::timeout() const
{
    return m_timeout;
}

void HttpRequest::setTimeout(std::chrono::milliseconds msecs)
{
    m_timeout = msecs;
}

HttpRequest::RedirectPolicy HttpRequest::redirectPolicy() const
{
    return m_redirectPolicy;
}

void HttpRequest::setRedirectPolicy(RedirectPolicy policy)
{
    m_redirectPolicy = policy;
}

int HttpRequest::maxRedirects() const
{
    return m_maxRedirects;
}

void HttpRequest::setMaxRedirects(int count)
{
    m_maxRedirects = count;
}

void HttpRequest::setBasicAuth(const std::string &username, const std::string &password)
{
    m_authType = AuthType::Basic;
    m_authUsername = username;
    m_authCredential = password;

    // Automatically add the Authorization header
    std::string authString = username + ":" + password;
    std::string encoded;

    // Simple base64 encoding (RFC 4648)
    static const char base64Chars[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    // Encode in base64
    size_t authLen = authString.size();
    encoded.reserve(4 * ((authLen + 2) / 3)); // Reserve space

    for (size_t i = 0; i < authLen; i += 3) {
        // Collect 3 bytes into a 24-bit number
        uint32_t chunk = static_cast<uint8_t>(authString[i]) << 16;
        if (i + 1 < authLen) {
            chunk |= static_cast<uint8_t>(authString[i + 1]) << 8;
        }
        if (i + 2 < authLen) {
            chunk |= static_cast<uint8_t>(authString[i + 2]);
        }

        // Extract 4 base64 characters (6 bits each)
        for (int j = 0; j < 4; ++j) {
            if (i + j / 4 * 3 < authLen) {
                encoded += base64Chars[(chunk >> (18 - j * 6)) & 0x3F];
            } else {
                encoded += '='; // Padding
            }
        }
    }

    setHeader("Authorization", "Basic " + encoded);
}

void HttpRequest::setBearerAuth(const std::string &token)
{
    m_authType = AuthType::Bearer;
    m_authUsername.clear();
    m_authCredential = token;

    setHeader("Authorization", "Bearer " + token);
}

HttpRequest::AuthType HttpRequest::authType() const
{
    return m_authType;
}

std::string HttpRequest::authUsername() const
{
    return m_authUsername;
}

std::string HttpRequest::authCredential() const
{
    return m_authCredential;
}

void HttpRequest::setAutoAddCommonHeaders(bool enabled)
{
    m_autoAddCommonHeaders = enabled;
}

bool HttpRequest::autoAddCommonHeaders() const
{
    return m_autoAddCommonHeaders;
}

} // namespace KDNetwork
