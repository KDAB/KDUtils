/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: GitHub Copilot <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDUtils/kdutils_global.h>
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <cstdint>

namespace KDUtils {

// Forward declaration
class Uri;

/**
 * @brief Handler for specific URI schemes like HTTP, FTP, etc.
 *
 * This abstract class provides scheme-specific functionality like validation
 * and default port information.
 */
class KDUTILS_API UriSchemeHandler
{
public:
    virtual ~UriSchemeHandler() = default;

    /**
     * @brief Returns the default port for this scheme
     */
    virtual std::string defaultPort() const = 0;

    /**
     * @brief Validates if the given URI conforms to scheme-specific rules
     * @param uri The URI to validate
     * @return true if the URI is valid for this scheme, false otherwise
     */
    virtual bool validate(const Uri &uri) const = 0;
};

/**
 * @brief A URI (Uniform Resource Identifier) class with full RFC 3986 support
 *
 * This class provides comprehensive handling of URIs, including parsing,
 * construction, normalization, and manipulation of all URI components:
 * scheme, authority (user info, host, port), path, query, and fragment.
 */
class KDUTILS_API Uri
{
public:
    /**
     * @brief Constructs an empty URI
     */
    Uri() = default;

    /**
     * @brief Constructs a URI from a string representation
     * @param uriString String representation of the URI
     */
    explicit Uri(const std::string &uriString);

    /**
     * @brief Constructs a URI from its components
     * @param scheme The scheme component
     * @param userInfo The user information component
     * @param host The host component
     * @param port The port number
     * @param path The path component
     * @param query The query component
     * @param fragment The fragment component
     */
    Uri(const std::string &scheme, const std::string &userInfo,
        const std::string &host, uint16_t port,
        const std::string &path, const std::string &query,
        const std::string &fragment);

    /**
     * @brief Constructs a URI from a local file path
     * @param path The file path to convert to a URI
     * @return A URI with the file scheme
     */
    static Uri fromLocalFile(const std::string &path);

    /**
     * @brief Parses a string into a URI
     * @param uriString String representation of the URI
     * @return The parsed URI
     */
    static Uri fromString(const std::string &uriString);

    /**
     * @brief Joins a base URI with a reference URI or path
     * @param base The base URI
     * @param reference The reference URI or path to join with the base
     * @return The resulting URI after joining
     */
    static Uri join(const Uri &base, const std::string &reference);

    /**
     * @brief Returns the scheme component of the URI
     * @return The scheme component
     */
    std::string scheme() const;

    /**
     * @brief Returns the user info component of the URI
     * @return The user info component
     */
    std::string userInfo() const;

    /**
     * @brief Returns the host component of the URI
     * @return The host component
     */
    std::string host() const;

    /**
     * @brief Returns the port component of the URI
     * @return The port number, or 0 if not specified
     */
    uint16_t port() const;

    /**
     * @brief Checks if the URI has an explicitly specified port
     * @return true if the URI has an explicit port, false otherwise
     */
    bool hasExplicitPort() const;

    /**
     * @brief Returns the authority component of the URI
     * @return The authority component (user info, host, and port)
     */
    std::string authority() const;

    /**
     * @brief Returns the path component of the URI
     * @return The path component
     */
    std::string path() const;

    /**
     * @brief Returns the query component of the URI
     * @return The query component
     */
    std::string query() const;

    /**
     * @brief Returns the fragment component of the URI
     * @return The fragment component
     */
    std::string fragment() const;

    /**
     * @brief Returns the path and query components combined
     * @return The path and query components
     */
    std::string pathAndQuery() const;

    /**
     * @brief Sets the scheme component of the URI
     * @param scheme The new scheme
     * @return Reference to this Uri for method chaining
     */
    Uri &withScheme(const std::string &scheme);

    /**
     * @brief Sets the user info component of the URI
     * @param userInfo The new user info
     * @return Reference to this Uri for method chaining
     */
    Uri &withUserInfo(const std::string &userInfo);

    /**
     * @brief Sets the host component of the URI
     * @param host The new host
     * @return Reference to this Uri for method chaining
     */
    Uri &withHost(const std::string &host);

    /**
     * @brief Sets the port component of the URI
     * @param port The new port number
     * @return Reference to this Uri for method chaining
     */
    Uri &withPort(uint16_t port);

    /**
     * @brief Sets the path component of the URI
     * @param path The new path
     * @return Reference to this Uri for method chaining
     */
    Uri &withPath(const std::string &path);

    /**
     * @brief Sets the query component of the URI
     * @param query The new query
     * @return Reference to this Uri for method chaining
     */
    Uri &withQuery(const std::string &query);

    /**
     * @brief Adds or updates a query parameter
     * @param key The parameter key
     * @param value The parameter value
     * @return Reference to this Uri for method chaining
     */
    Uri &withQueryParameter(const std::string &key, const std::string &value);

    /**
     * @brief Sets the fragment component of the URI
     * @param fragment The new fragment
     * @return Reference to this Uri for method chaining
     */
    Uri &withFragment(const std::string &fragment);

    /**
     * @brief Returns all query parameters as a key-value map
     * @return Map of query parameters
     */
    std::map<std::string, std::string> queryParameters() const;

