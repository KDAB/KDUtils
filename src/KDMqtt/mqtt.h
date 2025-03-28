/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Marco Thaller <marco.thaller@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#pragma once

#include <KDFoundation/file_descriptor_notifier.h>
#include <KDFoundation/timer.h>
#include <KDMqtt/kdmqtt_global.h>
#include <KDMqtt/mosquitto_wrapper.h>
#include <KDUtils/bytearray.h>
#include <KDUtils/file.h>
#include <KDUtils/flags.h>
#include <KDUtils/logging.h>
#include <KDUtils/url.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>

using namespace KDFoundation;
using namespace KDUtils;

namespace KDMqtt {

constexpr int c_defaultPort = 1883;
constexpr std::chrono::duration c_defaultKeepAlive = std::chrono::minutes(1);

class IMqttClient;

/*
 * Class: IMqttManager
 *
 * This is an abstract class specifying a generic
 * interface exposed to application / business logic
 * providing methods to orchestrate MQTT.
 */
class KDMQTT_API IMqttManager
{
protected:
    IMqttManager() = default;
    virtual ~IMqttManager() { }

public:
    IMqttManager(const IMqttManager &) = delete;
    IMqttManager &operator=(const IMqttManager &) = delete;

    virtual int init() = 0;
    virtual int cleanup() = 0;

    [[nodiscard]] virtual bool isInitialized() const = 0;
    virtual bool isValidTopicNameForSubscription(const std::string &topic) = 0;
};

/*
 * Class: MqttManager
 *
 * This class implements a MQTT manager based on
 * the mosquitto library.
 */
class KDMQTT_API MqttManager : public IMqttManager
{
    friend class MqttClient;
    friend class MqttUnitTestHarness;

private:
    MqttManager();
    ~MqttManager() = default;

public:
    enum ClientOption : uint32_t {
        CLEAN_SESSION = 0x00000001,
        DONT_USE_OS_CERTIFICATE_STORE = 0x00000002
    };
    using ClientOptions = KDUtils::Flags<ClientOption>;

    static MqttManager &instance();

    int init() override;
    int cleanup() override;

    std::shared_ptr<IMqttClient> createClient(const std::string &clientId, ClientOptions options);

    [[nodiscard]] bool isInitialized() const override;
    bool isValidTopicNameForSubscription(const std::string &topic) override;

protected:
    int version(int *major, int *minor, int *revision);

    bool checkAndLogMosquittoResult(int result, const char *func);

    std::string_view connackString(int connackCode);
    std::string_view errorString(int errorCode);
    std::string_view reasonString(int reasonCode);

private:
    bool m_isInitialized;
    MosquittoLib *m_mosquittoLib;
    std::shared_ptr<spdlog::logger> m_logger;
};

/*
 * Class: IMqttClient
 *
 * This is an abstract class specifying a generic
 * interface exposed to application / business logic
 * to access MQTT client implementations.
 */
class KDMQTT_API IMqttClient
{
public:
    enum class ConnectionState {
        CONNECTING,
        CONNECTED,
        DISCONNECTING,
        DISCONNECTED
    };

    enum class SubscriptionState {
        SUBSCRIBING,
        SUBSCRIBED,
        UNSUBSCRIBING,
        UNSUBSCRIBED
    };

    enum class QOS {
        AT_MOST_ONCE = 0,
        AT_LEAST_ONCE = 1,
        EXACTLY_ONCE = 2
    };

    struct Message {
        int msgId;
        std::string topic;
        ByteArray payload;
        QOS qos;
        bool retain;
    };

    KDBindings::Property<ConnectionState> connectionState{ ConnectionState::DISCONNECTED };
    KDBindings::Property<SubscriptionState> subscriptionState{ SubscriptionState::UNSUBSCRIBED };

    KDBindings::Property<std::vector<std::string>> subscriptions{};

    KDBindings::Signal<int /*msgId*/> msgPublished;
    KDBindings::Signal<const Message /*msg*/> msgReceived;

    KDBindings::Signal<> error;

    virtual int setTls(const File &cafile) = 0;
    virtual int setUsernameAndPassword(const std::string &username, const std::string &password) = 0;
    virtual int setWill(const std::string &topic, const ByteArray *payload = nullptr, QOS qos = QOS::AT_MOST_ONCE, bool retain = false) = 0;

    virtual int connect(const Url &host, uint16_t port = c_defaultPort, std::chrono::seconds keepalive = c_defaultKeepAlive) = 0;
    virtual int disconnect() = 0;

    virtual int publish(int *msgId, const std::string &topic, const ByteArray *payload = nullptr, QOS qos = QOS::AT_MOST_ONCE, bool retain = false) = 0;

    virtual int subscribe(const std::string &pattern, QOS qos = QOS::AT_MOST_ONCE) = 0;
    virtual int unsubscribe(const std::string &pattern) = 0;
};

/*
 * Class: MqttClient
 *
 * This class implements a MQTT client functions
 * based on the mosquitto library.
 */
class KDMQTT_API MqttClient : public IMqttClient
{
    friend class MqttManager;
    friend class MqttUnitTestHarness;

protected:
    MqttClient(std::shared_ptr<spdlog::logger> logger, const std::string &clientId, MqttManager::ClientOptions options = MqttManager::ClientOption::CLEAN_SESSION);

public:
    ~MqttClient() = default;

