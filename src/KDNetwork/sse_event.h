/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDNetwork/kdnetwork_export.h>

#include <string>
#include <optional>

namespace KDNetwork {

/**
 * @brief The SseEvent class represents a single Server-Sent Event
 *
 * Server-Sent Events (SSE) follow a specific format defined in the HTML5 specification.
 * Each event consists of optional fields: id, event type, data, and retry.
 */
class KDNETWORK_EXPORT SseEvent
{
public:
    /**
     * @brief Default constructor
     */
    SseEvent() = default;

    /**
     * @brief Constructor with all fields
     *
     * @param eventId The event ID (optional)
     * @param eventType The event type (defaults to "message" if empty)
     * @param eventData The data payload
     * @param retryMs The retry timeout in milliseconds (optional)
     */
    SseEvent(const std::string &eventId,
             const std::string &eventType,
             const std::string &eventData,
             std::optional<int> retryMs = std::nullopt);

    /**
     * @brief Get the event ID
     * @return The event ID, or empty string if no ID was specified
     */
    std::string id() const { return m_id; }

    /**
     * @brief Set the event ID
     * @param eventId The event ID
     */
    void setId(const std::string &eventId) { m_id = eventId; }

    /**
     * @brief Get the event type
     * @return The event type, defaults to "message" if not specified
     */
    std::string event() const { return m_eventType.empty() ? "message" : m_eventType; }

    /**
     * @brief Set the event type
     * @param eventType The event type
     */
    void setEvent(const std::string &eventType) { m_eventType = eventType; }

    /**
     * @brief Get the data payload
     * @return The data payload
     */
    std::string data() const { return m_data; }

    /**
     * @brief Set the data payload
     * @param eventData The data payload
     */
    void setData(const std::string &eventData) { m_data = eventData; }

    /**
     * @brief Get the retry timeout
     * @return The retry timeout in milliseconds, if specified
     */
    std::optional<int> retry() const { return m_retry; }

    /**
     * @brief Set the retry timeout
     * @param retryMs The retry timeout in milliseconds
     */
    void setRetry(std::optional<int> retryMs) { m_retry = retryMs; }

    /**
     * @brief Check if the event is empty
     * @return True if the event has no data, type, or ID
     */
    bool isEmpty() const { return m_id.empty() && m_eventType.empty() && m_data.empty(); }

private:
    std::string m_id; // Event ID
    std::string m_eventType; // Event type
    std::string m_data; // Event data
    std::optional<int> m_retry; // Retry timeout in milliseconds
};

} // namespace KDNetwork
