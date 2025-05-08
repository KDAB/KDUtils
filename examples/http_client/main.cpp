/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/http_client.h>
#include <KDNetwork/http_response.h>
#include <KDNetwork/http_session.h>
#include <KDNetwork/http_cookie_jar.h>

#include <KDFoundation/core_application.h>

#include <KDUtils/uri.h>

#include <iostream>
#include <csignal>
#include <thread>

using namespace KDNetwork;
using namespace KDFoundation;
using namespace KDUtils;

int main(int /*argc*/, char * /*argv*/[])
{
    CoreApplication app;

    // Example 1: Callback-based approach (works with event loop)
    HttpClient client;
    // client.get(Uri("http://localhost:3000/"), [&](const HttpResponse &response) {
    client.get(Uri("https://getstreamline.org/"), [&](const HttpResponse &response) {
        if (response.isSuccessful()) {
            std::cout << "Got response: " << response.bodyAsString() << std::endl;

            auto cookies = client.session()->cookieJar().allCookies();
            for (const auto &cookie : cookies) {
                std::cout << "Cookie: " << cookie.toCookieHeader() << std::endl;
            }

            std::cout << "Status code: " << response.statusCode() << std::endl;
            std::cout << "Reason phrase: " << response.reasonPhrase() << std::endl;

            std::cout << "Headers: " << std::endl;
            for (const auto &header : response.allHeaders()) {
                std::cout << header.first << ": " << header.second << std::endl;
            }
        } else {
            std::cout << "Error: " << response.error() << std::endl;
        }

        // Quit the event loop
        app.quit();
    });

    // Run the event loop - this keeps the network operations running
    return app.exec();
}

// Example 2: Future-based approach. Events processed on main thread. Worker thread waits for response.
// Needs timers and file descriptor notifiers to be called on the main thread.

// Example 3: Worker thread running its own event loop. Needs EventLoop class to be extracted from CoreApplication.

// Example 4: Using a client with a custom user agent and bearer token.
// HttpRequest request("https://api.example.com/protected");
// request.setMethod(HttpMethod::Post);
// request.setBody("{\"key\": \"value\"}");
// request.setHeader("Content-Type", "application/json");
// request.setBearerAuth("my-access-token");
// HttpClient client;
// client.send(request, [](const HttpResponse& response) {
//     // Process response
// });

// Example 5: Using a custom session to manage cookies and connections.
// auto session = std::make_shared<HttpSession>();
// session->setUserAgent("MyApp/1.0");
// HttpClient client(session);
// // First request (establishes connection)
// auto future1 = client.get("https://api.example.com/resource1");
// // Second request (reuses connection)
// auto future2 = client.get("https://api.example.com/resource2");
