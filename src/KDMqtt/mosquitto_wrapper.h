/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Marco Thaller <marco.thaller@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.


  This file implements C++ wrappers around the mosquitto C library.

  The classes implemented in this file differ from mosquittopp,
  a wrapper class maintained in the Eclipse mosquitto repository,
  in respect to the following aspects:
  - library and client functions are separated into individual classes
  - all functions are virtual for the purpose of convenient mocking
  - some functions wrapped by mosquittopp are not wrapped here [1]
  - some functions wrapped here are not wrapped by mosquittopp [1]
  - mosquitto event callbacks are exposed as KDBindings::Signals

  [1] as of January 2024
 */

#pragma once

#include <kdbindings/signal.h>
#include <mosquitto.h>
#include <optional>
#include <string>
#include <string_view>

namespace KDMqtt {

/*
 * This is a C++ class wrapping library specific functions
 * of the mosquitto C library.
 */
class MosquittoLib
{
private:
    MosquittoLib() = default;
    ~MosquittoLib() = default;

    MosquittoLib(const MosquittoLib &) = delete;
    MosquittoLib &operator=(const MosquittoLib &) = delete;

public:
    static MosquittoLib &instance()
    {
        static MosquittoLib instance;
        return instance;
    }

    virtual int init()
    {
        return mosquitto_lib_init();
    }

    virtual int cleanup()
    {
        return mosquitto_lib_cleanup();
    }

    virtual int version(int *major, int *minor, int *revision) const
    {
        return mosquitto_lib_version(major, minor, revision);
    }

    [[nodiscard]] virtual std::string_view connackString(int connack_code) const
    {
        return mosquitto_connack_string(connack_code);
    }

    [[nodiscard]] virtual std::string_view errorString(int mosq_errno) const
    {
        return mosquitto_strerror(mosq_errno);
    }

    [[nodiscard]] virtual std::string_view reasonString(int reason_code) const
    {
        return mosquitto_reason_string(reason_code);
    }

    [[nodiscard]] virtual bool isValidTopicNameForSubscription(const std::string &topic) const
    {
        const auto result = mosquitto_sub_topic_check(topic.c_str());
        return (result == MOSQ_ERR_SUCCESS);
    }
};

/*
 * This is a C++ class wrapping client specific functions
 * of the mosquitto C library.
 */
class MosquittoClient
{
public:
    MosquittoClient(const std::string &clientId, bool cleanSession = true)
    {
        m_clientInstance = mosquittoNew(clientId, cleanSession);

        mosquitto_connect_callback_set(m_clientInstance, onConnect);
        mosquitto_disconnect_callback_set(m_clientInstance, onDisconnect);
        mosquitto_publish_callback_set(m_clientInstance, onPublish);
        mosquitto_message_callback_set(m_clientInstance, onMessage);
        mosquitto_subscribe_callback_set(m_clientInstance, onSubscribe);
        mosquitto_unsubscribe_callback_set(m_clientInstance, onUnsubscribe);
        mosquitto_log_callback_set(m_clientInstance, onLog);
    }

    ~MosquittoClient()
    {
        mosquittoDestroy(m_clientInstance);
    }

    mosquitto *mosquittoNew(const std::string &clientId, bool cleanSession)
    {
        return mosquitto_new(clientId.c_str(), cleanSession, this);
    }

    void mosquittoDestroy(mosquitto *client)
    {
        mosquitto_destroy(client);
    }

    virtual int connectAsync(const std::string &host, int port, int keepalive)
    {
        return mosquitto_connect_async(m_clientInstance, host.c_str(), port, keepalive);
    }

    virtual int disconnect()
    {
        return mosquitto_disconnect(m_clientInstance);
    }

    virtual int publish(int *msg_id, const std::string &topic, int payloadlen = 0, const void *payload = nullptr, int qos = 0, bool retain = false)
    {
        return mosquitto_publish(m_clientInstance, msg_id, topic.c_str(), payloadlen, payload, qos, retain);
    }

    virtual int subscribe(int *msg_id, const std::string &sub, int qos = 0)
    {
        return mosquitto_subscribe(m_clientInstance, msg_id, sub.c_str(), qos);
    }

    virtual int unsubscribe(int *msg_id, const std::string &sub)
    {
        return mosquitto_unsubscribe(m_clientInstance, msg_id, sub.c_str());
    }

