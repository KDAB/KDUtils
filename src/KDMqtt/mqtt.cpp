/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Marco Thaller <marco.thaller@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "mqtt.h"
#include <memory>
#include <spdlog/spdlog.h>

#define CHECK_AND_LOG_MOSQUITTO_RESULT(result) checkAndLogMosquittoResult(result, __FUNCTION__)

namespace KDMqtt {

constexpr std::chrono::duration c_miscTaskInterval = std::chrono::seconds(1);

using namespace KDFoundation;

MqttManager::MqttManager()
    : m_isInitialized{ false }
    , m_mosquittoLib(&MosquittoLib::instance())
    , m_logger{ KDUtils::Logger::logger("mqtt", spdlog::level::info) }
{
}

MqttManager::~MqttManager()
{
    MqttManager::cleanup();
}

MqttManager &MqttManager::instance()
{
    static MqttManager s_instance;
    return s_instance;
}

int MqttManager::init()
{
    int result = MOSQ_ERR_UNKNOWN;

    if (!m_isInitialized) {
        result = m_mosquittoLib->init();
        const auto hasError = CHECK_AND_LOG_MOSQUITTO_RESULT(result);
        m_isInitialized = !hasError;
        if (m_isInitialized) {
            int major, minor, revision = 0;
            version(&major, &minor, &revision);
            SPDLOG_LOGGER_INFO(m_logger, "Using libmosquitto v{}.{}.{}", major, minor, revision);
        }
    } else {
        SPDLOG_LOGGER_WARN(m_logger, "Library is already initialized.");
    }
    return result;
}

int MqttManager::cleanup()
{
    const auto result = m_mosquittoLib->cleanup();
    const auto hasError = CHECK_AND_LOG_MOSQUITTO_RESULT(result);
    m_isInitialized = hasError ? m_isInitialized : false;
    return result;
}

std::shared_ptr<IMqttClient> MqttManager::createClient(const std::string &clientId, ClientOptions options)
{
    if (!m_isInitialized) {
        SPDLOG_LOGGER_ERROR(m_logger, "MqttManager is not initialized. Call MqttManager::init() before attempting to create MqttClient objects.");
        return {};
    }
    auto client = new MqttClient(m_logger, clientId, options);
    return std::shared_ptr<IMqttClient>(client);
}

bool MqttManager::isInitialized() const
{
    return m_isInitialized;
}

bool MqttManager::isValidTopicNameForSubscription(const std::string &topic)
{
    return m_mosquittoLib->isValidTopicNameForSubscription(topic);
}

int MqttManager::version(int *major, int *minor, int *revision)
{
    return m_mosquittoLib->version(major, minor, revision);
}

bool MqttManager::checkAndLogMosquittoResult(int result, const char *func)
{
    const auto isError = (result != MOSQ_ERR_SUCCESS);
    if (isError) {
        SPDLOG_LOGGER_ERROR(m_logger, "{}() - error({}): {}", func, result, errorString(result));
    }
    return isError;
}

std::string_view MqttManager::connackString(int connackCode)
{
    return m_mosquittoLib->connackString(connackCode);
}

std::string_view MqttManager::errorString(int errorCode)
{
    return m_mosquittoLib->errorString(errorCode);
}

std::string_view MqttManager::reasonString(int reasonCode)
{
    return m_mosquittoLib->reasonString(reasonCode);
}

MqttClient::MqttClient(std::shared_ptr<spdlog::logger> logger, const std::string &clientId, MqttManager::ClientOptions options)
    : m_logger{ std::move(logger) }
{
    auto client = std::make_unique<MosquittoClient>(clientId, static_cast<bool>(options & MqttManager::ClientOption::CLEAN_SESSION));
    m_mosquitto.init(std::move(client), this);

    if (!(options & MqttManager::ClientOption::DONT_USE_OS_CERTIFICATE_STORE)) {
        // NOTE: on Windows, OpenSSL used by mosquitto doesn't use the system store by default
        const auto result = m_mosquitto.client()->tlsEnableUseOsCertificates();
        MqttManager::instance().CHECK_AND_LOG_MOSQUITTO_RESULT(result);
    }

    m_establishConnectionTaskTimer.interval.set(std::chrono::milliseconds(200));
    m_establishConnectionTaskTimer.running.set(false);
    std::ignore = m_establishConnectionTaskTimer.timeout.connect(&MqttClient::establishConnectionTask, this);

    m_eventLoopHook.init(c_miscTaskInterval, this);
    m_subscriptionsRegistry.init(this);
}

int MqttClient::setTls(const File &cafile)
{
    SPDLOG_LOGGER_TRACE(m_logger, "{}() - cafile: {}", __FUNCTION__, cafile.path());

    if (connectionState.get() != ConnectionState::DISCONNECTED) {
        SPDLOG_LOGGER_ERROR(m_logger, "Setting TLS is only allowed when disconnected.");
        return MOSQ_ERR_UNKNOWN;
    }

    if (!cafile.exists()) {
        SPDLOG_LOGGER_ERROR(m_logger, "Specified cafile does not exist.");
        return MOSQ_ERR_UNKNOWN;
    }

    auto result = m_mosquitto.client()->tlsSet(cafile.path(), std::nullopt, std::nullopt, std::nullopt);
    MqttManager::instance().CHECK_AND_LOG_MOSQUITTO_RESULT(result);

    return result;
}

int MqttClient::setUsernameAndPassword(const std::string &username, const std::string &password)
{
    SPDLOG_LOGGER_TRACE(m_logger, "{}() - username: {}, password: {}", __FUNCTION__, username, password);

    if (connectionState.get() != ConnectionState::DISCONNECTED) {
        SPDLOG_LOGGER_ERROR(m_logger, "Setting username and password is only allowed when disconnected.");
        return MOSQ_ERR_UNKNOWN;
    }

    const auto result = m_mosquitto.client()->usernamePasswordSet(username, password);
    MqttManager::instance().CHECK_AND_LOG_MOSQUITTO_RESULT(result);
    return result;
}

int MqttClient::setWill(const std::string &topic, const ByteArray *payload, QOS qos, bool retain)
{
    SPDLOG_LOGGER_TRACE(m_logger, "{}() - topic: {}, qos: {}, retain: {}", __FUNCTION__, topic, static_cast<int>(qos), retain);

    if (connectionState.get() != ConnectionState::DISCONNECTED) {
        SPDLOG_LOGGER_ERROR(m_logger, "Setting will is only allowed when disconnected.");
        return MOSQ_ERR_UNKNOWN;
    }

    const int payloadlen = payload ? static_cast<int>(payload->size()) : 0;
    const auto payloadData = payload ? payload->constData() : nullptr;
    const auto result = m_mosquitto.client()->willSet(topic.c_str(), payloadlen, payloadData, static_cast<int>(qos), retain);
    MqttManager::instance().CHECK_AND_LOG_MOSQUITTO_RESULT(result);
    return result;
}

int MqttClient::connect(const Url &host, uint16_t port, std::chrono::seconds keepalive)
{
    SPDLOG_LOGGER_TRACE(m_logger, "{}() - host: {}, port: {}, keepalive: {}", __FUNCTION__, host.url(), port, keepalive.count());

    if (connectionState.get() == ConnectionState::CONNECTING) {
        SPDLOG_LOGGER_ERROR(m_logger, "Already connecting to host.");
        return MOSQ_ERR_UNKNOWN;
    }

    if (connectionState.get() == ConnectionState::CONNECTED) {
        SPDLOG_LOGGER_ERROR(m_logger, "Already connected to a host. Disconnect from current host first.");
        return MOSQ_ERR_UNKNOWN;
    }

    connectionState.set(ConnectionState::CONNECTING);
    const auto result = m_mosquitto.client()->connectAsync(host.url().c_str(), port, keepalive.count());

    const auto hasError = MqttManager::instance().CHECK_AND_LOG_MOSQUITTO_RESULT(result);
    if (!hasError) {
        m_establishConnectionTaskTimer.running.set(true);
        m_eventLoopHook.engage(m_mosquitto.client()->socket());
    }
    return result;
}

int MqttClient::disconnect()
{
    SPDLOG_LOGGER_TRACE(m_logger, "{}()", __FUNCTION__);

    m_establishConnectionTaskTimer.running.set(false);

    if (connectionState.get() == ConnectionState::DISCONNECTING) {
        SPDLOG_LOGGER_ERROR(m_logger, "Already disconnecting from host.");
        return MOSQ_ERR_UNKNOWN;
    }

    if (connectionState.get() == ConnectionState::DISCONNECTED) {
        SPDLOG_LOGGER_ERROR(m_logger, "Not connected to any host.");
        return MOSQ_ERR_UNKNOWN;
    }

    connectionState.set(ConnectionState::DISCONNECTING);
    const auto result = m_mosquitto.client()->disconnect();
    MqttManager::instance().CHECK_AND_LOG_MOSQUITTO_RESULT(result);
    return result;
}

int MqttClient::publish(int *msgId, const std::string &topic, const ByteArray *payload, QOS qos, bool retain)
{
    SPDLOG_LOGGER_TRACE(m_logger, "{}() - topic: {}, qos: {}, retain: {}", __FUNCTION__, topic, static_cast<int>(qos), retain);

    if (connectionState.get() == ConnectionState::DISCONNECTED) {
        SPDLOG_LOGGER_ERROR(m_logger, "Not connected to any host.");
        return MOSQ_ERR_UNKNOWN;
    }

    const int payloadlen = payload ? static_cast<int>(payload->size()) : 0;
    const auto payloadData = payload ? payload->constData() : nullptr;
    const auto result = m_mosquitto.client()->publish(msgId, topic, payloadlen, payloadData, static_cast<int>(qos), retain);
    MqttManager::instance().CHECK_AND_LOG_MOSQUITTO_RESULT(result);
    return result;
}

int MqttClient::subscribe(const std::string &pattern, QOS qos)
{
    SPDLOG_LOGGER_TRACE(m_logger, "{}() - subscribe pattern: {}, qos: {}", __FUNCTION__, pattern, static_cast<int>(qos));

    if (connectionState.get() == ConnectionState::DISCONNECTED) {
        SPDLOG_LOGGER_ERROR(m_logger, "Not connected to any host.");
        return MOSQ_ERR_UNKNOWN;
    }

    int msgId;
    const auto result = m_mosquitto.client()->subscribe(&msgId, pattern, static_cast<int>(qos));
    const auto hasError = MqttManager::instance().CHECK_AND_LOG_MOSQUITTO_RESULT(result);
    if (!hasError) {
        const auto topic = std::string(pattern);
        m_subscriptionsRegistry.registerPendingRegistryOperation(topic, msgId);
        subscriptionState.set(SubscriptionState::SUBSCRIBING);
    }
    return result;
}

int MqttClient::unsubscribe(const std::string &pattern)
{
    SPDLOG_LOGGER_TRACE(m_logger, "{}() - unsubscribe pattern: {}", __FUNCTION__, pattern);

    if (connectionState.get() == ConnectionState::DISCONNECTED) {
        SPDLOG_LOGGER_ERROR(m_logger, "Not connected to any host.");
        return MOSQ_ERR_UNKNOWN;
    }

    int msgId;
    const auto result = m_mosquitto.client()->unsubscribe(&msgId, pattern);
    const auto hasError = MqttManager::instance().CHECK_AND_LOG_MOSQUITTO_RESULT(result);
    if (!hasError) {
        const auto topic = std::string(pattern);
        m_subscriptionsRegistry.registerPendingRegistryOperation(topic, msgId);
        subscriptionState.set(SubscriptionState::UNSUBSCRIBING);
    }
    return result;
}

void MqttClient::onConnected(int connackCode)
{
    SPDLOG_LOGGER_TRACE(m_logger, "{}() - connackCode({}): {}", __FUNCTION__, connackCode, MqttManager::instance().connackString(connackCode));

    m_establishConnectionTaskTimer.running.set(false);

    const auto hasError = (connackCode != 0);
    if (hasError) {
        // TODO -> I'm uncertain if calling unhookFromEventLoop() here is perfectly fine in every case
        // I noticed on_diconnect (sometimes) gets called after on_connect was called with CONNACK!=0
        // in this case we may want to stay hooked to the event loop until we're finally disconnected
        // and on_disconnect is called.
        // For now I won't call unhookFromEventLoop() here and see if we ever run into a sw-path were
        // we never unhook from event loop. In this case we would need to add the following call here
        // (only for certain connackCodes):
        // unhookFromEventLoop();
    }

    const auto tlsIsEnabled = (m_mosquitto.client()->sslGet() != nullptr);
    SPDLOG_LOGGER_INFO(m_logger, "This connection {} TLS encrypted", tlsIsEnabled ? "is" : "is not");

    const auto state = hasError ? ConnectionState::DISCONNECTED : ConnectionState::CONNECTED;
    connectionState.set(state);
}

void MqttClient::onDisconnected(int reasonCode)
{
    SPDLOG_LOGGER_TRACE(m_logger, "{}() - reasonCode({}): {}", __FUNCTION__, reasonCode, MqttManager::instance().reasonString(reasonCode));

    m_eventLoopHook.disengage();

    connectionState.set(ConnectionState::DISCONNECTED);
}

void MqttClient::onPublished(int msgId)
{
    SPDLOG_LOGGER_TRACE(m_logger, "{}() - msgId: {}", __FUNCTION__, msgId);
    msgPublished.emit(msgId);
}

void MqttClient::onMessage(const mosquitto_message *msg)
{
    SPDLOG_LOGGER_TRACE(m_logger, "{}() - msgId: {}, topic: {}", __FUNCTION__, msg->mid, msg->topic);

    Message message{
        .msgId = msg->mid,
        .topic = msg->topic,
        .payload = ByteArray(static_cast<char *>(msg->payload), msg->payloadlen),
        .qos = static_cast<QOS>(msg->qos),
        .retain = msg->retain
    };
    msgReceived.emit(std::move(message));
}

void MqttClient::onSubscribed(int msgId, int qosCount, const int *grantedQos)
{
    // we only handle subscriptions to one single topic with one single QOS value for now.
    // in case mosquitto_subscribe_multiple is added to MosquittoClient some time in the future,
    // add handling of multiple topic/QOS pairs here.
    assert(qosCount == 1);

    const auto topic = m_subscriptionsRegistry.registerTopicSubscriptionAndReturnTopicName(msgId, static_cast<QOS>(grantedQos[0]));
    SPDLOG_LOGGER_TRACE(m_logger, "{}() - msgId: {}, topic: {}, qosCount: {}, grantedQos: {}", __FUNCTION__, msgId, topic, qosCount, grantedQos[0]);

    const auto state = m_subscriptionsRegistry.subscribedTopics().empty() ? SubscriptionState::UNSUBSCRIBED : SubscriptionState::SUBSCRIBED;
    subscriptionState.set(state);
    subscriptions.set(m_subscriptionsRegistry.subscribedTopics());
}

void MqttClient::onUnsubscribed(int msgId)
{
    const auto topic = m_subscriptionsRegistry.unregisterTopicSubscriptionAndReturnTopicName(msgId);
    SPDLOG_LOGGER_TRACE(m_logger, "{}() - msgId: {}, topic: {}", __FUNCTION__, msgId, topic);

    const auto state = m_subscriptionsRegistry.subscribedTopics().empty() ? SubscriptionState::UNSUBSCRIBED : SubscriptionState::SUBSCRIBED;
    subscriptionState.set(state);
    subscriptions.set(m_subscriptionsRegistry.subscribedTopics());
}

void MqttClient::onLog(int level, const char *str) const
{
    SPDLOG_LOGGER_DEBUG(m_logger, "{}() - level: {}, string: {})", __FUNCTION__, level, str);
}

void MqttClient::onError()
{
    SPDLOG_LOGGER_ERROR(m_logger, "{}()", __FUNCTION__);
    m_establishConnectionTaskTimer.running.set(false);
    error.emit();
}

void MqttClient::onReadOpRequested()
{
    auto result = m_mosquitto.client()->loopRead();
    MqttManager::instance().CHECK_AND_LOG_MOSQUITTO_RESULT(result);
}

void MqttClient::onWriteOpRequested()
{
    const auto writeOpIsPending = m_mosquitto.client()->wantWrite();
    if (!writeOpIsPending) {
        return;
    }

    auto result = m_mosquitto.client()->loopWrite();
    MqttManager::instance().CHECK_AND_LOG_MOSQUITTO_RESULT(result);
}

void MqttClient::onMiscTaskRequested()
{
    auto result = m_mosquitto.client()->loopMisc();
    MqttManager::instance().CHECK_AND_LOG_MOSQUITTO_RESULT(result);
}

void MqttClient::establishConnectionTask()
{
    onReadOpRequested();
    onWriteOpRequested();
    onMiscTaskRequested();
}

void MqttClient::EventLoopHook::init(const std::chrono::milliseconds miscTaskInterval, MqttClient *parent)
{
    assert(parent != nullptr);
    SPDLOG_LOGGER_TRACE(parent->m_logger, "{}() - miscTaskInterval: {} ms", __FUNCTION__, miscTaskInterval.count());

    this->parent = parent;

    miscTaskTimer.interval.set(miscTaskInterval);
    miscTaskTimer.running.set(false);
    std::ignore = miscTaskTimer.timeout.connect(&MqttClient::onMiscTaskRequested, parent);
}

void MqttClient::EventLoopHook::engage(const int socket)
{
    SPDLOG_LOGGER_TRACE(parent->m_logger, "{}() - socket: {}", __FUNCTION__, socket);

    if (!isSetup()) {
        SPDLOG_LOGGER_ERROR(parent->m_logger, "EventLoopHook is not initialized. Call MqttClient::EventLoopHook::init() first.");
        return;
    }

    if (isEngaged()) {
        SPDLOG_LOGGER_ERROR(parent->m_logger, "EventLoopHook is already engaged.");
        return;
    }

    if (socket < 0) {
        SPDLOG_LOGGER_ERROR(parent->m_logger, "Cannot engage EventLoopHook due to invalid socket.");
        return;
    }

    readOpNotifier = std::make_unique<FileDescriptorNotifier>(socket, FileDescriptorNotifier::NotificationType::Read);
    writeOpNotifier = std::make_unique<FileDescriptorNotifier>(socket, FileDescriptorNotifier::NotificationType::Write);

    std::ignore = readOpNotifier->triggered.connect(&MqttClient::onReadOpRequested, parent);
    std::ignore = writeOpNotifier->triggered.connect(&MqttClient::onWriteOpRequested, parent);

    miscTaskTimer.running.set(true);
}

void MqttClient::EventLoopHook::disengage()
{
    SPDLOG_LOGGER_TRACE(parent->m_logger, "{}()", __FUNCTION__);

    if (!isEngaged()) {
        SPDLOG_LOGGER_ERROR(parent->m_logger, "EventLoopHook is already disengaged.");
        return;
    }

    miscTaskTimer.running.set(false);

    readOpNotifier->triggered.disconnectAll();
    writeOpNotifier->triggered.disconnectAll();

    readOpNotifier = {};
    writeOpNotifier = {};
}

bool MqttClient::EventLoopHook::isSetup() const
{
    return (parent != nullptr);
}

bool MqttClient::EventLoopHook::isEngaged() const
{
    return (readOpNotifier && writeOpNotifier);
}

void MqttClient::MosquittoClientDependency::init(std::unique_ptr<MosquittoClient> &&client, MqttClient *parent)
{
    assert(parent != nullptr);
    SPDLOG_LOGGER_TRACE(parent->m_logger, "{}()", __FUNCTION__);

    mosquittoClient = std::move(client);

    std::ignore = mosquittoClient->connected.connect(&MqttClient::onConnected, parent);
    std::ignore = mosquittoClient->disconnected.connect(&MqttClient::onDisconnected, parent);
    std::ignore = mosquittoClient->published.connect(&MqttClient::onPublished, parent);
    std::ignore = mosquittoClient->message.connect(&MqttClient::onMessage, parent);
    std::ignore = mosquittoClient->subscribed.connect(&MqttClient::onSubscribed, parent);
    std::ignore = mosquittoClient->unsubscribed.connect(&MqttClient::onUnsubscribed, parent);
    std::ignore = mosquittoClient->log.connect(&MqttClient::onLog, parent);
    std::ignore = mosquittoClient->error.connect(&MqttClient::onError, parent);
}

MosquittoClient *MqttClient::MosquittoClientDependency::client()
{
    return mosquittoClient.get();
}

void MqttClient::SubscriptionsRegistry::init(MqttClient *parent)
{
    assert(parent != nullptr);
    SPDLOG_LOGGER_TRACE(parent->m_logger, "{}()", __FUNCTION__);

    this->parent = parent;
}

void MqttClient::SubscriptionsRegistry::registerPendingRegistryOperation(std::string_view topic, int msgId)
{
    SPDLOG_LOGGER_TRACE(parent->m_logger, "{}() - topic:{}, msgId: {}", __FUNCTION__, topic, msgId);

    topicByMsgIdOfPendingOperations[msgId] = topic;
}

std::string MqttClient::SubscriptionsRegistry::registerTopicSubscriptionAndReturnTopicName(int msgId, QOS grantedQos)
{
    SPDLOG_LOGGER_TRACE(parent->m_logger, "{}() - msgId: {}, grantedQos:{}", __FUNCTION__, msgId, static_cast<int>(grantedQos));

    auto it = topicByMsgIdOfPendingOperations.find(msgId);
    if (it == topicByMsgIdOfPendingOperations.end()) {
        SPDLOG_LOGGER_ERROR(parent->m_logger, "No pending operation with msgId: {}.", msgId);
        return {};
    }

    auto topic = it->second;
    topicByMsgIdOfPendingOperations.erase(it);

    qosByTopicOfActiveSubscriptions[topic] = grantedQos;

    return topic;
}

std::string MqttClient::SubscriptionsRegistry::unregisterTopicSubscriptionAndReturnTopicName(int msgId)
{
    SPDLOG_LOGGER_TRACE(parent->m_logger, "{}() - msgId: {}", __FUNCTION__, msgId);

    auto it = topicByMsgIdOfPendingOperations.find(msgId);
    if (it == topicByMsgIdOfPendingOperations.end()) {
        SPDLOG_LOGGER_ERROR(parent->m_logger, "No pending operation with msgId: {}.", msgId);
        return {};
    }

    const auto &topic = it->second;
    topicByMsgIdOfPendingOperations.erase(it);

    qosByTopicOfActiveSubscriptions.erase(topic);

    return topic;
}

std::vector<std::string> MqttClient::SubscriptionsRegistry::subscribedTopics() const
{
    std::vector<std::string> keys;
    keys.reserve(qosByTopicOfActiveSubscriptions.size());
    for (const auto &pair : qosByTopicOfActiveSubscriptions) {
        keys.push_back(pair.first);
    }
    std::sort(keys.begin(), keys.end());
    return keys;
}

IMqttClient::QOS MqttClient::SubscriptionsRegistry::grantedQosForTopic(const std::string &topic) const
{
    return qosByTopicOfActiveSubscriptions.at(topic);
}

} // namespace KDMqtt
