/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDNetwork/http_client.h>
#include <KDNetwork/sse_client.h>

#include <kdbindings/property.h>

#include <memory>

class OpenAiClientSse
{
public:
    KDBindings::Property<std::string> model{ "gpt-4o" };
    KDBindings::Property<std::string> instruction{ "You are a helpful assistant that is helping a customer with a problem. The customer says: " };

    KDBindings::Signal<std::string> textReceived;
    KDBindings::Signal<> responseCreated;
    KDBindings::Signal<> responseCompleted;
    KDBindings::Signal<std::string> errorOccurred;

    OpenAiClientSse(const std::string_view &apiKey);
    ~OpenAiClientSse();

    bool createResponse(const std::string_view &prompt);

private:
    void setupSseClient();

    const KDUtils::Uri m_baseUrl{ "https://api.openai.com/v1/" };
    const std::string m_apiKey;
    std::shared_ptr<KDNetwork::HttpClient> m_client;
    std::shared_ptr<KDNetwork::SseClient> m_sseClient;
};
