/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDNetwork/ip_address.h>
#include <KDNetwork/tcp_socket.h>

#include <memory>

class Client
{
public:
    Client();
    ~Client() = default;

    bool connectToServer(const KDNetwork::IpAddress &host, std::uint16_t port);

private:
    KDNetwork::TcpSocket m_socket;
};
