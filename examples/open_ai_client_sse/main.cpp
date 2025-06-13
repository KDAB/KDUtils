/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "open_ai_client_sse.h"

#include <KDFoundation/core_application.h>

#include <KDUtils/uri.h>

#include <iostream>

using namespace KDNetwork;
using namespace KDFoundation;
using namespace KDUtils;

int main(int /*argc*/, char * /*argv*/[])
{
    CoreApplication app;

    // Check if the OpenAI API key is set
    if (std::getenv("OPENAI_API_KEY") == nullptr) {
        std::cerr << "Please set the OPENAI_API_KEY environment variable." << std::endl;
        return 1;
    }
    OpenAiClientSse openAiClient(std::getenv("OPENAI_API_KEY"));

    std::cout << "Welcome to the KDNetwork OpenAI ChatGPT client! Now with streaming!" << std::endl;

    auto requestUserInput = []() {
        std::cout << "Enter your message: ";
        std::string input;
        std::getline(std::cin, input);
        return input;
    };

    auto beginResponse = openAiClient.responseCreated.connect([]() {
        std::cout << "ChatGPT says:" << std::endl;
    });

    auto outputText = openAiClient.textReceived.connect([](const std::string &text) {
        std::cout << text << std::flush;
    });

    // This lambda is called when the response is completed. We can request more user
    // input or exit the application.
    auto responseCompleted = openAiClient.responseCompleted.connect([&]() {
        std::cout << std::endl;

        // Ask for the next input or exit
        const std::string message = requestUserInput();
        if (message.empty() || message == "exit") {
            std::cout << "Exiting..." << std::endl;
            app.quit();
            return;
        }

        openAiClient.createResponse(message);
    });

    auto errorOccurred = openAiClient.errorOccurred.connect([](const std::string &error) {
        std::cerr << "Error: " << error << std::endl;
    });

    const std::string message = requestUserInput();
    if (message.empty() || message == "exit") {
        std::cout << "Exiting..." << std::endl;
        return 0;
    }
    openAiClient.createResponse(message);

    return app.exec();
}
