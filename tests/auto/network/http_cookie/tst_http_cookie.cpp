/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/http_cookie.h>
#include <KDUtils/uri.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDNetwork;
using namespace KDUtils;

TEST_CASE("HttpCookie Construction")
{
    SUBCASE("Default constructor creates an empty cookie")
    {
        HttpCookie cookie;
        CHECK(cookie.name().empty());
        CHECK(cookie.value().empty());
        CHECK(cookie.domain().empty());
        CHECK(cookie.path() == "/");
        CHECK_FALSE(cookie.expirationDate().has_value());
        CHECK_FALSE(cookie.isSecure());
        CHECK_FALSE(cookie.isHttpOnly());
        CHECK(cookie.sameSite() == HttpCookie::SameSitePolicy::None);
        CHECK_FALSE(cookie.isExpired());
        CHECK(cookie.isSessionCookie());
    }

    SUBCASE("Constructor with name and value")
    {
        HttpCookie cookie("name", "value");
        CHECK(cookie.name() == "name");
        CHECK(cookie.value() == "value");
        CHECK(cookie.domain().empty());
        CHECK(cookie.path() == "/");
        CHECK_FALSE(cookie.expirationDate().has_value());
        CHECK_FALSE(cookie.isSecure());
        CHECK_FALSE(cookie.isHttpOnly());
        CHECK(cookie.sameSite() == HttpCookie::SameSitePolicy::None);
        CHECK_FALSE(cookie.isExpired());
        CHECK(cookie.isSessionCookie());
    }
}

TEST_CASE("HttpCookie Setters")
{
    HttpCookie cookie;

    SUBCASE("Set name and value")
    {
        cookie.setName("test_name");
        cookie.setValue("test_value");
        CHECK(cookie.name() == "test_name");
        CHECK(cookie.value() == "test_value");
    }

    SUBCASE("Set domain and path")
    {
        cookie.setDomain("example.com");
        cookie.setPath("/test");
        CHECK(cookie.domain() == "example.com");
        CHECK(cookie.path() == "/test");
    }

    SUBCASE("Set expiration date")
    {
        auto now = std::chrono::system_clock::now();
        cookie.setExpirationDate(now);
        CHECK(cookie.expirationDate().has_value());
        CHECK_FALSE(cookie.isSessionCookie());

        // Set to nullopt for a session cookie
        cookie.setExpirationDate(std::nullopt);
        CHECK_FALSE(cookie.expirationDate().has_value());
        CHECK(cookie.isSessionCookie());
    }

    SUBCASE("Set secure and HttpOnly flags")
    {
        cookie.setSecure(true);
        cookie.setHttpOnly(true);
        CHECK(cookie.isSecure());
        CHECK(cookie.isHttpOnly());
    }

    SUBCASE("Set SameSite policy")
    {
        cookie.setSameSite(HttpCookie::SameSitePolicy::Strict);
        CHECK(cookie.sameSite() == HttpCookie::SameSitePolicy::Strict);

        cookie.setSameSite(HttpCookie::SameSitePolicy::Lax);
        CHECK(cookie.sameSite() == HttpCookie::SameSitePolicy::Lax);

        cookie.setSameSite(HttpCookie::SameSitePolicy::None);
        CHECK(cookie.sameSite() == HttpCookie::SameSitePolicy::None);
    }
}