    int setTls(const File &cafile) override;
    int setUsernameAndPassword(const std::string &username, const std::string &password) override;
    int setWill(const std::string &topic, const ByteArray *payload = nullptr, QOS qos = QOS::AT_MOST_ONCE, bool retain = false) override;

    int connect(const Url &host, uint16_t port = c_defaultPort, std::chrono::seconds keepalive = c_defaultKeepAlive) override;
    int disconnect() override;

    int publish(int *msgId, const std::string &topic, const ByteArray *payload = nullptr, QOS qos = QOS::AT_MOST_ONCE, bool retain = false) override;

    int subscribe(const std::string &pattern, QOS qos = QOS::AT_MOST_ONCE) override;
    int unsubscribe(const std::string &pattern) override;

private:
    /*
     * Mosquitto client event handlers
     */
    void onConnected(int connackCode);
    void onDisconnected(int reasonCode);
    void onPublished(int msgId);
    void onMessage(const struct mosquitto_message *msg);
    void onSubscribed(int msgId, int qosCount, const int *grantedQos);
    void onUnsubscribed(int msgId);
    void onLog(int level, const char *str) const;
    void onError();

    /*
     * Event loop handlers
     */
    void onReadOpRequested();
    void onWriteOpRequested();
    void onMiscTaskRequested();

    /*
     * Task to artificially invoke event handlers while the
     * connection is being established after calling
     * mosquitto_connect_async.
     * Using mosquitto_connect_async apparently does not work
     * in case one monitors the client socket themself and the
     * connection uses TLS.
     * Reading the mosquitto API docs, I am not sure if this
     * is a bug or not. Though my hunch would be, that this is
     * a bug, since using mosquitto_connect_async works in case
     * the client socket is monitored and the connection
     * DOES NOT use TLS.
     * Other people have encountered similar behaviour and have
     * mentioned it on GitHub:
     * e.g. https://github.com/eclipse/mosquitto/issues/990
     * To avoid the use of the blocking mosquitto_connect
     * I decided to use this workaround / hack to be able to
     * use the non-blocking mosquitto_connect_async.
     * Thus far I have not encountered any downsides using the
     * workaround.
     */
    void establishConnectionTask();

    /*
     * General member variables
     */
    Timer m_establishConnectionTaskTimer;
    std::shared_ptr<spdlog::logger> m_logger;

    /*
     * This struct modularizes the mechanism to hook mosquitto's
     * so called Network Loop to the application's event loop.
     * This is done by monitoring the client's network socket
     * using FileDescriptorNotifiers and having an additional
     * timer to trigger cyclic misc tasks.
     */
    struct EventLoopHook {
    public:
        void init(std::chrono::milliseconds miscTaskInterval, MqttClient *parent);

        void engage(int socket);
        void disengage();

        [[nodiscard]] bool isSetup() const;
        [[nodiscard]] bool isEngaged() const;

    private:
        std::unique_ptr<FileDescriptorNotifier> readOpNotifier;
        std::unique_ptr<FileDescriptorNotifier> writeOpNotifier;
        Timer miscTaskTimer;
        MqttClient *parent = nullptr;
    };
    EventLoopHook m_eventLoopHook;

    /*
     * This struct modularizes the dependency to the mosquitto
     * library's client implementation.
     * It owns the mosquitto client instance and is responsible
     * for initializing MqttClient with the provided mosquitto
     * client instance.
     * This is also relevant when passing a mosquitto client mock
     * for unit testing.
     */
    struct MosquittoClientDependency {
    public:
        void init(std::unique_ptr<MosquittoClient> &&client, MqttClient *parent);
        MosquittoClient *client();

    private:
        std::unique_ptr<MosquittoClient> m_client;
    };
    MosquittoClientDependency m_mosquitto;

    /*
     * This struct modularizes the registry maintaining all
     * subscriptions to MQTT topics this MqttClient has.
     */
    struct SubscriptionsRegistry {
    public:
        void init(MqttClient *parent);

        void registerPendingRegistryOperation(std::string_view topic, int msgId);
        std::string registerTopicSubscriptionAndReturnTopicName(int msgId, QOS grantedQos);
        std::string unregisterTopicSubscriptionAndReturnTopicName(int msgId);

        std::vector<std::string> subscribedTopics() const;
        QOS grantedQosForTopic(const std::string &topic) const;

    private:
        std::unordered_map<std::string, QOS> qosByTopicOfActiveSubscriptions;
        std::unordered_map<int, std::string> topicByMsgIdOfPendingOperations;
        MqttClient *parent = nullptr;
    };
    SubscriptionsRegistry m_subscriptionsRegistry;
};

} // namespace KDMqtt

OPERATORS_FOR_FLAGS(KDMqtt::MqttManager::ClientOptions);
