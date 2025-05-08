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
    , m_client(std::make_shared<KDNetwork::HttpClient>())
{
    setupSseClient();
}

OpenAiClient::~OpenAiClient()
{
    if (m_sseClient)
        m_sseClient->disconnect();
}

bool OpenAiClient::createResponse(const std::string_view &prompt)
{
    const Uri url = m_baseUrl.resolved(Uri{ "responses" });

    json message = json::array();
    message.push_back({ { "role", "user" }, { "content", prompt } });

    const json body = json::object({ { "model", model() },
                                     { "input", message },
                                     { "instructions", instruction() },
                                     { "stream", true } }); // Enable streaming vis Server Sent Events (SSE)
    const ByteArray bodyPayload(body.dump());

    // Set up the request
    // Note: The SSE client will handle the connection and streaming
    // The HTTP client is used to send the initial request.
    // Note: The SSE client will set the Accept and Cache-Control headers automatically
    // if not already set.
    HttpRequest request(url, HttpMethod::Post);
    request.setHeader("Content-Type", "application/json");
    request.setHeader("Authorization", "Bearer " + m_apiKey);
    request.setBody(bodyPayload);

    m_sseClient->connect(request);

    return true;
}

void OpenAiClient::setupSseClient()
{
    // Set up the SSE client
    m_sseClient = m_client->createSseClient();

    std::ignore = m_sseClient->messageReceived.connect([this](const KDNetwork::SseEvent &event) {
        // For OpenAI API specifically, parse the JSON data according to:
        // https://platform.openai.com/docs/api-reference/responses-streaming
        // TODO: Handle other event types to progressively build up the full OpenAI response object
        if (event.event() == "response.output_text.delta") {
            // Extract the text delta from the event data
            json data = json::parse(event.data());
            const std::string text = data["delta"];
            textReceived.emit(text);
        } else if (event.event() == "response.created") {
            responseCreated.emit();
        } else if (event.event() == "response.completed") {
            // Final event received, stream is complete.
            // Disconnect the SSE client and emit completion signal
            m_sseClient->disconnect();
            responseCompleted.emit();
        } else if (event.event() == "error") {
            // Handle error event
            json data = json::parse(event.data());
            const std::string errorMessage = data["message"];
            errorOccurred.emit(errorMessage);
        } else {
            Logger::logger("OpenAI Client")->warn("Unhandled event type: " + event.event());
        }
    });

    // Connect error handler
    std::ignore = m_sseClient->error.connect([](const std::string &error) {
        Logger::logger("OpenAI Client")->error("SSE Error: " + error);
    });

    // Set up connection/disconnection handlers
    std::ignore = m_sseClient->connected.connect([]() {
        Logger::logger("OpenAI Client")->info("Connected to SSE stream");
    });

    std::ignore = m_sseClient->disconnected.connect([]() {
        Logger::logger("OpenAI Client")->info("Disconnected from SSE stream");
    });
}
