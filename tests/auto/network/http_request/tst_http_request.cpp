/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/http_request.h>

#include <KDUtils/uri.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

namespace doctest {
template<>
struct StringMaker<KDNetwork::HttpMethod> {
    static String convert(const KDNetwork::HttpMethod &val)
    {
        return KDNetwork::toStdString(val).c_str();
    }
};
} // namespace doctest

using namespace KDNetwork;
using namespace KDUtils;

TEST_CASE("HttpRequest construction and basic methods")
{
    SUBCASE("Default constructor")
    {
        HttpRequest request;
        CHECK(request.url().toString() == "");
        CHECK(request.method() == HttpMethod::Get);
        CHECK(request.timeout() == std::chrono::milliseconds(30000));
        CHECK(request.redirectPolicy() == HttpRequest::RedirectPolicy::FollowAll);
        CHECK(request.maxRedirects() == 5);
        CHECK(request.autoAddCommonHeaders() == true);
        CHECK(request.authType() == HttpRequest::AuthType::None);
        CHECK(request.body().size() == 0);
    }

    SUBCASE("Constructor with URL and method")
    {
        Uri url("https://example.com/api");
        HttpRequest request(url, HttpMethod::Post);
        CHECK(request.url() == url);
        CHECK(request.method() == HttpMethod::Post);
    }

    SUBCASE("Set URL")
    {
        HttpRequest request;
        Uri url("https://example.com/api");
        request.setUrl(url);
        CHECK(request.url() == url);
    }

    SUBCASE("Set method")
    {
        HttpRequest request;
        request.setMethod(HttpMethod::Delete);
        CHECK(request.method() == HttpMethod::Delete);
    }

    SUBCASE("toString method")
    {
        CHECK(toStdString(HttpMethod::Get) == "GET");
        CHECK(toStdString(HttpMethod::Post) == "POST");
        CHECK(toStdString(HttpMethod::Put) == "PUT");
        CHECK(toStdString(HttpMethod::Delete) == "DELETE");
        CHECK(toStdString(HttpMethod::Head) == "HEAD");
        CHECK(toStdString(HttpMethod::Options) == "OPTIONS");
        CHECK(toStdString(HttpMethod::Patch) == "PATCH");
        CHECK(toStdString(HttpMethod::Connect) == "CONNECT");
        CHECK(toStdString(HttpMethod::Trace) == "TRACE");
    }
}

TEST_CASE("HttpRequest headers")
{
    HttpRequest request;

    SUBCASE("Set and get header")
    {
        request.setHeader("Content-Type", "application/json");
        CHECK(request.header("Content-Type") == "application/json");

        // Case insensitive retrieval
        CHECK(request.header("content-type") == "application/json");
    }

    SUBCASE("Add header")
    {
        request.addHeader("Accept", "application/json");
        request.addHeader("Accept", "text/html");

        auto acceptHeaders = request.headers("Accept");
        CHECK(acceptHeaders.size() == 2);
        CHECK(acceptHeaders[0] == "application/json");
        CHECK(acceptHeaders[1] == "text/html");
    }

    SUBCASE("Remove header")
    {
        request.setHeader("X-Custom", "value");
        CHECK(request.header("X-Custom") == "value");

        request.removeHeader("X-Custom");
        CHECK(request.header("X-Custom") == "");
        CHECK(request.headers("X-Custom").empty());
    }

    SUBCASE("Set header with overwrite flag")
    {
        request.setHeader("Accept", "application/json");
        request.setHeader("Accept", "text/html", false); // Don't overwrite

        auto acceptHeaders = request.headers("Accept");
        CHECK(acceptHeaders.size() == 2);

        request.setHeader("Accept", "application/xml", true); // Overwrite
        acceptHeaders = request.headers("Accept");
        CHECK(acceptHeaders.size() == 1);
        CHECK(acceptHeaders[0] == "application/xml");
    }

    SUBCASE("All headers")
    {
        request.setHeader("Content-Type", "application/json");
        request.addHeader("Accept", "application/json");
        request.addHeader("Accept", "text/html");

        auto headers = request.allHeaders();
        CHECK(headers.size() == 3);
        CHECK(headers.count("content-type") == 1);
        CHECK(headers.count("accept") == 2);
    }
}

TEST_CASE("HttpRequest body")
{
    HttpRequest request;

    SUBCASE("Set body from ByteArray")
    {
        ByteArray data{ std::vector<std::uint8_t>{ 0x01, 0x02, 0x03, 0x04 } };
        request.setBody(data);

        CHECK(request.body() == data);
        CHECK(request.body().size() == 4);
    }

    SUBCASE("Set body from string")
    {
        std::string content = "Hello, World!";
        request.setBody(content);

        CHECK(request.body().size() == content.size());

        // Convert ByteArray back to string for comparison
        std::string bodyStr = request.body().toStdString();
        CHECK(bodyStr == content);
    }
}

TEST_CASE("HttpRequest timeout")
{
    HttpRequest request;

    SUBCASE("Default timeout")
    {
        CHECK(request.timeout() == std::chrono::milliseconds(30000));
    }

    SUBCASE("Set timeout")
    {
        request.setTimeout(std::chrono::milliseconds(5000));
        CHECK(request.timeout() == std::chrono::milliseconds(5000));
    }
}

TEST_CASE("HttpRequest redirect handling")
{
    HttpRequest request;

    SUBCASE("Default redirect policy")
    {
        CHECK(request.redirectPolicy() == HttpRequest::RedirectPolicy::FollowAll);
        CHECK(request.maxRedirects() == 5);
    }

    SUBCASE("Set redirect policy")
    {
        request.setRedirectPolicy(HttpRequest::RedirectPolicy::DontFollow);
        CHECK(request.redirectPolicy() == HttpRequest::RedirectPolicy::DontFollow);
    }

    SUBCASE("Set max redirects")
    {
        request.setMaxRedirects(10);
        CHECK(request.maxRedirects() == 10);
    }
}

TEST_CASE("HttpRequest authentication")
{
    HttpRequest request;

    SUBCASE("Default auth type")
    {
        CHECK(request.authType() == HttpRequest::AuthType::None);
        CHECK(request.authUsername() == "");
        CHECK(request.authCredential() == "");
    }

    SUBCASE("Set basic auth")
    {
        request.setBasicAuth("username", "password");
        CHECK(request.authType() == HttpRequest::AuthType::Basic);
        CHECK(request.authUsername() == "username");
        CHECK(request.authCredential() == "password");
        CHECK(request.header("Authorization").substr(0, 6) == "Basic ");
    }

    SUBCASE("Set bearer auth")
    {
        request.setBearerAuth("token123");
        CHECK(request.authType() == HttpRequest::AuthType::Bearer);
        CHECK(request.authUsername() == "");
        CHECK(request.authCredential() == "token123");
        CHECK(request.header("Authorization") == "Bearer token123");
    }
}

TEST_CASE("HttpRequest common headers")
{
    HttpRequest request;

    SUBCASE("Default auto add common headers")
    {
        CHECK(request.autoAddCommonHeaders() == true);
    }

    SUBCASE("Set auto add common headers")
    {
        request.setAutoAddCommonHeaders(false);
        CHECK(request.autoAddCommonHeaders() == false);
    }
}
