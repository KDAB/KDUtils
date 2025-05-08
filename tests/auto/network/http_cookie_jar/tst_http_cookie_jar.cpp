/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/http_cookie_jar.h>
#include <KDNetwork/http_cookie.h>
#include <KDUtils/uri.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDNetwork;
using namespace KDUtils;

TEST_CASE("HttpCookieJar Basic Operations")
{
    HttpCookieJar jar;

    SUBCASE("Empty jar has no cookies")
    {
        CHECK(jar.allCookies().empty());
        CHECK(jar.cookiesForUrl(Uri("https://example.com/")).empty());
        CHECK(jar.cookieHeaderForUrl(Uri("https://example.com/")).empty());
    }

    SUBCASE("Insert and retrieve cookie")
    {
        HttpCookie cookie("name", "value");
        cookie.setDomain("example.com");

        CHECK(jar.insertCookie(cookie));
        CHECK(jar.allCookies().size() == 1);

        auto cookies = jar.cookiesForUrl(Uri("https://example.com/"));
        REQUIRE(cookies.size() == 1);
        CHECK(cookies[0].name() == "name");
        CHECK(cookies[0].value() == "value");

        // Cookie header should be formed correctly
        CHECK(jar.cookieHeaderForUrl(Uri("https://example.com/")) == "name=value");
    }

    SUBCASE("Insert duplicate cookie")
    {
        HttpCookie cookie1("name", "value1");
        cookie1.setDomain("example.com");

        HttpCookie cookie2("name", "value2");
        cookie2.setDomain("example.com");

        // First insertion should succeed
        CHECK(jar.insertCookie(cookie1));

        // Second insertion should fail because cookie with same name, domain, path exists
        CHECK_FALSE(jar.insertCookie(cookie2));

        // Should still have only one cookie
        CHECK(jar.allCookies().size() == 1);

        // Value should be the original one
        auto cookies = jar.cookiesForUrl(Uri("https://example.com/"));
        REQUIRE(cookies.size() == 1);
        CHECK(cookies[0].value() == "value1");
    }

    SUBCASE("Update cookie")
    {
        HttpCookie cookie1("name", "value1");
        cookie1.setDomain("example.com");

        HttpCookie cookie2("name", "value2");
        cookie2.setDomain("example.com");

        // Insert first cookie
        jar.insertCookie(cookie1);

        // Update should succeed
        CHECK(jar.updateCookie(cookie2));

        // Should still have only one cookie
        CHECK(jar.allCookies().size() == 1);

        // Value should be updated
        auto cookies = jar.cookiesForUrl(Uri("https://example.com/"));
        REQUIRE(cookies.size() == 1);
        CHECK(cookies[0].value() == "value2");
    }

    SUBCASE("Remove cookie")
    {
        HttpCookie cookie("name", "value");
        cookie.setDomain("example.com");

        jar.insertCookie(cookie);
        CHECK(jar.allCookies().size() == 1);

        // Remove should return true
        CHECK(jar.removeCookie(cookie));

        // Jar should be empty
        CHECK(jar.allCookies().empty());
    }

    SUBCASE("Remove non-existent cookie")
    {
        HttpCookie cookie("name", "value");
        cookie.setDomain("example.com");

        // Remove should return false for non-existent cookie
        CHECK_FALSE(jar.removeCookie(cookie));
    }

    SUBCASE("Clear all cookies")
    {
        HttpCookie cookie1("name1", "value1");
        cookie1.setDomain("example.com");

        HttpCookie cookie2("name2", "value2");
        cookie2.setDomain("example.org");

        jar.insertCookie(cookie1);
        jar.insertCookie(cookie2);
        CHECK(jar.allCookies().size() == 2);

        jar.clear();
        CHECK(jar.allCookies().empty());
    }
}

