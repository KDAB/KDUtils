/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "client.h"

#include <KDNetwork/ip_address.h>
#include <KDFoundation/core_application.h>

#include <iostream>
#include <csignal>

int main(int argc, char *argv[])
{
    KDFoundation::CoreApplication app;

    Client client;
    if (!client.connectToServer(KDNetwork::IpAddress::localhost(), 3001)) {
        std::cout << "Failed to connect to server." << std::endl;
        return 1;
    }

    return app.exec();
}
