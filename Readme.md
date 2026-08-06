# SerialForge

SerialForge is a lightweight, cross-platform serial communication tool built with modern C++ and Qt.

The project is designed to provide a clean foundation for discovering, opening, monitoring, and communicating with serial devices on Linux and macOS.

## Features

* Automatic serial-port discovery
* Detection of connected and disconnected ports
* Thread-safe access to shared port data
* Event notification using C++20 semaphores
* Configurable serial-port settings
* Background serial-management thread
* Linux and macOS support
* Qt-based serial communication

## Current Status

SerialForge is under active development.

Currently implemented:

* Serial-port scanning
* Port-list change detection
* Thread lifecycle management
* Mutex-protected shared data
* Semaphore-based change notification
* Opening and closing serial ports
* Basic automatic reconnect detection

Planned features:

* Asynchronous data reception
* Thread-safe transmit queue
* Multiple simultaneous connections
* Serial data logging
* Hex and text display modes
* Qt graphical interface
* Device identification using VID, PID, and serial number
* Linux and macOS packaging

## Requirements

* C++20-compatible compiler
* CMake
* Qt 6
* Qt Serial Port module

## Build

```bash
git clone https://github.com/YOUR_USERNAME/SerialForge.git
cd SerialForge

cmake -S . -B build
cmake --build build
```

Run:

```bash
./build/serialforge
```

On macOS, the executable may be generated inside an application bundle:

```bash
open build/serialforge.app
```

## Basic Usage

```cpp
serialforge::SerialConnection connection;

connection.start();

serialforge::SerialConnection::SerialPortSettings settings;
settings.path = "/dev/cu.usbserial-0001";
settings.baud = QSerialPort::Baud115200;

if (connection.open(settings))
{
    // Serial port opened successfully
}
```

## Architecture

SerialForge separates serial-port management from application logic.

```text
Application / GUI
        │
        ├── Read available ports
        ├── Request open or close
        └── Handle port-change events
        │
Serial Manager Thread
        ├── Scan serial ports
        ├── Detect connection changes
        ├── Manage serial connections
        └── Notify consumers
```

Shared data is protected with `std::mutex`, while `std::binary_semaphore` is used to notify another thread when the port list changes.

## Supported Platforms

| Platform | Status         |
| -------- | -------------- |
| macOS    | In development |
| Linux    | In development |
| Windows  | Planned        |

## Contributing

Contributions, bug reports, and design discussions are welcome.

Please open an issue before making major architectural changes.

## License

This project is licensed under the MIT License.

---

SerialForge aims to become a simple, reliable, and reusable serial communication foundation for desktop and embedded-development tools.
