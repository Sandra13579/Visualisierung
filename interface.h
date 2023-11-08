#ifndef INTERFACE_H
#define INTERFACE_H

#include <QObject>
#include <QMqttClient>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>

class Interface : public QObject
{
    Q_OBJECT
public:
    explicit Interface(QObject *parent = nullptr);
    void ConnectToBroker(QString ip, int port = 1883);
    void DisconnectFromBroker();

public slots:
    void SendCharging(bool chargingState, int stationId);

private:
    const QString topicCharging = "Charging/<No>/Load"; //begin/abort charging -> GW4

    QMqttClient *m_mqttClient;
    void PublishMqttMessage(QString topic, QString payload);
    void PublishMqttMessage(QString topic, QString payload, quint8 qos, bool retain);
    void ReconnectToBroker();

private slots:
     void UpdateConnectionState(QMqttClient::ClientState state);


signals:

};

#endif // INTERFACE_H
