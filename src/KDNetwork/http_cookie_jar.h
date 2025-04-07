/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDNetwork/kdnetwork_export.h>
#include <KDNetwork/http_cookie.h>

#include <mutex>
#include <vector>
#include <string>
#include <optional>

namespace KDUtils {
class Uri;
}

namespace KDNetwork {

/**
 * @brief The HttpCookieJar class stores and manages HTTP cookies
 *
 * This class provides functionality for storing cookies, retrieving cookies
 * that match a specific URL, and handling cookie expiration.
 */
class KDNETWORK_EXPORT HttpCookieJar
{
public:
    /**
     * @brief Default constructor
     */
    HttpCookieJar();

    /**
     * @brief Copy constructor
     *
     * Creates a new cookie jar with the same cookies but a new mutex
     */
    HttpCookieJar(const HttpCookieJar &other);

    /**
     * @brief Copy assignment operator
     *
     * Copies cookies from another jar but keeps the existing mutex
     */
    HttpCookieJar &operator=(const HttpCookieJar &other);

    /**
     * @brief Get all cookies in the jar
     */
    std::vector<HttpCookie> allCookies() const;

    /**
     * @brief Get cookies that match a URL
     *
     * @param url The URL to match cookies against
     * @return A vector of cookies that should be sent with requests to the URL
     */
    std::vector<HttpCookie> cookiesForUrl(const KDUtils::Uri &url);

    /**
     * @brief Add a cookie to the jar
     *
     * @param cookie The cookie to add
     * @return True if the cookie was added successfully
     */
    bool insertCookie(const HttpCookie &cookie);

    /**
     * @brief Update a cookie in the jar or insert it if it doesn't exist
     *
     * @param cookie The cookie to update or insert
     * @return True if the cookie was updated or inserted successfully
     */
    bool updateCookie(const HttpCookie &cookie);

    /**
     * @brief Remove a cookie from the jar
     *
     * @param cookie The cookie to remove
     * @return True if the cookie was found and removed
     */
    bool removeCookie(const HttpCookie &cookie);

    /**
     * @brief Remove all cookies with the given name and domain
     *
     * @param name The name of the cookies to remove
     * @param domain The domain of the cookies to remove
     * @return The number of cookies removed
     */
    int removeCookies(const std::string &name, const std::string &domain);

    /**
     * @brief Remove all cookies in the jar
     */
    void clear();

    /**
     * @brief Remove all expired cookies from the jar
     *
     * @return The number of cookies removed
     */
    int removeExpiredCookies();

    /**
     * @brief Parse cookies from a response and add them to the jar
     *
     * @param url The URL that sent the cookies
     * @param setCookieHeaders A vector of Set-Cookie header values
     * @return The number of cookies successfully parsed and inserted
     */
    int parseCookies(const KDUtils::Uri &url, const std::vector<std::string> &setCookieHeaders);

    /**
     * @brief Create a Cookie header value for the given URL
     *
     * @param url The URL to create a Cookie header for
     * @return A string suitable for a Cookie header, or an empty string if no cookies match
     */
    std::string cookieHeaderForUrl(const KDUtils::Uri &url);

private:
    std::vector<HttpCookie> m_cookies;
    mutable std::mutex m_mutex;

    // Find a cookie with matching name, domain, and path
    std::optional<size_t> findCookie(const HttpCookie &cookie) const;

    // Helper method to remove expired cookies without locking the mutex
    // Returns the number of cookies removed
    int removeExpiredCookiesInternal();
};

} // namespace KDNetwork
