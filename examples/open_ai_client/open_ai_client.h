/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDNetwork/http_client.h>

#include <kdbindings/property.h>

class OpenAiClient
{
public:
    KDBindings::Property<std::string> model{ "gpt-4o" };
    KDBindings::Property<std::string> instruction{ "You are a helpful assistant that is helping a customer with a problem. The customer says: " };

    OpenAiClient(const std::string_view &apiKey);
    ~OpenAiClient();

    bool createResponse(const std::string_view &prompt, std::function<void(const std::string &)> callback);

private:
    const KDUtils::Uri m_baseUrl{ "https://api.openai.com/v1/" };
    const std::string m_apiKey;
    KDNetwork::HttpClient m_client;
};
