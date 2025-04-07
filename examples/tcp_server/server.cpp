/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "server.h"

#include <KDNetwork/ip_address.h>
#include <KDNetwork/tcp_socket.h>

#include <iostream>
#include <random>
#include <vector>

using namespace KDNetwork;

bool Server::start()
{
    m_server.setNewConnectionCallback([this](std::unique_ptr<TcpSocket> socket) {
        newConnection(std::move(socket));
    });

    const auto result = m_server.listen(IpAddress::localhost(), 3001); // Listen on localhost, port 3001
    if (!result) {
        std::cerr << "Failed to start server: " << m_server.lastErrorCode().message() << std::endl;
        return false;
    }
    std::cout << "Server is listening on address: " << m_server.serverAddress().toString() << ", port: " << m_server.serverPort() << std::endl;
    return true;
}

void Server::newConnection(std::unique_ptr<KDNetwork::TcpSocket> socket)
{
    static std::vector<std::string> responses = {
        "Good news, everyone!",
        "Why not Zoidberg?",
        "Bite my shiny metal ass!",
        "Shut up and take my money!",
        "Wooo! Single female lawyer!",
        "I'm gonna build my own theme park! With blackjack and hookers! In fact, forget the park!",
        "To shreds, you say?",
        "Sweet zombie Jesus!"
    };

    // Generate a random index to select a phrase from the vector
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, responses.size() - 1);
    int randomIndex = distrib(gen);

    std::string message = responses[randomIndex];
    std::cout << "New connection accepted. Sending message: \"" << message << "\"" << std::endl;

    socket->write(KDUtils::ByteArray{ message });
    socket->close();
}
