/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "open_ai_client.h"

#include <KDFoundation/core_application.h>

#include <KDUtils/uri.h>

#include <iostream>

using namespace KDNetwork;
using namespace KDFoundation;
using namespace KDUtils;

int main(int argc, char *argv[])
{
    CoreApplication app;

    // Check if the OpenAI API key is set
    if (std::getenv("OPENAI_API_KEY") == nullptr) {
        std::cerr << "Please set the OPENAI_API_KEY environment variable." << std::endl;
        return 1;
    }
    OpenAiClient openAiClient(std::getenv("OPENAI_API_KEY"));

    std::string message = "Why is swimming such a good sport?";
    std::cout << "Prompt: " << message << std::endl;

    auto beginResponse = openAiClient.responseCreated.connect([]() {
        std::cout << "ChatGPT says:" << std::endl;
    });

    auto outputText = openAiClient.textReceived.connect([](const std::string &text) {
        std::cout << text << std::flush;
    });

    auto responseCompleted = openAiClient.responseCompleted.connect([&]() {
        std::cout << std::endl;

        // TODO: Ask for the next input or quit.

        app.quit();
    });

    auto errorOccurred = openAiClient.errorOccurred.connect([](const std::string &error) {
        std::cerr << "Error: " << error << std::endl;
    });

    openAiClient.createResponse(message);

    return app.exec();
}
