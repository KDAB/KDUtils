/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDNetwork/kdnetwork_export.h>
#include <KDNetwork/http_request.h>

#include <KDUtils/uri.h>
#include <KDUtils/bytearray.h>

#include <chrono>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace KDNetwork {

class HttpRequest;

/**
 * @brief The HttpResponse class represents an HTTP response
 *
 * This class provides access to the response data, including status code,
 * headers, and body, for an HTTP request.
 */
class KDNETWORK_EXPORT HttpResponse
{
public:
    /**
     * @brief Default constructor creates an empty response
     */
    HttpResponse();

    /**
     * @brief Construct a response for a request
     *
     * @param request The request that generated this response
     */
    explicit HttpResponse(const HttpRequest &request);

    /**
     * @brief Get the request that generated this response
     */
    const HttpRequest &request() const;

    /**
     * @brief Get the HTTP status code
     */
    int statusCode() const;

    /**
     * @brief Set the HTTP status code
     *
     * @param code The status code
     */
    void setStatusCode(int code);

    /**
     * @brief Get the reason phrase (e.g., "OK" for 200)
     */
    std::string reasonPhrase() const;

    /**
     * @brief Set the reason phrase
     *
     * @param reason The reason phrase
     */
    void setReasonPhrase(const std::string &reason);

    /**
     * @brief Get the HTTP version (e.g., "1.1")
     */
    std::string httpVersion() const;

    /**
     * @brief Set the HTTP version
     *
     * @param version The HTTP version
     */
    void setHttpVersion(const std::string &version);

    /**
     * @brief Get a header value
     *
     * @param name The name of the header (case-insensitive)
     * @return The value of the header, or empty string if not found
     */
    std::string header(const std::string &name) const;

    /**
     * @brief Get all values of a header
     *
     * Some headers can appear multiple times in a response.
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
     * @brief Get the response body
     */
    const KDUtils::ByteArray &body() const;

    /**
     * @brief Set the response body
     *
     * @param body The response body
     */
    void setBody(const KDUtils::ByteArray &body);

    /**
     * @brief Get the response body as a string
     */
    std::string bodyAsString() const;

    /**
     * @brief Get the content type
     */
    std::string contentType() const;

    /**
     * @brief Get the content length
     *
     * @return The content length, or -1 if not specified
     */
    int64_t contentLength() const;

    /**
     * @brief Check if the response is successful (2xx)
     *
     * @return True if the response indicates success
     */
    bool isSuccessful() const;

    /**
     * @brief Check if the response is a redirect
     */
    bool isRedirect() const;

    /**
     * @brief Get the redirect URL from a Location header
     */
    std::optional<KDUtils::Uri> redirectUrl() const;

    /**
     * @brief Check if the response uses chunked transfer encoding
     */
    bool isChunked() const;

    /**
     * @brief Check if the response indicates a server error (5xx)
     */
    bool isServerError() const;

    /**
     * @brief Check if the response indicates a client error (4xx)
     */
    bool isClientError() const;

    /**
     * @brief Check if the response indicates success (2xx)
     */
    bool isSuccess() const;

    /**
     * @brief Check if the response is an error
     */
    bool isError() const;

    /**
     * @brief Get the error message, if any
     */
    std::string error() const;

    /**
     * @brief Set an error message
     *
     * @param error The error message
     */
    void setError(const std::string &error);

    /**
     * @brief Check if the response indicates that the connection should be kept alive
     */
    bool isKeepAlive() const;

    /**
     * @brief Get the elapsed time for the request/response
     */
    std::chrono::milliseconds elapsed() const;

    /**
     * @brief Set the elapsed time
     *
     * @param elapsed The elapsed time
     */
    void setElapsed(std::chrono::milliseconds elapsed);

    /**
     * @brief Get the number of redirects followed
     */
    int redirectCount() const;

    /**
     * @brief Set the number of redirects followed
     *
     * @param count The number of redirects
     */
    void setRedirectCount(int count);

private:
    HttpRequest m_request;
    int m_statusCode = 0;
    std::string m_reasonPhrase;
    std::string m_httpVersion;
    std::multimap<std::string, std::string> m_headers;
    KDUtils::ByteArray m_body;
    std::string m_error;
    std::chrono::milliseconds m_elapsed{ 0 };
    int m_redirectCount = 0;
    bool m_isError = false;
    std::string m_errorString;

    /**
     * @brief Set of status codes that indicate redirects
     */
    static const std::set<int> redirectCodes;
};

} // namespace KDNetwork
