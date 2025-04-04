/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDNetwork/kdnetwork_export.h>

#include <system_error>

namespace KDNetwork {

/**
 * @brief Represents socket-specific errors.
 *
 * Complements system errors (errno) by providing context specific to the socket operations.
 */
enum class SocketError {
    NoError = 0,
    SocketCreationError,
    BindError,
    ListenError, // Primarily for server sockets
    ConnectError, // Primarily for client sockets
    ReadError,
    WriteError,
    CloseError,
    SetNonBlockingError,
    InvalidSocketError, // Operation on an uninitialized/closed socket
    UnsupportedOperationError,
    AddressResolutionError, // For DNS related issues if handled here
    TlsHandshakeError, // For SSL/TLS sockets
    UnknownError
};

// Function to integrate SocketError with std::error_code system
KDNETWORK_EXPORT std::error_code make_error_code(SocketError e);

} // namespace KDNetwork

// Specialization required by <system_error> to recognize SocketError as an error code enum.
namespace std {
template<>
struct is_error_code_enum<KDNetwork::SocketError> : true_type {
};
} // namespace std