    virtual int loopMisc()
    {
        return mosquitto_loop_misc(m_clientInstance);
    }

    virtual int loopRead(int max_packets = 1)
    {
        return mosquitto_loop_read(m_clientInstance, max_packets);
    }

    virtual int loopWrite(int max_packets = 1)
    {
        return mosquitto_loop_write(m_clientInstance, max_packets);
    }

    virtual int socket()
    {
        return mosquitto_socket(m_clientInstance);
    }

    virtual bool wantWrite()
    {
        return mosquitto_want_write(m_clientInstance);
    }

    virtual void *sslGet()
    {
        return mosquitto_ssl_get(m_clientInstance);
    }

    virtual int tlsSet(std::optional<const std::string> cafile, std::optional<const std::string> capath, std::optional<const std::string> certfile, std::optional<const std::string> keyfile, int (*pw_callback)(char *buf, int size, int rwflag, void *userdata) = nullptr)
    {
        return mosquitto_tls_set(m_clientInstance, cafile ? cafile->c_str() : nullptr, capath ? capath->c_str() : nullptr, certfile ? certfile->c_str() : nullptr, keyfile ? keyfile->c_str() : nullptr, pw_callback);
    }

    // NOTE: on Windows, OpenSSL used by mosquitto doesn't use the system store by default
    virtual int tlsEnableUseOsCertificates()
    {
        return mosquitto_int_option(m_clientInstance, MOSQ_OPT_TLS_USE_OS_CERTS, 1);
    }

    virtual int tlsDisableUseOsCertificates()
    {
        return mosquitto_int_option(m_clientInstance, MOSQ_OPT_TLS_USE_OS_CERTS, 0);
    }

    virtual int usernamePasswordSet(const std::string &username, const std::string &password)
    {
        return mosquitto_username_pw_set(m_clientInstance, username.c_str(), password.c_str());
    }

    virtual int willSet(const std::string &topic, int payloadlen = 0, const void *payload = nullptr, int qos = 0, bool retain = false)
    {
        return mosquitto_will_set(m_clientInstance, topic.c_str(), payloadlen, payload, qos, retain);
    }

    KDBindings::Signal<int /*connackCode*/> connected;
    KDBindings::Signal<int /*reasonCode*/> disconnected;
    KDBindings::Signal<int /*msgId*/> published;
    KDBindings::Signal<const struct mosquitto_message * /*msg*/> message;
    KDBindings::Signal<int /*msgId*/, int /*qosCount*/, const int * /*grantedQos*/> subscribed;
    KDBindings::Signal<int /*msgId*/> unsubscribed;
    KDBindings::Signal<int /*level*/, const char * /*str*/> log;
    KDBindings::Signal<> error;

private:
    static void onConnect([[maybe_unused]] struct mosquitto *client, void *self, int connackCode)
    {
        static_cast<MosquittoClient *>(self)->connected.emit(connackCode);
    }
    static void onDisconnect([[maybe_unused]] struct mosquitto *client, void *self, int reasonCode)
    {
        static_cast<MosquittoClient *>(self)->disconnected.emit(reasonCode);
    }
    static void onPublish([[maybe_unused]] struct mosquitto *client, void *self, int msgId)
    {
        static_cast<MosquittoClient *>(self)->published.emit(msgId);
    }
    static void onMessage([[maybe_unused]] struct mosquitto *client, void *self, const struct mosquitto_message *message)
    {
        static_cast<MosquittoClient *>(self)->message.emit(message);
    }
    static void onSubscribe([[maybe_unused]] struct mosquitto *client, void *self, int msg_id, int qos_count, const int *granted_qos)
    {
        static_cast<MosquittoClient *>(self)->subscribed.emit(msg_id, qos_count, granted_qos);
    }
    static void onUnsubscribe([[maybe_unused]] struct mosquitto *client, void *self, int msg_id)
    {
        static_cast<MosquittoClient *>(self)->unsubscribed.emit(msg_id);
    }
    static void onLog([[maybe_unused]] struct mosquitto *client, void *self, int level, const char *str)
    {
        static_cast<MosquittoClient *>(self)->log.emit(level, str);
    }
    static void onError([[maybe_unused]] struct mosquitto *client, void *self)
    {
        static_cast<MosquittoClient *>(self)->error.emit();
    }

    struct mosquitto *m_clientInstance;
};

} // namespace KDMqtt
