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

#include <KDUtils/bytearray.h>
#include <functional>
#include <memory>
#include <string>
#include <map>

// Forward declaration of llhttp types to avoid exposing the C library in our headers
struct llhttp__internal_s;
typedef struct llhttp__internal_s llhttp_t;

namespace KDNetwork {

/**
 * @brief The HttpParser class provides HTTP parsing functionality using llhttp
 *
 * This class wraps the llhttp C library to provide HTTP/1.1 request and response
 * parsing in a C++ friendly way that integrates with the KDNetwork API.
 */
class KDNETWORK_EXPORT HttpParser
{
public:
    enum class ParserError {
        NoError = 0, ///< No error
        Internal = 1,
        Strict = 2,
        CrExpected = 25,
        LfExpected = 3,
        UnexpectedContentLength = 4,
        UnexpectedSpace = 30,
        ClosedConnection = 5,
        InvalidMethod = 6,
        InvalidUrl = 7,
        InvalidConstant = 8,
        InvalidVersion = 9,
        InvalidHeaderToken = 10,
        InvalidContentLength = 11,
        InvalidChunkSize = 12,
        InvalidStatus = 13,
        InvalidEofState = 14,
        InvalidTransferEncoding = 15,
        CbMessageBegin = 16,
        CbHeadersComplete = 17,
        CbMessageComplete = 18,
        CbChunkHeader = 19,
        CbChunkComplete = 20,
        Paused = 21,
        PausedUpgrade = 22,
        PausedH2Upgrade = 23,
        User = 24,
        CbUrlComplete = 26,
        CbStatusComplete = 27,
        CbMethodComplete = 32,
        CbVersionComplete = 33,
        CbHeaderFieldComplete = 28,
        CbHeaderValueComplete = 29,
        CbChunkExtensionNameComplete = 34,
        CbChunkExtensionValueComplete = 35,
        CbReset = 31
    };

    /**
     * @brief Parser type enumeration
     */
    enum class Type {
        Request, ///< Parse HTTP requests (server-side)
        Response ///< Parse HTTP responses (client-side)
    };

    /**
     * @brief Callback for when a header is complete
     * @param firstLine The first line of the HTTP message (request or status line)
     * @param headers The parsed headers
     */
    using HeaderCompleteCallback = std::function<void(const std::string &firstLine, const std::multimap<std::string, std::string> &headers)>;

    /**
     * @brief Callback for when a body chunk is received
     * @param data Pointer to the body data
     * @param length Length of the body data
     */
    using BodyDataCallback = std::function<void(const uint8_t *data, size_t length)>;

    /**
     * @brief Callback for when the message is complete
     */
    using MessageCompleteCallback = std::function<void()>;

    /**
     * @brief Callback for when a parsing error occurs
     * @param error Error message
     */
    using ErrorCallback = std::function<void(const std::string &error)>;

    /**
     * @brief Constructor
     * @param type The type of parser to create (Request or Response)
     */
    explicit HttpParser(Type type);

    /**
     * @brief Destructor
     */
    ~HttpParser();

    /**
     * @brief Reset the parser state
     *
     * Call this before parsing a new message with the same parser instance.
     */
    void reset();

    /**
     * @brief Set the header complete callback
     */
    void setHeaderCompleteCallback(HeaderCompleteCallback callback);

    /**
     * @brief Set the body data callback
     */
    void setBodyDataCallback(BodyDataCallback callback);

    /**
     * @brief Set the message complete callback
     */
    void setMessageCompleteCallback(MessageCompleteCallback callback);

    /**
     * @brief Set the error callback
     */
    void setErrorCallback(ErrorCallback callback);

    /**
     * @brief Parse a chunk of HTTP data
     *
     * @param data Pointer to the data to parse
     * @param length Length of the data
     * @return True if parsing was successful, false if an error occurred
     */
    bool parse(const uint8_t *data, size_t length);

    /**
     * @brief Parse a chunk of HTTP data
     *
     * @param data ByteArray containing the data to parse
     * @return True if parsing was successful, false if an error occurred
     */
    bool parse(const KDUtils::ByteArray &data);

    /**
     * @brief Check if the parser is currently parsing headers
     */
    bool isParsingHeaders() const;

    /**
     * @brief Check if the parser is currently parsing the body
     */
    bool isParsingBody() const;

    /**
     * @brief Get the HTTP status code (for Response parsers only)
     *
     * @return The HTTP status code, or 0 if not available/applicable
     */
    int statusCode() const;

    /**
     * @brief Get the HTTP method (for Request parsers only)
     *
     * @return The HTTP method enum value, or HttpMethod::Invalid if not available/applicable
     */
    HttpMethod method() const;

    /**
     * @brief Get the URL (for Request parsers only)
     */
    std::string url() const;

    /**
     * @brief Get the HTTP version as a string (e.g., "1.1")
     */
    std::string httpVersion() const;

    /**
     * @brief Get the content length from headers
     *
     * @return The content length, or -1 if not specified
     */
    int64_t contentLength() const;

    /**
     * @brief Check if the response is using chunked transfer encoding
     */
    bool isChunked() const;

    /**
     * @brief Get the current parsed headers
     */
    const std::multimap<std::string, std::string> &headers() const;

    ParserError error() const;
    std::string errorString() const;
    size_t errorLocation() const;

private:
    // Private implementation details
    struct Private;
    std::unique_ptr<Private> d;

    // Static C callbacks that delegate to instance methods
    static int onMessageBegin(llhttp_t *parser);
    static int onUrl(llhttp_t *parser, const char *at, size_t length);
    static int onStatus(llhttp_t *parser, const char *at, size_t length);
    static int onHeaderField(llhttp_t *parser, const char *at, size_t length);
    static int onHeaderValue(llhttp_t *parser, const char *at, size_t length);
    static int onHeadersComplete(llhttp_t *parser);
    static int onBody(llhttp_t *parser, const char *at, size_t length);
    static int onMessageComplete(llhttp_t *parser);
    static int onChunkHeader(llhttp_t *parser);
    static int onChunkComplete(llhttp_t *parser);
};

} // namespace KDNetwork
