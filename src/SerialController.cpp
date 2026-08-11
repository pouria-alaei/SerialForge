//
// Created by Pooria Alaei on 8/11/2026 AD.
//

#include "../inc/SerialController.h"

void SerialController::notifyStateChanged(SerialConnectionState state)
{
    QMetaObject::invokeMethod(
        this,
        [this, state]()
        {
            emit stateChanged(state);
        },
        Qt::QueuedConnection
    );
}

void SerialController::notifyPortsChanged(const std::vector<std::string>& ports)
{
    QMetaObject::invokeMethod(
        this,
        [this, ports]()
        {
            emit portsChanged(ports);
        },
        Qt::QueuedConnection
    );
}

void SerialController::notifyDataReceived()
{
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            emit dataReceived();
        },
        Qt::QueuedConnection
    );
}

SerialController::SerialController(SerialConnection& connection)
    : QObject(nullptr),
      connection_(connection)
{
    connection_.setStateCallback(
        [this](SerialConnectionState state)
        {
            notifyStateChanged(state);
        }
    );


    connection_.setPortsCallback(
        [this](std::vector<std::string> ports)
        {
            notifyPortsChanged(ports);
        }
    );

    connection_.setDataRXCallback(
        [this]()
        {
            notifyDataReceived();
        }
    );

}

