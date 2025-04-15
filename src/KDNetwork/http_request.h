/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDNetwork/kdnetwork_export.h>
#include <KDUtils/uri.h>

#include <KDUtils/bytearray.h>

#include <chrono>
#include <map>
#include <string>

namespace KDNetwork {

/**
 * @brief HTTP method enumeration
 */
enum class HttpMethod {
    Get,
    Head,
    Post,
    Put,
    Delete,
    Options,
    Patch,
    Connect,
    Trace
};

/**
 * @brief Convert HttpMethod to std::string
 */
KDNETWORK_EXPORT std::string toStdString(HttpMethod method);

/**
 * @brief The HttpRequest class represents an HTTP request
 *
 * This class provides methods to configure an HTTP request, including the URL,
 * method, headers, and body.
 */
class KDNETWORK_EXPORT HttpRequest
{
public:
    /**
     * @brief Controls how redirect responses are handled
     */
    enum class RedirectPolicy {
        DontFollow, ///< Don't follow redirects
        FollowSameHost, ///< Follow redirects only if they go to the same host
        FollowSameHostAndProtocol, ///< Follow redirects only if they go to the same host and use the same protocol
        FollowAll ///< Follow all redirects
    };

    enum class AuthType {
        None,
        Basic,
        Bearer
    };

    /**
     * @brief Default constructor creates an empty request
     */
    HttpRequest();

    /**
     * @brief Construct a request with a URL object and method
     *
     * @param url The URL to request
     * @param method The HTTP method to use
     */
    HttpRequest(const KDUtils::Uri &url, HttpMethod method = HttpMethod::Get);

    /**
     * @brief Get the URL of the request
     */
    KDUtils::Uri url() const;

    /**
     * @brief Set the URL of the request
     *
     * @param url The URL to request
     */
    void setUrl(const KDUtils::Uri &url);

    /**
     * @brief Get the HTTP method of the request
     */
    HttpMethod method() const;

    /**
     * @brief Set the HTTP method of the request
     *
     * @param method The HTTP method to use
     */
    void setMethod(HttpMethod method);

    /**
     * @brief Checks for the presence of a header
     *
     * @param name The name of the header (case-insensitive)
     * @return true if the header exists, false otherwise
     */
    bool hasHeader(const std::string &name) const;

    /**
     * @brief Get a header value
     *
     * @param name The name of the header (case-insensitive)
     * @return The value of the header, or empty string if not found
     */
    std::string header(const std::string &name) const;

    /**
     * @brief Get all values for a header
     *
     * @param name The name of the header (case-insensitive)
     * @return A vector of header values, or empty vector if not found
     */
    std::vector<std::string> headers(const std::string &name) const;

    /**
     * @brief Set a header value
     *
     * @param name The name of the header
     * @param value The value of the header
     */
    void setHeader(const std::string &name, const std::string &value, bool overwrite = true);

    /**
     * @brief Add a header value
     *
     * @param name The name of the header
     * @param value The value of the header
     */
    void addHeader(const std::string &name, const std::string &value);

    /**
     * @brief Remove a header
     *
     * @param name The name of the header to remove (case-insensitive)
     */
    void removeHeader(const std::string &name);

    /**
     * @brief Get all headers
     *
     * @return A multimap of header names to values
     */
    std::multimap<std::string, std::string> allHeaders() const;

    /**
     * @brief Get the request body
     */
    const KDUtils::ByteArray &body() const;

    /**
     * @brief Set the request body
     *
     * @param body The request body
     */
    void setBody(const KDUtils::ByteArray &body);

    /**
     * @brief Set the request body from a string
     *
     * @param body The request body as a string
     */
    void setBody(const std::string &body);

    /**
     * @brief Get the request timeout
     */
    std::chrono::milliseconds timeout() const;

    /**
     * @brief Set the request timeout
     *
     * @param timeout The timeout in milliseconds
     */
    void setTimeout(std::chrono::milliseconds timeout);

    /**
     * @brief Get the redirect policy
     */
    RedirectPolicy redirectPolicy() const;

    /**
     * @brief Set the redirect policy
     *
     * @param policy The redirect policy to use
     */
    void setRedirectPolicy(RedirectPolicy policy);

    /**
     * @brief Get the maximum number of redirects to follow
     */
    int maxRedirects() const;

    /**
     * @brief Set the maximum number of redirects to follow
     *
     * @param count The maximum number of redirects
     */
    void setMaxRedirects(int count);

    /**
     * @brief Set basic authentication credentials
     *
     * @param username The username for authentication
     * @param password The password for authentication
     */
    void setBasicAuth(const std::string &username, const std::string &password);

    /**
     * @brief Set bearer authentication token
     *
     * @param token The bearer token for authentication
     */
    void setBearerAuth(const std::string &token);

    /**
     * @brief Get the authentication type
     *
     * @return The authentication type
     */
    AuthType authType() const;

    /**
     * @brief Get the authentication username
     *
     * @return The username for authentication
     */
    std::string authUsername() const;

    /**
     * @brief Get the authentication credential
     *
     * @return The credential for authentication
     */
    std::string authCredential() const;

    /**
     * @brief Check if common headers should be added automatically
     */
    bool autoAddCommonHeaders() const;

    /**
     * @brief Set whether common headers should be added automatically
     *
     * If true, headers like User-Agent, Accept, etc. will be added from the session
     * if not already present in the request.
     *
     * @param add Whether to add common headers automatically
     */
    void setAutoAddCommonHeaders(bool add);

private:
    KDUtils::Uri m_url;
    HttpMethod m_method = HttpMethod::Get;
    std::multimap<std::string, std::string> m_headers;
    KDUtils::ByteArray m_body;
    std::chrono::milliseconds m_timeout{ 30000 }; // 30 second default timeout
    RedirectPolicy m_redirectPolicy = RedirectPolicy::FollowAll;
    int m_maxRedirects = 5;
    AuthType m_authType = AuthType::None;
    std::string m_authUsername;
    std::string m_authCredential;
    std::string m_authHeader;
    bool m_autoAddCommonHeaders = true;
};

} // namespace KDNetwork
