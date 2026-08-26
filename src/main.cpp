#include <QApplication>
#include <iostream>

#include "../inc/SerialConnection.h"
#include "../inc/mainwindow.h"
#include "SerialController.h"
using namespace std;
using namespace serialforge;
constexpr uint32_t PORT_UPDATE_THREAD_SLEEP_MS{100};
namespace
{


} // namespace

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);


    SerialConnection serial_connection;
    SerialPortSettings serial_port_settings;
    serial_connection.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    SerialController controller(serial_connection);
    MainWindow main_window(controller);

    // serial_port_settings.path = "/dev/cu.usbserial-0001";
    // serial_port_settings.baud = QSerialPort::Baud115200;
    // serial_connection.open(serial_port_settings);

    main_window.show();


    // while (true)
    // {
    //   serial_connection.send("Pouria");
    //   std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    // }

    return a.exec();
}
