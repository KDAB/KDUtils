/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Marco Thaller <marco.thaller@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include <KDFoundation/core_application.h>
#include <KDMqtt/mqtt.h>
#include <memory>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <signal_spy.h>
#include "fakeit.h" // <= include after doctest.h

using namespace fakeit;

namespace KDMqtt {

class MqttUnitTestHarness
{
public:
    MqttUnitTestHarness()
        : m_mqttManager{ MqttManager::instance() }
    {
        m_mqttManager.m_isInitialized = false;
        m_mqttManager.m_mosquittoLib = &m_mosquittoLibMock.get();

        // Setup default return values for faked methods
        When(Method(libMock(), init)).AlwaysReturn(MOSQ_ERR_SUCCESS);
        When(Method(libMock(), cleanup)).AlwaysReturn(MOSQ_ERR_SUCCESS);
        When(Method(libMock(), version)).AlwaysReturnAndSet(MOSQ_ERR_SUCCESS, 0, 0, 0);

        // Have string conversion methods not be faked/mocked and call actual implementations
        When(Method(libMock(), connackString)).AlwaysDo([](auto connack_code) { return MosquittoLib::instance().connackString(connack_code); });
        When(Method(libMock(), errorString)).AlwaysDo([](auto mosq_errno) { return MosquittoLib::instance().errorString(mosq_errno); });
        When(Method(libMock(), reasonString)).AlwaysDo([](auto reason_code) { return MosquittoLib::instance().reasonString(reason_code); });
    }

    std::shared_ptr<MqttClient> initMqttManagerAndCreateMqttClient(MqttManager::ClientOptions options = MqttManager::ClientOption::CLEAN_SESSION)
    {
        m_mqttManager.init();
        auto mqttClientInterface = m_mqttManager.createClient("tst_mqtt", options);
        return std::dynamic_pointer_cast<MqttClient>(mqttClientInterface);
    }

    void createMosquittoClientMockAndInjectIntoMqttClient(const std::shared_ptr<MqttClient> &mqttClient)
    {
        m_mosquittoClientMock = new Mock<MosquittoClient>();
        auto mosquittoClient = std::unique_ptr<MosquittoClient>(&m_mosquittoClientMock->get());
        mqttClient->m_mosquitto.init(std::move(mosquittoClient), mqttClient.get());
    }

    static MqttClient::EventLoopHook &eventLoopHook(MqttClient *mqttClient)
    {
        return mqttClient->m_eventLoopHook;
    }

    static void onConnected(MqttClient *mqttClient, int connackCode)
    {
        mqttClient->onConnected(connackCode);
    }

    static void onDisconnected(MqttClient *mqttClient, int reasonCode)
    {
        mqttClient->onDisconnected(reasonCode);
    }

    static void onPublished(MqttClient *mqttClient, int msgId)
    {
        mqttClient->onPublished(msgId);
    }

    static void onMessage(MqttClient *mqttClient, const mosquitto_message *message)
    {
        mqttClient->onMessage(message);
    }

    static void onSubscribed(MqttClient *mqttClient, int msgId, int qosCount, const int *grantedQos)
    {
        mqttClient->onSubscribed(msgId, qosCount, grantedQos);
    }

    static void onUnsubscribed(MqttClient *mqttClient, int msgId)
    {
        mqttClient->onUnsubscribed(msgId);
    }

    static void onError(MqttClient *mqttClient)
    {
        mqttClient->onError();
    }

    static void onReadOpRequested(MqttClient *mqttClient)
    {
        mqttClient->onReadOpRequested();
    }

    static void onWriteOpRequested(MqttClient *mqttClient)
    {
        mqttClient->onWriteOpRequested();
    }

    static void onMiscTaskRequested(MqttClient *mqttClient)
    {
        mqttClient->onMiscTaskRequested();
    }

    Mock<MosquittoLib> &libMock() { return m_mosquittoLibMock; }
    Mock<MosquittoClient> &clientMock()
    {
        assert(m_mosquittoClientMock != nullptr);
        return *m_mosquittoClientMock;
    }
    MqttManager &mqttManager() { return m_mqttManager; }

private:
    CoreApplication m_app; // we need an app instance while running the tests (MqttClient uses Timers e.g.)
    Mock<MosquittoLib> m_mosquittoLibMock;
    Mock<MosquittoClient> *m_mosquittoClientMock = nullptr;
    MqttManager &m_mqttManager;
};

} // namespace KDMqtt

using namespace KDMqtt;

