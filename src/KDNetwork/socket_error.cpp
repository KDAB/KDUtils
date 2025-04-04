/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "socket.h"

namespace KDNetwork {

// Define a custom error category for SocketError
class SocketErrorCategoryImpl : public std::error_category
{
public:
    const char *name() const noexcept override { return "KDFoundation::Net::Socket"; }
    std::string message(int ev) const override
    {
        switch (static_cast<SocketError>(ev)) {
        case SocketError::NoError:
            return "No error";
        case SocketError::SocketCreationError:
            return "Socket creation failed";
        case SocketError::BindError:
            return "Socket bind failed";
        case SocketError::ListenError:
            return "Socket listen failed";
        case SocketError::ConnectError:
            return "Socket connect failed";
        case SocketError::ReadError:
            return "Socket read error";
        case SocketError::WriteError:
            return "Socket write error";
        case SocketError::CloseError:
            return "Socket close error";
        case SocketError::SetNonBlockingError:
            return "Failed to set non-blocking mode";
        case SocketError::InvalidSocketError:
            return "Operation on invalid socket";
        case SocketError::UnsupportedOperationError:
            return "Unsupported operation";
        case SocketError::AddressResolutionError:
            return "Address resolution failed";
        case SocketError::TlsHandshakeError:
            return "TLS handshake failed";
        case SocketError::UnknownError:
            return "Unknown socket error";
        default:
            return "Unrecognized socket error code";
        }
    }
};

// Global instance of the category
const SocketErrorCategoryImpl socketErrorCategoryInstance{};

// Factory function for std::error_code
std::error_code make_error_code(SocketError e)
{
    return { static_cast<int>(e), socketErrorCategoryInstance };
}

} // namespace KDNetwork