TEST_CASE("HttpCookie Parse from Set-Cookie header")
{
    SUBCASE("Basic cookie")
    {
        Uri url("https://example.com/path");
        auto cookie = HttpCookie::fromSetCookieHeader("name=value", url);
        REQUIRE(cookie.has_value());
        CHECK(cookie->name() == "name");
        CHECK(cookie->value() == "value");
        CHECK(cookie->domain() == "example.com");
        CHECK(cookie->path() == "/");
        CHECK_FALSE(cookie->expirationDate().has_value());
        CHECK_FALSE(cookie->isSecure());
        CHECK_FALSE(cookie->isHttpOnly());
        CHECK(cookie->isSessionCookie());
    }

    SUBCASE("Cookie with attributes")
    {
        Uri url("https://example.com/path");
        auto cookie = HttpCookie::fromSetCookieHeader(
                "name=value; Domain=.example.com; Path=/test; Secure; HttpOnly; SameSite=Strict",
                url);
        REQUIRE(cookie.has_value());
        CHECK(cookie->name() == "name");
        CHECK(cookie->value() == "value");
        CHECK(cookie->domain() == ".example.com");
        CHECK(cookie->path() == "/test");
        CHECK(cookie->isSecure());
        CHECK(cookie->isHttpOnly());
        CHECK(cookie->sameSite() == HttpCookie::SameSitePolicy::Strict);
    }

    SUBCASE("Invalid cookie")
    {
        Uri url("https://example.com/path");
        auto cookie = HttpCookie::fromSetCookieHeader("invalid_cookie", url);
        CHECK_FALSE(cookie.has_value());
    }

    SUBCASE("Cookie with Max-Age")
    {
        Uri url("https://example.com/path");
        auto cookie = HttpCookie::fromSetCookieHeader(
                "name=value; Max-Age=3600",
                url);
        REQUIRE(cookie.has_value());
        CHECK(cookie->name() == "name");
        CHECK(cookie->value() == "value");
        CHECK(cookie->expirationDate().has_value());
        CHECK_FALSE(cookie->isSessionCookie());
    }

    SUBCASE("Cookie with empty name")
    {
        Uri url("https://example.com/path");
        auto cookie = HttpCookie::fromSetCookieHeader("=value", url);
        CHECK_FALSE(cookie.has_value());
    }

    SUBCASE("SameSite values")
    {
        Uri url("https://example.com/path");

        auto laxCookie = HttpCookie::fromSetCookieHeader("name=value; SameSite=Lax", url);
        REQUIRE(laxCookie.has_value());
        CHECK(laxCookie->sameSite() == HttpCookie::SameSitePolicy::Lax);

        auto strictCookie = HttpCookie::fromSetCookieHeader("name=value; SameSite=Strict", url);
        REQUIRE(strictCookie.has_value());
        CHECK(strictCookie->sameSite() == HttpCookie::SameSitePolicy::Strict);

        auto noneCookie = HttpCookie::fromSetCookieHeader("name=value; SameSite=None", url);
        REQUIRE(noneCookie.has_value());
        CHECK(noneCookie->sameSite() == HttpCookie::SameSitePolicy::None);

        // Invalid SameSite value should default to None
        auto invalidCookie = HttpCookie::fromSetCookieHeader("name=value; SameSite=Invalid", url);
        REQUIRE(invalidCookie.has_value());
        CHECK(invalidCookie->sameSite() == HttpCookie::SameSitePolicy::None);
    }
}

TEST_CASE("HttpCookie URL Matching")
{
    SUBCASE("Domain matching")
    {
        HttpCookie cookie("name", "value");
        cookie.setDomain("example.com");

        // Exact match
        CHECK(cookie.matchesUrl(Uri("http://example.com/")));

        // Subdomain match
        CHECK(cookie.matchesUrl(Uri("http://sub.example.com/")));

        // Non-matching domain
        CHECK_FALSE(cookie.matchesUrl(Uri("http://other.com/")));
    }

    SUBCASE("Path matching")
    {
        HttpCookie cookie("name", "value");
        cookie.setDomain("example.com");
        cookie.setPath("/test");

        // Exact path match
        CHECK(cookie.matchesUrl(Uri("http://example.com/test")));

        // Subpath match
        CHECK(cookie.matchesUrl(Uri("http://example.com/test/subpath")));

        // Non-matching path
        CHECK_FALSE(cookie.matchesUrl(Uri("http://example.com/other")));
        CHECK_FALSE(cookie.matchesUrl(Uri("http://example.com/")));
    }

    SUBCASE("Secure flag")
    {
        HttpCookie cookie("name", "value");
        cookie.setDomain("example.com");
        cookie.setSecure(true);

        // HTTPS URL
        CHECK(cookie.matchesUrl(Uri("https://example.com/")));

        // HTTP URL (should not match secure cookie)
        CHECK_FALSE(cookie.matchesUrl(Uri("http://example.com/")));
    }
}

TEST_CASE("HttpCookie To Header Value")
{
    HttpCookie cookie("name", "value");
    CHECK(cookie.toCookieHeader() == "name=value");

    // Special characters in value
    cookie.setValue("value with spaces");
    CHECK(cookie.toCookieHeader() == "name=value with spaces");
}

TEST_CASE("HttpCookie Expiration")
{
    HttpCookie cookie("name", "value");

    SUBCASE("Session cookie")
    {
        CHECK(cookie.isSessionCookie());
        CHECK_FALSE(cookie.isExpired());
    }

    SUBCASE("Expired cookie")
    {
        // Set expiration date in the past
        auto pastTime = std::chrono::system_clock::now() - std::chrono::hours(1);
        cookie.setExpirationDate(pastTime);
        CHECK_FALSE(cookie.isSessionCookie());
        CHECK(cookie.isExpired());
    }

    SUBCASE("Future expiration")
    {
        // Set expiration date in the future
        auto futureTime = std::chrono::system_clock::now() + std::chrono::hours(1);
        cookie.setExpirationDate(futureTime);
        CHECK_FALSE(cookie.isSessionCookie());
        CHECK_FALSE(cookie.isExpired());
    }
}
