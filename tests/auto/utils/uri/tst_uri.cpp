/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: GitHub Copilot <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDUtils/uri.h>
#include <KDUtils/url.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

using namespace KDUtils;

TEST_SUITE("Uri")
{
    TEST_CASE("initialization")
    {
        static_assert(std::is_destructible<Uri>{},
                      "Uri should be destructible");
        static_assert(std::is_default_constructible<Uri>{},
                      "Uri should be default constructible");
        static_assert(std::is_copy_constructible<Uri>{},
                      "Uri should be copy constructible");
        static_assert(std::is_copy_assignable<Uri>{},
                      "Uri should be copy assignable");
        static_assert(std::is_move_constructible<Uri>{},
                      "Uri should be move constructible");
        static_assert(std::is_move_assignable<Uri>{},
                      "Uri should be move assignable");

        Uri uri;
        CHECK(uri.isEmpty());
        CHECK(uri.isRelative());
        CHECK(!uri.isAbsolute());
    }

    TEST_CASE("basic parsing")
    {
        Uri uri("https://user:pass@example.com:8080/path/to/resource?query=value#fragment");

        CHECK(uri.scheme() == "https");
        CHECK(uri.userInfo() == "user:pass");
        CHECK(uri.host() == "example.com");
        CHECK(uri.port() == 8080);
        CHECK(uri.path() == "/path/to/resource");
        CHECK(uri.query() == "query=value");
        CHECK(uri.fragment() == "fragment");
        CHECK(uri.authority() == "user:pass@example.com:8080");
        CHECK(!uri.isEmpty());
        CHECK(!uri.isRelative());
        CHECK(uri.isAbsolute());
    }

    TEST_CASE("query parameters")
    {
        Uri uri("https://example.com?name=John&age=25&active");

        auto params = uri.queryParameters();
        CHECK(params.size() == 3);
        CHECK(uri.hasQueryParameter("name"));
        CHECK(uri.hasQueryParameter("age"));
        CHECK(uri.hasQueryParameter("active"));
        CHECK(uri.queryParameter("name") == "John");
        CHECK(uri.queryParameter("age") == "25");
        CHECK(uri.queryParameter("active") == "");
        CHECK(!uri.hasQueryParameter("nonexistent"));
        CHECK(uri.queryParameter("nonexistent") == "");

        // Modify query parameters
        const Uri modified = uri.withQueryParameter("name", "Jane").withQueryParameter("height", "170");

        CHECK(modified.queryParameter("name") == "Jane");
        CHECK(modified.queryParameter("height") == "170");
    }

    TEST_CASE("builder pattern")
    {
        Uri uri;
        uri.withScheme("https")
                .withHost("api.example.com")
                .withPort(443)
                .withPath("/v1/users")
                .withQueryParameter("page", "1")
                .withQueryParameter("limit", "10")
                .withFragment("results");

        CHECK(uri.scheme() == "https");
        CHECK(uri.host() == "api.example.com");
        CHECK(uri.port() == 443);
        CHECK(uri.path() == "/v1/users");
        CHECK(uri.hasQueryParameter("page"));
        CHECK(uri.hasQueryParameter("limit"));
        CHECK(uri.fragment() == "results");

        // Test toString
        std::string expected = "https://api.example.com:443/v1/users?limit=10&page=1#results";
        // Note: Query parameters might be in different order, so we'll check parts separately
        std::string actual = uri.toString();
        CHECK(actual.find("https://api.example.com:443/v1/users?") == 0);
        CHECK(actual.find("limit=10") != std::string::npos);
        CHECK(actual.find("page=1") != std::string::npos);
        CHECK(actual.find("#results") != std::string::npos);
    }

    TEST_CASE("encoding and decoding")
    {
        std::string original = "a b+c %<>&?/\\";
        std::string encoded = Uri::encodeComponent(original);
        std::string decoded = Uri::decodeComponent(encoded);

        CHECK(encoded != original); // Should be different
        CHECK(decoded == original); // Should round-trip correctly

        // Check that specific characters are encoded
        CHECK(encoded.find('%') != std::string::npos);
        CHECK(encoded.find('+') == std::string::npos); // + should be encoded
        CHECK(encoded.find(' ') == std::string::npos); // spaces should be encoded
        CHECK(encoded.find('a') != std::string::npos); // letters should not be encoded
    }

    TEST_CASE("normalization")
    {
        // Test case normalization
        Uri uri1("HTTP://ExAmPle.CoM/path");
        Uri normalized1 = uri1.normalized();
        CHECK(normalized1.scheme() == "http");
        CHECK(normalized1.host() == "example.com");

        // Test port normalization (remove default ports)
        Uri uri2("http://example.com:80/path");
        Uri normalized2 = uri2.normalized();
        CHECK(!normalized2.hasExplicitPort());

        // Test path normalization
        Uri uri3("http://example.com/a/b/../c/./d");
        Uri normalized3 = uri3.normalized();
        CHECK(normalized3.path() == "/a/c/d");
    }

    TEST_CASE("resolving relative URIs")
    {
        Uri base("http://example.com/a/b/c");

        // Relative path
        Uri rel1 = base.resolved(Uri("d"));
        CHECK(rel1.toString() == "http://example.com/a/b/d");

        // Absolute path
        Uri rel2 = base.resolved(Uri("/x/y/z"));
        CHECK(rel2.toString() == "http://example.com/x/y/z");

        // Up-level reference
        Uri rel3 = base.resolved(Uri("../e/f"));
        CHECK(rel3.toString() == "http://example.com/a/e/f");

        // Authority component
        Uri rel4 = base.resolved(Uri("//other.example.com/path"));
        CHECK(rel4.toString() == "http://other.example.com/path");

        // Fragment only
        Uri rel5 = base.resolved(Uri("#fragment"));
        CHECK(rel5.toString() == "http://example.com/a/b/c#fragment");

        // Query only
        Uri rel6 = base.resolved(Uri("?query=value"));
        CHECK(rel6.toString() == "http://example.com/a/b/c?query=value");
    }

    TEST_CASE("local file URIs")
    {
#ifdef _WIN32
        // Windows paths
        Uri winFile = Uri::fromLocalFile("C:\\folder\\file.txt");
        CHECK(winFile.isLocalFile());
        CHECK(winFile.scheme() == "file");
        CHECK(winFile.path() == "/C:/folder/file.txt");
        CHECK(winFile.toLocalFile() == "C:/folder/file.txt");

        Uri winFileUNC = Uri::fromLocalFile("\\\\server\\share\\file.txt");
        CHECK(winFileUNC.isLocalFile());
        CHECK(winFileUNC.path().find("//server/share/file.txt") != std::string::npos);
#else
        // Unix paths
        Uri unixFile = Uri::fromLocalFile("/usr/local/bin/app");
        CHECK(unixFile.isLocalFile());
        CHECK(unixFile.scheme() == "file");
        CHECK(unixFile.path() == "/usr/local/bin/app");
        CHECK(unixFile.toLocalFile() == "/usr/local/bin/app");
#endif

        // Relative path
        Uri relativeFile = Uri::fromLocalFile("folder/file.txt");
        CHECK(relativeFile.isLocalFile());
        CHECK(relativeFile.toLocalFile() == "/folder/file.txt"); // Will have added leading slash
    }

    TEST_CASE("scheme handlers")
    {
        // Get HTTP handler
        const UriSchemeHandler *httpHandler =
                UriSchemeRegistry::instance().handlerForScheme("http");
        REQUIRE(httpHandler != nullptr);
        CHECK(httpHandler->defaultPort() == "80");

        // Valid HTTP URI
        Uri validHttpUri("http://example.com");
        CHECK(httpHandler->validate(validHttpUri));

        // Invalid HTTP URI (no host)
        Uri invalidHttpUri("http:");
        CHECK_FALSE(httpHandler->validate(invalidHttpUri));

        // Register custom handler
        class CustomSchemeHandler : public UriSchemeHandler
        {
        public:
            std::string defaultPort() const override { return "9000"; }
            bool validate(const Uri &uri) const override
            {
                return !uri.host().empty() && uri.path().find("/api") == 0;
            }
        };

        UriSchemeRegistry::instance().registerSchemeHandler(
                "custom", std::make_unique<CustomSchemeHandler>());

        const UriSchemeHandler *customHandler =
                UriSchemeRegistry::instance().handlerForScheme("custom");
        REQUIRE(customHandler != nullptr);

        // Check custom handler behavior
        Uri validCustomUri("custom://example.com/api/resource");
        Uri invalidCustomUri("custom://example.com/invalid");

        CHECK(customHandler->validate(validCustomUri));
        CHECK_FALSE(customHandler->validate(invalidCustomUri));
    }

    TEST_CASE("compatibility with existing Url class")
    {
        // Test conversion from Url to Uri
        Url oldUrl("https://example.com/path");
        Uri newUri(oldUrl.url());

        CHECK(newUri.scheme() == oldUrl.scheme());
        // CHECK(newUri.path() == oldUrl.path());
        CHECK(newUri.host() + newUri.path() == oldUrl.fileName()); // Url mistakenly joins the host and path parts as fileName

        // Test conversion from Uri to Url
        Uri uri("https://example.com/path?query=value#fragment");
        Url url(uri.toString());

        CHECK(url.scheme() == uri.scheme());

        // Local file compatibility
        Url oldFileUrl = Url::fromLocalFile("/path/to/file.txt");
        Uri newFileUri = Uri::fromLocalFile("/path/to/file.txt");

        CHECK(oldFileUrl.isLocalFile());
        CHECK(newFileUri.isLocalFile());
        CHECK(newFileUri.toLocalFile() == oldFileUrl.toLocalFile());
    }

    TEST_CASE("uri validation")
    {
        // Valid URIs
        Uri validHttpUri("http://example.com/path");
        Uri validHttpsUri("https://user:pass@example.com:8443/path?query=value#fragment");
        Uri validFtpUri("ftp://example.com:21/");
        Uri validFileUri = Uri::fromLocalFile("/path/to/file.txt");
        Uri validRelativeUri("/relative/path");
        Uri validPathOnlyUri("path/to/resource");

        CHECK(validHttpUri.isValid());
        CHECK(validHttpsUri.isValid());
        CHECK(validFtpUri.isValid());
        CHECK(validFileUri.isValid());
        CHECK(validRelativeUri.isValid());
        CHECK(validPathOnlyUri.isValid());

        // Invalid URIs
        Uri emptyUri;
        Uri invalidSchemeUri("inv@lid://example.com"); // Invalid scheme characters
        Uri noHostHttpUri("http://"); // HTTP requires a host

        CHECK_FALSE(emptyUri.isValid());
        CHECK_FALSE(invalidSchemeUri.isValid());
        CHECK_FALSE(noHostHttpUri.isValid());

        // Custom validation via scheme handlers
        class CustomSchemeHandler : public UriSchemeHandler
        {
        public:
            std::string defaultPort() const override { return "1234"; }
            bool validate(const Uri &uri) const override
            {
                return uri.path().find(".custom") != std::string::npos;
            }
        };

        UriSchemeRegistry::instance().registerSchemeHandler(
                "custom", std::make_unique<CustomSchemeHandler>());

        Uri validCustomUri("custom://example.com/file.custom");
        Uri invalidCustomUri("custom://example.com/file.txt");

        CHECK(validCustomUri.isValid());
        CHECK_FALSE(invalidCustomUri.isValid());
    }
}