    /**
     * @brief Returns the value of a specific query parameter
     * @param key The key of the parameter to retrieve
     * @return The value of the parameter, or empty string if not found
     */
    std::string queryParameter(const std::string &key) const;

    /**
     * @brief Checks if a query parameter exists
     * @param key The key of the parameter to check
     * @return true if the parameter exists, false otherwise
     */
    bool hasQueryParameter(const std::string &key) const;

    /**
     * @brief Checks if the URI is empty
     * @return true if the URI is empty, false otherwise
     */
    bool isEmpty() const;

    /**
     * @brief Checks if the URI is relative (has no scheme)
     * @return true if the URI is relative, false otherwise
     */
    bool isRelative() const;

    /**
     * @brief Checks if the URI is absolute (has a scheme)
     * @return true if the URI is absolute, false otherwise
     */
    bool isAbsolute() const;

    /**
     * @brief Checks if the URI is valid according to its scheme-specific rules
     *
     * If the URI has a registered scheme handler, this method calls that handler's
     * validate() function. If no handler is available for the scheme, basic
     * structural validation is performed instead.
     *
     * @return true if the URI is valid, false otherwise
     */
    bool isValid() const;

    /**
     * @brief Checks if the URI is a file URI
     * @return true if the URI has the "file" scheme, false otherwise
     */
    bool isLocalFile() const;

    /**
     * @brief Converts a file URI to a local path
     * @return The local file path, or empty string if not a file URI
     */
    std::string toLocalFile() const;

    /**
     * @brief Returns the complete string representation of the URI
     * @return The string representation of the URI
     */
    std::string toString() const;

    /**
     * @brief Returns a normalized form of the URI
     *
     * Normalization includes converting the scheme and host to lowercase,
     * removing dot segments from the path, and decoding/encoding as needed.
     *
     * @return The normalized URI
     */
    Uri normalized() const;

    /**
     * @brief Resolves a relative URI against this URI as the base
     * @param relative The relative URI to resolve
     * @return The resolved absolute URI
     */
    Uri resolved(const Uri &relative) const;

    /**
     * @brief Encodes a URI component according to RFC 3986
     * @param component The component to encode
     * @return The encoded component
     */
    static std::string encodeComponent(const std::string &component);

    /**
     * @brief Decodes a URI component according to RFC 3986
     * @param component The encoded component to decode
     * @return The decoded component
     */
    static std::string decodeComponent(const std::string &component);

    /**
     * @brief Equality operator
     * @param other The URI to compare with
     * @return true if the URIs are equal, false otherwise
     */
    bool operator==(const Uri &other) const;

    /**
     * @brief Inequality operator
     * @param other The URI to compare with
     * @return true if the URIs are not equal, false otherwise
     */
    bool operator!=(const Uri &other) const;

private:
    std::string m_scheme;
    std::string m_userInfo;
    std::string m_host;
    uint16_t m_port = 0;
    bool m_hasExplicitPort = false;
    std::string m_path;
    std::string m_query;
    std::string m_fragment;

    void parse(const std::string &uriString);
    std::string buildQueryString(const std::map<std::string, std::string> &params) const;
};

/**
 * @brief Handler for the HTTP scheme
 */
class KDUTILS_API HttpUriHandler : public UriSchemeHandler
{
public:
    std::string defaultPort() const override;
    bool validate(const Uri &uri) const override;
};

/**
 * @brief Handler for the HTTPS scheme
 */
class KDUTILS_API HttpsUriHandler : public UriSchemeHandler
{
public:
    std::string defaultPort() const override;
    bool validate(const Uri &uri) const override;
};

/**
 * @brief Handler for the FTP scheme
 */
class KDUTILS_API FtpUriHandler : public UriSchemeHandler
{
public:
    std::string defaultPort() const override;
    bool validate(const Uri &uri) const override;
};

/**
 * @brief Registry for scheme handlers
 *
 * Maintains a collection of handlers for various URI schemes.
 */
class KDUTILS_API UriSchemeRegistry
{
public:
    // Not copyable
    UriSchemeRegistry(const UriSchemeRegistry &) = delete;
    UriSchemeRegistry &operator=(const UriSchemeRegistry &) = delete;

    // Movable
    UriSchemeRegistry(UriSchemeRegistry &&) = default;
    UriSchemeRegistry &operator=(UriSchemeRegistry &&) = default;

    /**
     * @brief Returns the singleton instance of the registry
     * @return The registry instance
     */
    static UriSchemeRegistry &instance();

    /**
     * @brief Registers a scheme handler
     * @param scheme The scheme name
     * @param handler The handler to register
     */
    void registerSchemeHandler(const std::string &scheme, std::unique_ptr<UriSchemeHandler> &&handler);

    /**
     * @brief Gets the handler for a specific scheme
     * @param scheme The scheme name
     * @return The handler, or nullptr if not found
     */
    const UriSchemeHandler *handlerForScheme(const std::string &scheme) const;

private:
    UriSchemeRegistry(); // Initialize with standard handlers
    std::map<std::string, std::unique_ptr<UriSchemeHandler>> m_handlers;
};

} // namespace KDUtils
