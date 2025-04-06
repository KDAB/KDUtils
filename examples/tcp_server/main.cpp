/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "server.h"

#include <KDFoundation/core_application.h>

#include <iostream>
#include <csignal>

int main(int argc, char *argv[])
{
    KDFoundation::CoreApplication app;

    // Install signal handler to exit cleanly on Ctrl+C
    std::signal(SIGINT, [](int signal) {
        std::cout << "Ctrl+C pressed. Exiting..." << std::endl;
        KDFoundation::CoreApplication::instance()->quit();
    });

    Server server;
    if (!server.start()) {
        std::cerr << "Failed to start server!" << std::endl;
        return 1;
    }

    std::cout << "Press Ctrl+C to exit." << std::endl;
    return app.exec();
}
