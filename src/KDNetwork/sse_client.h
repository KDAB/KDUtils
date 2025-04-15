/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDNetwork/kdnetwork_export.h>
#include <KDNetwork/http_request.h>
#include <KDNetwork/sse_event.h>

#include <KDBindings/signal.h>

#include <memory>
#include <string>

namespace KDNetwork {

// Forward declarations
class HttpClient;
class HttpResponse;

/**
 * @brief The SseClient class provides functionality for Server-Sent Events connections
 *
 * This class allows receiving real-time events from servers that support
 * Server-Sent Events (SSE) as specified in the HTML5 specification.
 */
class KDNETWORK_EXPORT SseClient : public std::enable_shared_from_this<SseClient>
{
public:
    /**
     * @brief Destructor
     */
    ~SseClient();

    /**
     * @brief Connect to an SSE endpoint
     *
     * @param request The HTTP request to send to the SSE endpoint
     * If Accept header isn't set, it will be set to "text/event-stream"
     * If Cache-Control header isn't set, it will be set to "no-cache"
     */
    void connect(const HttpRequest &request);

    /**
     * @brief Disconnect from the current SSE stream
     */
    void disconnect();

    /**
     * @brief Check if the client is currently connected
     * @return True if connected to an SSE stream
     */
    bool isConnected() const;

    /**
     * @brief Get the last-received event ID
     *
     * This ID can be used when reconnecting to resume the stream
     * from where it left off.
     *
     * @return The last event ID, or empty string if none
     */
    std::string lastEventId() const;

    /**
     * @brief Signal emitted when a message is received
     *
     * This signal is triggered for each complete SSE event that is received.
     */
    KDBindings::Signal<const SseEvent &> messageReceived;

    /**
     * @brief Signal emitted when the connection is established
     */
    KDBindings::Signal<> connected;

    /**
     * @brief Signal emitted when the connection is closed
     */
    KDBindings::Signal<> disconnected;

    /**
     * @brief Signal emitted when an error occurs
     */
    KDBindings::Signal<const std::string &> error;

private:
    // Make constructor private, with HttpClient as friend
    friend class HttpClient;

    /**
     * @brief Constructor - only accessible via HttpClient
     *
     * @param httpClient The HttpClient instance to use for requests
     */
    explicit SseClient(std::shared_ptr<HttpClient> httpClient);

    /**
     * @brief Process a chunk of SSE data
     *
     * This method is called by HttpClient when new data arrives in an SSE stream.
     *
     * @param chunk The new data chunk to process
     */
    void processDataChunk(const KDUtils::ByteArray &chunk);

    // Private implementation data
    class Private;
    std::unique_ptr<Private> d;

    // Flag to track explicit disconnection
    bool isDisconnecting = false;

    // Disable copying and assignment
    SseClient(const SseClient &) = delete;
    SseClient &operator=(const SseClient &) = delete;
};

} // namespace KDNetwork
