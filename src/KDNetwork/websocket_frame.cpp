/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/websocket_frame.h>
#include <KDNetwork/websocket_common.h>

#include <algorithm>
#include <cstring>
#include <random>

namespace KDNetwork {

WebSocketFrame::WebSocketFrame()
    : m_opCode(OpCode::Text)
    , m_final(true)
{
}

WebSocketFrame::WebSocketFrame(OpCode opCode, bool isFinal, const KDUtils::ByteArray &payload)
    : m_opCode(opCode)
    , m_final(isFinal)
    , m_payload(payload)
{
    // Control frames (Close, Ping, Pong) must have a payload <= 125 bytes and cannot be fragmented
    if (m_opCode == OpCode::Close || m_opCode == OpCode::Ping || m_opCode == OpCode::Pong) {
        if (m_payload.size() > WebSocket::MAX_CONTROL_FRAME_PAYLOAD) {
            m_payload = m_payload.left(WebSocket::MAX_CONTROL_FRAME_PAYLOAD);
        }
        m_final = true; // Control frames must be final
    }
}

WebSocketFrame WebSocketFrame::createTextFrame(const std::string &text, bool isFinalFragment)
{
    return WebSocketFrame(OpCode::Text, isFinalFragment, KDUtils::ByteArray(text.data(), text.size()));
}

WebSocketFrame WebSocketFrame::createBinaryFrame(const KDUtils::ByteArray &data, bool isFinalFragment)
{
    return WebSocketFrame(OpCode::Binary, isFinalFragment, data);
}

WebSocketFrame WebSocketFrame::createCloseFrame(uint16_t code, const std::string &reason)
{
    KDUtils::ByteArray payload;
    if (code != 0) {
        // Add the status code (network byte order - big endian)
        payload.append(static_cast<uint8_t>((code >> 8) & 0xFF));
        payload.append(static_cast<uint8_t>(code & 0xFF));

        // Add the reason if provided
        if (!reason.empty()) {
            payload.append(reason);
        }
    }

    return WebSocketFrame(OpCode::Close, true, payload);
}

WebSocketFrame WebSocketFrame::createPingFrame(const KDUtils::ByteArray &payload)
{
    return WebSocketFrame(OpCode::Ping, true, payload);
}

WebSocketFrame WebSocketFrame::createPongFrame(const KDUtils::ByteArray &payload)
{
    return WebSocketFrame(OpCode::Pong, true, payload);
}

WebSocketFrame::OpCode WebSocketFrame::opCode() const
{
    return m_opCode;
}

bool WebSocketFrame::isFinal() const
{
    return m_final;
}

KDUtils::ByteArray WebSocketFrame::payload() const
{
    return m_payload;
}

KDUtils::ByteArray WebSocketFrame::encode(bool maskFrame) const
{
    KDUtils::ByteArray frame;

    // First byte: FIN bit (bit 0) + RSV1-3 (bits 1-3) + opcode (bits 4-7)
    uint8_t firstByte = static_cast<uint8_t>(m_opCode) & 0x0F;
    if (m_final) {
        firstByte |= 0x80; // Set FIN bit
    }
    frame.append(firstByte);

    // Second byte: MASK bit (bit 0) + payload length (bits 1-7)
    size_t payloadLen = m_payload.size();
    uint8_t secondByte = 0;

    if (maskFrame) {
        secondByte |= 0x80; // Set MASK bit
    }

    // Encode the payload length
    if (payloadLen <= 125) {
        secondByte |= static_cast<uint8_t>(payloadLen);
        frame.append(secondByte);
    } else if (payloadLen <= 65535) {
        secondByte |= 126; // Use 16-bit length
        frame.append(secondByte);
        frame.append(static_cast<uint8_t>((payloadLen >> 8) & 0xFF));
        frame.append(static_cast<uint8_t>(payloadLen & 0xFF));
    } else {
        secondByte |= 127; // Use 64-bit length
        frame.append(secondByte);

        // Write 64-bit length (network byte order - big endian)
        for (int i = 7; i >= 0; --i) {
            frame.append(static_cast<uint8_t>((payloadLen >> (i * 8)) & 0xFF));
        }
    }

    // Add masking key if needed
    uint8_t maskingKey[4] = { 0 };
    if (maskFrame) {
        // Generate random masking key
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, 255);

        for (int i = 0; i < 4; ++i) {
            maskingKey[i] = static_cast<uint8_t>(dist(gen));
        }

        frame.append(maskingKey, 4);
    }