TEST_CASE("HttpCookieJar URL matching")
{
    HttpCookieJar jar;

    // Setup cookies for different domains
    HttpCookie cookie1("name1", "value1");
    cookie1.setDomain("example.com");

    HttpCookie cookie2("name2", "value2");
    cookie2.setDomain("subdomain.example.com");

    HttpCookie cookie3("name3", "value3");
    cookie3.setDomain("other.com");

    jar.insertCookie(cookie1);
    jar.insertCookie(cookie2);
    jar.insertCookie(cookie3);

    SUBCASE("Match by domain")
    {
        // example.com should match cookie1
        auto cookies = jar.cookiesForUrl(Uri("https://example.com/"));
        REQUIRE(cookies.size() == 1);
        CHECK(cookies[0].name() == "name1");

        // subdomain.example.com should match cookie1 and cookie2
        cookies = jar.cookiesForUrl(Uri("https://subdomain.example.com/"));
        REQUIRE(cookies.size() == 2);
        CHECK(cookies[0].name() == "name1");
        CHECK(cookies[1].name() == "name2");

        // other.com should match cookie3
        cookies = jar.cookiesForUrl(Uri("https://other.com/"));
        REQUIRE(cookies.size() == 1);
        CHECK(cookies[0].name() == "name3");

        // unrelated.com shouldn't match any cookies
        cookies = jar.cookiesForUrl(Uri("https://unrelated.com/"));
        CHECK(cookies.empty());
    }

    SUBCASE("Domain cookie matching subdomains")
    {
        // Setup a cookie with a domain with leading dot
        HttpCookie dotCookie("dotname", "dotvalue");
        dotCookie.setDomain(".example.org");
        jar.insertCookie(dotCookie);

        // Should match the main domain
        auto cookies = jar.cookiesForUrl(Uri("https://example.org/"));
        REQUIRE(cookies.size() == 1);
        CHECK(cookies[0].name() == "dotname");

        // Should match subdomains
        cookies = jar.cookiesForUrl(Uri("https://sub.example.org/"));
        REQUIRE(cookies.size() == 1);
        CHECK(cookies[0].name() == "dotname");

        // Should match sub-subdomains too
        cookies = jar.cookiesForUrl(Uri("https://subsub.sub.example.org/"));
        REQUIRE(cookies.size() == 1);
        CHECK(cookies[0].name() == "dotname");
    }
}

TEST_CASE("HttpCookieJar Path matching")
{
    HttpCookieJar jar;

    // Setup cookies with different paths
    HttpCookie rootCookie("rootname", "rootvalue");
    rootCookie.setDomain("example.com");
    rootCookie.setPath("/");

    HttpCookie pathCookie("pathname", "pathvalue");
    pathCookie.setDomain("example.com");
    pathCookie.setPath("/path");

    HttpCookie subpathCookie("subname", "subvalue");
    subpathCookie.setDomain("example.com");
    subpathCookie.setPath("/path/sub");

    jar.insertCookie(rootCookie);
    jar.insertCookie(pathCookie);
    jar.insertCookie(subpathCookie);

    SUBCASE("Root path")
    {
        // Root path should only match root cookie
        auto cookies = jar.cookiesForUrl(Uri("https://example.com/"));
        REQUIRE(cookies.size() == 1);
        CHECK(cookies[0].name() == "rootname");
    }

    SUBCASE("Path matching")
    {
        // /path should match root and path cookies
        auto cookies = jar.cookiesForUrl(Uri("https://example.com/path"));
        REQUIRE(cookies.size() == 2);
        const bool hasRootName = (cookies[0].name() == "rootname") || (cookies[1].name() == "rootname");
        const bool hasPathName = (cookies[0].name() == "pathname") || (cookies[1].name() == "pathname");
        CHECK(hasRootName == true);
        CHECK(hasPathName == true);
    }

    SUBCASE("Subpath matching")
    {
        // /path/sub should match all three cookies
        auto cookies = jar.cookiesForUrl(Uri("https://example.com/path/sub"));
        REQUIRE(cookies.size() == 3);

        // Verify all three names are present
        std::vector<std::string> names;
        names.reserve(cookies.size());
        for (const auto &cookie : cookies) {
            names.push_back(cookie.name());
        }
        CHECK(std::find(names.begin(), names.end(), "rootname") != names.end());
        CHECK(std::find(names.begin(), names.end(), "pathname") != names.end());
        CHECK(std::find(names.begin(), names.end(), "subname") != names.end());
    }

    SUBCASE("Non-matching path")
    {
        // /other should only match root cookie
        auto cookies = jar.cookiesForUrl(Uri("https://example.com/other"));
        REQUIRE(cookies.size() == 1);
        CHECK(cookies[0].name() == "rootname");
    }
}

