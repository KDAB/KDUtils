/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "client.h"

#include <KDFoundation/core_application.h>
#include <iostream>

Client::Client()
{
    std::ignore = m_socket.bytesReceived.connect([this]() {
        std::string data = m_socket.readAll().toStdString();
        std::cout << "Received message: \"" << data << "\"" << std::endl;

        // Now quit the application after receiving the message
        KDFoundation::CoreApplication::instance()->quit();
    });
}

bool Client::connectToServer(const KDNetwork::IpAddress &host, std::uint16_t port)
{
    bool connected = m_socket.connectToHost(host, port);
    if (!connected) {
        std::cout << "Failed to connect. Error code: " << m_socket.lastError() << std::endl;
    }
    return connected;
}