    // Add payload (masked if required)
    if (maskFrame && !m_payload.isEmpty()) {
        KDUtils::ByteArray maskedPayload = m_payload;
        for (size_t i = 0; i < maskedPayload.size(); ++i) {
            maskedPayload[i] ^= maskingKey[i % 4];
        }
        frame.append(maskedPayload);
    } else {
        frame.append(m_payload);
    }

    return frame;
}

std::optional<WebSocketFrame> WebSocketFrame::decode(const KDUtils::ByteArray &data, size_t &bytesProcessed)
{
    bytesProcessed = 0;

    // We need at least 2 bytes for the header
    if (data.size() < 2) {
        return std::nullopt;
    }

    // Parse first byte
    uint8_t firstByte = data[0];
    bool fin = (firstByte & 0x80) != 0;
    // uint8_t rsv = (firstByte & 0x70) >> 4; // RSV1-3 bits
    OpCode opCode = static_cast<OpCode>(firstByte & 0x0F);

    // Parse second byte
    uint8_t secondByte = data[1];
    bool masked = (secondByte & 0x80) != 0;
    uint8_t payloadLenIndicator = secondByte & 0x7F;

    // Calculate actual header size
    size_t headerSize = 2;
    if (payloadLenIndicator == 126) {
        headerSize += 2; // 16-bit length
    } else if (payloadLenIndicator == 127) {
        headerSize += 8; // 64-bit length
    }

    if (masked) {
        headerSize += 4; // Add space for masking key
    }

    // Check if we have enough data for the header
    if (data.size() < headerSize) {
        return std::nullopt;
    }

    // Extract the payload length
    uint64_t payloadLen = 0;
    if (payloadLenIndicator <= 125) {
        payloadLen = payloadLenIndicator;
    } else if (payloadLenIndicator == 126) {
        // 16-bit length
        payloadLen = (static_cast<uint16_t>(data[2]) << 8) | data[3];
    } else if (payloadLenIndicator == 127) {
        // 64-bit length
        for (int i = 0; i < 8; ++i) {
            payloadLen = (payloadLen << 8) | data[2 + i];
        }
    }

    // Check if the payload is larger than our maximum allowed size
    if (payloadLen > WebSocket::MAX_PAYLOAD_SIZE) {
        // Payload too large, return an error frame
        return WebSocketFrame::createCloseFrame(
                static_cast<uint16_t>(WebSocket::CloseCode::MessageTooBig),
                "Message too large");
    }

    // Check if we have enough data for the full frame
    if (data.size() < headerSize + payloadLen) {
        return std::nullopt;
    }

    // Extract the masking key if present
    uint8_t maskingKey[4] = { 0 };
    if (masked) {
        std::memcpy(maskingKey, data.data() + headerSize - 4, 4);
    }

    // Extract and unmask the payload if necessary
    KDUtils::ByteArray payload;
    if (payloadLen > 0) {
        payload = data.mid(headerSize, payloadLen);

        if (masked) {
            for (size_t i = 0; i < payload.size(); ++i) {
                payload[i] ^= maskingKey[i % 4];
            }
        }
    }

    // Set the total bytes processed
    bytesProcessed = headerSize + payloadLen;

    // Create and return the frame
    return WebSocketFrame(opCode, fin, payload);
}

} // namespace KDNetwork
