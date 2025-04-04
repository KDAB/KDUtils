/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/tcp_socket.h>

#include <KDFoundation/object.h>

#include <numeric>
#include <string>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDFoundation;
using namespace KDNetwork;

static_assert(std::is_destructible<TcpSocket>{});
static_assert(std::is_default_constructible<TcpSocket>{});
static_assert(!std::is_copy_constructible<TcpSocket>{});
static_assert(!std::is_copy_assignable<TcpSocket>{});
static_assert(std::is_move_constructible<TcpSocket>{});
static_assert(std::is_move_assignable<TcpSocket>{});

TEST_CASE("Basic usage")
{
    SUBCASE("Can create a TcpSocket")
    {
        TcpSocket socket;
        CHECK(socket.type() == Socket::SocketType::Tcp);
        CHECK(socket.socketFileDescriptor() == -1);
        CHECK(socket.state() == Socket::State::Unconnected);
        CHECK(socket.lastError() == SocketError::NoError);
        CHECK(socket.lastErrorCode() == KDNetwork::make_error_code(SocketError::NoError));
    }

    SUBCASE("Can open a TcpSocket")
    {
        TcpSocket socket;
        const auto result = socket.open(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        CHECK(result == true);
        CHECK(socket.isValid());
        CHECK(socket.state() == Socket::State::Opening);
        CHECK(socket.lastError() == SocketError::NoError);
        CHECK(socket.lastErrorCode() == KDNetwork::make_error_code(SocketError::NoError));
    }

    SUBCASE("Can close a TcpSocket")
    {
        TcpSocket socket;
        CHECK(socket.open(AF_INET, SOCK_STREAM, IPPROTO_TCP));
        CHECK(socket.isValid());
        socket.close();
        CHECK(!socket.isValid());
        CHECK(socket.state() == Socket::State::Unconnected);
        CHECK(socket.lastError() == SocketError::NoError);
        CHECK(socket.lastErrorCode() == KDNetwork::make_error_code(SocketError::NoError));
    }
}
