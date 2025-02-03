/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Marco Thaller <marco.thaller@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include <KDFoundation/core_application.h>
#include <KDMqtt/mqtt.h>

using namespace KDFoundation;
using namespace KDMqtt;

int main()
{
    const bool useEncryptedConnection = true;

    const Url url("test.mosquitto.org");
    const int port = useEncryptedConnection ? 8883 : 1883;

    const auto caFilePath = std::filesystem::current_path().append("mosquitto.org.crt").string();
    const File caFile(caFilePath);

    const std::string topic = "mytopic";
    const std::string payload = "Hello World!";

    CoreApplication app;

    MqttManager::instance().init();
    auto mqttClient = MqttManager::instance().createClient("KDMqttClient", MqttManager::ClientOption::CLEAN_SESSION);

    auto onMqttConnectionStateChanged = [&](const MqttClient::ConnectionState &connectionState) {
        if (connectionState == MqttClient::ConnectionState::CONNECTED) {
            mqttClient->subscribe(topic.c_str());
        }
        if (connectionState == MqttClient::ConnectionState::DISCONNECTED) {
            app.quit();
        }
    };
    std::ignore = mqttClient->connectionState.valueChanged().connect(onMqttConnectionStateChanged);

    auto onMqttSubscriptionStateChanged = [&](const MqttClient::SubscriptionState &subscriptionState) {
        if (subscriptionState == MqttClient::SubscriptionState::SUBSCRIBED) {
            mqttClient->publish(nullptr, topic.c_str(), payload.length(), payload.c_str());
        }
        if (subscriptionState == MqttClient::SubscriptionState::UNSUBSCRIBED) {
            mqttClient->disconnect();
        }
    };
    std::ignore = mqttClient->subscriptionState.valueChanged().connect(onMqttSubscriptionStateChanged);

    auto onMqttMessageReceived = [&](const MqttClient::Message &message) {
        const auto topic = message.topic;
        const auto payload = std::string(message.payload.toStdString());
        spdlog::info("Received MQTT message. Topic: {}. Payload: {}", topic, payload);
        mqttClient->unsubscribe(topic.c_str());
    };
    std::ignore = mqttClient->msgReceived.connect(onMqttMessageReceived);

    if (useEncryptedConnection) {
        mqttClient->setTls(caFile);
    }

    mqttClient->connect(url, port);

    app.exec();
}
