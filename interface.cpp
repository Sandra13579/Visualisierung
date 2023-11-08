#include "interface.h"

Interface::Interface(QObject *parent)
    : QObject{parent}
{
    m_mqttClient = new QMqttClient(parent);
    connect(m_mqttClient, &QMqttClient::stateChanged, this, &Interface::UpdateConnectionState);
}

void Interface::ConnectToBroker(QString ip, int port)
{
    m_mqttClient->setHostname(ip);
    m_mqttClient->setPort(port);
    m_mqttClient->setUsername("VPJ");
    m_mqttClient->setPassword("R462");
    m_mqttClient->setClientId("Visualisierung");
    m_mqttClient->connectToHost();
}

void Interface::DisconnectFromBroker()
{
    m_mqttClient->disconnectFromHost();
}

void Interface::UpdateConnectionState(QMqttClient::ClientState state)
{
    switch (state)
    {
    case QMqttClient::Connected:
        qDebug() << "MQTT Client with ID" << m_mqttClient->clientId() << "connected to broker:" << m_mqttClient->hostname() << ":" << m_mqttClient->port();
        //QTimer::singleShot(1, this, [this]() { emit connected(); });   //Zum Einschalten der Agenten
        break;
    case QMqttClient::Disconnected:
        qDebug() << "MQTT Client with ID" << m_mqttClient->clientId() << "disconnected!";
        QTimer::singleShot(2000, this, &Interface::ReconnectToBroker);
        break;
    case QMqttClient::Connecting:
        qDebug() << "MQTT connection pending...";
        break;
    }
}

void Interface::ReconnectToBroker()
{
    m_mqttClient->connectToHost();
}

// Publish an MQTT message with with the given payload and topic
void Interface::PublishMqttMessage(QString topic, QString payload)
{
    PublishMqttMessage(topic, payload, 0, false);
}

void Interface::PublishMqttMessage(QString topic, QString payload, quint8 qos, bool retain)
{
    QByteArray message;
    message.append(payload.toStdString());
    m_mqttClient->publish(QMqttTopicName(topic), message, qos, retain);
}

void Interface::SendCharging (bool chargingState, int stationId)
{
    QJsonObject object
        {
            {"charge", chargingState}
        };

    QString payload = QJsonDocument(object).toJson(QJsonDocument::Compact);
    PublishMqttMessage(QString(topicCharging).replace("<No>", QString::number(stationId)), payload);
}