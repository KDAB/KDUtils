/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Marco Thaller <marco.thaller@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "mosquitto_wrapper.h"
#include "fakeit.h"

class MosquittoLibMock : public MosquittoLib
{
public:
    MosquittoLibMock() = default;
    ~MosquittoLibMock() = default;

    MOCK_METHOD(int, init, (), (override));
    MOCK_METHOD(int, cleanup, (), (override));
    MOCK_METHOD(int, version, (int *major, int *minor, int *revision), (const, override));
    MOCK_METHOD(const std::string_view, connackString, (int connack_code), (const, override));
    MOCK_METHOD(const std::string_view, errorString, (int mosq_errno), (const, override));
    MOCK_METHOD(const std::string_view, reasonString, (int reason_code), (const, override));
    MOCK_METHOD(bool, isValidTopicNameForSubscription, (const std::string &topic), (const, override));
};

class MosquittoClientMock : public MosquittoClient
{
public:
    MosquittoClientMock()
        : MosquittoClient("mock") { }
    ~MosquittoClientMock() = default;

    MOCK_METHOD(int, connectAsync, (const std::string &host, int port, int keepalive), (override));
    MOCK_METHOD(int, disconnect, (), (override));
    MOCK_METHOD(int, publish, (int *msg_id, const std::string &topic, int payloadlen, const void *payload, int qos, bool retain), (override));
    MOCK_METHOD(int, subscribe, (int *msg_id, const std::string &sub, int qos = 0), (override));
    MOCK_METHOD(int, unsubscribe, (int *msg_id, const std::string &sub), (override));
    MOCK_METHOD(int, loopMisc, (), (override));
    MOCK_METHOD(int, loopRead, (int max_packets), (override));
    MOCK_METHOD(int, loopWrite, (int max_packets), (override));
    MOCK_METHOD(int, socket, (), (override));
    MOCK_METHOD(bool, wantWrite, (), (override));
    MOCK_METHOD(void *, sslGet, (), (override));
    MOCK_METHOD(int, tlsSet, (std::optional<const std::string> cafile, std::optional<const std::string> capath, std::optional<const std::string> certfile, std::optional<const std::string> keyfile, int (*pw_callback)(char *buf, int size, int rwflag, void *userdata)), (override));
    MOCK_METHOD(int, tlsEnableUseOsCertificates, (), (override));
    MOCK_METHOD(int, tlsDisableUseOsCertificates, (), (override));
    MOCK_METHOD(int, usernamePasswordSet, (const std::string &username, const std::string &password), (override));
    MOCK_METHOD(int, willSet, (const std::string &topic, int payloadlen, const void *payload, int qos, bool retain), (override));
};
