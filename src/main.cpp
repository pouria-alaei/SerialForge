#include <iostream>
#include <QApplication>
#include "../inc/mainwindow.h"
#include "../inc/SerialConnection.h"
using namespace std;
using namespace serialforge;
constexpr uint32_t PORT_UPDATE_THREAD_SLEEP_MS {100};
namespace
{
  void updatePortListLoop(SerialConnection& serial_connection)
  {
    while (true)
    {
      serial_connection.waitForPortListChanged();
      cout << "serial ports change detected" << endl;
    }
  }
}

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  MainWindow main_window;

  SerialConnection serial_connection;
  SerialPortSettings serial_port_settings;
  serial_port_settings.path = "/dev/cu.usbserial-0001";
  serial_port_settings.baud = QSerialPort::Baud115200;
  serial_connection.start();
  serial_connection.open(serial_port_settings);


  main_window.show();
  auto updatePortsListThread = std::thread(&updatePortListLoop, std::ref(serial_connection));
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  return 0;
}