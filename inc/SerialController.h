//
// Created by Pooria Alaei on 8/11/2026 AD.
//

#ifndef SERIALFORGE_SERIALCONTROLLER_H
#define SERIALFORGE_SERIALCONTROLLER_H

#include "SerialConnection.h"


using namespace serialforge;

class SerialController : public QObject
{
    Q_OBJECT

public:
    explicit SerialController(SerialConnection& connection);

    void notifyStateChanged(SerialConnectionState state);
    void notifyPortsChanged(const std::vector<std::string>& ports);
    void notifyDataReceived();

    signals:
    void stateChanged(SerialConnectionState state);
    void portsChanged(const std::vector<std::string>& ports);
    void dataReceived();

private:
    SerialConnection& connection_;
};


#endif // SERIALFORGE_SERIALCONTROLLER_H
