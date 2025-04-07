/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDNetwork/kdnetwork_export.h>
#include <KDFoundation/config.h>

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <variant>

// Platform-specific includes
#if defined(KD_PLATFORM_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#define KDNETWORK_UNDEF_WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#define KDNETWORK_UNDEF_NOMINMAX
#endif // NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h> // For sockaddr_in, sockaddr_in6
#else
#include <arpa/inet.h> // For inet_pton, inet_ntop
#include <netinet/in.h> // For sockaddr_in, sockaddr_in6
#include <sys/socket.h> // For sockaddr, socklen_t
#endif

namespace KDNetwork {

/**
 * @brief The IpAddress class represents an IPv4 or IPv6 address.
 *
 * This class provides functionality to work with IP addresses including
 * construction from various formats, comparison, and utility functions
 * to determine address properties (loopback, broadcast, etc.).
 */
class KDNETWORK_EXPORT IpAddress
{
public:
    /**
     * @brief Enumeration of IP address types
     */
    enum class Type {
        Invalid, ///< Invalid or uninitialized address
        IPv4, ///< IPv4 address
        IPv6 ///< IPv6 address
    };

    /**
     * @brief Default constructor creates a null/invalid address
     */
    IpAddress();

    /**
     * @brief Construct an IpAddress from a string representation
     *
     * @param address String representation of an IPv4 or IPv6 address
     */
    explicit IpAddress(std::string_view address);

    /**
     * @brief Construct an IPv4 IpAddress from a 32-bit integer
     *
     * @param ipv4Addr 32-bit IPv4 address in host byte order
     */
    explicit IpAddress(std::uint32_t ipv4Addr);

    /**
     * @brief Construct an IpAddress from a sockaddr structure
     *
     * @param sockaddr Pointer to a sockaddr structure (sockaddr_in, sockaddr_in6)
     * @param len Length of the sockaddr structure
     */
    IpAddress(const struct sockaddr *sockaddr, socklen_t len);

    /**
     * @brief Construct an IPv6 IpAddress from a 16-byte array
     *
     * @param ipv6Addr 16-byte array containing the IPv6 address
     */
    explicit IpAddress(const std::array<std::uint8_t, 16> &ipv6Addr);

    /**
     * @brief Returns the type of IP address (IPv4, IPv6, or Invalid)
     */
    Type type() const noexcept;

    /**
     * @brief Returns whether the address is valid (not Invalid)
     */
    bool isValid() const noexcept { return type() != Type::Invalid; }

    /**
     * @brief Converts the IP address to a string representation
     *
     * @return String representation of the IP address
     */
    std::string toString() const;

    /**
     * @brief Returns whether the address is a null/invalid address
     */
    bool isNull() const noexcept;

    /**
     * @brief Returns whether the address is a loopback address (127.0.0.1 for IPv4, ::1 for IPv6)
     */
    bool isLoopback() const noexcept;

    /**
     * @brief Returns whether the address is a broadcast address (255.255.255.255 for IPv4)
     *
     * @note IPv6 does not have a broadcast address, so this will always return false for IPv6
     */
    bool isBroadcast() const noexcept;

    /**
     * @brief Returns whether the address is a link-local address
     *
     * IPv4 link-local addresses are in the range 169.254.0.0/16
     * IPv6 link-local addresses start with fe80::/10
     */
    bool isLinkLocal() const noexcept;

    /**
     * @brief Returns whether the address is within the specified subnet
     *
     * @param subnet The subnet address
     * @param prefixLength The prefix length (e.g., 24 for a /24 subnet)
     * @return True if this address is within the specified subnet
     */
    bool isWithinSubnet(const IpAddress &subnet, int prefixLength) const;

    /**
     * @brief Returns whether the address is a multicast address
     *
     * IPv4 multicast addresses are in the range 224.0.0.0/4
     * IPv6 multicast addresses start with ff00::/8
     */
    bool isMulticast() const noexcept;