TEST_CASE("HttpCookieJar Security and Protocol")
{
    HttpCookieJar jar;

    // Create a secure cookie
    HttpCookie secureCookie("secure", "value");
    secureCookie.setDomain("example.com");
    secureCookie.setSecure(true);

    // Create a regular cookie
    HttpCookie regularCookie("regular", "value");
    regularCookie.setDomain("example.com");

    jar.insertCookie(secureCookie);
    jar.insertCookie(regularCookie);

    SUBCASE("HTTPS URL should match both cookies")
    {
        auto cookies = jar.cookiesForUrl(Uri("https://example.com/"));
        REQUIRE(cookies.size() == 2);

        // Check both secure and regular cookies are included
        bool hasSecure = false;
        bool hasRegular = false;
        for (const auto &cookie : cookies) {
            if (cookie.name() == "secure")
                hasSecure = true;
            if (cookie.name() == "regular")
                hasRegular = true;
        }
        CHECK(hasSecure);
        CHECK(hasRegular);
    }

    SUBCASE("HTTP URL should only match regular cookie")
    {
        auto cookies = jar.cookiesForUrl(Uri("http://example.com/"));
        REQUIRE(cookies.size() == 1);
        CHECK(cookies[0].name() == "regular");
    }
}

TEST_CASE("HttpCookieJar Expiration")
{
    HttpCookieJar jar;

    // Create a session cookie
    HttpCookie sessionCookie("session", "value");
    sessionCookie.setDomain("example.com");

    // Create an expired cookie
    HttpCookie expiredCookie("expired", "value");
    expiredCookie.setDomain("example.com");
    expiredCookie.setExpirationDate(std::chrono::system_clock::now() - std::chrono::hours(1));

    // Create a future cookie
    HttpCookie futureCookie("future", "value");
    futureCookie.setDomain("example.com");
    futureCookie.setExpirationDate(std::chrono::system_clock::now() + std::chrono::hours(1));

    jar.insertCookie(sessionCookie);
    jar.insertCookie(expiredCookie);
    jar.insertCookie(futureCookie);

    SUBCASE("All cookies initially in jar")
    {
        CHECK(jar.allCookies().size() == 3);
    }

    SUBCASE("cookiesForUrl filters out expired cookies")
    {
        auto cookies = jar.cookiesForUrl(Uri("https://example.com/"));
        REQUIRE(cookies.size() == 2);

        // Check we only have session and future cookies
        bool hasSession = false;
        bool hasFuture = false;
        for (const auto &cookie : cookies) {
            if (cookie.name() == "session")
                hasSession = true;
            if (cookie.name() == "future")
                hasFuture = true;
        }
        CHECK(hasSession);
        CHECK(hasFuture);
    }

    SUBCASE("removeExpiredCookies removes only expired cookies")
    {
        // Should remove exactly 1 cookie
        CHECK(jar.removeExpiredCookies() == 1);

        // Should now have 2 cookies left
        CHECK(jar.allCookies().size() == 2);

        // Check remaining cookies are the expected ones
        bool hasSession = false;
        bool hasFuture = false;
        for (const auto &cookie : jar.allCookies()) {
            if (cookie.name() == "session")
                hasSession = true;
            if (cookie.name() == "future")
                hasFuture = true;
        }
        CHECK(hasSession);
        CHECK(hasFuture);
    }
}

