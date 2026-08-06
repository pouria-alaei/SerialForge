//
// Created by Pooria Alaei on 8/6/2026 AD.
//

#include "../inc/SerialConnection.h"
#include <filesystem>
#include <iostream>
#include <mutex>

#include "../inc/common.h"
namespace fs = std::filesystem;
constexpr uint32_t LOOP_SLEEP {100};
constexpr uint32_t SCAN_INTERVAL_MS {1000};
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
        serial_port_.setBaudRate(settings.baud);
        serial_port_.setParity(settings.parity);
        serial_port_.setDataBits(settings.data_bits);
        serial_port_.setStopBits(settings.stop_bits);
        serial_port_.setFlowControl(settings.flow_control);
        serial_port_.setParity(settings.parity);
        serial_port_.setPortName(settings.path);
        if (serial_port_.open(QIODevice::ReadWrite))
        {
            std::cout<<std::format("SerialPort::open({}) OK",settings.path.toStdString())<<std::endl;
            opened_port_=settings.path;
            last_settings_ = settings;
            isOpen_ = true;
            return true;
        }else
        {
            std::cerr<<std::format("SerialPort::open({}) error ",settings.path.toStdString())<< std::endl;
            return false;
        }
    }

    void SerialConnection::close(const bool& byUser)
    {
        if (byUser)
        {
            reopen_ = false;
            opened_port_.clear();
        }

        if (!serial_port_.isOpen())
        {
            return;
        }

        serial_port_.close();
        isOpen_=false;

        std::cout << "SerialPort::close() OK" << std::endl;
    }

    bool SerialConnection::isOpen()const
    {
        return isOpen_;
    }

    qint64 SerialConnection::send(const QByteArray& data)
    {
        return 0;
    }

    std::vector<std::string> SerialConnection::getPortList() const
    {
        std::lock_guard<std::mutex> lock(port_list_mutex_);
        return port_list_;
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
                    open(last_settings_);
                    reopen_=false;
                }
            }
        }
        ports_changed_semaphore_.acquire();
    }

    void SerialConnection::serialLoop()
    {
        const fs::path devPath {"/dev/"};
        {
            std::lock_guard<std::mutex> lock(port_list_mutex_);
            port_list_ =  listPorts(devPath);
            for (const auto& p: port_list_)
            {
                std::cout << p << std::endl;
            }
        }
        uint32_t loopCnt {0};
        uint32_t updatePortsTickStart {0};
        while (running_)
        {
            if (loopCnt==0)
            {
                updatePortsTickStart=0;
            }

            if ((loopCnt-updatePortsTickStart)*LOOP_SLEEP>SCAN_INTERVAL_MS)
            {
                updatePortsTickStart=loopCnt;
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

            std::this_thread::sleep_for(std::chrono::milliseconds(LOOP_SLEEP));
            loopCnt++;
        }
    }
}