    /**
     * @brief Returns whether the address is a private/local address
     *
     * IPv4 private addresses:
     *  - 10.0.0.0/8
     *  - 172.16.0.0/12
     *  - 192.168.0.0/16
     *
     * IPv6 private addresses:
     *  - fc00::/7 (Unique Local Addresses)
     */
    bool isPrivate() const noexcept;

    /**
     * @brief Returns whether the address is an IPv4 address
     */
    bool isIPv4() const noexcept;

    /**
     * @brief Returns whether the address is an IPv6 address
     */
    bool isIPv6() const noexcept;

    /**
     * @brief Get the IPv4 address as a 32-bit unsigned integer
     *
     * @return The IPv4 address in host byte order
     * @note If this is not an IPv4 address, returns 0
     */
    std::uint32_t toIPv4() const noexcept;

    /**
     * @brief Get the IPv6 address as a 16-byte array
     *
     * @return The IPv6 address as a 16-byte array
     * @note If this is not an IPv6 address, returns an array of zeros
     */
    std::array<std::uint8_t, 16> toIPv6() const noexcept;

    /**
     * @brief Convert the address to a sockaddr_in or sockaddr_in6 structure
     *
     * @param sockaddr Pointer to a sockaddr structure to fill
     * @param len Length of the sockaddr structure (will be updated with actual length)
     * @param port Optional port number to include in the sockaddr
     * @return True if the conversion was successful
     */
    bool toSockAddr(struct sockaddr *sockaddr, socklen_t &len, std::uint16_t port = 0) const;

    /**
     * @brief Create a loopback address
     *
     * @param type The type of address to create (IPv4 or IPv6)
     * @return IpAddress A loopback address (127.0.0.1 for IPv4, ::1 for IPv6)
     */
    static IpAddress loopback(Type type = Type::IPv4);

    /**
     * @brief Create a broadcast address (255.255.255.255)
     *
     * @return IpAddress The IPv4 broadcast address
     */
    static IpAddress broadcast();

    /**
     * @brief Create an any address (0.0.0.0 for IPv4, :: for IPv6)
     *
     * @param type The type of address to create (IPv4 or IPv6)
     * @return IpAddress An "any" address
     */
    static IpAddress any(Type type = Type::IPv4);

    /**
     * @brief Create a localhost address
     *
     * @param type The type of address to create (IPv4 or IPv6)
     * @return IpAddress A localhost address (127.0.0.1 for IPv4, ::1 for IPv6)
     */
    static IpAddress localhost(Type type = Type::IPv4);

    /**
     * @brief Assignment operator for string address
     */
    IpAddress &operator=(std::string_view address);

    /**
     * @brief Assignment operator for 32-bit IPv4 address
     */
    IpAddress &operator=(std::uint32_t ipv4Addr);

    /**
     * @brief Equality operator
     */
    bool operator==(const IpAddress &other) const;

    /**
     * @brief Inequality operator
     */
    bool operator!=(const IpAddress &other) const;

    /**
     * @brief Less-than operator for ordered containers
     */
    bool operator<(const IpAddress &other) const;

private:
    // Using a variant to store either IPv4 (uint32_t) or IPv6 (16-byte array) data
    using IPv4Data = std::uint32_t;
    using IPv6Data = std::array<std::uint8_t, 16>;
    std::variant<std::monostate, IPv4Data, IPv6Data> m_data;
};

} // namespace KDNetwork

#ifdef KDNETWORK_UNDEF_WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#undef KDNETWORK_UNDEF_WIN32_LEAN_AND_MEAN
#endif // KDNETWORK_UNDEF_WIN32_LEAN_AND_MEAN

#ifdef KDNETWORK_UNDEF_NOMINMAX
#undef NOMINMAX
#undef KDNETWORK_UNDEF_NOMINMAX
#endif // KDNETWORK_UNDEF_NOMINMAX
