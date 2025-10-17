/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDNetwork/kdnetwork_export.h>

#include <cstdint>
#include <string>

namespace KDNetwork {

/**
 * @brief WebSocket protocol constants and utility functions
 */
namespace WebSocket {

/**
 * @brief WebSocket close status codes as defined in RFC 6455
 * @see https://tools.ietf.org/html/rfc6455#section-7.4
 */
enum class CloseCode : uint16_t {
    NormalClosure = 1000, ///< Normal closure; the connection successfully completed
    GoingAway = 1001, ///< The endpoint is going away (server shutdown, browser navigating away)
    ProtocolError = 1002, ///< Protocol error
    UnsupportedData = 1003, ///< Received data of a type that cannot be accepted (e.g., binary only server got text)
    Reserved = 1004, ///< Reserved. The specific meaning might be defined in the future.
    NoStatusReceived = 1005, ///< No status code was provided even though one was expected
    AbnormalClosure = 1006, ///< Connection closed abnormally (no close frame received)
    InvalidPayload = 1007, ///< Received message data inconsistent with type (e.g., non-UTF-8 in text frame)
    PolicyViolation = 1008, ///< Generic status code used when no other applies
    MessageTooBig = 1009, ///< Message too big for processing
    MissingExtension = 1010, ///< Client expected server to negotiate an extension but server didn't
    InternalError = 1011, ///< Server encountered an unexpected condition
    ServiceRestart = 1012, ///< Server is restarting
    TryAgainLater = 1013, ///< Server is temporarily unavailable (e.g., overloaded)
    BadGateway = 1014, ///< Gateway or proxy received invalid response from upstream server
    TlsHandshakeFailed = 1015 ///< TLS handshake failure (not returned directly but used internally)
};

/**
 * @brief Standard WebSocket GUID used for computing the accept key
 * @see https://tools.ietf.org/html/rfc6455#section-1.3
 */
constexpr const char *GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

/**
 * @brief Default ping interval in milliseconds
 */
constexpr int DEFAULT_PING_INTERVAL_MS = 30000;

/**
 * @brief Default close timeout in milliseconds
 */
constexpr int DEFAULT_CLOSE_TIMEOUT_MS = 5000;

/**
 * @brief Maximum allowed payload size
 */
constexpr size_t MAX_PAYLOAD_SIZE = 1024 * 1024; // 1 MB

/**
 * @brief Maximum allowed control frame payload size
 */
constexpr size_t MAX_CONTROL_FRAME_PAYLOAD = 125;

} // namespace WebSocket

} // namespace KDNetwork
