/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/dns_resolver.h>
#include <KDFoundation/config.h>

#include <KDUtils/logging.h>

#include <ares.h>
#include <ares_dns.h>

#include <map>
#include <system_error>

// Platform-specific includes
#if defined(KD_PLATFORM_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#endif

namespace KDNetwork {

namespace {

// Thread-local storage for DnsResolver instance
thread_local std::unique_ptr<DnsResolver> t_instance;

// Error category for DNS errors
class DnsErrorCategory : public std::error_category
{
public:
    const char *name() const noexcept override
    {
        return "dns";
    }

    std::string message(int ev) const override
    {
        return ares_strerror(ev);
    }
};

const DnsErrorCategory dnsErrorCategory{};

std::error_code make_error_code(int code)
{
    return std::error_code(code, dnsErrorCategory);
}

// Helper function to convert ares_addrinfo to AddressInfoList
DnsResolver::AddressInfoList addrInfoToList(const ares_addrinfo *addrInfo)
{
    DnsResolver::AddressInfoList addresses;

    if (!addrInfo)
        return addresses;

    for (const ares_addrinfo_node *node = addrInfo->nodes; node; node = node->ai_next) {
        switch (node->ai_family) {
        case AF_INET: {
            const sockaddr_in *addr = reinterpret_cast<const sockaddr_in *>(node->ai_addr);
            addresses.emplace_back(IpAddress(reinterpret_cast<const struct sockaddr *>(addr), sizeof(sockaddr_in)));
            break;
        }
        case AF_INET6: {
            const sockaddr_in6 *addr = reinterpret_cast<const sockaddr_in6 *>(node->ai_addr);
            addresses.emplace_back(IpAddress(reinterpret_cast<const struct sockaddr *>(addr), sizeof(sockaddr_in6)));
            break;
        }
        default:
            // Unsupported address family
            continue;
        }
    }

    return addresses;
}

} // anonymous namespace

/**
 * @brief Gets the thread-local singleton instance of DnsResolver
 *
 * This method ensures that each thread has its own DnsResolver instance,
 * following c-ares recommendations while maintaining thread safety.
 *
 * @return DnsResolver& Reference to the thread-local DnsResolver instance
 */
DnsResolver &DnsResolver::instance()
{
    if (!t_instance) {
        t_instance = std::make_unique<DnsResolver>();
        KDUtils::Logger::logger("KDNetwork")->debug("Created thread-local DnsResolver instance");
    }
    return *t_instance;
}

DnsResolver::DnsResolver()
{
    if (!initializeAres())
        KDUtils::Logger::logger("KDNetwork")->error("Failed to initialize c-ares library");
}

DnsResolver::~DnsResolver()
{
    cleanupAres();
}

/**
 * @brief Performs an asynchronous DNS lookup for the specified hostname
 *
 * @param hostname The hostname to resolve
 * @param callback Function to be called when the lookup is complete
 * @return true if the lookup was initiated successfully, false otherwise
 */
bool DnsResolver::lookup(const std::string &hostname, LookupCallback callback)
{
    if (!m_initialized) {
        if (!initializeAres()) {
            KDUtils::Logger::logger("KDNetwork")->warn("Cannot perform DNS lookup; c-ares not initialized");
            return false;
        }
    }

    uint64_t requestId = m_nextRequestId++;
    m_lookupRequests[requestId] = { hostname, std::move(callback) };

    struct ares_addrinfo_hints hints = {};
    hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
    hints.ai_flags = ARES_AI_CANONNAME; // Return canonical name

    ares_addrinfo_callback callback_func = [](void *arg, int status, int timeouts, struct ares_addrinfo *result) {
        addressInfoCallback(arg, status, timeouts, result);
    };

    // Create context with both resolver pointer and request ID
    auto *context = new CallbackContext{ this, requestId };

    ares_getaddrinfo(m_channel,
                     hostname.c_str(),
                     nullptr, // No service required
                     &hints,
                     callback_func,
                     context);

    return true;
}

/**
 * @brief Cancels all ongoing lookups
 */
void DnsResolver::cancelLookups()
{
    if (m_initialized) {
        ares_cancel(m_channel);
        m_lookupRequests.clear();
    }
}

bool DnsResolver::initializeAres()
{
    int status = ares_library_init(ARES_LIB_INIT_ALL);
    if (status != ARES_SUCCESS) {
        KDUtils::Logger::logger("KDNetwork")->error("c-ares library initialization failed: {}", ares_strerror(status));
        return false;
    }

    ares_options options = {};
    int optmask = 0;

    // Set socket state callback to monitor socket events
    options.sock_state_cb = (ares_sock_state_cb)&DnsResolver::socketStateCallback;
    options.sock_state_cb_data = this;
    optmask |= ARES_OPT_SOCK_STATE_CB;

#if defined(KD_PLATFORM_WIN32)
    // Windows may need additional flags
    optmask |= ARES_OPT_NOROTATE; // Don't rotate DNS servers
#endif

    // Initialize channel
    status = ares_init_options(&m_channel, &options, optmask);
    if (status != ARES_SUCCESS) {
        KDUtils::Logger::logger("KDNetwork")->error("c-ares channel initialization failed: {}", ares_strerror(status));
        ares_library_cleanup();
        return false;
    }

    m_initialized = true;
    return true;
}

void DnsResolver::cleanupAres()
{
    // Clear all notifiers
    m_readNotifiers.clear();
    m_writeNotifiers.clear();

    if (m_initialized) {
        ares_destroy(m_channel);
        m_channel = nullptr;
        ares_library_cleanup();
        m_initialized = false;
    }
}

// Gets called by c-ares when it wants to monitor a socket for read or write events
// or to stop monitoring a socket.
void DnsResolver::socketStateCallback(void *data, int socket, int read, int write)
{
    DnsResolver *resolver = static_cast<DnsResolver *>(data);
    if (!resolver) {
        return;
    }

    // Handle read socket notifications
    if (read) {
        // Socket needs to be monitored for read events
        if (resolver->m_readNotifiers.find(socket) == resolver->m_readNotifiers.end()) {
            auto notifier = std::make_unique<KDFoundation::FileDescriptorNotifier>(
                    socket, KDFoundation::FileDescriptorNotifier::NotificationType::Read);

            std::ignore = notifier->triggered.connect([resolver, socket]() {
                // Process read events for this specific socket
                ares_process_fd(resolver->m_channel, socket, ARES_SOCKET_BAD);
            });

            resolver->m_readNotifiers[socket] = std::move(notifier);
        }
    } else {
        // Stop monitoring for read events
        resolver->m_readNotifiers.erase(socket);
    }

    // Handle write socket notifications
    if (write) {
        // Socket needs to be monitored for write events
        if (resolver->m_writeNotifiers.find(socket) == resolver->m_writeNotifiers.end()) {
            auto notifier = std::make_unique<KDFoundation::FileDescriptorNotifier>(
                    socket, KDFoundation::FileDescriptorNotifier::NotificationType::Write);

            std::ignore = notifier->triggered.connect([resolver, socket]() {
                // Process write events for this specific socket
                ares_process_fd(resolver->m_channel, ARES_SOCKET_BAD, socket);
            });

            resolver->m_writeNotifiers[socket] = std::move(notifier);
        }
    } else {
        // Stop monitoring for write events
        resolver->m_writeNotifiers.erase(socket);
    }
}

void DnsResolver::addressInfoCallback(void *arg, int status, int timeouts, struct ares_addrinfo *result)
{
    // Convert arg back to CallbackContext
    auto *context = static_cast<CallbackContext *>(arg);

    if (!context) {
        KDUtils::Logger::logger("KDNetwork")->error("Invalid context in DNS resolver callback");
        if (result) {
            ares_freeaddrinfo(result);
        }
        return;
    }

    DnsResolver *resolver = context->resolver;
    uint64_t requestId = context->requestId;

    // Free the context as we no longer need it
    delete context;

    if (!resolver) {
        KDUtils::Logger::logger("KDNetwork")->error("NULL resolver in DNS callback");
        if (result) {
            ares_freeaddrinfo(result);
        }
        return;
    }

    // Look up the request in the resolver
    auto it = resolver->m_lookupRequests.find(requestId);
    if (it != resolver->m_lookupRequests.end()) {
        auto callback = std::move(it->second.callback);
        auto hostname = std::move(it->second.hostname);
        resolver->m_lookupRequests.erase(it);

        // Create error code if status is not success
        std::error_code ec;
        if (status != ARES_SUCCESS) {
            ec = make_error_code(status);
            KDUtils::Logger::logger("KDNetwork")->warn("DNS lookup failed for {}: {} (timeout: {})", hostname, ec.message(), timeouts);
            callback(ec, {});
        } else {
            // Convert address information to list of addresses
            auto addresses = addrInfoToList(result);
            KDUtils::Logger::logger("KDNetwork")->debug("DNS lookup succeeded for {}: {} addresses", hostname, addresses.size());
            callback(ec, addresses);
        }

        // Free the result
        if (result) {
            ares_freeaddrinfo(result);
        }
    } else {
        // If we got here, the request was not found (could be canceled)
        KDUtils::Logger::logger("KDNetwork")->warn("DNS resolver callback for unknown request ID: {}", requestId);

        // Free the result
        if (result) {
            ares_freeaddrinfo(result);
        }
    }
}

} // namespace KDNetwork
