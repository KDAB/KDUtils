/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/http_parser.h>

#include <llhttp.h>

#include <algorithm>
#include <cctype>
#include <sstream>

namespace KDNetwork {

// Conversion from llhttp method enum to our HttpMethod enum
static HttpMethod convertLlhttpMethod(llhttp_method_t method)
{
    switch (method) {
    case HTTP_GET:
        return HttpMethod::Get;
    case HTTP_HEAD:
        return HttpMethod::Head;
    case HTTP_POST:
        return HttpMethod::Post;
    case HTTP_PUT:
        return HttpMethod::Put;
    case HTTP_DELETE:
        return HttpMethod::Delete;
    case HTTP_CONNECT:
        return HttpMethod::Connect;
    case HTTP_OPTIONS:
        return HttpMethod::Options;
    case HTTP_TRACE:
        return HttpMethod::Trace;
    case HTTP_PATCH:
        return HttpMethod::Patch;
    default:
        return HttpMethod::Get; // Default to GET for unknown methods
    }
}

// Convert a string to lowercase for case-insensitive header comparison
static std::string toLower(const std::string &str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

// Private implementation (PIMPL pattern)
struct HttpParser::Private {
    llhttp_t *parser = nullptr;
    llhttp_settings_t settings = {};

    Type parserType;
    bool parsingHeaders = false;
    bool parsingBody = false;

    std::string currentHeaderField;
    std::string currentHeaderValue;
    std::string url;
    std::string statusMessage;
    int statusCode = 0;
    std::string firstLine;

    std::multimap<std::string, std::string> headers;

    HeaderCompleteCallback headerCompleteCallback;
    BodyDataCallback bodyDataCallback;
    MessageCompleteCallback messageCompleteCallback;
    ErrorCallback errorCallback;

    Private(Type type)
        : parserType(type)
    {
        parser = new llhttp_t();

        // Initialize llhttp settings with callbacks
        llhttp_settings_init(&settings);
        settings.on_message_begin = HttpParser::onMessageBegin;
        settings.on_url = HttpParser::onUrl;
        settings.on_status = HttpParser::onStatus;
        settings.on_header_field = HttpParser::onHeaderField;
        settings.on_header_value = HttpParser::onHeaderValue;
        settings.on_headers_complete = HttpParser::onHeadersComplete;
        settings.on_body = HttpParser::onBody;
        settings.on_message_complete = HttpParser::onMessageComplete;
        settings.on_chunk_header = HttpParser::onChunkHeader;
        settings.on_chunk_complete = HttpParser::onChunkComplete;

        // Initialize the parser
        llhttp_type_t llhttpType = (type == Type::Request) ? HTTP_REQUEST : HTTP_RESPONSE;
        llhttp_init(parser, llhttpType, &settings);

        // Store the this pointer for callbacks
        parser->data = this;
    }

    ~Private()
    {
        if (parser) {
            delete parser;
            parser = nullptr;
        }
    }

    void reset()
    {
        llhttp_reset(parser);
        parsingHeaders = false;
        parsingBody = false;
        currentHeaderField.clear();
        currentHeaderValue.clear();
        url.clear();
        statusMessage.clear();
        statusCode = 0;
        firstLine.clear();
        headers.clear();
    }

    void finalizeCurrentHeader()
    {
        if (!currentHeaderField.empty() && !currentHeaderValue.empty()) {
            headers.emplace(toLower(currentHeaderField), currentHeaderValue);
            currentHeaderField.clear();
            currentHeaderValue.clear();
        }
    }
};

// Constructor
HttpParser::HttpParser(Type type)
    : d(std::make_unique<Private>(type))
{
}

// Destructor
HttpParser::~HttpParser() = default;

void HttpParser::reset()
{
    d->reset();
}

void HttpParser::setHeaderCompleteCallback(HeaderCompleteCallback callback)
{
    d->headerCompleteCallback = std::move(callback);
}

void HttpParser::setBodyDataCallback(BodyDataCallback callback)
{
    d->bodyDataCallback = std::move(callback);
}

void HttpParser::setMessageCompleteCallback(MessageCompleteCallback callback)
{
    d->messageCompleteCallback = std::move(callback);
}

void HttpParser::setErrorCallback(ErrorCallback callback)
{
    d->errorCallback = std::move(callback);
}

bool HttpParser::parse(const uint8_t *data, size_t length)
{
    enum llhttp_errno err = llhttp_execute(d->parser, reinterpret_cast<const char *>(data), length);
    if (err != HPE_OK) {
        std::string errorMessage = llhttp_get_error_reason(d->parser);
        if (d->errorCallback) {
            d->errorCallback(errorMessage);
        }
        return false;
    }
    return true;
}

bool HttpParser::parse(const KDUtils::ByteArray &data)
{
    return parse(data.data(), data.size());
}

bool HttpParser::isParsingHeaders() const
{
    return d->parsingHeaders;
}

bool HttpParser::isParsingBody() const
{
    return d->parsingBody;
}

int HttpParser::statusCode() const
{
    return d->statusCode;
}

HttpMethod HttpParser::method() const
{
    if (d->parserType == Type::Request) {
        return convertLlhttpMethod(static_cast<llhttp_method_t>(d->parser->method));
    }
    return HttpMethod::Get; // Default for non-request parsers
}

std::string HttpParser::url() const
{
    return d->url;
}

std::string HttpParser::httpVersion() const
{
    std::ostringstream oss;
    oss << d->parser->http_major << "." << d->parser->http_minor;
    return oss.str();
}

int64_t HttpParser::contentLength() const
{
    auto it = d->headers.find("content-length");
    if (it != d->headers.end()) {
        try {
            return std::stoull(it->second);
        } catch (...) {
            return -1;
        }
    }
    return -1;
}

bool HttpParser::isChunked() const
{
    auto it = d->headers.find("transfer-encoding");
    if (it != d->headers.end()) {
        return it->second.find("chunked") != std::string::npos;
    }
    return false;
}

const std::multimap<std::string, std::string> &HttpParser::headers() const
{
    return d->headers;
}

// Static C callbacks that delegate to instance methods
int HttpParser::onMessageBegin(llhttp_t *parser)
{
    auto *p = static_cast<Private *>(parser->data);
    p->parsingHeaders = true;
    p->parsingBody = false;
    return 0;
}

int HttpParser::onUrl(llhttp_t *parser, const char *at, size_t length)
{
    auto *p = static_cast<Private *>(parser->data);
    p->url.append(at, length);
    return 0;
}

int HttpParser::onStatus(llhttp_t *parser, const char *at, size_t length)
{
    auto *p = static_cast<Private *>(parser->data);
    p->statusMessage.append(at, length);
    return 0;
}

int HttpParser::onHeaderField(llhttp_t *parser, const char *at, size_t length)
{
    auto *p = static_cast<Private *>(parser->data);

    // If we were previously processing a value, this is a new header, so finalize the previous one
    if (!p->currentHeaderValue.empty()) {
        p->finalizeCurrentHeader();
    }

    p->currentHeaderField.append(at, length);
    return 0;
}

int HttpParser::onHeaderValue(llhttp_t *parser, const char *at, size_t length)
{
    auto *p = static_cast<Private *>(parser->data);
    p->currentHeaderValue.append(at, length);
    return 0;
}

int HttpParser::onHeadersComplete(llhttp_t *parser)
{
    auto *p = static_cast<Private *>(parser->data);

    // Finalize any pending header
    p->finalizeCurrentHeader();

    p->parsingHeaders = false;
    p->statusCode = parser->status_code;

    // Create the first line based on parser type
    if (p->parserType == Type::Response) {
        std::ostringstream oss;
        oss << "HTTP/" << parser->http_major << "." << parser->http_minor
            << " " << parser->status_code << " " << p->statusMessage;
        p->firstLine = oss.str();
    } else {
        std::ostringstream oss;
        oss << llhttp_method_name(static_cast<llhttp_method_t>(parser->method))
            << " " << p->url << " HTTP/" << parser->http_major << "." << parser->http_minor;
        p->firstLine = oss.str();
    }

    // Call the callback if set
    if (p->headerCompleteCallback) {
        p->headerCompleteCallback(p->firstLine, p->headers);
    }

    return 0;
}

int HttpParser::onBody(llhttp_t *parser, const char *at, size_t length)
{
    auto *p = static_cast<Private *>(parser->data);
    p->parsingBody = true;

    if (p->bodyDataCallback) {
        p->bodyDataCallback(reinterpret_cast<const uint8_t *>(at), length);
    }

    return 0;
}

int HttpParser::onMessageComplete(llhttp_t *parser)
{
    auto *p = static_cast<Private *>(parser->data);
    p->parsingBody = false;

    if (p->messageCompleteCallback) {
        p->messageCompleteCallback();
    }

    return 0;
}

int HttpParser::onChunkHeader(llhttp_t *parser)
{
    // No specific action needed for chunk header in this implementation
    return 0;
}

int HttpParser::onChunkComplete(llhttp_t *parser)
{
    // No specific action needed for chunk complete in this implementation
    return 0;
}

} // namespace KDNetwork
