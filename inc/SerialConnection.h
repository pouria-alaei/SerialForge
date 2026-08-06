//
// Created by Pooria Alaei on 8/6/2026 AD.
//

#ifndef SERIALFORGE_SERIALCONNECTION_H
#define SERIALFORGE_SERIALCONNECTION_H
#include <QSerialPort>
#include <thread>
#include <semaphore>

namespace serialforge
{
    struct SerialPortSettings
    {
        QSerialPort::BaudRate baud = QSerialPort::Baud115200;
        QString path = "/dev/cu.serial0";
        QSerialPort::DataBits data_bits = QSerialPort::Data8;
        QSerialPort::StopBits stop_bits = QSerialPort::OneStop;
        QSerialPort::Parity parity = QSerialPort::NoParity;
        QSerialPort::FlowControl flow_control = QSerialPort::NoFlowControl;
    };

    class SerialConnection
    {

        SerialConnection(const SerialConnection&) = delete;
        SerialConnection& operator=(const SerialConnection&) = delete;

        QSerialPort serial_port_;
        //std::thread worker_thread_;
        std::thread serial_thread_;
        void serialLoop();
        std::atomic<bool> running_{false};
        std::vector<std::string> port_list_{};
        mutable std::mutex port_list_mutex_;
        std::binary_semaphore ports_changed_semaphore_ {0};
        QString opened_port_ {};
        SerialPortSettings last_settings_;
        bool reopen_ {false};
        bool isOpen_ {false};
    public:

        SerialConnection();
        ~SerialConnection();

        bool open(const SerialPortSettings& settings);
        void close(const bool& byUser);
        bool isOpen() const;
        qint64 send(const QByteArray& data);

        void start();
        void stop();
        bool isRunning() const;
        std::vector<std::string> getPortList() const;
        void waitForPortListChanged();
    };
}


#endif //SERIALFORGE_SERIALCONNECTION_H
