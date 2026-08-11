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

    enum class SerialConnectionState
    {
        Disconnected,
        Opening,
        Connected,
        Reconnecting,
        Failed
    };
    using StateCallback = std::function<void(SerialConnectionState)>;
    using PortsCallback = std::function<void(std::vector<std::string>)>;
    using DataRXCallback = std::function<void()>;


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
        QString opened_port_ {};
        SerialPortSettings next_settings_;
        std::atomic<bool> reopen_ {false};
        std::atomic<bool> isOpen_ {false};
        std::atomic<bool> open_requested_ {false};
        std::atomic<bool> close_requested_ {false};

        mutable std::mutex settings_mutex_;
        mutable std::mutex tx_settings_mutex_;
        mutable std::mutex state_mutex_;

        std::atomic<bool> tx_requested_ {false};
        std::binary_semaphore requested_semaphore_ {0};
        SerialTXSchedule local_schedule_;
        SerialTXSchedule next_schedule_;
        SerialTXRequest local_request_;
        SerialTXRequest next_request_;
        std::atomic<bool> scheduled_ {false};
        std::atomic<bool> txError_ {false};

        std::queue<QByteArray> rx_queue_;
        std::mutex rx_mutex_;
        SerialConnectionState state_ = SerialConnectionState::Disconnected;
        StateCallback state_callback_;
        PortsCallback port_callback_;
        DataRXCallback data_callback_;
        void handlePortListChanged_();
        void handleRx_();

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
        bool setSchedule(const std::vector<SerialTXRequest>& serial_tx_requests,const bool loop, const std::chrono::milliseconds interval);
        void clearSchedule();
        void setStateCallback(StateCallback callback);
        void setPortsCallback(PortsCallback callback);
        void setDataRXCallback(DataRXCallback callback);

        bool hasData();

        QByteArray readData();

    };
}


#endif //SERIALFORGE_SERIALCONNECTION_H
