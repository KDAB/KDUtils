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

using namespace KDNetwork;
using namespace KDFoundation;
using namespace KDUtils;

int main(int argc, char *argv[])
{
    CoreApplication app;

    HttpClient client;
    client.get(Uri("http://localhost:3000/"), [&](const HttpResponse &response) {
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

            // Quit the event loop
            app.quit();
        } else {
            std::cout << "Error: " << response.error() << std::endl;
        }
    });

    return app.exec();
}

// HttpClient client;
// auto future = client.get("https://api.example.com/data");
// // Do other work...
// auto response = future.get(); // Blocks until response is ready
// if (response.isSuccessful()) {
//     std::cout << "Response: " << response.bodyAsString() << std::endl;
// }

// HttpRequest request("https://api.example.com/protected");
// request.setMethod(HttpMethod::Post);
// request.setBody("{\"key\": \"value\"}");
// request.setHeader("Content-Type", "application/json");
// request.setBearerAuth("my-access-token");
// HttpClient client;
// client.send(request, [](const HttpResponse& response) {
//     // Process response
// });

// auto session = std::make_shared<HttpSession>();
// session->setUserAgent("MyApp/1.0");
// HttpClient client(session);
// // First request (establishes connection)
// auto future1 = client.get("https://api.example.com/resource1");
// // Second request (reuses connection)
// auto future2 = client.get("https://api.example.com/resource2");
