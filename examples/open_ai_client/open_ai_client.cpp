/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "open_ai_client.h"

#include <KDNetwork/http_response.h>
#include <KDNetwork/http_session.h>

#include <KDUtils/bytearray.h>
#include <KDUtils/logging.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

using namespace KDUtils;
using namespace KDNetwork;

OpenAiClient::OpenAiClient(const std::string_view &apiKey)
    : m_apiKey(apiKey)
{
    m_client.session()->setDefaultHeader("Authorization", "Bearer " + m_apiKey);
    m_client.session()->setDefaultHeader("Content-Type", "application/json");
}

OpenAiClient::~OpenAiClient()
{
    m_client.cancelAll();
}

bool OpenAiClient::createResponse(const std::string_view &prompt, std::function<void(const std::string &)> callback)
{
    const Uri url = m_baseUrl.resolved(Uri{ "responses" });

    json message = json::array();
    message.push_back({ { "role", "user" }, { "content", prompt } });

    const json body = json::object({ { "model", model() },
                                     { "input", message },
                                     { "instructions", instruction() } });
    const ByteArray bodyPayload(body.dump());

    m_client.post(url, bodyPayload, [callback](const HttpResponse &response) {
        if (response.isSuccessful()) {
            auto jsonResponse = json::parse(response.bodyAsString());
            KDUtils::Logger::logger("OpenAiClient")->debug("Response: " + jsonResponse.dump(2));
            if (jsonResponse["/output/0/content/0/text"_json_pointer].is_null()) {
                callback("Sorry, I couldn't get a response from OpenAI.");
                return;
            }
            const std::string responseText = jsonResponse["/output/0/content/0/text"_json_pointer];
            callback(responseText);
        } else {
            callback("Sorry, I couldn't get a response from OpenAI.");
        }
    });

    return true;
}
