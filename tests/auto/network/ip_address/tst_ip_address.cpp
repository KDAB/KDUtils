/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/ip_address.h>

#include <array>
#include <string>
#include <string_view>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDNetwork;

// Test class properties
static_assert(std::is_destructible<IpAddress>{});
static_assert(std::is_default_constructible<IpAddress>{});
static_assert(std::is_copy_constructible<IpAddress>{});
static_assert(std::is_copy_assignable<IpAddress>{});
static_assert(std::is_move_constructible<IpAddress>{});
static_assert(std::is_move_assignable<IpAddress>{});

TEST_CASE("Constructor tests")
{
    SUBCASE("Default constructor creates invalid address")
    {
        IpAddress addr;
        CHECK(addr.isNull());
        CHECK(addr.type() == IpAddress::Type::Invalid);
        CHECK(addr.toString() == "");
    }

    SUBCASE("String view constructor - IPv4")
    {
        IpAddress addr("192.168.1.1");
        CHECK(!addr.isNull());
        CHECK(addr.type() == IpAddress::Type::IPv4);
        CHECK(addr.isIPv4());
        CHECK(!addr.isIPv6());
        CHECK(addr.toString() == "192.168.1.1");

        // Test with string literal
        IpAddress addrLiteral("192.168.1.1");
        CHECK(addrLiteral.toString() == "192.168.1.1");

        // Test with std::string
        std::string ipString = "192.168.1.1";
        IpAddress addrString(ipString);
        CHECK(addrString.toString() == "192.168.1.1");

        // Test with std::string_view
        std::string_view ipStringView = "192.168.1.1";
        IpAddress addrStringView(ipStringView);
        CHECK(addrStringView.toString() == "192.168.1.1");
    }

    SUBCASE("String constructor - IPv6")
    {
        IpAddress addr("2001:0db8:85a3:0000:0000:8a2e:0370:7334");
        CHECK(!addr.isNull());
        CHECK(addr.type() == IpAddress::Type::IPv6);
        CHECK(!addr.isIPv4());
        CHECK(addr.isIPv6());
        // Note: The string representation might be in a compressed format
        CHECK(addr.toString() == "2001:db8:85a3::8a2e:370:7334");
    }

    SUBCASE("uint32_t constructor - IPv4")
    {
        // 192.168.1.1 = 0xC0A80101
        IpAddress addr(0xC0A80101);
        CHECK(!addr.isNull());
        CHECK(addr.type() == IpAddress::Type::IPv4);
        CHECK(addr.toString() == "192.168.1.1");
    }

    SUBCASE("16-byte array constructor - IPv6")
    {
        // 2001:0db8::1
        std::array<std::uint8_t, 16> ipv6 = {
            0x20, 0x01, 0x0d, 0xb8,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x01
        };

        IpAddress addr(ipv6);
        CHECK(!addr.isNull());
        CHECK(addr.type() == IpAddress::Type::IPv6);
        CHECK(addr.isIPv6());
    }

    SUBCASE("sockaddr constructor - IPv4")
    {
        struct sockaddr_in addr4{};
        addr4.sin_family = AF_INET;
        addr4.sin_addr.s_addr = htonl(0xC0A80101); // 192.168.1.1

        IpAddress addr(reinterpret_cast<struct sockaddr *>(&addr4), sizeof(addr4));
        CHECK(!addr.isNull());
        CHECK(addr.type() == IpAddress::Type::IPv4);
        CHECK(addr.toString() == "192.168.1.1");
    }

    SUBCASE("sockaddr constructor - IPv6")
    {
        struct sockaddr_in6 addr6{};
        addr6.sin6_family = AF_INET6;

        // ::1 (loopback)
        std::memset(addr6.sin6_addr.s6_addr, 0, 16);
        addr6.sin6_addr.s6_addr[15] = 1;

        IpAddress addr(reinterpret_cast<struct sockaddr *>(&addr6), sizeof(addr6));
        CHECK(!addr.isNull());
        CHECK(addr.type() == IpAddress::Type::IPv6);
    }
}

