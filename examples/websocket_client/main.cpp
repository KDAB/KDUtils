/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/websocket_client.h>
#include <KDNetwork/websocket_common.h>
#include <KDFoundation/core_application.h>

#include <KDUtils/uri.h>
#include <KDUtils/logging.h>
#include <KDUtils/bytearray.h>

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <csignal>

int main(int /*argc*/, char * /*argv*/[])
{
    KDFoundation::CoreApplication app;

    // Set up signal handling
    auto signalHandler = [](int signal) {
        if (signal == SIGINT || signal == SIGTERM) {
            std::cout << "Received signal " << signal << ", shutting down..." << std::endl;
            KDFoundation::CoreApplication::instance()->quit();
        }
    };

    // Register signal handlers for Ctrl+C and termination signals
    std::ignore = std::signal(SIGINT, signalHandler);
    std::ignore = std::signal(SIGTERM, signalHandler);

    // Parse command line arguments
    const std::string url = "wss://echo.websocket.events/";
    std::cout << "WebSocket Client Example" << std::endl;
    std::cout << "Connecting to: " << url << std::endl;
    std::cout << "Type 'exit' to quit. Press Ctrl+C to force quit." << std::endl;

    // Create WebSocket client
    auto client = std::make_shared<KDNetwork::WebSocketClient>();

    // Set up event handlers
    auto requestUserInput = [&app]() {
        std::cout << "Enter a message to send: ";
        std::string input;
        std::getline(std::cin, input);
        if (input.empty() || input == "exit") {
            std::cout << "Exiting..." << std::endl;
            app.quit();
            return std::string();
        }
        return input;
    };

    std::ignore = client->connected.connect([client, requestUserInput]() {
        std::cout << "Connected to WebSocket server" << std::endl;
    });

    std::ignore = client->disconnected.connect([&app](uint16_t code, const std::string &reason) {
        std::cout << "Disconnected from server: Code " << code << " - " << reason << std::endl;
        app.quit();
    });

    std::ignore = client->textMessageReceived.connect([client, requestUserInput](const std::string &message) {
        std::cout << "Received: " << message << std::endl;
        const std::string userMessage = requestUserInput();
        if (!userMessage.empty()) {
            client->sendTextMessage(userMessage);
        }
    });

    std::ignore = client->binaryMessageReceived.connect([](const KDUtils::ByteArray &data) {
        std::cout << "Received binary message of " << data.size() << " bytes" << std::endl;
        std::cout << "Enter a message to send: ";
    });

    std::ignore = client->errorOccurred.connect([&app](const std::string &error) {
        std::cout << "Error: " << error << std::endl;
        app.quit();
    });

    std::ignore = client->pongReceived.connect([](const KDUtils::ByteArray &) {
        // We don't need to print anything for pongs as they're just keep-alive responses
    });

    // Enable auto-reconnect
    client->setAutoReconnect(true);
    client->setMaxReconnectAttempts(3);
    client->setReconnectInterval(std::chrono::seconds(2));

    // Connect to the WebSocket server
    const KDUtils::Uri wsUrl(url);
    client->connectToUrl(wsUrl);

    // Start the application event loop
    return app.exec();
}
