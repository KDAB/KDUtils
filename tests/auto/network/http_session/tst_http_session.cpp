/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/http_session.h>
#include <KDNetwork/http_request.h>
#include <KDNetwork/http_cookie_jar.h>

#include <KDUtils/uri.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDNetwork;
using namespace KDUtils;

TEST_CASE("HttpSession default initialization")
{
    HttpSession session;

    // Check default values
    CHECK(!session.userAgent().empty());
    CHECK(session.userAgent() == "KDNetwork HttpClient/1.0");
    CHECK(session.connectionTimeout() == std::chrono::milliseconds(30000));
    CHECK(session.idleConnectionTimeout() == std::chrono::milliseconds(60000));
    CHECK(session.maxConnectionsPerHost() == 6);
    CHECK(session.followRedirects() == true);
    CHECK(session.maxRedirects() == 5);
    CHECK(session.defaultHeaders().size() == 2); // User-Agent and Accept
    CHECK(session.defaultHeader("User-Agent") == "KDNetwork HttpClient/1.0");
    CHECK(session.defaultHeader("Accept") == "*/*");
    CHECK(session.cookieJar().allCookies().empty());
    CHECK(session.cookieJar().cookiesForUrl(Uri("http://example.com/")).empty());
}

TEST_CASE("HttpSession default headers management")
{
    HttpSession session;

    // Set default headers
    session.setDefaultHeader("Accept", "application/json");
    session.setDefaultHeader("X-Custom-Header", "CustomValue");

    // Check header retrieval
    CHECK(session.defaultHeader("Accept") == "application/json");
    CHECK(session.defaultHeader("X-Custom-Header") == "CustomValue");
    CHECK(session.defaultHeader("Non-Existent-Header").empty());

    // Check all default headers
    auto headers = session.defaultHeaders();
    CHECK(headers.size() == 3); // 1 default + 1 replaced default + 1 custom
    CHECK(headers["accept"] == "application/json");
    CHECK(headers["x-custom-header"] == "CustomValue");

    // Test removing a header
    session.removeDefaultHeader("Accept");
    CHECK(session.defaultHeader("Accept").empty());
    CHECK(session.defaultHeaders().size() == 2);
}

TEST_CASE("HttpSession applying default headers to request")
{
    HttpSession session;
    HttpRequest request(Uri("http://example.com"));

    // Set default headers on session
    session.setDefaultHeader("Accept", "application/json");
    session.setDefaultHeader("User-Agent", "TestUserAgent");

    // Apply default headers to request
    session.applyDefaultHeaders(request);

    // Check headers were applied
    CHECK(request.header("Accept") == "application/json");
    CHECK(request.header("User-Agent") == "TestUserAgent");

    // Test that existing request headers take precedence
    HttpRequest requestWithHeaders(Uri("http://example.com"));
    requestWithHeaders.setHeader("Accept", "text/html");

    session.applyDefaultHeaders(requestWithHeaders);
    CHECK(requestWithHeaders.header("Accept") == "text/html"); // Should keep original value
    CHECK(requestWithHeaders.header("User-Agent") == "TestUserAgent"); // Should get default value
}

TEST_CASE("HttpSession user agent handling")
{
    HttpSession session;

    // Set user agent
    session.setUserAgent("MyCustomUserAgent/1.0");

    // Check user agent value
    CHECK(session.userAgent() == "MyCustomUserAgent/1.0");

    // Check user agent is used in default headers
    CHECK(session.defaultHeader("User-Agent") == "MyCustomUserAgent/1.0");
}

TEST_CASE("HttpSession timeout settings")
{
    HttpSession session;

    // Set timeouts
    session.setConnectionTimeout(std::chrono::milliseconds(5000));
    session.setIdleConnectionTimeout(std::chrono::milliseconds(10000));

    // Check timeout values
    CHECK(session.connectionTimeout() == std::chrono::milliseconds(5000));
    CHECK(session.idleConnectionTimeout() == std::chrono::milliseconds(10000));
}

TEST_CASE("HttpSession redirect settings")
{
    HttpSession session;

    // Test default values
    CHECK(session.followRedirects() == true);
    CHECK(session.maxRedirects() == 5);

    // Change values
    session.setFollowRedirects(false);
    session.setMaxRedirects(10);

    // Check updated values
    CHECK(session.followRedirects() == false);
    CHECK(session.maxRedirects() == 10);
}

TEST_CASE("HttpSession connection limit")
{
    HttpSession session;

    // Test default value
    CHECK(session.maxConnectionsPerHost() == 6);

    // Change value
    session.setMaxConnectionsPerHost(10);

    // Check updated value
    CHECK(session.maxConnectionsPerHost() == 10);
}

TEST_CASE("HttpSession cookie jar")
{
    HttpSession session;

    // Get cookie jar and add a cookie
    HttpCookieJar &jar = session.cookieJar();
    HttpCookie cookie;
    cookie.setName("testCookie");
    cookie.setValue("testValue");
    cookie.setDomain("example.com");
    cookie.setPath("/");
    jar.insertCookie(cookie);

    // Check cookie is stored
    auto cookies = jar.cookiesForUrl(Uri("http://example.com/"));
    REQUIRE(cookies.size() == 1);
    CHECK(cookies[0].name() == "testCookie");
    CHECK(cookies[0].value() == "testValue");

    // Create new jar and set it
    HttpCookieJar newJar;
    HttpCookie newCookie;
    newCookie.setName("newCookie");
    newCookie.setValue("newValue");
    newCookie.setDomain("example.org");
    newCookie.setPath("/");
    newJar.insertCookie(newCookie);

    session.setCookieJar(newJar);

    // Check new cookie jar is used
    auto newCookies = session.cookieJar().cookiesForUrl(Uri("http://example.org/"));
    REQUIRE(newCookies.size() == 1);
    CHECK(newCookies[0].name() == "newCookie");
    CHECK(newCookies[0].value() == "newValue");

    // Old cookies should be gone
    CHECK(session.cookieJar().cookiesForUrl(Uri("http://example.com/")).empty());
}
