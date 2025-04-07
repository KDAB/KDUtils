/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDNetwork/kdnetwork_export.h>
#include <KDUtils/url.h>

#include <chrono>
#include <optional>
#include <string>
#include <vector>

namespace KDUtils {
class Uri;
}

namespace KDNetwork {

/**
 * @brief The HttpCookie class represents an HTTP cookie
 *
 * This class handles parsing and formatting HTTP cookies according to RFC 6265.
 */
class KDNETWORK_EXPORT HttpCookie
{
public:
    /**
     * @brief Construct an empty cookie
     */
    HttpCookie();

    /**
     * @brief Construct a cookie with name and value
     */
    HttpCookie(const std::string &name, const std::string &value);

    /**
     * @brief Parse a cookie from a Set-Cookie header value
     *
     * @param setCookieValue The value of a Set-Cookie header
     * @param url The URL that sent the cookie
     * @return A parsed cookie, or std::nullopt if parsing failed
     */
    static std::optional<HttpCookie> fromSetCookieHeader(const std::string &setCookieValue, const KDUtils::Uri &url);

    /**
     * @brief Create a cookie header value (for Cookie headers)
     *
     * @return A string in the format "name=value"
     */
    std::string toCookieHeader() const;

    /**
     * @brief Get the name of the cookie
     */
    std::string name() const;

    /**
     * @brief Set the name of the cookie
     */
    void setName(const std::string &name);

    /**
     * @brief Get the value of the cookie
     */
    std::string value() const;

    /**
     * @brief Set the value of the cookie
     */
    void setValue(const std::string &value);

    /**
     * @brief Get the domain of the cookie
     */
    std::string domain() const;

    /**
     * @brief Set the domain of the cookie
     */
    void setDomain(const std::string &domain);

    /**
     * @brief Get the path of the cookie
     */
    std::string path() const;

    /**
     * @brief Set the path of the cookie
     */
    void setPath(const std::string &path);

    /**
     * @brief Get the expiration date of the cookie
     *
     * @return The expiration point in time, or std::nullopt for a session cookie
     */
    std::optional<std::chrono::system_clock::time_point> expirationDate() const;

    /**
     * @brief Set the expiration date of the cookie
     *
     * @param expiration The expiration point in time, or std::nullopt for a session cookie
     */
    void setExpirationDate(std::optional<std::chrono::system_clock::time_point> expiration);

    /**
     * @brief Check if the cookie is secure (only sent over HTTPS)
     */
    bool isSecure() const;

    /**
     * @brief Set whether the cookie is secure
     */
    void setSecure(bool secure);

    /**
     * @brief Check if the cookie is HTTP only (not accessible via JavaScript)
     */
    bool isHttpOnly() const;

    /**
     * @brief Set whether the cookie is HTTP only
     */
    void setHttpOnly(bool httpOnly);

    /**
     * @brief Get the SameSite policy of the cookie
     */
    enum class SameSitePolicy {
        None,
        Lax,
        Strict
    };

    /**
     * @brief Get the SameSite policy of the cookie
     */
    SameSitePolicy sameSite() const;

    /**
     * @brief Set the SameSite policy of the cookie
     */
    void setSameSite(SameSitePolicy policy);

    /**
     * @brief Check if the cookie is a session cookie (no expiration date)
     */
    bool isSessionCookie() const;

    /**
     * @brief Check if the cookie is expired
     */
    bool isExpired() const;

    /**
     * @brief Check if the cookie matches the given URL
     *
     * @param url The URL to check
     * @return True if the cookie should be sent with requests to this URL
     */
    bool matchesUrl(const KDUtils::Uri &url) const;

    /**
     * @brief Convert a SameSitePolicy to string
     */
    static std::string toString(SameSitePolicy policy);

    /**
     * @brief Parse a SameSitePolicy from string
     */
    static SameSitePolicy sameSitePolicyFromString(const std::string &policy);

private:
    std::string m_name;
    std::string m_value;
    std::string m_domain;
    std::string m_path{ "/" };
    std::optional<std::chrono::system_clock::time_point> m_expirationDate;
    bool m_secure{ false };
    bool m_httpOnly{ false };
    SameSitePolicy m_sameSite{ SameSitePolicy::None };

    // Helper methods for domain matching
    static bool isIpAddress(const std::string &domain);
    static bool domainMatch(const std::string &cookieDomain, const std::string &host);
    static bool pathMatch(const std::string &cookiePath, const std::string &requestPath);
};

} // namespace KDNetwork
