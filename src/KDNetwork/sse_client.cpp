/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/sse_client.h>
#include <KDNetwork/http_client.h>
#include <KDNetwork/http_response.h>

namespace KDNetwork {

class SseClient::Private
{
public:
    explicit Private(std::shared_ptr<HttpClient> client)
        : httpClient(client ? std::move(client) : std::make_shared<HttpClient>())
        , isConnected(false)
    {
    }

    std::shared_ptr<HttpClient> httpClient;
    bool isConnected;
    std::string lastEventId;
    HttpRequest activeRequest;
    bool isDisconnecting = false; // Flag to track explicit disconnection

    // Internal parser for SSE events
    class SseParser
    {
    public:
        SseParser()
        {
        }

        // Process incoming data and emit complete events
        void processData(const KDUtils::ByteArray &data, const std::function<void(const SseEvent &)> &eventCallback)
        {
            // Append the new data to our buffer
            m_buffer.append(data.toStdString());

            // Process lines
            size_t pos = 0;
            while ((pos = m_buffer.find('\n')) != std::string::npos) {
                // Extract the line
                std::string line = m_buffer.substr(0, pos);
                m_buffer.erase(0, pos + 1); // Remove the line and the newline char

                // Remove trailing \r if present (for CRLF line endings)
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }

                // An empty line marks the end of an event
                if (line.empty()) {
                    if (!m_lastEvent.isEmpty()) {
                        // Emit the complete event
                        eventCallback(m_lastEvent);

                        // Store the last event id for reconnection
                        if (!m_lastEvent.id().empty()) {
                            m_lastEventId = m_lastEvent.id();
                        }

                        // Reset for next event
                        m_lastEvent = SseEvent();
                    }
                    continue;
                }

                // Process the line based on field name
                const size_t colonPos = line.find(':');
                if (colonPos == std::string::npos) {
                    // Line with no colon is treated as a field name with empty value
                    processField(line, "");
                } else {
                    // Extract field name and value
                    const std::string fieldName = line.substr(0, colonPos);
                    std::string fieldValue = colonPos + 1 < line.size() ? line.substr(colonPos + 1) : "";

                    // Skip initial space in the value if present
                    if (!fieldValue.empty() && fieldValue[0] == ' ') {
                        fieldValue = fieldValue.substr(1);
                    }

                    processField(fieldName, fieldValue);
                }
            }
        }

        [[nodiscard]] const std::string &getLastEventId() const
        {
            return m_lastEventId;
        }

    private:
        void processField(const std::string &fieldName, const std::string &fieldValue)
        {
            if (fieldName == "event") {
                m_lastEvent.setEvent(fieldValue);
            } else if (fieldName == "data") {
                // For multiple data fields, append with newline
                std::string currentData = m_lastEvent.data();
                if (!currentData.empty()) {
                    currentData += "\n";
                }
                m_lastEvent.setData(currentData + fieldValue);
            } else if (fieldName == "id") {
                if (fieldValue.find('\0') == std::string::npos) { // Null char is not allowed in IDs
                    m_lastEvent.setId(fieldValue);
                }
            } else if (fieldName == "retry") {
                try {
                    int retryMs = std::stoi(fieldValue);
                    if (retryMs > 0) {
                        m_lastEvent.setRetry(retryMs);
                    }
                } catch (const std::exception &) {
                    // Invalid retry value, ignore
                }
            }
            // Ignore other field names as per the spec
        }

        std::string m_buffer;
        SseEvent m_lastEvent;
        std::string m_lastEventId;
    };

    SseParser parser;
};

SseClient::SseClient(std::shared_ptr<HttpClient> httpClient)
    : d(std::make_unique<Private>(std::move(httpClient)))
{
}

SseClient::~SseClient()
{
    disconnect();
}

void SseClient::connect(const HttpRequest &request)
{
    // Already connected? Disconnect first
    if (d->isConnected) {
        disconnect();
    }

    // Reset the disconnecting flag when starting a new connection
    d->isDisconnecting = false;

    // Make a copy of the request that we can modify
    HttpRequest sseRequest = request;

    // Set required headers for SSE if not already set
    if (!sseRequest.hasHeader("Accept")) {
        sseRequest.setHeader("Accept", "text/event-stream");
    }
    if (!sseRequest.hasHeader("Cache-Control")) {
        sseRequest.setHeader("Cache-Control", "no-cache");
    }

    // If we have a last event ID from a previous connection, include it
    if (!d->lastEventId.empty()) {
        sseRequest.setHeader("Last-Event-ID", d->lastEventId);
    }

    // Store the active request
    d->activeRequest = sseRequest;

    // We'll use a custom response handler to process the headers early
    // This will be called as soon as headers are received
    auto responseCallback = [this](const HttpResponse &response) {
        // Parse the response status
        if (response.statusCode() >= 200 && response.statusCode() < 300) {
            // Successful connection - check content type
            const std::string contentType = response.header("Content-Type");
            if (contentType.find("text/event-stream") != std::string::npos) {
                // This is a valid SSE stream - set connected state and emit signal
                if (!d->isConnected) {
                    d->isConnected = true;
                    connected.emit();
                }
            } else {
                // Wrong content type
                d->isConnected = false;
                error.emit("Invalid content type for SSE: " + contentType);
                disconnected.emit();
            }
        } else {
            // Connection failed with error status
            d->isConnected = false;
            error.emit("HTTP error: " + std::to_string(response.statusCode()) + " " + response.reasonPhrase());
            disconnected.emit();
        }
    };

    // Set up error handling - only emit errors if we're not deliberately disconnecting
    std::ignore = d->httpClient->error.connect([this](const HttpRequest &, const std::string &errorMessage) {
        d->isConnected = false;

        // Only emit error if this wasn't an explicit disconnect
        if (!d->isDisconnecting) {
            error.emit("Connection error: " + errorMessage);
        }

        disconnected.emit();
    });

    // Send the request using the special method that associates this SseClient with the request
    // This allows the HttpClient to call our processDataChunk method directly with each new chunk
    auto future = d->httpClient->sendWithSseClient(sseRequest, shared_from_this(), responseCallback);
}

void SseClient::disconnect()
{
    if (d->isConnected) {
        // Set the disconnecting flag to avoid emitting error on deliberate disconnect
        d->isDisconnecting = true;

        // Cancel the request and close the connection
        d->httpClient->cancelAll();
        d->isConnected = false;
        disconnected.emit();
    }
}

bool SseClient::isConnected() const
{
    return d->isConnected;
}

std::string SseClient::lastEventId() const
{
    return d->lastEventId;
}

void SseClient::processDataChunk(const KDUtils::ByteArray &chunk)
{
    // Instead of processing the full response body each time,
    // we only process the new chunk of data that arrived
    d->parser.processData(chunk, [this](const SseEvent &event) {
        d->lastEventId = event.id(); // Store last event ID
        messageReceived.emit(event);
    });
}

} // namespace KDNetwork
