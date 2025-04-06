/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/tcp_socket.h>

#include <KDFoundation/object.h>
#include <KDFoundation/core_application.h>

#include <numeric>
#include <string>
#include <thread>
#include <chrono>
#include <future>

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

    SUBCASE("Can set socket blocking mode")
    {
        TcpSocket socket;
        CHECK(socket.open(AF_INET, SOCK_STREAM, IPPROTO_TCP));

        // Socket should be in non-blocking mode by default
        CHECK(socket.isBlocking() == false);

        // Set to blocking mode
        CHECK(socket.setBlocking(true));
        CHECK(socket.isBlocking() == true);

        // Set back to non-blocking mode
        CHECK(socket.setBlocking(false));
        CHECK(socket.isBlocking() == false);
    }

    SUBCASE("Can bind to an address")
    {
        TcpSocket socket;
        CHECK(socket.open(AF_INET, SOCK_STREAM, IPPROTO_TCP));

        // Prepare a sockaddr_in structure for binding to localhost:0 (random port)
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(0); // Let the OS pick a free port
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // Localhost

        // Bind to the address
        CHECK(socket.bind(reinterpret_cast<const struct sockaddr *>(&addr), sizeof(addr)));
        CHECK(socket.state() == Socket::State::Bound);
    }

    SUBCASE("Failed open returns error")
    {
        TcpSocket socket;
        // Try to open with an invalid domain
        const auto result = socket.open(-1, SOCK_STREAM, IPPROTO_TCP);
        CHECK(result == false);
        CHECK(!socket.isValid());
        CHECK(socket.state() == Socket::State::Unconnected);
        CHECK(socket.lastError() != SocketError::NoError);
    }
}

TEST_CASE("Signal handling")
{
    SUBCASE("State changed signal is emitted")
    {
        TcpSocket socket;
        bool stateChangedEmitted = false;
        Socket::State newState = Socket::State::Unconnected;

        std::ignore = socket.stateChanged.connect([&](Socket::State state) {
            stateChangedEmitted = true;
            newState = state;
        });

        CHECK(socket.open(AF_INET, SOCK_STREAM, IPPROTO_TCP));
        CHECK(stateChangedEmitted == true);
        CHECK(newState == Socket::State::Opening);

        stateChangedEmitted = false;
        socket.close();
        CHECK(stateChangedEmitted == true);
        CHECK(newState == Socket::State::Unconnected);
    }

    SUBCASE("Error signal is emitted on failure")
    {
        TcpSocket socket;
        bool errorEmitted = false;
        std::error_code errorCode;

        std::ignore = socket.errorOccurred.connect([&](std::error_code error) {
            errorEmitted = true;
            errorCode = error;
        });

        // Try to open with an invalid domain
        socket.open(-1, SOCK_STREAM, IPPROTO_TCP);
        CHECK(errorEmitted == true);
        CHECK(errorCode != KDNetwork::make_error_code(SocketError::NoError));
    }
}

TEST_CASE("Move semantics")
{
    SUBCASE("Can move construct")
    {
        TcpSocket socket1;
        CHECK(socket1.open(AF_INET, SOCK_STREAM, IPPROTO_TCP));
        const int sockFd = socket1.socketFileDescriptor();

        // Move-construct socket2 from socket1
        TcpSocket socket2(std::move(socket1));

        // socket2 should now have the file descriptor
        CHECK(socket2.socketFileDescriptor() == sockFd);
        CHECK(socket2.state() == Socket::State::Opening);

        // socket1 should be reset to initial state
        CHECK(socket1.socketFileDescriptor() == -1);
        CHECK(socket1.state() == Socket::State::Unconnected);
    }

    SUBCASE("Can move assign")
    {
        TcpSocket socket1;
        TcpSocket socket2;

        CHECK(socket1.open(AF_INET, SOCK_STREAM, IPPROTO_TCP));
        const int sockFd = socket1.socketFileDescriptor();

        // Move-assign socket2 from socket1
        socket2 = std::move(socket1);

        // socket2 should now have the file descriptor
        CHECK(socket2.socketFileDescriptor() == sockFd);
        CHECK(socket2.state() == Socket::State::Opening);

        // socket1 should be reset to initial state
        CHECK(socket1.socketFileDescriptor() == -1);
        CHECK(socket1.state() == Socket::State::Unconnected);
    }
}