TEST_CASE("Assignment operators")
{
    SUBCASE("String assignment")
    {
        IpAddress addr;
        addr = "192.168.1.1";
        CHECK(addr.type() == IpAddress::Type::IPv4);
        CHECK(addr.toString() == "192.168.1.1");
    }

    SUBCASE("uint32_t assignment")
    {
        IpAddress addr;
        addr = 0xC0A80101; // 192.168.1.1
        CHECK(addr.type() == IpAddress::Type::IPv4);
        CHECK(addr.toString() == "192.168.1.1");
    }
}

TEST_CASE("Comparison operators")
{
    SUBCASE("Equality operator")
    {
        IpAddress addr1("192.168.1.1");
        IpAddress addr2("192.168.1.1");
        IpAddress addr3("192.168.1.2");

        CHECK(addr1 == addr2);
        CHECK(addr1 != addr3);

        IpAddress addr4("2001:0db8::1");
        IpAddress addr5("2001:0db8::1");
        IpAddress addr6("2001:0db8::2");

        CHECK(addr4 == addr5);
        CHECK(addr4 != addr6);
    }

    SUBCASE("Less than operator")
    {
        IpAddress addr1("192.168.1.1");
        IpAddress addr2("192.168.1.2");

        CHECK(addr1 < addr2);
        CHECK(!(addr2 < addr1));

        // IPv4 is "less than" IPv6
        IpAddress addr3("192.168.1.1");
        IpAddress addr4("2001:0db8::1");

        CHECK(addr3 < addr4);
        CHECK(!(addr4 < addr3));
    }
}

TEST_CASE("IPv4 special address tests")
{
    SUBCASE("Loopback address")
    {
        IpAddress loopback("127.0.0.1");
        CHECK(loopback.isLoopback());
        CHECK(!loopback.isBroadcast());
        CHECK(!loopback.isLinkLocal());
        CHECK(!loopback.isMulticast());

        // Check static constructor
        IpAddress staticLoopback = IpAddress::loopback();
        CHECK(staticLoopback.isLoopback());
        CHECK(staticLoopback.type() == IpAddress::Type::IPv4);
    }

    SUBCASE("Broadcast address")
    {
        IpAddress broadcast("255.255.255.255");
        CHECK(broadcast.isBroadcast());
        CHECK(!broadcast.isLoopback());
        CHECK(!broadcast.isLinkLocal());

        // Check static constructor
        IpAddress staticBroadcast = IpAddress::broadcast();
        CHECK(staticBroadcast.isBroadcast());
        CHECK(staticBroadcast.type() == IpAddress::Type::IPv4);
    }

    SUBCASE("Any address")
    {
        IpAddress any("0.0.0.0");
        CHECK(!any.isLoopback());
        CHECK(!any.isBroadcast());
        CHECK(!any.isLinkLocal());
        CHECK(!any.isMulticast());

        // Check static constructor
        IpAddress staticAny = IpAddress::any();
        CHECK(staticAny.type() == IpAddress::Type::IPv4);
        CHECK(staticAny.toString() == "0.0.0.0");
    }

    SUBCASE("Link local address")
    {
        IpAddress linkLocal("169.254.1.1");
        CHECK(linkLocal.isLinkLocal());
        CHECK(!linkLocal.isLoopback());
        CHECK(!linkLocal.isBroadcast());
    }

    SUBCASE("Multicast address")
    {
        IpAddress multicast("224.0.0.1");
        CHECK(multicast.isMulticast());
        CHECK(!multicast.isLoopback());
        CHECK(!multicast.isBroadcast());
    }

    SUBCASE("Private address")
    {
        IpAddress private1("10.0.0.1");
        IpAddress private2("172.16.0.1");
        IpAddress private3("192.168.1.1");
        IpAddress public1("8.8.8.8");

        CHECK(private1.isPrivate());
        CHECK(private2.isPrivate());
        CHECK(private3.isPrivate());
        CHECK(!public1.isPrivate());
    }
}

