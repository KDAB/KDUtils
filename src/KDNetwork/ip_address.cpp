/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/ip_address.h>
#include <KDUtils/logging.h>

#include <algorithm>
#include <cstring>
#include <bit>

namespace KDNetwork {

// Default constructor - creates an invalid address
IpAddress::IpAddress()
{
    // m_data already contains std::monostate (Invalid)
}

// Constructor from string_view representation
IpAddress::IpAddress(std::string_view address)
{
    // Since inet_pton requires a null-terminated string, we need to create a temporary std::string
    std::string addrStr(address);

    // Try IPv4 first
    struct in_addr addr4;
    if (inet_pton(AF_INET, addrStr.c_str(), &addr4) == 1) {
        // Successfully parsed as IPv4
        m_data = std::uint32_t(ntohl(addr4.s_addr)); // Convert to host byte order
        return;
    }

    // Try IPv6
    struct in6_addr addr6;
    if (inet_pton(AF_INET6, addrStr.c_str(), &addr6) == 1) {
        // Successfully parsed as IPv6
        std::array<std::uint8_t, 16> ipv6Data;
        std::memcpy(ipv6Data.data(), addr6.s6_addr, 16);
        m_data = ipv6Data;
        return;
    }

    // Failed to parse, keep as Invalid
}

// Constructor from 32-bit IPv4 address
IpAddress::IpAddress(std::uint32_t ipv4Addr)
    : m_data(ipv4Addr)
{
}

// Constructor from sockaddr
IpAddress::IpAddress(const struct sockaddr *sockaddr, socklen_t len)
{
    if (!sockaddr || len < sizeof(struct sockaddr)) {
        // Invalid input, keep as Invalid
        return;
    }

    if (sockaddr->sa_family == AF_INET && len >= sizeof(struct sockaddr_in)) {
        const struct sockaddr_in *addr4 = reinterpret_cast<const struct sockaddr_in *>(sockaddr);
        m_data = std::uint32_t(ntohl(addr4->sin_addr.s_addr)); // Convert to host byte order
    } else if (sockaddr->sa_family == AF_INET6 && len >= sizeof(struct sockaddr_in6)) {
        const struct sockaddr_in6 *addr6 = reinterpret_cast<const struct sockaddr_in6 *>(sockaddr);
        std::array<std::uint8_t, 16> ipv6Data;
        std::memcpy(ipv6Data.data(), addr6->sin6_addr.s6_addr, 16);
        m_data = ipv6Data;
    }
    // else: unsupported address family, keep as Invalid
}

// Constructor from IPv6 16-byte array
IpAddress::IpAddress(const std::array<std::uint8_t, 16> &ipv6Addr)
    : m_data(ipv6Addr)
{
}

// Returns the address type
IpAddress::Type IpAddress::type() const noexcept
{
    if (std::holds_alternative<std::monostate>(m_data)) {
        return Type::Invalid;
    } else if (std::holds_alternative<IPv4Data>(m_data)) {
        return Type::IPv4;
    } else {
        return Type::IPv6;
    }
}

// Converts the address to string representation
std::string IpAddress::toString() const
{
    if (std::holds_alternative<IPv4Data>(m_data)) {
        // IPv4 address
        const std::uint32_t ipv4 = std::get<IPv4Data>(m_data);
        struct in_addr addr{};
        addr.s_addr = htonl(ipv4); // Convert to network byte order

        char buf[INET_ADDRSTRLEN] = {};
        if (inet_ntop(AF_INET, &addr, buf, sizeof(buf))) {
            return buf;
        }
    } else if (std::holds_alternative<IPv6Data>(m_data)) {
        // IPv6 address
        const auto &ipv6 = std::get<IPv6Data>(m_data);
        struct in6_addr addr{};
        std::memcpy(addr.s6_addr, ipv6.data(), 16);

        char buf[INET6_ADDRSTRLEN] = {};
        if (inet_ntop(AF_INET6, &addr, buf, sizeof(buf))) {
            return buf;
        }
    }

    return ""; // Invalid address
}

// Checks if address is null (invalid)
bool IpAddress::isNull() const noexcept
{
    return std::holds_alternative<std::monostate>(m_data);
}

// Checks if address is a loopback address
bool IpAddress::isLoopback() const noexcept
{
    if (std::holds_alternative<IPv4Data>(m_data)) {
        // IPv4 loopback: 127.0.0.0/8
        const std::uint32_t ipv4 = std::get<IPv4Data>(m_data);
        return (ipv4 & 0xFF000000) == 0x7F000000;
    } else if (std::holds_alternative<IPv6Data>(m_data)) {
        // IPv6 loopback: ::1
        const auto &ipv6 = std::get<IPv6Data>(m_data);

        // All bytes should be 0 except the last one which should be 1
        for (size_t i = 0; i < 15; ++i) {
            if (ipv6[i] != 0) {
                return false;
            }
        }
        return ipv6[15] == 1;
    }

    return false; // Invalid address
}

// Checks if address is a broadcast address (IPv4 only)
bool IpAddress::isBroadcast() const noexcept
{
    if (std::holds_alternative<IPv4Data>(m_data)) {
        // IPv4 broadcast: 255.255.255.255
        const std::uint32_t ipv4 = std::get<IPv4Data>(m_data);
        return ipv4 == 0xFFFFFFFF;
    }

    return false; // IPv6 doesn't have broadcast addresses
}

// Checks if address is a link-local address
bool IpAddress::isLinkLocal() const noexcept
{
    if (std::holds_alternative<IPv4Data>(m_data)) {
        // IPv4 link-local: 169.254.0.0/16
        const std::uint32_t ipv4 = std::get<IPv4Data>(m_data);
        return (ipv4 & 0xFFFF0000) == 0xA9FE0000;
    } else if (std::holds_alternative<IPv6Data>(m_data)) {
        // IPv6 link-local: fe80::/10
        const auto &ipv6 = std::get<IPv6Data>(m_data);
        return (ipv6[0] == 0xFE) && ((ipv6[1] & 0xC0) == 0x80);
    }

    return false; // Invalid address
}

// Checks if address is within the specified subnet
bool IpAddress::isWithinSubnet(const IpAddress &subnet, int prefixLength) const
{
    // Ensure addresses are of the same type
    if (type() != subnet.type() || isNull() || subnet.isNull()) {
        return false;
    }

    if (std::holds_alternative<IPv4Data>(m_data)) {
        // IPv4 subnet check
        if (prefixLength < 0 || prefixLength > 32) {
            return false; // Invalid prefix length
        }

        const std::uint32_t mask = prefixLength == 0 ? 0 : (0xFFFFFFFF << (32 - prefixLength));
        const std::uint32_t addr = std::get<IPv4Data>(m_data);
        const std::uint32_t subnetAddr = std::get<IPv4Data>(subnet.m_data);

        return (addr & mask) == (subnetAddr & mask);
    } else if (std::holds_alternative<IPv6Data>(m_data)) {
        // IPv6 subnet check
        if (prefixLength < 0 || prefixLength > 128) {
            return false; // Invalid prefix length
        }

        const auto &addr = std::get<IPv6Data>(m_data);
        const auto &subnetAddr = std::get<IPv6Data>(subnet.m_data);

        // Check each byte according to the prefix length
        int bytesFullyInPrefix = prefixLength / 8;
        int remainingBits = prefixLength % 8;

        // Check the fully covered bytes
        for (int i = 0; i < bytesFullyInPrefix; ++i) {
            if (addr[i] != subnetAddr[i]) {
                return false;
            }
        }

        // Check the partially covered byte, if any
        if (remainingBits > 0 && bytesFullyInPrefix < 16) {
            std::uint8_t mask = 0xFF << (8 - remainingBits);
            if ((addr[bytesFullyInPrefix] & mask) != (subnetAddr[bytesFullyInPrefix] & mask)) {
                return false;
            }
        }

        return true;
    }

    return false; // Invalid address
}

// Checks if address is a multicast address
bool IpAddress::isMulticast() const noexcept
{
    if (std::holds_alternative<IPv4Data>(m_data)) {
        // IPv4 multicast: 224.0.0.0/4
        const std::uint32_t ipv4 = std::get<IPv4Data>(m_data);
        return (ipv4 & 0xF0000000) == 0xE0000000;
    } else if (std::holds_alternative<IPv6Data>(m_data)) {
        // IPv6 multicast: ff00::/8
        const auto &ipv6 = std::get<IPv6Data>(m_data);
        return ipv6[0] == 0xFF;
    }

    return false; // Invalid address
}

// Checks if address is a private address
bool IpAddress::isPrivate() const noexcept
{
    if (std::holds_alternative<IPv4Data>(m_data)) {
        const std::uint32_t ipv4 = std::get<IPv4Data>(m_data);

        // 10.0.0.0/8
        if ((ipv4 & 0xFF000000) == 0x0A000000) {
            return true;
        }

        // 172.16.0.0/12
        if ((ipv4 & 0xFFF00000) == 0xAC100000) {
            return true;
        }

        // 192.168.0.0/16
        if ((ipv4 & 0xFFFF0000) == 0xC0A80000) {
            return true;
        }

        return false;
    } else if (std::holds_alternative<IPv6Data>(m_data)) {
        // IPv6 ULA: fc00::/7
        const auto &ipv6 = std::get<IPv6Data>(m_data);
        return (ipv6[0] & 0xFE) == 0xFC;
    }

    return false; // Invalid address
}

// Checks if address is IPv4
bool IpAddress::isIPv4() const noexcept
{
    return std::holds_alternative<IPv4Data>(m_data);
}

// Checks if address is IPv6
bool IpAddress::isIPv6() const noexcept
{
    return std::holds_alternative<IPv6Data>(m_data);
}

// Returns the IPv4 address as a 32-bit integer
std::uint32_t IpAddress::toIPv4() const noexcept
{
    if (std::holds_alternative<IPv4Data>(m_data)) {
        return std::get<IPv4Data>(m_data);
    }
    return 0; // Not an IPv4 address
}

// Returns the IPv6 address as a 16-byte array
std::array<std::uint8_t, 16> IpAddress::toIPv6() const noexcept
{
    if (std::holds_alternative<IPv6Data>(m_data)) {
        return std::get<IPv6Data>(m_data);
    }
    return std::array<std::uint8_t, 16>{}; // Not an IPv6 address, return zeros
}

// Converts the address to a sockaddr structure
bool IpAddress::toSockAddr(struct sockaddr *sockaddr, socklen_t &len, std::uint16_t port) const
{
    if (!sockaddr) {
        return false;
    }

    if (std::holds_alternative<IPv4Data>(m_data)) {
        // IPv4
        if (len < sizeof(struct sockaddr_in)) {
            return false; // Buffer too small
        }

        struct sockaddr_in *addr4 = reinterpret_cast<struct sockaddr_in *>(sockaddr);
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(port);
        addr4->sin_addr.s_addr = htonl(std::get<IPv4Data>(m_data)); // Convert to network byte order

        // Some implementations need this to be zeroed
        std::memset(addr4->sin_zero, 0, sizeof(addr4->sin_zero));

        len = sizeof(struct sockaddr_in);
        return true;
    } else if (std::holds_alternative<IPv6Data>(m_data)) {
        // IPv6
        if (len < sizeof(struct sockaddr_in6)) {
            return false; // Buffer too small
        }

        struct sockaddr_in6 *addr6 = reinterpret_cast<struct sockaddr_in6 *>(sockaddr);
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(port);
        addr6->sin6_flowinfo = 0; // Flow information

        // Copy the address bytes
        const auto &ipv6Data = std::get<IPv6Data>(m_data);
        std::memcpy(addr6->sin6_addr.s6_addr, ipv6Data.data(), 16);

        // Set scope ID to 0 (global scope)
        addr6->sin6_scope_id = 0;

        len = sizeof(struct sockaddr_in6);
        return true;
    }

    return false; // Invalid address
}

// Creates a loopback address
IpAddress IpAddress::loopback(Type type)
{
    if (type == Type::IPv4) {
        // IPv4 loopback: 127.0.0.1
        return IpAddress(0x7F000001);
    } else if (type == Type::IPv6) {
        // IPv6 loopback: ::1
        std::array<std::uint8_t, 16> ipv6Data{};
        ipv6Data[15] = 1;
        return IpAddress(ipv6Data);
    }
    return IpAddress(); // Invalid type
}

// Creates a broadcast address
IpAddress IpAddress::broadcast()
{
    // IPv4 broadcast: 255.255.255.255
    return IpAddress(0xFFFFFFFF);
}

// Creates an "any" address
IpAddress IpAddress::any(Type type)
{
    if (type == Type::IPv4) {
        // IPv4 any: 0.0.0.0
        return IpAddress(0x00000000);
    } else if (type == Type::IPv6) {
        // IPv6 any: ::
        return IpAddress(std::array<std::uint8_t, 16>{});
    }
    return IpAddress(); // Invalid type
}

// Creates a localhost address (alias for loopback)
IpAddress IpAddress::localhost(Type type)
{
    return loopback(type);
}

// Assignment operator for string
IpAddress &IpAddress::operator=(std::string_view address)
{
    *this = IpAddress(address);
    return *this;
}

// Assignment operator for IPv4 integer
IpAddress &IpAddress::operator=(std::uint32_t ipv4Addr)
{
    m_data = ipv4Addr;
    return *this;
}

// Equality operator
bool IpAddress::operator==(const IpAddress &other) const
{
    return m_data == other.m_data;
}

// Inequality operator
bool IpAddress::operator!=(const IpAddress &other) const
{
    return !(*this == other);
}

// Less-than operator for ordered containers
bool IpAddress::operator<(const IpAddress &other) const
{
    // First compare by type
    if (type() != other.type()) {
        return type() < other.type();
    }

    // Same type, compare values
    if (std::holds_alternative<IPv4Data>(m_data)) {
        return std::get<IPv4Data>(m_data) < std::get<IPv4Data>(other.m_data);
    } else if (std::holds_alternative<IPv6Data>(m_data)) {
        return std::get<IPv6Data>(m_data) < std::get<IPv6Data>(other.m_data);
    }

    // Both invalid, so they're equal - return false for less-than
    return false;
}

} // namespace KDNetwork