TEST_CASE("HttpCookieJar Parse Set-Cookie Headers")
{
    HttpCookieJar jar;

    SUBCASE("Parse single cookie header")
    {
        const std::vector<std::string> headers = { "name=value; Domain=example.com; Path=/path" };

        int count = jar.parseCookies(Uri("https://example.com/"), headers);
        CHECK(count == 1);

        auto cookies = jar.allCookies();
        REQUIRE(cookies.size() == 1);
        CHECK(cookies[0].name() == "name");
        CHECK(cookies[0].value() == "value");
        CHECK(cookies[0].domain() == "example.com");
        CHECK(cookies[0].path() == "/path");
    }

    SUBCASE("Parse multiple cookie headers")
    {
        const std::vector<std::string> headers = {
            "name1=value1; Domain=example.com",
            "name2=value2; Domain=example.org",
            "name3=value3; Domain=example.net"
        };

        int count = jar.parseCookies(Uri("https://example.com/"), headers);
        CHECK(count == 3);
        CHECK(jar.allCookies().size() == 3);

        // Test for each domain
        CHECK(jar.cookiesForUrl(Uri("https://example.com/")).size() == 1);
        CHECK(jar.cookiesForUrl(Uri("https://example.org/")).size() == 1);
        CHECK(jar.cookiesForUrl(Uri("https://example.net/")).size() == 1);
    }

    SUBCASE("Update existing cookies")
    {
        // First add a cookie
        HttpCookie cookie("name", "old-value");
        cookie.setDomain("example.com");
        jar.insertCookie(cookie);

        // Then parse a Set-Cookie header with the same name but different value
        const std::vector<std::string> headers = { "name=new-value; Domain=example.com" };

        int count = jar.parseCookies(Uri("https://example.com/"), headers);
        CHECK(count == 1);

        // Still only one cookie but with updated value
        auto cookies = jar.allCookies();
        REQUIRE(cookies.size() == 1);
        CHECK(cookies[0].name() == "name");
        CHECK(cookies[0].value() == "new-value");
    }

    SUBCASE("Ignore invalid cookies")
    {
        const std::vector<std::string> headers = {
            "=no-name; Domain=example.com", // No name
            "invalid", // No name=value format
            "name=value; InvalidAttribute" // Valid cookie with invalid attribute
        };

        // Should add only the valid cookie
        int count = jar.parseCookies(Uri("https://example.com/"), headers);
        CHECK(count == 1);

        auto cookies = jar.allCookies();
        REQUIRE(cookies.size() == 1);
        CHECK(cookies[0].name() == "name");
        CHECK(cookies[0].value() == "value");
    }
}

TEST_CASE("HttpCookieJar Cookie Header Construction")
{
    HttpCookieJar jar;

    // Add multiple cookies
    HttpCookie cookie1("name1", "value1");
    cookie1.setDomain("example.com");

    HttpCookie cookie2("name2", "value2");
    cookie2.setDomain("example.com");

    HttpCookie cookie3("name3", "value3");
    cookie3.setDomain("other.com");

    jar.insertCookie(cookie1);
    jar.insertCookie(cookie2);
    jar.insertCookie(cookie3);

    SUBCASE("Cookie header for matching domain")
    {
        const std::string header = jar.cookieHeaderForUrl(Uri("https://example.com/"));

        // Should include both cookies, separated by semicolon and space
        CHECK(header.find("name1=value1") != std::string::npos);
        CHECK(header.find("name2=value2") != std::string::npos);
        CHECK(header.find("; ") != std::string::npos);
    }

    SUBCASE("Cookie header for other domain")
    {
        std::string header = jar.cookieHeaderForUrl(Uri("https://other.com/"));

        // Should only include the one cookie for other.com
        CHECK(header == "name3=value3");
    }

    SUBCASE("Cookie header for non-matching domain")
    {
        const std::string header = jar.cookieHeaderForUrl(Uri("https://unrelated.com/"));

        // Should be empty
        CHECK(header.empty());
    }
}
