/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDNetwork/kdnetwork_export.h>
#include <KDUtils/bytearray.h>

#include <cstdint>
#include <optional>
#include <string>

namespace KDNetwork {

/**
 * @brief The WebSocketFrame class represents a WebSocket protocol frame
 *
 * This class handles the encoding and decoding of WebSocket frames
 * as defined in RFC 6455 (https://tools.ietf.org/html/rfc6455).
 * It's used internally by the WebSocketClient.
 */
class KDNETWORK_EXPORT WebSocketFrame
{
public:
    /**
     * @brief Operation codes defined by the WebSocket protocol
     */
    enum class OpCode : uint8_t {
        Continuation = 0x0, ///< Continuation frame
        Text = 0x1, ///< Text frame
        Binary = 0x2, ///< Binary frame
        Close = 0x8, ///< Connection close frame
        Ping = 0x9, ///< Ping frame
        Pong = 0xA ///< Pong frame
    };

    /**
     * @brief Default constructor
     */
    WebSocketFrame();

    /**
     * @brief Create a text message frame
     *
     * @param text The text message
     * @param isFinalFragment True if this is the final fragment in a message
     * @return A WebSocketFrame configured for the text message
     */
    static WebSocketFrame createTextFrame(const std::string &text, bool isFinalFragment = true);

    /**
     * @brief Create a binary message frame
     *
     * @param data The binary data
     * @param isFinalFragment True if this is the final fragment in a message
     * @return A WebSocketFrame configured for the binary message
     */
    static WebSocketFrame createBinaryFrame(const KDUtils::ByteArray &data, bool isFinalFragment = true);

    /**
     * @brief Create a close frame
     *
     * @param code The close status code (https://tools.ietf.org/html/rfc6455#section-7.4)
     * @param reason A human-readable reason for closing
     * @return A WebSocketFrame configured for the close message
     */
    static WebSocketFrame createCloseFrame(uint16_t code, const std::string &reason);

    /**
     * @brief Create a ping frame
     *
     * @param payload Optional application data (max 125 bytes)
     * @return A WebSocketFrame configured for a ping
     */
    static WebSocketFrame createPingFrame(const KDUtils::ByteArray &payload);

    /**
     * @brief Create a pong frame
     *
     * @param payload The payload from the ping frame being responded to
     * @return A WebSocketFrame configured for a pong
     */
    static WebSocketFrame createPongFrame(const KDUtils::ByteArray &payload);

    /**
     * @brief Get the operation code for this frame
     */
    OpCode opCode() const;

    /**
     * @brief Check if this is the final fragment in a message
     */
    bool isFinal() const;

    /**
     * @brief Get the payload data
     */
    KDUtils::ByteArray payload() const;

    /**
     * @brief Encode the frame for sending
     *
     * @param maskFrame Whether to mask the frame (required for client-to-server)
     * @return The encoded frame data
     */
    KDUtils::ByteArray encode(bool maskFrame = true) const;

    /**
     * @brief Decode a WebSocket frame from raw data
     *
     * @param data The raw data buffer
     * @param bytesProcessed Output parameter: number of bytes consumed from the buffer
     * @return The decoded WebSocketFrame, or nullopt if more data is needed
     */
    static std::optional<WebSocketFrame> decode(const KDUtils::ByteArray &data, size_t &bytesProcessed);

private:
    OpCode m_opCode = OpCode::Text;
    bool m_final = true;
    KDUtils::ByteArray m_payload;

    // For internal construction
    WebSocketFrame(OpCode opCode, bool isFinal, const KDUtils::ByteArray &payload);
};

} // namespace KDNetwork
