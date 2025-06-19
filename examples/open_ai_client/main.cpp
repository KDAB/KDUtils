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

#include <cstdlib>
#include <iostream>

using namespace KDNetwork;
using namespace KDFoundation;
using namespace KDUtils;

namespace {
// NOLINTBEGIN(concurrency-mt-unsafe)
std::string getOpenAiKey()
{
    if (std::getenv("OPENAI_API_KEY") == nullptr) {
        std::cerr << "Please set the OPENAI_API_KEY environment variable." << std::endl;
        return {};
    }
    return std::getenv("OPENAI_API_KEY");
}
// NOLINTEND(concurrency-mt-unsafe)
} // namespace

int main(int /*argc*/, char * /*argv*/[]) // NOLINT(bugprone-exception-escape)
{
    CoreApplication app;

    // Check if the OpenAI API key is set
    const std::string openAiKey = getOpenAiKey();
    if (openAiKey.empty()) {
        return 1; // Exit if the API key is not set
    }
    OpenAiClient openAiClient(openAiKey);

    const std::string message = "Why is swimming such a good sport?";
    std::cout << "Prompt: " << message << std::endl;

    auto outputResponse = [&app](const std::string &response) {
        std::cout << "Response: " << response << std::endl;
        app.quit();
    };

    openAiClient.createResponse(message, outputResponse);

    return app.exec();
}
