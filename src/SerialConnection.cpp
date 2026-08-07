//
// Created by Pooria Alaei on 8/6/2026 AD.
//

#include "../inc/SerialConnection.h"
#include <filesystem>
#include <iostream>
#include <mutex>

#include "../inc/common.h"
namespace fs = std::filesystem;
constexpr std::chrono::milliseconds  LOOP_SLEEP {100};
constexpr std::chrono::milliseconds  PORTS_UPDATE_INTERVAL {500};

namespace serialforge
{
    SerialConnection::SerialConnection()=default;

    SerialConnection::~SerialConnection()
    {
        stop();
    }

    void SerialConnection::stop()
    {
        running_=false;
        if (serial_thread_.joinable())
        {
            serial_thread_.join();
        }
    }

    void SerialConnection::start()
    {
        if (running_)return;
        running_=true;
        serial_thread_= std::thread(&SerialConnection::serialLoop, this);
    }

    bool SerialConnection::open(const SerialPortSettings& settings)
    {
        {
            std::lock_guard<std::mutex> lock(settings_mutex_);
            serial_port_->setBaudRate(settings.baud);
            serial_port_->setParity(settings.parity);
            serial_port_->setDataBits(settings.data_bits);
            serial_port_->setStopBits(settings.stop_bits);
            serial_port_->setFlowControl(settings.flow_control);
            serial_port_->setParity(settings.parity);
            serial_port_->setPortName(settings.path);
            next_settings_ = settings;
            open_requested_ = true;
        }
        requested_semaphore_.release();

        return true;
    }

    void SerialConnection::close(const bool& byUser)
    {
        if (byUser)
        {
            reopen_ = false;
            opened_port_.clear();
        }

        if (!serial_port_->isOpen())
        {
            return;
        }
        close_requested_ = true;
        requested_semaphore_.release();
    }

    bool SerialConnection::isOpen()const
    {
        return isOpen_;
    }


    qint64 SerialConnection::send(const QByteArray& data)
    {
        std::lock_guard<std::mutex> lock(tx_settings_mutex_);
        SerialTXRequest request;
        request.data = data;
        request.timeTag = std::chrono::steady_clock::now();
        if (next_request_==SerialTXRequest{})
        {
            next_request_ = request;
        }else if (local_request_ == SerialTXRequest{})
        {
            local_request_ = next_request_;
            next_request_ = request;
        }else
        {
            scheduled_=true;
            next_schedule_.runtime_queue.push(request);
            next_schedule_.loop = false;
        }
        tx_requested_ = true;
        requested_semaphore_.release();
        return 0;
    }


    std::vector<std::string> SerialConnection::getPortList() const
    {
        std::lock_guard<std::mutex> lock(port_list_mutex_);
        return port_list_;
    }

    bool SerialConnection::setSchedule(const std::vector<SerialTXRequest>& serial_tx_requests,const bool loop, const std::chrono::milliseconds interval)
    {
        if (serial_tx_requests.empty())return false;
        std::lock_guard<std::mutex> lock(tx_settings_mutex_);
        next_schedule_ = {};
        next_schedule_.interval = interval;
        next_schedule_.loop = loop;
        next_schedule_.requestsScript = serial_tx_requests;
        requested_semaphore_.release();
        return true;
    }

    void SerialConnection::clearSchedule()
    {
        std::lock_guard<std::mutex> lock(tx_settings_mutex_);
        scheduled_ = false;
        next_schedule_ = {};
        local_schedule_ = {};
    }
    namespace
    {
        std::vector<std::string> listPorts(fs::path path)
        {
            std::vector<std::string> port_list;
            const OsType osType = get_os();
            if (!port_list.empty()) port_list.clear();
            try
            {
                if (fs::exists(path) && fs::is_directory(path))
                {
                    for (const auto& p : fs::directory_iterator(path))
                    {
                        const std::string path {p.path().filename().string()};
                        if (osType==OsType::MAC_OS)
                        {
                            if (std::string_view(path).starts_with("cu.")  )
                            {
                                // std::cout << p.path().filename().string() << std::endl;
                                port_list.push_back(path);
                            }
                        }else
                        {
                            if (std::string_view(path).starts_with("ttyS")  )
                            {
                                // std::cout << p.path().filename().string() << std::endl;
                                port_list.push_back(path);
                            }
                        }
                    }
                }else
                {
                    std::cerr << "Provided path does not exist" << std::endl;
                }
            }catch (const fs::filesystem_error& e)
            {
                std::cerr << "Error: " << e.what() << std::endl;
            }
            return port_list;
        }
    }


    void SerialConnection::waitForPortListChanged()
    {
        std::vector<std::string> new_port_list = getPortList();
        auto last_port = opened_port_.toStdString();
        for (auto& p: new_port_list)
        {
            p = "/dev/" + p;
            std::cout << "New Port After Change:" << p << std::endl;
        }
        if (isOpen())
        {
            std::cout << "Last open port" << opened_port_.toStdString() << std::endl;
            reopen_ = true;
            if(std::ranges::find(new_port_list, last_port)==new_port_list.end())
            {
                std::cerr << "Port disconnected closing port" << std::endl;
                close(false);
            }
        }else
        {
            if(std::ranges::find(new_port_list, last_port)!=new_port_list.end())
            {
                if (reopen_){
                    std::cout << "Port Connected Reopening" << std::endl;
                    open(local_settings_);
                    reopen_=false;
                }
            }
        }
        ports_changed_semaphore_.acquire();
    }