// Test for behavior when socket is destroyed
TEST_CASE("Destruction behavior")
{
    SUBCASE("Socket is properly closed when destroyed")
    {
        int sockFd = -1;
        {
            TcpSocket socket;
            CHECK(socket.open(AF_INET, SOCK_STREAM, IPPROTO_TCP));
            sockFd = socket.socketFileDescriptor();
            CHECK(sockFd != -1);

            // Socket will go out of scope and be destroyed here
        }

        // Create a new socket with the same file descriptor to check if it's been closed
        // If the old socket wasn't properly closed, this will fail
        TcpSocket newSocket(sockFd, Socket::State::Unconnected);
        CHECK(newSocket.open(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    }
}

// Helper class to encapsulate minimal TcpServer functionality for testing
class TestTcpServer
{
public:
    // Define our own state enum to match Socket's state for consistency
    enum class State {
        NotListening, // Initial or closed state
        Listening, // Server is listening for connections
        Error // An error has occurred
    };

    TestTcpServer()
        : m_state(State::NotListening)
    {
        m_socket.open(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        m_socket.setBlocking(false);

        // Use the socket's built-in readyRead signal instead of our own FileDescriptorNotifier
        std::ignore = m_socket.readyRead.connect([this]() {
            acceptConnection();
        });
    }

    ~TestTcpServer()
    {
        for (auto &clientSocket : m_clientSockets) {
            clientSocket->close();
        }
        m_socket.close();
    }

    bool listen(uint16_t port = 0)
    {
        if (m_state == State::Listening) {
            // Already listening
            return false;
        }

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        if (!m_socket.bind(reinterpret_cast<const struct sockaddr *>(&addr), sizeof(addr))) {
            setState(State::Error);
            return false;
        }

        // Get the assigned port if we used port 0
        if (port == 0) {
            struct sockaddr_in boundAddr;
            socklen_t len = sizeof(boundAddr);
            if (getsockname(m_socket.socketFileDescriptor(),
                            reinterpret_cast<struct sockaddr *>(&boundAddr),
                            &len) == 0) {
                m_port = ntohs(boundAddr.sin_port);
            }
        } else {
            m_port = port;
        }

        // Start listening with a backlog of 1
        if (::listen(m_socket.socketFileDescriptor(), 1) != 0) {
            setState(State::Error);
            return false;
        }

        // Update state to Listening
        setState(State::Listening);

        return true;
    }

    uint16_t port() const { return m_port; }
    State state() const { return m_state; }

    KDBindings::Signal<uint16_t> serverStarted;
    KDBindings::Signal<TcpSocket *> clientConnected;

private:
    void setState(State newState)
    {
        if (m_state != newState) {
            m_state = newState;

            // Emit serverStarted signal when transition to listening state
            if (m_state == State::Listening) {
                serverStarted.emit(m_port);
            }
        }
    }

    void acceptConnection()
    {
        struct sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);

        int clientFd = accept(m_socket.socketFileDescriptor(),
                              reinterpret_cast<struct sockaddr *>(&clientAddr),
                              &addrLen);

        if (clientFd >= 0) {
            // Create a new client socket using the accepted file descriptor
            auto clientSocket = std::make_unique<TcpSocket>(clientFd, Socket::State::Connected);

            // Store the client socket and notify
            TcpSocket *rawPtr = clientSocket.get();
            m_clientSockets.push_back(std::move(clientSocket));
            clientConnected.emit(rawPtr);
        }
    }

    TcpSocket m_socket;
    std::vector<std::unique_ptr<TcpSocket>> m_clientSockets;
    uint16_t m_port = 0;
    State m_state;
};

TEST_CASE("Client-Server interaction")
{
    CoreApplication app;

    SUBCASE("Basic connection test")
    {
        TestTcpServer server;
        bool serverStarted = false;
        uint16_t serverPort = 0;
        bool clientConnected = false;
        bool connectionSucceeded = false;

        std::ignore = server.serverStarted.connect([&](uint16_t port) {
            serverStarted = true;
            serverPort = port;
        });

        std::ignore = server.clientConnected.connect([&](TcpSocket *) {
            clientConnected = true;
            app.quit();
        });

        CHECK(server.listen(0)); // Use port 0 to let the OS assign a free port
        CHECK(serverStarted);
        CHECK(serverPort > 0);

        // Create client socket and connect to server
        TcpSocket clientSocket;
        std::ignore = clientSocket.connected.connect([&]() {
            connectionSucceeded = true;
            app.quit();
        });

        std::ignore = clientSocket.errorOccurred.connect([&](std::error_code) {
            app.quit();
        });

        // Connect to server
        clientSocket.connectToHost(IpAddress::localhost(), serverPort);

        // Run the event loop to process events
        app.exec();

        CHECK(clientConnected);
        CHECK(connectionSucceeded);
        CHECK(clientSocket.state() == Socket::State::Connected);
    }

    SUBCASE("Data transmission test")
    {
        TestTcpServer server;
        uint16_t serverPort = 0;
        std::string receivedData;
        bool dataReceived = false;
        TcpSocket *serverSideClient = nullptr;

        std::ignore = server.serverStarted.connect([&](uint16_t port) {
            serverPort = port;
        });

        std::ignore = server.clientConnected.connect([&](TcpSocket *client) {
            serverSideClient = client;

            std::ignore = serverSideClient->bytesReceived.connect([&]() {
                receivedData = serverSideClient->readAll().toStdString();
                dataReceived = true;
                app.quit();
            });
        });

        CHECK(server.listen(0));
        CHECK(serverPort > 0);

        TcpSocket clientSocket;
        std::ignore = clientSocket.connected.connect([&]() {
            // Send test message after connection established
            const std::string testMessage = "Hello, TCP Socket!";
            clientSocket.write(KDUtils::ByteArray(testMessage));
        });

        clientSocket.connectToHost(IpAddress::localhost(), serverPort);

        app.exec();

        CHECK(dataReceived);
        CHECK(receivedData == "Hello, TCP Socket!");
        CHECK(serverSideClient != nullptr);
    }

    SUBCASE("Multiple data exchanges")
    {
        TestTcpServer server;
        uint16_t serverPort = 0;
        std::vector<std::string> receivedMessages;
        TcpSocket *serverSideSocket = nullptr;

        std::ignore = server.serverStarted.connect([&](uint16_t port) {
            serverPort = port;
        });

        std::ignore = server.clientConnected.connect([&](TcpSocket *client) {
            serverSideSocket = client;

            std::ignore = serverSideSocket->bytesReceived.connect([&]() {
                const std::string data = serverSideSocket->readAll().toStdString();
                receivedMessages.push_back(data);

                if (receivedMessages.size() >= 3) {
                    app.quit();
                } else {
                    // Echo the message back to the client
                    const std::string echo = "Echo: " + data;
                    serverSideSocket->write(KDUtils::ByteArray(echo));
                }
            });
        });

        CHECK(server.listen(0));
        CHECK(serverPort > 0);

        TcpSocket clientSocket;
        std::vector<std::string> clientReceivedMessages;

        std::ignore = clientSocket.bytesReceived.connect([&]() {
            const std::string data = clientSocket.readAll().toStdString();
            clientReceivedMessages.push_back(data);

            // Send next message back to server
            const std::string nextMessage = "Message " + std::to_string(clientReceivedMessages.size());
            clientSocket.write(KDUtils::ByteArray(nextMessage));
        });

        std::ignore = clientSocket.connected.connect([&]() {
            // Send initial message
            const std::string initialMessage = "Message 0";
            clientSocket.write(KDUtils::ByteArray(initialMessage));
        });

        // Connect to server
        clientSocket.connectToHost(IpAddress::localhost(), serverPort);

        app.exec();

        CHECK(receivedMessages.size() == 3);
        CHECK(clientReceivedMessages.size() == 2);
        CHECK(receivedMessages[0] == "Message 0");
        CHECK(receivedMessages[1] == "Message 1");
        CHECK(receivedMessages[2] == "Message 2");
    }
}