TEST_CASE("IPv6 special address tests")
{
    SUBCASE("Loopback address")
    {
        IpAddress loopback("::1");
        CHECK(loopback.isLoopback());
        CHECK(!loopback.isBroadcast()); // IPv6 has no broadcast
        CHECK(!loopback.isLinkLocal());

        // Check static constructor
        IpAddress staticLoopback = IpAddress::loopback(IpAddress::Type::IPv6);
        CHECK(staticLoopback.isLoopback());
        CHECK(staticLoopback.type() == IpAddress::Type::IPv6);
    }

    SUBCASE("Any address")
    {
        IpAddress any("::");
        CHECK(!any.isLoopback());
        CHECK(!any.isLinkLocal());
        CHECK(!any.isMulticast());

        // Check static constructor
        IpAddress staticAny = IpAddress::any(IpAddress::Type::IPv6);
        CHECK(staticAny.type() == IpAddress::Type::IPv6);
    }

    SUBCASE("Link local address")
    {
        IpAddress linkLocal("fe80::1");
        CHECK(linkLocal.isLinkLocal());
        CHECK(!linkLocal.isLoopback());
    }

    SUBCASE("Multicast address")
    {
        IpAddress multicast("ff02::1");
        CHECK(multicast.isMulticast());
        CHECK(!multicast.isLoopback());
    }

    SUBCASE("Private address")
    {
        IpAddress private1("fc00::1");
        IpAddress public1("2001:0db8::1");

        CHECK(private1.isPrivate());
        CHECK(!public1.isPrivate());
    }
}

TEST_CASE("Subnet tests")
{
    SUBCASE("IPv4 subnet")
    {
        IpAddress ip("192.168.1.16");
        IpAddress subnet("192.168.1.0");

        CHECK(ip.isWithinSubnet(subnet, 24));
        CHECK(!ip.isWithinSubnet(subnet, 28)); // 192.168.1.0/28 is 192.168.1.0-15

        IpAddress ip2("192.168.2.10");
        CHECK(!ip2.isWithinSubnet(subnet, 24));
        CHECK(ip2.isWithinSubnet(subnet, 16)); // 192.168.0.0/16
    }

    SUBCASE("IPv6 subnet")
    {
        IpAddress ip("2001:0db8:0000:0000:0000:0000:0000:0100");
        IpAddress subnet("2001:0db8::");

        CHECK(ip.isWithinSubnet(subnet, 64));
        CHECK(!ip.isWithinSubnet(subnet, 120));

        IpAddress ip2("2001:0db9::");
        CHECK(!ip2.isWithinSubnet(subnet, 64));
        CHECK(ip2.isWithinSubnet(subnet, 15)); // First 15 bits match
    }
}

TEST_CASE("Conversion methods")
{
    SUBCASE("toIPv4")
    {
        IpAddress ip("192.168.1.1");
        CHECK(ip.toIPv4() == 0xC0A80101);

        // IPv6 address returns 0
        IpAddress ip2("::1");
        CHECK(ip2.toIPv4() == 0);
    }

    SUBCASE("toIPv6")
    {
        // Create a known IPv6 address
        std::array<std::uint8_t, 16> ipv6Data = {
            0x20, 0x01, 0x0d, 0xb8,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x01
        };
        IpAddress ip(ipv6Data);

        auto result = ip.toIPv6();
        CHECK(result == ipv6Data);

        // IPv4 address returns zero array
        IpAddress ip2("192.168.1.1");
        auto emptyResult = ip2.toIPv6();
        std::array<std::uint8_t, 16> zeros{};
        CHECK(emptyResult == zeros);
    }

    SUBCASE("toSockAddr - IPv4")
    {
        IpAddress ip("192.168.1.1");
        struct sockaddr_storage storage;
        socklen_t len = sizeof(storage);

        bool success = ip.toSockAddr(reinterpret_cast<struct sockaddr *>(&storage), len, 8080);
        CHECK(success);
        CHECK(len == sizeof(struct sockaddr_in));
        CHECK(storage.ss_family == AF_INET);

        // Check port
        struct sockaddr_in *addr = reinterpret_cast<struct sockaddr_in *>(&storage);
        CHECK(ntohs(addr->sin_port) == 8080);
    }

    SUBCASE("toSockAddr - IPv6")
    {
        IpAddress ip("2001:0db8::1");
        struct sockaddr_storage storage;
        socklen_t len = sizeof(storage);

        bool success = ip.toSockAddr(reinterpret_cast<struct sockaddr *>(&storage), len, 8080);
        CHECK(success);
        CHECK(len == sizeof(struct sockaddr_in6));
        CHECK(storage.ss_family == AF_INET6);

        // Check port
        struct sockaddr_in6 *addr = reinterpret_cast<struct sockaddr_in6 *>(&storage);
        CHECK(ntohs(addr->sin6_port) == 8080);
    }
}
