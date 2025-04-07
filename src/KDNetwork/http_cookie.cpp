/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/http_cookie.h>
#include <KDNetwork/ip_address.h>

#include <KDUtils/uri.h>

#include <algorithm>
#include <cctype>
#include <charconv>
#include <chrono>
#include <regex>
#include <sstream>
#include <unordered_map>

namespace KDNetwork {

HttpCookie::HttpCookie()
{
}

HttpCookie::HttpCookie(const std::string &name, const std::string &value)
    : m_name(name)
    , m_value(value)
{
}

// Helper function to trim whitespace
static std::string trim(const std::string &str)
{
    const auto start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    const auto end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

// Helper function to parse cookie date format (RFC 6265)
static std::optional<std::chrono::system_clock::time_point> parseDate(const std::string &dateStr)
{
    // Implementation of date parsing for cookie expiration
    // This is a simplified implementation; a full implementation would handle
    // all the date formats specified in RFC 6265 section 5.1.1

    // Example format: "Wed, 21 Oct 2015 07:28:00 GMT"
    std::regex dateRegex(R"((\d{1,2})\s+([a-zA-Z]+)\s+(\d{4})\s+(\d{2}):(\d{2}):(\d{2})\s+GMT)");
    std::smatch match;

    if (std::regex_search(dateStr, match, dateRegex) && match.size() == 7) {
        int day = std::stoi(match[1].str());
        std::string month = match[2].str();
        int year = std::stoi(match[3].str());
        int hour = std::stoi(match[4].str());
        int minute = std::stoi(match[5].str());
        int second = std::stoi(match[6].str());

        // Convert month name to number
        std::unordered_map<std::string, int> monthMap = {
            { "Jan", 0 }, { "Feb", 1 }, { "Mar", 2 }, { "Apr", 3 }, { "May", 4 }, { "Jun", 5 }, { "Jul", 6 }, { "Aug", 7 }, { "Sep", 8 }, { "Oct", 9 }, { "Nov", 10 }, { "Dec", 11 }
        };

        int monthNum = -1;
        for (const auto &[name, num] : monthMap) {
            if (month.find(name) != std::string::npos) {
                monthNum = num;
                break;
            }
        }

        if (monthNum >= 0) {
            std::tm tm = {};
            tm.tm_year = year - 1900;
            tm.tm_mon = monthNum;
            tm.tm_mday = day;
            tm.tm_hour = hour;
            tm.tm_min = minute;
            tm.tm_sec = second;

            // Convert to time_t, then to system_clock::time_point
            auto time = std::mktime(&tm);
            return std::chrono::system_clock::from_time_t(time);
        }
    }

    // Secondary attempt with "Expires=<seconds-since-epoch>"
    try {
        auto seconds = std::stoll(dateStr);
        return std::chrono::system_clock::from_time_t(seconds);
    } catch (...) {
        // Not a valid number
    }

    // Failed to parse
    return std::nullopt;
}

std::optional<HttpCookie> HttpCookie::fromSetCookieHeader(const std::string &setCookieValue, const KDUtils::Uri &url)
{
    std::istringstream stream(setCookieValue);
    std::string token;
    std::getline(stream, token, ';');

    // Extract name/value pair
    std::string name, value;
    auto equalPos = token.find('=');
    if (equalPos != std::string::npos) {
        name = trim(token.substr(0, equalPos));
        value = trim(token.substr(equalPos + 1));
    } else {
        // RFC 6265 requires a name-value pair separated by '='
        // If there's no equals sign, it's an invalid cookie
        return std::nullopt;
    }

    // Cookie name cannot be empty according to RFC 6265
    if (name.empty()) {
        return std::nullopt;
    }

    HttpCookie cookie(name, value);

    // Parse attributes
    while (std::getline(stream, token, ';')) {
        auto equalPos = token.find('=');
        std::string attrName, attrValue;

        if (equalPos != std::string::npos) {
            attrName = trim(token.substr(0, equalPos));
            attrValue = trim(token.substr(equalPos + 1));
        } else {
            attrName = trim(token);
            attrValue = "";
        }

        // Convert attribute name to lowercase for case-insensitive comparison
        std::transform(attrName.begin(), attrName.end(), attrName.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        if (attrName == "expires") {
            auto expiry = parseDate(attrValue);
            if (expiry) {
                cookie.m_expirationDate = expiry;
            }
        } else if (attrName == "max-age") {
            try {
                int seconds = std::stoi(attrValue);
                auto now = std::chrono::system_clock::now();
                cookie.m_expirationDate = now + std::chrono::seconds(seconds);
            } catch (...) {
                // Invalid max-age, ignore
            }
        } else if (attrName == "domain") {
            std::string domain = attrValue;
            // According to RFC 6265, section 5.2.3, we should remove a leading dot from the domain
            if (!domain.empty() && domain[0] == '.') {
                domain = domain.substr(1);
            }
            cookie.m_domain = domain;
        } else if (attrName == "path") {
            cookie.m_path = attrValue;
        } else if (attrName == "secure") {
            cookie.m_secure = true;
        } else if (attrName == "httponly") {
            cookie.m_httpOnly = true;
        } else if (attrName == "samesite") {
            cookie.m_sameSite = sameSitePolicyFromString(attrValue);
        }
    }

    // Extract domain from URL if not specified in cookie
    if (cookie.m_domain.empty())
        cookie.m_domain = url.host();

    return cookie;
}

std::string HttpCookie::toCookieHeader() const
{
    return m_name + "=" + m_value;
}

std::string HttpCookie::name() const
{
    return m_name;
}

void HttpCookie::setName(const std::string &name)
{
    m_name = name;
}

std::string HttpCookie::value() const
{
    return m_value;
}

void HttpCookie::setValue(const std::string &value)
{
    m_value = value;
}

std::string HttpCookie::domain() const
{
    return m_domain;
}

void HttpCookie::setDomain(const std::string &domain)
{
    m_domain = domain;
}

std::string HttpCookie::path() const
{
    return m_path;
}

void HttpCookie::setPath(const std::string &path)
{
    m_path = path;
}

std::optional<std::chrono::system_clock::time_point> HttpCookie::expirationDate() const
{
    return m_expirationDate;
}

void HttpCookie::setExpirationDate(std::optional<std::chrono::system_clock::time_point> expiration)
{
    m_expirationDate = expiration;
}

bool HttpCookie::isSecure() const
{
    return m_secure;
}

void HttpCookie::setSecure(bool secure)
{
    m_secure = secure;
}

bool HttpCookie::isHttpOnly() const
{
    return m_httpOnly;
}

void HttpCookie::setHttpOnly(bool httpOnly)
{
    m_httpOnly = httpOnly;
}

HttpCookie::SameSitePolicy HttpCookie::sameSite() const
{
    return m_sameSite;
}

void HttpCookie::setSameSite(SameSitePolicy policy)
{
    m_sameSite = policy;
}

bool HttpCookie::isSessionCookie() const
{
    return !m_expirationDate.has_value();
}

bool HttpCookie::isExpired() const
{
    if (!m_expirationDate) {
        return false; // Session cookie never expires
    }

    auto now = std::chrono::system_clock::now();
    return *m_expirationDate < now;
}

bool HttpCookie::matchesUrl(const KDUtils::Uri &url) const
{
    // Extract host and path from URL
    const std::string scheme = url.scheme();
    const std::string host = url.host();
    const std::string path = url.path();

    // Secure cookies only match HTTPS URLs
    if (m_secure && scheme != "https") {
        return false;
    }

    // Check domain match
    if (!domainMatch(m_domain, host)) {
        return false;
    }

    // Check path match
    if (!pathMatch(m_path, path)) {
        return false;
    }

    return true;
}

std::string HttpCookie::toString(SameSitePolicy policy)
{
    switch (policy) {
    case SameSitePolicy::None:
        return "None";
    case SameSitePolicy::Lax:
        return "Lax";
    case SameSitePolicy::Strict:
        return "Strict";
    default:
        return "None";
    }
}

HttpCookie::SameSitePolicy HttpCookie::sameSitePolicyFromString(const std::string &policy)
{
    std::string lowerPolicy = policy;
    std::transform(lowerPolicy.begin(), lowerPolicy.end(), lowerPolicy.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (lowerPolicy == "strict") {
        return SameSitePolicy::Strict;
    } else if (lowerPolicy == "lax") {
        return SameSitePolicy::Lax;
    } else {
        return SameSitePolicy::None;
    }
}

bool HttpCookie::isIpAddress(const std::string &domain)
{
    KDNetwork::IpAddress ip(domain);
    return ip.isValid();
}

bool HttpCookie::domainMatch(const std::string &cookieDomain, const std::string &host)
{
    // IP address exact match only
    if (isIpAddress(host)) {
        return cookieDomain == host;
    }

    // Domain matching per RFC 6265 section 5.1.3
    if (cookieDomain.empty() || host.empty()) {
        return false;
    }

    // Handle leading dot in cookie domain
    std::string domain = cookieDomain;
    if (!domain.empty() && domain[0] == '.') {
        domain = domain.substr(1);
    }

    // Exact match
    if (host == domain) {
        return true;
    }

    // Host must be a subdomain of the cookie domain
    if (host.length() > domain.length() &&
        host.substr(host.length() - domain.length() - 1) == ("." + domain)) {
        return true;
    }

    return false;
}

bool HttpCookie::pathMatch(const std::string &cookiePath, const std::string &requestPath)
{
    // Path matching per RFC 6265 section 5.1.4
    if (cookiePath == requestPath) {
        return true;
    }

    if (cookiePath.empty() || requestPath.empty()) {
        return false;
    }

    // Check if cookie path is a prefix of request path
    if (requestPath.length() >= cookiePath.length() &&
        requestPath.substr(0, cookiePath.length()) == cookiePath) {

        // Cookie path must end with slash or request path has slash after cookie path
        if (cookiePath.back() == '/' ||
            requestPath.length() > cookiePath.length() && requestPath[cookiePath.length()] == '/') {
            return true;
        }
    }

    return false;
}

} // namespace KDNetwork