TEST_SUITE("Mqtt")
{
    TEST_CASE("MqttManager::MqttManager()")
    {
        MqttUnitTestHarness muth;

        SUBCASE("initial isInitialized state is false")
        {
            // GIVEN
            const auto result = muth.mqttManager().isInitialized();

            // THEN
            REQUIRE_FALSE(result);
        }
    }

    TEST_CASE("MqttManager::init()")
    {
        MqttUnitTestHarness muth;

        SUBCASE("triggers initialization on first call")
        {
            // GIVEN
            const auto result = muth.mqttManager().init();

            // THEN
            REQUIRE(result == MOSQ_ERR_SUCCESS);
            Verify(Method(muth.libMock(), init)).Once();
        }

        SUBCASE("does NOT trigger initialization on consecutive calls")
        {
            // GIVEN
            muth.mqttManager().init();
            Verify(Method(muth.libMock(), init)).Once();

            // WHEN
            const auto result = muth.mqttManager().init();

            // THEN
            REQUIRE(result == MOSQ_ERR_UNKNOWN);
            Verify(Method(muth.libMock(), init)).Once();
        }
    }

    TEST_CASE("MqttManager::cleanup()")
    {
        MqttUnitTestHarness muth;

        SUBCASE("triggers cleanup")
        {
            // GIVEN
            const auto result = muth.mqttManager().cleanup();

            // THEN
            REQUIRE(result == MOSQ_ERR_SUCCESS);
            Verify(Method(muth.libMock(), cleanup)).Once();
        }
    }

    TEST_CASE("MqttManager::isInitialized()")
    {
        MqttUnitTestHarness muth;

        SUBCASE("returns true after init()")
        {
            // GIVEN
            const auto result = muth.mqttManager().init();

            // WHEN
            REQUIRE(result == MOSQ_ERR_SUCCESS);
            Verify(Method(muth.libMock(), init)).Once();

            // THEN
            const auto isInitialized = muth.mqttManager().isInitialized();
            REQUIRE(isInitialized);
        }
    }

    // TODO -> bool isValidTopicNameForSubscription(const std::string &topic) override;
    // TODO -> std::shared_ptr<IMqttClient> createClient(const std::string &clientId, ClientOptions options);

    TEST_CASE("MqttClient::MqttClient()")
    {
        MqttUnitTestHarness muth;
        auto mqttClient = muth.initMqttManagerAndCreateMqttClient();

        SUBCASE("initial connection state is DISCONNECTED")
        {
            REQUIRE(mqttClient->connectionState.get() == MqttClient::ConnectionState::DISCONNECTED);
        }

        SUBCASE("initial subscription state is UNSUBSCRIBED")
        {
            REQUIRE(mqttClient->subscriptionState.get() == MqttClient::SubscriptionState::UNSUBSCRIBED);
        }

        SUBCASE("initial subscriptions list is empty")
        {
            REQUIRE(mqttClient->subscriptions.get().empty());
        }

        // TODO -> add test: calls tlsEnableUseOsCertificates()
    }

    // TODO -> MqttClient::~MqttClient()

    TEST_CASE("MqttClient::setTls()")
    {
        MqttUnitTestHarness muth;
        auto mqttClient = muth.initMqttManagerAndCreateMqttClient();

        muth.createMosquittoClientMockAndInjectIntoMqttClient(mqttClient);
        Fake(Method(muth.clientMock(), tlsSet));

        const File invalidCaFile{ "this_file_does_not_exist.crt" };

        SUBCASE("returns MOSQ_ERR_UNKNOWN if connection state is CONNECTING")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTING);

            // WHEN
            const auto result = mqttClient->setTls(invalidCaFile);

            // THEN
            REQUIRE(result == MOSQ_ERR_UNKNOWN);
        }

        SUBCASE("returns MOSQ_ERR_UNKNOWN if connection state is CONNECTED")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTED);

            // WHEN
            const auto result = mqttClient->setTls(invalidCaFile);

            // THEN
            REQUIRE(result == MOSQ_ERR_UNKNOWN);
        }

        SUBCASE("returns MOSQ_ERR_UNKNOWN if connection state is DISCONNECTING")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::DISCONNECTING);

            // WHEN
            const auto result = mqttClient->setTls(invalidCaFile);

            // THEN
            REQUIRE(result == MOSQ_ERR_UNKNOWN);
        }

        SUBCASE("returns MOSQ_ERR_UNKNOWN if specified CA file does not exist")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::DISCONNECTED);

            // WHEN
            const auto result = mqttClient->setTls(invalidCaFile);

            // THEN
            REQUIRE(result == MOSQ_ERR_UNKNOWN);
        }

        SUBCASE("calls MosquittoClient::tlsSet once with correct arguments")
        {
            // GIVEN
            File dummyCaFile{ std::filesystem::current_path().append("dummy.org.crt").string() };
            dummyCaFile.open(std::ios::out);
            dummyCaFile.close();
            REQUIRE(dummyCaFile.exists());

            mqttClient->connectionState.set(MqttClient::ConnectionState::DISCONNECTED);

            // WHEN
            mqttClient->setTls(dummyCaFile);

            // THEN
            const auto dummyCaFilePath = std::optional<const std::string>(dummyCaFile.path());
            Verify(Method(muth.clientMock(), tlsSet)).Once();
            Verify(Method(muth.clientMock(), tlsSet).Using(dummyCaFilePath, std::nullopt, std::nullopt, std::nullopt, nullptr));
        }
    }

    TEST_CASE("MqttClient::setUsernameAndPassword()")
    {
        MqttUnitTestHarness muth;
        auto mqttClient = muth.initMqttManagerAndCreateMqttClient();

        muth.createMosquittoClientMockAndInjectIntoMqttClient(mqttClient);
        Fake(Method(muth.clientMock(), usernamePasswordSet));

        const std::string user{ "username" };
        const std::string password{ "password" };

        SUBCASE("returns MOSQ_ERR_UNKNOWN if connection state is CONNECTING")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTING);

            // WHEN
            const auto result = mqttClient->setUsernameAndPassword(user, password);

            // THEN
            REQUIRE(result == MOSQ_ERR_UNKNOWN);
        }

        SUBCASE("returns MOSQ_ERR_UNKNOWN if connection state is CONNECTED")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTED);

            // WHEN
            const auto result = mqttClient->setUsernameAndPassword(user, password);

            // THEN
            REQUIRE(result == MOSQ_ERR_UNKNOWN);
        }

        SUBCASE("returns MOSQ_ERR_UNKNOWN if connection state is DISCONNECTING")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::DISCONNECTING);

            // WHEN
            const auto result = mqttClient->setUsernameAndPassword(user, password);

            // THEN
            REQUIRE(result == MOSQ_ERR_UNKNOWN);
        }

        SUBCASE("calls MosquittoClient::usernamePasswordSet once with correct arguments")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::DISCONNECTED);

            // WHEN
            mqttClient->setUsernameAndPassword(user, password);

            // THEN
            Verify(Method(muth.clientMock(), usernamePasswordSet)).Once();
            Verify(Method(muth.clientMock(), usernamePasswordSet).Using(user, password));
        }
    }

    TEST_CASE("MqttClient::setWill()")
    {
        MqttUnitTestHarness muth;
        auto mqttClient = muth.initMqttManagerAndCreateMqttClient();

        muth.createMosquittoClientMockAndInjectIntoMqttClient(mqttClient);
        Fake(Method(muth.clientMock(), willSet));

        const ByteArray payload{ "payload" };
        const std::string topic{ "last_will" };
        const IMqttClient::QOS qos{ IMqttClient::QOS::AT_MOST_ONCE };
        const bool retain{ false };

        SUBCASE("returns MOSQ_ERR_UNKNOWN if connection state is CONNECTING")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTING);

            // WHEN
            const auto result = mqttClient->setWill(topic, &payload, qos, retain);

            // THEN
            REQUIRE(result == MOSQ_ERR_UNKNOWN);
        }

        SUBCASE("returns MOSQ_ERR_UNKNOWN if connection state is CONNECTED")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTED);

            // WHEN
            const auto result = mqttClient->setWill(topic, &payload, qos, retain);

            // THEN
            REQUIRE(result == MOSQ_ERR_UNKNOWN);
        }

        SUBCASE("returns MOSQ_ERR_UNKNOWN if connection state is DISCONNECTING")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::DISCONNECTING);

            // WHEN
            const auto result = mqttClient->setWill(topic, &payload, qos, retain);

            // THEN
            REQUIRE(result == MOSQ_ERR_UNKNOWN);
        }

        SUBCASE("calls MosquittoClient::willSet once with correct arguments")
        {
            // GIVEN
            ByteArray payloadData;
            When(Method(muth.clientMock(), willSet)).AlwaysDo([&](const std::string & /*topic*/, int payloadlen, const void *payload, int /*qos*/, bool /*retain*/) { // NOLINT(bugprone-easily-swappable-parameters)
                // Manually capture payload data
                payloadData = ByteArray(static_cast<const char *>(payload), payloadlen);
                return MOSQ_ERR_SUCCESS;
            });
            mqttClient->connectionState.set(MqttClient::ConnectionState::DISCONNECTED);

            // WHEN
            mqttClient->setWill(topic, &payload, qos, retain);

            // THEN
            Verify(Method(muth.clientMock(), willSet)).Once();
            Verify(Method(muth.clientMock(), willSet).Using(topic, payload.size(), Any(), static_cast<int>(qos), retain));
            REQUIRE(payloadData == payload);
        }
    }

    TEST_CASE("MqttClient::connect()")
    {
        MqttUnitTestHarness muth;
        auto mqttClient = muth.initMqttManagerAndCreateMqttClient();

        muth.createMosquittoClientMockAndInjectIntoMqttClient(mqttClient);
        Fake(Method(muth.clientMock(), connectAsync));
        Fake(Method(muth.clientMock(), socket));

        const Url host{ "test.org" };
        const std::uint16_t port{ 0 };
        const std::chrono::seconds keepalive{ 60 };

        SUBCASE("returns MOSQ_ERR_UNKNOWN if connection state is CONNECTING")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTING);

            // WHEN
            const auto result = mqttClient->connect(host, port, keepalive);

            // THEN
            REQUIRE(result == MOSQ_ERR_UNKNOWN);
        }

        SUBCASE("returns MOSQ_ERR_UNKNOWN if connection state is CONNECTED")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTED);

            // WHEN
            const auto result = mqttClient->connect(host, port, keepalive);

            // THEN
            REQUIRE(result == MOSQ_ERR_UNKNOWN);
        }

        SUBCASE("sets connection state to CONNECTING")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::DISCONNECTED);

            // WHEN
            mqttClient->connect(host, port, keepalive);

            // THEN
            REQUIRE(mqttClient->connectionState.get() == MqttClient::ConnectionState::CONNECTING);
        }

        SUBCASE("calls MosquittoClient::connect once with correct arguments")
        {
            // GIVEN
            std::string hostUrl;
            int keepaliveSeconds;
            When(Method(muth.clientMock(), connectAsync)).AlwaysDo([&](const std::string &host, int /*port*/, int keepalive) { // NOLINT(bugprone-easily-swappable-parameters)
                // Manually capture argument: host + keepalive
                // I ran into issues when validating host + keepalive the default way
                // May be related to https://github.com/eranpeer/FakeIt/issues/31 (for host)
                hostUrl = host;
                keepaliveSeconds = keepalive;
                return MOSQ_ERR_SUCCESS;
            });
            mqttClient->connectionState.set(MqttClient::ConnectionState::DISCONNECTED);

            // WHEN
            mqttClient->connect(host, port, keepalive);

            // THEN
            Verify(Method(muth.clientMock(), connectAsync)).Once();
            Verify(Method(muth.clientMock(), connectAsync).Using(Any(), port, Any()));
            REQUIRE(hostUrl == host.url());
            REQUIRE(keepaliveSeconds == keepalive.count());
        }

        SUBCASE("engages event loop hook if MosquittoClient::connect returns SUCCESS")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::DISCONNECTED);

            // WHEN
            When(Method(muth.clientMock(), connectAsync)).Return(MOSQ_ERR_SUCCESS);
            mqttClient->connect(host, port, keepalive);

            // THEN
            REQUIRE(MqttUnitTestHarness::eventLoopHook(mqttClient.get()).isEngaged());
        }

        SUBCASE("does NOT engage event loop hook if MosquittoClient::connect returns ERROR")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::DISCONNECTED);

            // WHEN
            When(Method(muth.clientMock(), connectAsync)).Return(MOSQ_ERR_UNKNOWN);
            mqttClient->connect(host, port, keepalive);

            // THEN
            REQUIRE_FALSE(MqttUnitTestHarness::eventLoopHook(mqttClient.get()).isEngaged());
        }
    }

    TEST_CASE("MqttClient::disconnect()")
    {
        MqttUnitTestHarness muth;
        auto mqttClient = muth.initMqttManagerAndCreateMqttClient();

        muth.createMosquittoClientMockAndInjectIntoMqttClient(mqttClient);
        Fake(Method(muth.clientMock(), disconnect));

        SUBCASE("returns MOSQ_ERR_UNKNOWN if connection state is DISCONNECTING")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::DISCONNECTING);

            // WHEN
            const auto result = mqttClient->disconnect();

            // THEN
            REQUIRE(result == MOSQ_ERR_UNKNOWN);
        }

        SUBCASE("returns MOSQ_ERR_UNKNOWN if connection state is DISCONNECTED")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::DISCONNECTED);

            // WHEN
            const auto result = mqttClient->disconnect();

            // THEN
            REQUIRE(result == MOSQ_ERR_UNKNOWN);
        }

        SUBCASE("sets connection state to DISCONNECTING")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTED);

            // WHEN
            mqttClient->disconnect();

            // THEN
            REQUIRE(mqttClient->connectionState.get() == MqttClient::ConnectionState::DISCONNECTING);
        }

        SUBCASE("calls MosquittoClient::disconnect once")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTED);

            // WHEN
            mqttClient->disconnect();

            // THEN
            Verify(Method(muth.clientMock(), disconnect)).Once();
        }
    }

    TEST_CASE("MqttClient::publish()")
    {
        MqttUnitTestHarness muth;
        auto mqttClient = muth.initMqttManagerAndCreateMqttClient();

        muth.createMosquittoClientMockAndInjectIntoMqttClient(mqttClient);
        Fake(Method(muth.clientMock(), publish));

        int msgId{ 0 };
        const ByteArray payload{ "payload" };
        const std::string topic{ "test" };
        const IMqttClient::QOS qos{ IMqttClient::QOS::AT_MOST_ONCE };
        const bool retain{ false };

        SUBCASE("returns MOSQ_ERR_UNKNOWN if connection state is DISCONNECTED")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::DISCONNECTED);

            // WHEN
            const auto result = mqttClient->publish(&msgId, topic, &payload, qos, retain);

            // THEN
            REQUIRE(result == MOSQ_ERR_UNKNOWN);
        }

        SUBCASE("calls MosquittoClient::publish once with correct arguments")
        {
            // GIVEN
            ByteArray payloadData;
            When(Method(muth.clientMock(), publish)).AlwaysDo([&](int * /*msg_id*/, const std::string & /*topic*/, int payloadlen, const void *payload, int /*qos*/, bool /*retain*/) { // NOLINT(bugprone-easily-swappable-parameters)
                // Manually capture payload data
                payloadData = ByteArray(static_cast<const char *>(payload), payloadlen);
                return MOSQ_ERR_SUCCESS;
            });
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTED);

            // WHEN
            mqttClient->publish(&msgId, topic, &payload, qos, retain);

            // THEN
            Verify(Method(muth.clientMock(), publish)).Once();
            Verify(Method(muth.clientMock(), publish).Using(&msgId, topic, payload.size(), Any(), static_cast<int>(qos), retain));
            REQUIRE(payloadData == payload);
        }
    }

    TEST_CASE("MqttClient::subscribe()")
    {
        MqttUnitTestHarness muth;
        auto mqttClient = muth.initMqttManagerAndCreateMqttClient();

        muth.createMosquittoClientMockAndInjectIntoMqttClient(mqttClient);
        Fake(Method(muth.clientMock(), subscribe));

        const std::string pattern{ "pattern" };
        const IMqttClient::QOS qos{ IMqttClient::QOS::AT_MOST_ONCE };

        SUBCASE("returns MOSQ_ERR_UNKNOWN if connection state is DISCONNECTED")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::DISCONNECTED);

            // WHEN
            const auto result = mqttClient->subscribe(pattern, qos);

            // THEN
            REQUIRE(result == MOSQ_ERR_UNKNOWN);
        }

        SUBCASE("calls MosquittoClient::subscribe once with correct arguments")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTED);

            // WHEN
            mqttClient->subscribe(pattern, qos);

            // THEN
            Verify(Method(muth.clientMock(), subscribe)).Once();
            Verify(Method(muth.clientMock(), subscribe).Using(Any(), pattern, static_cast<int>(qos)));
        }

        SUBCASE("sets subscription state to SUBSCRIBING if MosquittoClient::subscribe returns SUCCESS")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTED);

            // WHEN
            When(Method(muth.clientMock(), subscribe)).Return(MOSQ_ERR_SUCCESS);
            mqttClient->subscribe(pattern, qos);

            // THEN
            REQUIRE(mqttClient->subscriptionState.get() == MqttClient::SubscriptionState::SUBSCRIBING);
        }

        SUBCASE("does NOT set subscription state to SUBSCRIBING if MosquittoClient::subscribe returns ERROR")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTED);

            // WHEN
            When(Method(muth.clientMock(), subscribe)).Return(MOSQ_ERR_UNKNOWN);
            mqttClient->subscribe(pattern, qos);

            // THEN
            REQUIRE_FALSE(mqttClient->subscriptionState.get() == MqttClient::SubscriptionState::SUBSCRIBING);
        }
    }

    TEST_CASE("MqttClient::unsubscribe()")
    {
        MqttUnitTestHarness muth;
        auto mqttClient = muth.initMqttManagerAndCreateMqttClient();

        muth.createMosquittoClientMockAndInjectIntoMqttClient(mqttClient);
        Fake(Method(muth.clientMock(), unsubscribe));

        const std::string pattern{ "pattern" };

        SUBCASE("returns MOSQ_ERR_UNKNOWN if connection state is DISCONNECTED")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::DISCONNECTED);

            // WHEN
            const auto result = mqttClient->unsubscribe(pattern);

            // THEN
            REQUIRE(result == MOSQ_ERR_UNKNOWN);
        }

        SUBCASE("calls MosquittoClient::unsubscribe once with correct arguments")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTED);

            // WHEN
            mqttClient->unsubscribe(pattern);

            // THEN
            Verify(Method(muth.clientMock(), unsubscribe)).Once();
            Verify(Method(muth.clientMock(), unsubscribe).Using(Any(), pattern));
        }

        SUBCASE("sets subscription state to UNSUBSCRIBING if MosquittoClient::unsubscribe returns SUCCESS")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTED);

            // WHEN
            When(Method(muth.clientMock(), unsubscribe)).Return(MOSQ_ERR_SUCCESS);
            mqttClient->unsubscribe(pattern);

            // THEN
            REQUIRE(mqttClient->subscriptionState.get() == MqttClient::SubscriptionState::UNSUBSCRIBING);
        }

        SUBCASE("does NOT set subscription state to UNSUBSCRIBING if MosquittoClient::unsubscribe returns ERROR")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTED);

            // WHEN
            When(Method(muth.clientMock(), unsubscribe)).Return(MOSQ_ERR_UNKNOWN);
            mqttClient->unsubscribe(pattern);

            // THEN
            REQUIRE_FALSE(mqttClient->subscriptionState.get() == MqttClient::SubscriptionState::UNSUBSCRIBING);
        }
    }

    TEST_CASE("MqttClient::onConnected()")
    {
        MqttUnitTestHarness muth;
        auto mqttClient = muth.initMqttManagerAndCreateMqttClient();

        SUBCASE("sets connection state to CONNECTED")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTING);

            // WHEN
            MqttUnitTestHarness::onConnected(mqttClient.get(), MOSQ_ERR_SUCCESS);

            // THEN
            REQUIRE(mqttClient->connectionState.get() == MqttClient::ConnectionState::CONNECTED);
        }

        SUBCASE("sets connection state to DISCONNECTED")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTING);

            // WHEN
            MqttUnitTestHarness::onConnected(mqttClient.get(), MOSQ_ERR_CONN_REFUSED);

            // THEN
            REQUIRE(mqttClient->connectionState.get() == MqttClient::ConnectionState::DISCONNECTED);
        }
    }

    TEST_CASE("MqttClient::onDisconnected()")
    {
        MqttUnitTestHarness muth;
        auto mqttClient = muth.initMqttManagerAndCreateMqttClient();

        SUBCASE("disengages event loop hook")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::DISCONNECTING);

            // WHEN
            MqttUnitTestHarness::onDisconnected(mqttClient.get(), 0);

            // THEN
            REQUIRE_FALSE(MqttUnitTestHarness::eventLoopHook(mqttClient.get()).isEngaged());
        }

        SUBCASE("sets connection state to DISCONNECTED")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::DISCONNECTING);

            // WHEN
            MqttUnitTestHarness::onDisconnected(mqttClient.get(), 0);

            // THEN
            REQUIRE(mqttClient->connectionState.get() == MqttClient::ConnectionState::DISCONNECTED);
        }
    }

    TEST_CASE("MqttClient::onPublished")
    {
        MqttUnitTestHarness muth;
        auto mqttClient = muth.initMqttManagerAndCreateMqttClient();

        SUBCASE("emits signal msgPublished")
        {
            // GIVEN
            const SignalSpy spy(mqttClient->msgPublished);

            // WHEN
            MqttUnitTestHarness::onPublished(mqttClient.get(), 0);

            // THEN
            REQUIRE(spy.count() == 1);
        }
    }

    TEST_CASE("MqttClient::onMessage")
    {
        MqttUnitTestHarness muth;
        auto mqttClient = muth.initMqttManagerAndCreateMqttClient();

        SUBCASE("emits signal msgReceived")
        {
            // GIVEN
            const SignalSpy spy(mqttClient->msgReceived);

            // WHEN
            char topic[]{ "testTopic" }; // NOLINT(modernize-avoid-c-arrays)
            const mosquitto_message msg{
                .mid = 0,
                .topic = topic,
                .payload = nullptr,
                .payloadlen = 0,
                .qos = 0,
                .retain = false,
            };
            MqttUnitTestHarness::onMessage(mqttClient.get(), &msg);

            // THEN
            REQUIRE(spy.count() == 1);
        }
    }

    TEST_CASE("MqttClient::onSubscribed()")
    {
        MqttUnitTestHarness muth;
        auto mqttClient = muth.initMqttManagerAndCreateMqttClient();

        muth.createMosquittoClientMockAndInjectIntoMqttClient(mqttClient);

        const int grantedQos[]{ 0 }; // NOLINT(modernize-avoid-c-arrays)
        const int msgId{ 1 };
        const std::string topic{ "testTopic" };

        When(Method(muth.clientMock(), subscribe)).AlwaysReturnAndSet(MOSQ_ERR_SUCCESS, msgId);

        SUBCASE("sets SUBSCRIPTION state to SUBSCRIBED if a subscription operation is pending for topic")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTED);
            mqttClient->subscribe(topic);

            // WHEN
            MqttUnitTestHarness::onSubscribed(mqttClient.get(), msgId, 1, grantedQos);

            // THEN
            REQUIRE(mqttClient->subscriptionState.get() == MqttClient::SubscriptionState::SUBSCRIBED);
        }

        SUBCASE("adds topic to list of subscribed topics if a subscription operation is pending for topic")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTED);
            mqttClient->subscribe(topic);

            // WHEN
            MqttUnitTestHarness::onSubscribed(mqttClient.get(), msgId, 1, grantedQos);

            // THEN
            REQUIRE(mqttClient->subscriptions.get().at(0) == topic);
        }
    }

    TEST_CASE("MqttClient::onUnsubscribed()")
    {
        MqttUnitTestHarness muth;
        auto mqttClient = muth.initMqttManagerAndCreateMqttClient();

        muth.createMosquittoClientMockAndInjectIntoMqttClient(mqttClient);

        const int grantedQos[]{ 0 }; // NOLINT(modernize-avoid-c-arrays)
        const int msgId{ 1 };
        const std::string topic{ "testTopic" };

        When(Method(muth.clientMock(), subscribe)).AlwaysReturnAndSet(MOSQ_ERR_SUCCESS, msgId);
        When(Method(muth.clientMock(), unsubscribe)).AlwaysReturnAndSet(MOSQ_ERR_SUCCESS, msgId);

        SUBCASE("sets SUBSCRIPTION state to UNSUBSCRIBED if a subscription operation is pending for topic")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTED);
            mqttClient->subscribe(topic);
            MqttUnitTestHarness::onSubscribed(mqttClient.get(), msgId, 1, grantedQos);
            REQUIRE(mqttClient->subscriptionState.get() == MqttClient::SubscriptionState::SUBSCRIBED);
            mqttClient->unsubscribe(topic);

            // WHEN
            MqttUnitTestHarness::onUnsubscribed(mqttClient.get(), msgId);

            // THEN
            REQUIRE(mqttClient->subscriptionState.get() == MqttClient::SubscriptionState::UNSUBSCRIBED);
        }

        SUBCASE("adds topic to list of subscribed topics if a subscription operation is pending for topic")
        {
            // GIVEN
            mqttClient->connectionState.set(MqttClient::ConnectionState::CONNECTED);
            mqttClient->subscribe(topic);
            MqttUnitTestHarness::onSubscribed(mqttClient.get(), msgId, 1, grantedQos);
            REQUIRE(mqttClient->subscriptions.get().at(0) == topic);
            mqttClient->unsubscribe(topic);

            // WHEN
            MqttUnitTestHarness::onUnsubscribed(mqttClient.get(), msgId);

            // THEN
            REQUIRE(mqttClient->subscriptions.get().empty());
        }
    }

    TEST_CASE("MqttClient::onError()")
    {
        MqttUnitTestHarness muth;
        auto mqttClient = muth.initMqttManagerAndCreateMqttClient();

        SUBCASE("emits signal error")
        {
            // GIVEN
            const SignalSpy spy(mqttClient->error);

            // WHEN
            MqttUnitTestHarness::onError(mqttClient.get());

            // THEN
            REQUIRE(spy.count() == 1);
        }
    }

    TEST_CASE("MqttClient::onReadOpRequested()")
    {
        MqttUnitTestHarness muth;
        auto mqttClient = muth.initMqttManagerAndCreateMqttClient();

        muth.createMosquittoClientMockAndInjectIntoMqttClient(mqttClient);
        Fake(Method(muth.clientMock(), loopRead));

        SUBCASE("calls MosquittoClient::loopRead once")
        {
            // GIVEN
            MqttUnitTestHarness::onReadOpRequested(mqttClient.get());

            // THEN
            Verify(Method(muth.clientMock(), loopRead)).Once();
        }
    }

    TEST_CASE("MqttClient::onWriteOpRequested()")
    {
        MqttUnitTestHarness muth;
        auto mqttClient = muth.initMqttManagerAndCreateMqttClient();

        muth.createMosquittoClientMockAndInjectIntoMqttClient(mqttClient);
        Fake(Method(muth.clientMock(), wantWrite));
        Fake(Method(muth.clientMock(), loopWrite));

        SUBCASE("calls MosquittoClient::wantWrite once")
        {
            // GIVEN
            MqttUnitTestHarness::onWriteOpRequested(mqttClient.get());

            // THEN
            Verify(Method(muth.clientMock(), wantWrite)).Once();
        }

        SUBCASE("calls MosquittoClient::loopWrite once if MosquittoClient::wantWrite returns true")
        {
            // GIVEN
            When(Method(muth.clientMock(), wantWrite)).Return(true);

            // WHEN
            MqttUnitTestHarness::onWriteOpRequested(mqttClient.get());

            // THEN
            Verify(Method(muth.clientMock(), loopWrite)).Once();
        }

        SUBCASE("calls MosquittoClient::loopWrite once if MosquittoClient::wantWrite returns false")
        {
            // GIVEN
            When(Method(muth.clientMock(), wantWrite)).Return(false);

            // WHEN
            MqttUnitTestHarness::onWriteOpRequested(mqttClient.get());

            // THEN
            Verify(Method(muth.clientMock(), loopWrite)).Never();
        }
    }

    TEST_CASE("MqttClient::onMiscTaskRequested()")
    {
        MqttUnitTestHarness muth;
        auto mqttClient = muth.initMqttManagerAndCreateMqttClient();

        muth.createMosquittoClientMockAndInjectIntoMqttClient(mqttClient);
        Fake(Method(muth.clientMock(), loopMisc));

        SUBCASE("calls MosquittoClient::loopMisc once")
        {
            // GIVEN
            MqttUnitTestHarness::onMiscTaskRequested(mqttClient.get());

            // THEN
            Verify(Method(muth.clientMock(), loopMisc)).Once();
        }
    }

    TEST_CASE("MqttClient::EventLoopHook")
    {
        MqttUnitTestHarness muth;
        auto mqttClient = muth.initMqttManagerAndCreateMqttClient();

        muth.createMosquittoClientMockAndInjectIntoMqttClient(mqttClient);
        Fake(Method(muth.clientMock(), wantWrite));
        Fake(Method(muth.clientMock(), loopWrite));

        SUBCASE("MqttClient::EventLoopHook::engage() engages hook if socket >= 0")
        {
            // GIVEN
            auto socket = 0;

            // WHEN
            MqttUnitTestHarness::eventLoopHook(mqttClient.get()).engage(socket);

            // THEN
            REQUIRE(MqttUnitTestHarness::eventLoopHook(mqttClient.get()).isEngaged());
        }

        SUBCASE("MqttClient::EventLoopHook::engage() does not engage hook if socket < 0")
        {
            // GIVEN
            auto socket = -1;

            // WHEN
            MqttUnitTestHarness::eventLoopHook(mqttClient.get()).engage(socket);

            // THEN
            REQUIRE_FALSE(MqttUnitTestHarness::eventLoopHook(mqttClient.get()).isEngaged());
        }

        SUBCASE("MqttClient::EventLoopHook::disengage() disengages engaged hook")
        {
            // GIVEN
            auto socket = 0;
            MqttUnitTestHarness::eventLoopHook(mqttClient.get()).engage(socket);
            REQUIRE(MqttUnitTestHarness::eventLoopHook(mqttClient.get()).isEngaged());

            // WHEN
            MqttUnitTestHarness::eventLoopHook(mqttClient.get()).disengage();

            // THEN
            REQUIRE_FALSE(MqttUnitTestHarness::eventLoopHook(mqttClient.get()).isEngaged());
        }
    }

    // TODO -> SubscriptionsRegistry
}
