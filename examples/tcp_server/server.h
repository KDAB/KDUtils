/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDNetwork/tcp_server.h>

#include <memory>

class Server
{
public:
    bool start();

private:
    void newConnection(std::unique_ptr<KDNetwork::TcpSocket> socket);

    KDNetwork::TcpServer m_server;
};