    void SerialConnection::serialLoop()
    {
        serial_port_ = std::make_unique<QSerialPort>();
        const fs::path devPath {"/dev/"};
        {
            std::lock_guard<std::mutex> lock(port_list_mutex_);
            port_list_ =  listPorts(devPath);
            for (const auto& p: port_list_)
            {
                std::cout << p << std::endl;
            }
        }
        auto updatePortsTickStart = std::chrono::steady_clock::now();
        auto updateScheduleLoopIntervalStart = std::chrono::steady_clock::now();
        bool schedulePendingRestart {false};
        while (running_)
        {
            requested_semaphore_.try_acquire_for(LOOP_SLEEP);

            if (close_requested_)
            {
                close_requested_ = false;
                serial_port_->close();
                isOpen_=false;
                std::cout << "SerialPort::close() OK" << std::endl;
            }

            if (open_requested_)
            {
                {
                    std::lock_guard<std::mutex> lock(settings_mutex_);
                    local_settings_ = next_settings_;
                }
                if (serial_port_->open(QIODevice::ReadWrite))
                {
                    std::cout<<std::format("SerialPort::open({}) OK",local_settings_.path.toStdString())<<std::endl;
                    opened_port_=local_settings_.path;
                    isOpen_ = true;
                    open_requested_=false;
                }else
                {
                    std::cerr<<std::format("SerialPort::open({}) error ",local_settings_.path.toStdString())<< std::endl;
                }
            }

            if (tx_requested_)
            {
                {
                    std::lock_guard<std::mutex> lock(tx_settings_mutex_);
                    if (local_request_ == SerialTXRequest {})
                    {
                        local_request_ = next_request_;
                        next_request_ = {};
                    }
                }
                if (local_request_.timeTag<=std::chrono::steady_clock::now())
                {

                    if (serial_port_->isOpen())
                    {
                        if (serial_port_->write(local_request_.data)!=-1)
                        {
                            std::cout<<std::format("TX:{} Bytes",local_request_.data.size())<<std::endl;

                            std::lock_guard<std::mutex> lock(tx_settings_mutex_);
                            if (next_request_==SerialTXRequest {})tx_requested_ = false;
                            local_request_ = {};
                        }else
                        {
                            std::cerr<<std::format("TX_ERR:{} Bytes",local_request_.data.size())<<std::endl;
                            std::lock_guard<std::mutex> lock(tx_settings_mutex_);
                            txError_=true;
                            local_request_={};
                            next_request_={};
                            local_schedule_ = SerialTXSchedule {};
                            next_schedule_ = SerialTXSchedule {};
                            tx_requested_ = false;
                        }
                    }else
                    {
                        std::cerr<<std::format("TX_ERR_NOT_OPEN:{} Bytes",local_request_.data.size())<<std::endl;
                        std::lock_guard<std::mutex> lock(tx_settings_mutex_);
                        txError_=true;
                        local_request_={};
                        next_request_={};
                        local_schedule_ = SerialTXSchedule {};
                        next_schedule_ = SerialTXSchedule {};
                        tx_requested_ = false;
                    }
                }
            }

            if (scheduled_)
            {
                if (next_schedule_.restart)
                {
                    std::lock_guard<std::mutex> lock(tx_settings_mutex_);
                    std::chrono::milliseconds time_shift {};
                    bool shift_calculated = false;
                    for (auto& request : next_schedule_.requestsScript)
                    {
                        if (!shift_calculated)
                        {
                            shift_calculated=true;
                            time_shift = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - request.timeTag);;
                        }
                        request.timeTag += time_shift;
                        next_schedule_.runtime_queue.push(request);
                    }
                    local_schedule_.restart = false;
                    local_schedule_ = next_schedule_;
                    next_schedule_ = {};
                    local_schedule_.restart = false;
                    schedulePendingRestart = false;
                }

                if (local_schedule_.runtime_queue.empty() && local_schedule_.loop && !schedulePendingRestart)
                {
                    updateScheduleLoopIntervalStart = std::chrono::steady_clock::now();
                    schedulePendingRestart = true;
                }

                if (schedulePendingRestart && std::chrono::steady_clock::now() - updateScheduleLoopIntervalStart > local_schedule_.interval)
                {
                    std::lock_guard<std::mutex> lock(tx_settings_mutex_);
                    local_schedule_.restart = true;
                    next_schedule_ = local_schedule_;
                    local_schedule_ = {};
                }

                if (!local_schedule_.runtime_queue.empty())
                {
                    std::lock_guard<std::mutex> lock(tx_settings_mutex_);
                    const auto& first = local_schedule_.runtime_queue.top();
                    if (next_request_ == SerialTXRequest {})
                    {
                        next_request_ = first;
                        local_schedule_.runtime_queue.pop();
                        tx_requested_ = true;
                    }
                }

            }

            if (std::chrono::steady_clock::now() - updatePortsTickStart >= PORTS_UPDATE_INTERVAL)
            {
                updatePortsTickStart = std::chrono::steady_clock::now();
                bool ports_changed = false;
                {
                    std::vector<std::string> new_port_list = listPorts(devPath);

                    std::lock_guard<std::mutex> lock(port_list_mutex_);
                    if (new_port_list!=port_list_)
                    {
                        port_list_ = new_port_list;
                        ports_changed =true;
                        for (const auto& p: port_list_)
                        {
                            std::cout << p << std::endl;
                        }
                        std::cout << "-----------" << std::endl;
                    }
                }
                if (ports_changed)
                {
                    ports_changed_semaphore_.release();
                }
            }
            if (serial_port_->isOpen())
                serial_port_->close();

            serial_port_.reset();
        }
    }
}
