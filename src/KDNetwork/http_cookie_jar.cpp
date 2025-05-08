/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/http_cookie_jar.h>

#include <KDUtils/uri.h>

#include <algorithm>

namespace KDNetwork {

HttpCookieJar::HttpCookieJar()
{
}

HttpCookieJar::HttpCookieJar(const HttpCookieJar &other)
{
    // Copy cookies while holding the other's mutex
    const std::lock_guard<std::mutex> lock(other.m_mutex);
    m_cookies = other.m_cookies;
    // The mutex is automatically created and doesn't need to be copied
}

HttpCookieJar &HttpCookieJar::operator=(const HttpCookieJar &other)
{
    if (this != &other) {
        // Lock both mutexes to prevent deadlock
        // Always lock mutexes in the same order to avoid deadlock
        std::lock(m_mutex, other.m_mutex);
        const std::lock_guard<std::mutex> lockThis(m_mutex, std::adopt_lock);
        const std::lock_guard<std::mutex> lockOther(other.m_mutex, std::adopt_lock);

        m_cookies = other.m_cookies;
    }
    return *this;
}

std::vector<HttpCookie> HttpCookieJar::allCookies() const
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    return m_cookies;
}

std::vector<HttpCookie> HttpCookieJar::cookiesForUrl(const KDUtils::Uri &url)
{
    const std::lock_guard<std::mutex> lock(m_mutex);

    // Remove expired cookies without calling removeExpiredCookies()
    removeExpiredCookiesInternal();

    // Find matching cookies
    std::vector<HttpCookie> result;
    for (const auto &cookie : m_cookies) {
        if (cookie.matchesUrl(url)) {
            result.push_back(cookie);
        }
    }

    return result;
}

bool HttpCookieJar::insertCookie(const HttpCookie &cookie)
{
    const std::lock_guard<std::mutex> lock(m_mutex);

    // Check if cookie is already in the jar
    auto existingIdx = findCookie(cookie);
    if (existingIdx) {
        return false; // Cookie already exists
    }

    // Add the cookie
    m_cookies.push_back(cookie);
    return true;
}

bool HttpCookieJar::updateCookie(const HttpCookie &cookie)
{
    const std::lock_guard<std::mutex> lock(m_mutex);

    // Try to find existing cookie
    auto existingIdx = findCookie(cookie);
    if (existingIdx) {
        // Replace existing cookie
        m_cookies[*existingIdx] = cookie;
        return true;
    }

    // Insert new cookie
    m_cookies.push_back(cookie);
    return true;
}

bool HttpCookieJar::removeCookie(const HttpCookie &cookie)
{
    const std::lock_guard<std::mutex> lock(m_mutex);

    auto existingIdx = findCookie(cookie);
    if (!existingIdx) {
        return false; // Cookie not found
    }

    // Remove the cookie
    m_cookies.erase(m_cookies.begin() + *existingIdx);
    return true;
}

int HttpCookieJar::removeCookies(const std::string &name, const std::string &domain)
{
    const std::lock_guard<std::mutex> lock(m_mutex);

    int count = 0;
    auto it = m_cookies.begin();
    while (it != m_cookies.end()) {
        if (it->name() == name && it->domain() == domain) {
            it = m_cookies.erase(it);
            count++;
        } else {
            ++it;
        }
    }

    return count;
}

void HttpCookieJar::clear()
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    m_cookies.clear();
}

int HttpCookieJar::removeExpiredCookiesInternal()
{
    // Note: This method assumes the mutex is already locked by the caller

    int count = 0;
    auto it = m_cookies.begin();
    while (it != m_cookies.end()) {
        if (it->isExpired()) {
            it = m_cookies.erase(it);
            count++;
        } else {
            ++it;
        }
    }

    return count;
}

int HttpCookieJar::removeExpiredCookies()
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    return removeExpiredCookiesInternal();
}

int HttpCookieJar::parseCookies(const KDUtils::Uri &url, const std::vector<std::string> &setCookieHeaders)
{
    int count = 0;

    for (const auto &setCookieHeader : setCookieHeaders) {
        auto cookie = HttpCookie::fromSetCookieHeader(setCookieHeader, url);
        if (cookie) {
            updateCookie(*cookie);
            count++;
        }
    }

    return count;
}

std::string HttpCookieJar::cookieHeaderForUrl(const KDUtils::Uri &url)
{
    auto cookies = cookiesForUrl(url);
    if (cookies.empty()) {
        return {};
    }

    std::string result;
    bool first = true;

    for (const auto &cookie : cookies) {
        if (!first) {
            result += "; ";
        }
        result += cookie.toCookieHeader();
        first = false;
    }

    return result;
}

std::optional<size_t> HttpCookieJar::findCookie(const HttpCookie &cookie) const
{
    for (size_t i = 0; i < m_cookies.size(); ++i) {
        if (m_cookies[i].name() == cookie.name() &&
            m_cookies[i].domain() == cookie.domain() &&
            m_cookies[i].path() == cookie.path()) {
            return i;
        }
    }

    return std::nullopt;
}

} // namespace KDNetwork
