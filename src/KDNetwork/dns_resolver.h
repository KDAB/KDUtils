/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDNetwork/kdnetwork_export.h>
#include <KDFoundation/file_descriptor_notifier.h>
#include <KDNetwork/ip_address.h>

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <system_error>
#include <map>
#include <thread>

// Forward declarations
struct ares_channeldata;
typedef struct ares_channeldata *ares_channel;
struct ares_addrinfo;

namespace KDFoundation {
class FileDescriptorNotifier;
}

namespace KDNetwork {

/**
 * @brief The DnsResolver class provides asynchronous DNS resolution using the c-ares library.
 *
 * This class integrates with KDFoundation's event loop via FileDescriptorNotifier to
 * provide non-blocking DNS lookups.
 *
 * According to c-ares recommendations, a single channel should be used for all DNS lookups.
 * This class implements a thread-local singleton pattern to ensure one DnsResolver instance
 * per thread, which is the recommended usage pattern.
 *
 * @note Preferred usage is to call DnsResolver::instance() rather than creating instances directly.
 */
class KDNETWORK_EXPORT DnsResolver
{
public:
    // Per-thread singleton instance
    static DnsResolver &instance();

    explicit DnsResolver();
    virtual ~DnsResolver();

    // Not copyable
    DnsResolver(const DnsResolver &) = delete;
    DnsResolver &operator=(const DnsResolver &) = delete;

    // Movable
    DnsResolver(DnsResolver &&) = default;
    DnsResolver &operator=(DnsResolver &&) = default;

    /**
     * @brief Result list of resolved IP addresses
     *
     * Using IpAddress objects instead of strings allows for better type safety
     * and provides immediate access to all address properties and functionality.
     */
    using AddressInfoList = std::vector<IpAddress>;

    /**
     * @brief Result callback type for DNS lookup operations
     *
     * The first parameter is a std::error_code, which will be set if an error occurred.
     * The second parameter is a list of resolved IP addresses.
     */
    using LookupCallback = std::function<void(std::error_code, const AddressInfoList &)>;

    bool lookup(const std::string &hostname, LookupCallback callback);

    void cancelLookups();

protected:
    // Context structure passed to c-ares callback (addressInfoCallback)
    struct CallbackContext {
        DnsResolver *resolver;
        uint64_t requestId;
    };

    bool initializeAres();
    void cleanupAres();

    // Called when a socket's state changes in the c-ares library
    static void socketStateCallback(void *data, int socket, int read, int write);

    // Static callback for c-ares address info completion
    static void addressInfoCallback(void *arg, int status, int timeouts, struct ares_addrinfo *result);

    // c-ares channel for DNS operations
    ares_channel m_channel{ nullptr };
    bool m_initialized{ false };

    // Track lookup requests by id
    struct LookupRequest {
        std::string hostname;
        LookupCallback callback;
    };
    std::map<uint64_t, LookupRequest> m_lookupRequests;
    uint64_t m_nextRequestId{ 1 };

    // Socket notifier tracking
    std::map<int, std::unique_ptr<KDFoundation::FileDescriptorNotifier>> m_readNotifiers;
    std::map<int, std::unique_ptr<KDFoundation::FileDescriptorNotifier>> m_writeNotifiers;
};

} // namespace KDNetwork
