//
// Created by Pooria Alaei on 8/6/2026 AD.
//

#ifndef SERIALFORGE_SERIALCONNECTION_H
#define SERIALFORGE_SERIALCONNECTION_H
#include <QSerialPort>
#include <queue>
#include <thread>
#include <semaphore>

namespace serialforge
{
    constexpr uint BAUDRATES_VALUES[] {115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200};
    constexpr std::string PARITY_VALUES[] {"NONE","EVEN", "ODD"};
    constexpr uint8_t STOP_BITS_VALUES[] {1, 2};
    constexpr uint8_t DATA_BITS_VALUES[] {8, 7, 6, 5};
    struct SerialPortSettings
    {
        QSerialPort::BaudRate baud = QSerialPort::Baud115200;
        QString path = "/dev/cu.serial0";
        QSerialPort::DataBits data_bits = QSerialPort::Data8;
        QSerialPort::StopBits stop_bits = QSerialPort::OneStop;
        QSerialPort::Parity parity = QSerialPort::NoParity;
        QSerialPort::FlowControl flow_control = QSerialPort::NoFlowControl;
    };

    struct SerialTXRequest
    {
        QByteArray data;
        std::chrono::steady_clock::time_point timeTag;
        bool operator==(const SerialTXRequest&) const = default;
    };

    struct SerialTXCompare
    {
        bool operator()(const SerialTXRequest& a,
                        const SerialTXRequest& b) const
        {
            return a.timeTag > b.timeTag;
        }
    };

    struct SerialTXSchedule
    {
        std::vector<SerialTXRequest> requestsScript;
        std::priority_queue<SerialTXRequest, std::vector<SerialTXRequest>, SerialTXCompare> runtime_queue {};
        bool loop = false;
        std::chrono::milliseconds interval {100};
        bool restart {false};
    };

    class SerialConnection
    {

        SerialConnection(const SerialConnection&) = delete;
        SerialConnection& operator=(const SerialConnection&) = delete;

        // QSerialPort serial_port_;
        std::unique_ptr<QSerialPort> serial_port_;
        //std::thread worker_thread_;
        std::thread serial_thread_;
        void serialLoop();
        std::atomic<bool> running_{false};
        std::vector<std::string> port_list_{};
        mutable std::mutex port_list_mutex_;
        std::binary_semaphore ports_changed_semaphore_ {0};
        QString opened_port_ {};
        SerialPortSettings next_settings_;
        bool reopen_ {false};
        bool isOpen_ {false};
        bool open_requested_ {false};
        bool close_requested_ {false};
        mutable std::mutex settings_mutex_;
        mutable std::mutex tx_settings_mutex_;
        bool tx_requested_ {false};
        std::binary_semaphore requested_semaphore_ {0};
        SerialTXSchedule local_schedule_;
        SerialTXSchedule next_schedule_;
        SerialTXRequest local_request_;
        SerialTXRequest next_request_;
        bool scheduled_ {false};
        bool txError_ {false};

        std::queue<QByteArray> rx_queue_;
        std::mutex rx_mutex_;
        std::counting_semaphore<> rx_semaphore_{0};


    public:

        SerialConnection();
        ~SerialConnection();

        void open(const SerialPortSettings& settings);
        void close(const bool& byUser);
        bool isOpen() const;
        qint64 send(const QByteArray& data);

        void start();
        void stop();
        bool isRunning() const;
        std::vector<std::string> getPortList() const;
        void waitForPortListChanged();
        bool setSchedule(const std::vector<SerialTXRequest>& serial_tx_requests,const bool loop, const std::chrono::milliseconds interval);
        void clearSchedule();
        QByteArray waitForData();
    };
}


#endif //SERIALFORGE_SERIALCONNECTION_H
