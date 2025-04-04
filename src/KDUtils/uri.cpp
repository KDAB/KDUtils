/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: GitHub Copilot <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "uri.h"
#include "dir.h"

#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>
#include <iomanip>
#include <stdexcept>

using namespace std::string_literals;

namespace KDUtils {

// Helper functions
namespace {

// Convert string to lowercase
std::string toLower(const std::string &str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

// Check if character is unreserved according to RFC 3986
bool isUnreserved(char c)
{
    return std::isalnum(static_cast<unsigned char>(c)) ||
            c == '-' || c == '.' || c == '_' || c == '~';
}

// Check if character is reserved according to RFC 3986
bool isReserved(char c)
{
    static const std::string reserved = ":/?#[]@!$&'()*+,;=";
    return reserved.find(c) != std::string::npos;
}

// Remove dot segments from a path according to RFC 3986
std::string removeDotSegments(const std::string &input)
{
    std::string output;

    // Process the input path segment by segment
    size_t pos = 0;
    while (pos < input.length()) {
        // A. If the input begins with "../", remove it
        if (input.substr(pos, 3) == "../") {
            pos += 3;
        }
        // B. If the input begins with "./", remove it
        else if (input.substr(pos, 2) == "./") {
            pos += 2;
        }
        // C. If the input begins with "/./", replace it with "/"
        else if (input.substr(pos, 3) == "/./") {
            pos += 2; // Skip "/."; keep the last "/" for next iteration
        }
        // D. If the input begins with "/..", remove it and the last segment from output
        else if (input.substr(pos, 4) == "/../") {
            size_t lastSlash = output.find_last_of('/');
            if (lastSlash != std::string::npos) {
                output.erase(lastSlash);
            } else {
                output.clear();
            }
            pos += 3; // Skip "/.."; keep the last "/" for next iteration
        }
        // E. If the input begins with ".." or ".", remove it
        else if (input.substr(pos, 2) == ".." && (pos + 2 == input.length() || input[pos + 2] == '/')) {
            pos += 2;
        } else if (input.substr(pos, 1) == "." && (pos + 1 == input.length() || input[pos + 1] == '/')) {
            pos += 1;
        }
        // F. Move the first segment from input to output
        else {
            size_t nextSlash = input.find('/', pos + 1);
            if (nextSlash == std::string::npos) {
                output += input.substr(pos);
                break; // No more segments
            } else {
                output += input.substr(pos, nextSlash - pos);
                pos = nextSlash; // Position at the next /
            }
        }
    }

    return output;
}

// Parse query string into key-value pairs
std::map<std::string, std::string> parseQueryString(const std::string &query)
{
    std::map<std::string, std::string> params;

    // Empty query string
    if (query.empty()) {
        return params;
    }

    std::string queryStr = query;
    // Remove leading '?' if present
    if (queryStr[0] == '?') {
        queryStr = queryStr.substr(1);
    }

    std::istringstream iss(queryStr);
    std::string pair;

    // Split by & or ;
    while (std::getline(iss, pair, '&')) {
        size_t equalsPos = pair.find('=');
        if (equalsPos == std::string::npos) {
            // No value, just a key
            params[Uri::decodeComponent(pair)] = "";
        } else {
            // Key=value pair
            std::string key = Uri::decodeComponent(pair.substr(0, equalsPos));
            std::string value = Uri::decodeComponent(pair.substr(equalsPos + 1));
            params[key] = value;
        }
    }

    return params;
}

} // anonymous namespace

// Uri implementation

Uri::Uri(const std::string &uriString)
{
    parse(uriString);
}

Uri::Uri(const std::string &scheme, const std::string &userInfo,
         const std::string &host, uint16_t port,
         const std::string &path, const std::string &query,
         const std::string &fragment)
    : m_scheme(scheme)
    , m_userInfo(userInfo)
    , m_host(host)
    , m_port(port)
    , m_hasExplicitPort(port != 0)
    , m_path(path)
    , m_query(query)
    , m_fragment(fragment)
{
}

void Uri::parse(const std::string &uriString)
{
    // RFC 3986 URI regex
    // Group 1: Scheme
    // Group 2: Authority (user:pass@host:port)
    // Group 3: Path
    // Group 4: Query
    // Group 5: Fragment
    const std::regex uriRegex(
            R"(^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
            std::regex::extended);

    std::smatch match;
    if (std::regex_match(uriString, match, uriRegex)) {
        // Scheme
        if (match[1].matched) {
            m_scheme = match[2].str();
        }

        // Authority
        if (match[3].matched) {
            std::string authority = match[4].str();

            // Parse authority: [userinfo@]host[:port]
            size_t atPos = authority.find('@');
            if (atPos != std::string::npos) {
                m_userInfo = authority.substr(0, atPos);
                authority = authority.substr(atPos + 1);
            }

            // Handle IPv6 address in square brackets
            if (!authority.empty() && authority[0] == '[') {
                size_t closeBracket = authority.find(']');
                if (closeBracket != std::string::npos) {
                    m_host = authority.substr(0, closeBracket + 1);
                    if (closeBracket + 2 < authority.length() && authority[closeBracket + 1] == ':') {
                        try {
                            m_port = static_cast<uint16_t>(std::stoi(authority.substr(closeBracket + 2)));
                            m_hasExplicitPort = true;
                        } catch (const std::exception &) {
                            // If port is invalid, ignore it
                        }
                    }
                } else {
                    m_host = authority; // Malformed IPv6, but store it as-is
                }
            } else {
                // Regular hostname[:port]
                size_t colonPos = authority.find(':');
                if (colonPos != std::string::npos) {
                    m_host = authority.substr(0, colonPos);
                    try {
                        m_port = static_cast<uint16_t>(std::stoi(authority.substr(colonPos + 1)));
                        m_hasExplicitPort = true;
                    } catch (const std::exception &) {
                        // If port is invalid, ignore it
                    }
                } else {
                    m_host = authority;
                }
            }
        }

        // Path
        m_path = match[5].str();

        // Query
        if (match[6].matched) {
            m_query = match[7].str();
        }

        // Fragment
        if (match[8].matched) {
            m_fragment = match[9].str();
        }
    } else {
        // If we couldn't parse it as a standard URI, treat it as a path
        m_path = uriString;
    }
}

Uri Uri::fromLocalFile(const std::string &path)
{
    std::string normalizedPath = Dir::fromNativeSeparators(path);
    Uri uri;

    // Check if it already has a scheme
    if (path.find("://") != std::string::npos) {
        uri = Uri(path);
        if (!uri.scheme().empty()) {
            return uri;
        }
    }

    // Add leading slash for absolute paths on Windows (C:/ becomes /C:/)
    const bool isWindowsPath = normalizedPath.size() > 1 &&
            normalizedPath[1] == ':' &&
            normalizedPath[0] != '/';
    if (isWindowsPath) {
        normalizedPath = "/" + normalizedPath;
    }

    // Add the appropriate number of slashes
    if (normalizedPath[0] == '/') {
        uri.m_scheme = "file";
        uri.m_path = normalizedPath;
    } else {
        uri.m_scheme = "file";
        uri.m_path = "/" + normalizedPath;
    }

    return uri;
}

Uri Uri::fromString(const std::string &uriString)
{
    return Uri(uriString);
}

Uri Uri::join(const Uri &base, const std::string &reference)
{
    Uri refUri(reference);
    return base.resolved(refUri);
}

std::string Uri::scheme() const
{
    return m_scheme;
}

std::string Uri::userInfo() const
{
    return m_userInfo;
}

std::string Uri::host() const
{
    return m_host;
}

uint16_t Uri::port() const
{
    // If no port specified but scheme is known, try to get default port
    if (m_port == 0 && !m_scheme.empty()) {
        const UriSchemeHandler *handler = UriSchemeRegistry::instance().handlerForScheme(m_scheme);
        if (handler) {
            try {
                return static_cast<uint16_t>(std::stoi(handler->defaultPort()));
            } catch (...) {
                return 0;
            }
        }
    }
    return m_port;
}

bool Uri::hasExplicitPort() const
{
    return m_hasExplicitPort;
}

std::string Uri::authority() const
{
    std::string result;

    if (!m_userInfo.empty()) {
        result += m_userInfo + "@";
    }

    result += m_host;

    if (m_hasExplicitPort && m_port != 0) {
        result += ":" + std::to_string(m_port);
    }

    return result;
}

std::string Uri::path() const
{
    return m_path;
}

std::string Uri::query() const
{
    return m_query;
}

std::string Uri::fragment() const
{
    return m_fragment;
}

std::string Uri::pathAndQuery() const
{
    if (m_query.empty()) {
        return m_path;
    } else {
        return m_path + "?" + m_query;
    }
}

Uri &Uri::withScheme(const std::string &scheme)
{
    m_scheme = scheme;
    return *this;
}

Uri &Uri::withUserInfo(const std::string &userInfo)
{
    m_userInfo = userInfo;
    return *this;
}

Uri &Uri::withHost(const std::string &host)
{
    m_host = host;
    return *this;
}

Uri &Uri::withPort(uint16_t port)
{
    m_port = port;
    m_hasExplicitPort = true;
    return *this;
}

Uri &Uri::withPath(const std::string &path)
{
    m_path = path;
    return *this;
}

Uri &Uri::withQuery(const std::string &query)
{
    m_query = query;
    return *this;
}

Uri &Uri::withQueryParameter(const std::string &key, const std::string &value)
{
    auto params = queryParameters();
    params[key] = value;

    m_query = buildQueryString(params);
    return *this;
}

std::string Uri::buildQueryString(const std::map<std::string, std::string> &params) const
{
    if (params.empty()) {
        return "";
    }

    std::ostringstream oss;
    bool first = true;

    for (const auto &pair : params) {
        if (!first) {
            oss << "&";
        }
        first = false;

        oss << encodeComponent(pair.first);
        if (!pair.second.empty()) {
            oss << "=" << encodeComponent(pair.second);
        }
    }

    return oss.str();
}

Uri &Uri::withFragment(const std::string &fragment)
{
    m_fragment = fragment;
    return *this;
}

std::map<std::string, std::string> Uri::queryParameters() const
{
    return parseQueryString(m_query);
}

std::string Uri::queryParameter(const std::string &key) const
{
    auto params = queryParameters();
    auto it = params.find(key);
    return it != params.end() ? it->second : "";
}

bool Uri::hasQueryParameter(const std::string &key) const
{
    auto params = queryParameters();
    return params.find(key) != params.end();
}

bool Uri::isEmpty() const
{
    return m_scheme.empty() && m_host.empty() && m_path.empty() &&
            m_query.empty() && m_fragment.empty();
}

bool Uri::isRelative() const
{
    return m_scheme.empty();
}

bool Uri::isAbsolute() const
{
    return !isRelative();
}

bool Uri::isLocalFile() const
{
    return toLower(m_scheme) == "file";
}

std::string Uri::toLocalFile() const
{
    if (!isLocalFile()) {
        return {};
    }

    std::string path = m_path;

    // Remove leading slash on Windows path
    if (path.size() > 2 && path[0] == '/' && path[2] == ':') {
        path = path.substr(1);
    }

    return path;
}

std::string Uri::toString() const
{
    std::ostringstream oss;

    if (!m_scheme.empty()) {
        oss << m_scheme << ":";
    }

    if (!m_host.empty() || !m_userInfo.empty() || (m_hasExplicitPort && m_port != 0)) {
        oss << "//";

        if (!m_userInfo.empty()) {
            oss << m_userInfo << "@";
        }

        oss << m_host;

        if (m_hasExplicitPort && m_port != 0) {
            oss << ":" << m_port;
        }
    } else if (!m_scheme.empty() && !m_path.empty() && m_path[0] != '/') {
        // Special case for non-hierarchical URIs like "mailto:user@example.com"
        // Don't add slashes for these
    }

    oss << m_path;

    if (!m_query.empty()) {
        oss << "?" << m_query;
    }

    if (!m_fragment.empty()) {
        oss << "#" << m_fragment;
    }

    return oss.str();
}

Uri Uri::normalized() const
{
    Uri result(*this);

    // Convert scheme to lowercase
    result.m_scheme = toLower(result.m_scheme);

    // Convert hostname to lowercase
    result.m_host = toLower(result.m_host);

    // Remove default port if it matches the scheme's default
    if (result.m_hasExplicitPort && !result.m_scheme.empty()) {
        const UriSchemeHandler *handler = UriSchemeRegistry::instance().handlerForScheme(result.m_scheme);
        if (handler) {
            try {
                uint16_t defaultPort = static_cast<uint16_t>(std::stoi(handler->defaultPort()));
                if (result.m_port == defaultPort) {
                    result.m_port = 0;
                    result.m_hasExplicitPort = false;
                }
            } catch (...) {
                // Ignore conversion errors
            }
        }
    }

    // Normalize path (remove dot segments)
    result.m_path = removeDotSegments(result.m_path);

    return result;
}

Uri Uri::resolved(const Uri &relative) const
{
    if (relative.isAbsolute()) {
        return relative.normalized();
    }

    Uri result(*this);

    // If relative URI has an authority component, then set result's components
    if (!relative.authority().empty()) {
        result.m_userInfo = relative.m_userInfo;
        result.m_host = relative.m_host;
        result.m_port = relative.m_port;
        result.m_hasExplicitPort = relative.m_hasExplicitPort;
        result.m_path = removeDotSegments(relative.m_path);
        result.m_query = relative.m_query;
    } else {
        // If relative URI has an empty path and no query, keep base URI's path and query
        if (relative.m_path.empty() && relative.m_query.empty()) {
            // Keep result's path and query
        } else if (relative.m_path.empty()) {
            result.m_query = relative.m_query;
        } else {
            if (relative.m_path[0] == '/') {
                result.m_path = removeDotSegments(relative.m_path);
            } else {
                // Merge paths
                size_t lastSlash = result.m_path.find_last_of('/');
                if (lastSlash == std::string::npos) {
                    result.m_path = relative.m_path;
                } else {
                    result.m_path = result.m_path.substr(0, lastSlash + 1) + relative.m_path;
                }
                result.m_path = removeDotSegments(result.m_path);
            }
            result.m_query = relative.m_query;
        }
    }

    result.m_fragment = relative.m_fragment;

    return result.normalized();
}

std::string Uri::encodeComponent(const std::string &component)
{
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (char c : component) {
        if (isUnreserved(c)) {
            escaped << c;
        } else {
            escaped << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
        }
    }

    return escaped.str();
}

std::string Uri::decodeComponent(const std::string &component)
{
    std::string result;
    result.reserve(component.length());

    for (size_t i = 0; i < component.length(); ++i) {
        if (component[i] == '%' && i + 2 < component.length()) {
            try {
                std::string hex = component.substr(i + 1, 2);
                char decoded = static_cast<char>(std::stoi(hex, nullptr, 16));
                result += decoded;
                i += 2;
            } catch (const std::exception &) {
                // If decoding fails, keep the % character
                result += '%';
            }
        } else if (component[i] == '+') {
            // In query strings, + often represents space
            result += ' ';
        } else {
            result += component[i];
        }
    }

    return result;
}

bool Uri::operator==(const Uri &other) const
{
    // Compare normalized forms to handle case differences, default ports, etc.
    return this->normalized().toString() == other.normalized().toString();
}

bool Uri::operator!=(const Uri &other) const
{
    return !(*this == other);
}

bool Uri::isValid() const
{
    // Empty URIs are considered invalid
    if (isEmpty()) {
        return false;
    }

    // If we have a scheme, try to use a registered scheme handler for validation
    if (!m_scheme.empty()) {
        const UriSchemeHandler *handler = UriSchemeRegistry::instance().handlerForScheme(m_scheme);
        if (handler) {
            return handler->validate(*this);
        }
    }

    // For schemes without handlers or relative URIs,
    // perform basic structural validation

    // If it has a scheme, it should be well-formed (letters followed by letters, digits, +, -, .)
    if (!m_scheme.empty()) {
        // First character must be a letter
        if (!std::isalpha(static_cast<unsigned char>(m_scheme[0]))) {
            return false;
        }

        // Other characters must be letters, digits, +, -, .
        for (size_t i = 1; i < m_scheme.length(); ++i) {
            char c = m_scheme[i];
            if (!std::isalnum(static_cast<unsigned char>(c)) &&
                c != '+' && c != '-' && c != '.') {
                return false;
            }
        }
    }

    // If it has a port, it should be a valid port number (0-65535)
    if (m_hasExplicitPort && (m_port > 65535)) {
        return false;
    }

    // Other basic checks can be added here as needed

    // If we got here, the URI passes basic validation
    return true;
}

// UriSchemeHandler implementations
std::string HttpUriHandler::defaultPort() const
{
    return "80";
}

bool HttpUriHandler::validate(const Uri &uri) const
{
    // HTTP URIs must have a host component
    return !uri.host().empty();
}

std::string HttpsUriHandler::defaultPort() const
{
    return "443";
}

bool HttpsUriHandler::validate(const Uri &uri) const
{
    // HTTPS URIs must have a host component
    return !uri.host().empty();
}

std::string FtpUriHandler::defaultPort() const
{
    return "21";
}

bool FtpUriHandler::validate(const Uri &uri) const
{
    // FTP URIs must have a host component
    return !uri.host().empty();
}

// UriSchemeRegistry implementation
UriSchemeRegistry::UriSchemeRegistry()
{
    // Register standard handlers
    registerSchemeHandler("http", std::make_unique<HttpUriHandler>());
    registerSchemeHandler("https", std::make_unique<HttpsUriHandler>());
    registerSchemeHandler("ftp", std::make_unique<FtpUriHandler>());
}

UriSchemeRegistry &UriSchemeRegistry::instance()
{
    static UriSchemeRegistry registry;
    return registry;
}

void UriSchemeRegistry::registerSchemeHandler(const std::string &scheme, std::unique_ptr<UriSchemeHandler> &&handler)
{
    m_handlers[toLower(scheme)] = std::move(handler);
}

const UriSchemeHandler *UriSchemeRegistry::handlerForScheme(const std::string &scheme) const
{
    auto it = m_handlers.find(toLower(scheme));
    return (it != m_handlers.end()) ? it->second.get() : nullptr;
}

} // namespace KDUtils
