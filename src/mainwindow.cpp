//
// Created by Pooria Alaei on 8/6/2026 AD.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "../inc/mainwindow.h"
#include "ui_mainwindow.h"

#include <QAction>
#include <QLabel>
#include <QMessageBox>
#include <QSerialPort>
#include <QtWidgets/QComboBox>
#include <iostream>

#include "SerialConnection.h"
#include "SerialController.h"


void MainWindow::updatePortList(const std::vector<std::string>& ports)
{
    // std::cout << ports.size() << std::endl;
    portComboBox_->clear();
    for (auto& port: ports)
    {
        portComboBox_->addItem(QString(port.c_str()));
    }
    if (portComboBox_->count()!=0)
    {
        if (portComboBox_->currentIndex()==-1)
        {
            portComboBox_->setCurrentIndex(0);
        }
    }
}

void MainWindow::onStateChanged(serialforge::SerialConnectionState state)
{
    if(state == serialforge::SerialConnectionState::Connected)
    {
        // ui->statusLabel->setText("Connected");
    }
}

MainWindow::MainWindow(SerialController& controller_,QWidget* parent) :
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->menubar->setNativeMenuBar(true);
    ui->statusbar->showMessage("Ready");

    portComboBox_ = new QComboBox(this);
    baudComboBox_ = new QComboBox(this);
    dataBitsComboBox_ = new QComboBox(this);
    stopBitsComboBox_ = new QComboBox(this);
    parityComboBox_ = new QComboBox(this);
    openPort_ = new QPushButton(this);
    closePort_ = new QPushButton(this);

    //Toolbar For Connection
    ui->toolBar->addWidget(openPort_);
    ui->toolBar->addWidget(closePort_);
    ui->toolBar->addSeparator();
    ui->toolBar->addWidget(new QLabel("Port:"));
    ui->toolBar->addWidget(portComboBox_);
    ui->toolBar->addSeparator();
    ui->toolBar->addWidget(new QLabel("Baudrate:"));
    ui->toolBar->addWidget(baudComboBox_);
    ui->toolBar->addSeparator();
    ui->toolBar->addWidget(new QLabel("Data Bits:"));
    ui->toolBar->addWidget(dataBitsComboBox_);
    ui->toolBar->addSeparator();
    ui->toolBar->addWidget(new QLabel("Stop Bits:"));
    ui->toolBar->addWidget(stopBitsComboBox_);
    ui->toolBar->addSeparator();
    ui->toolBar->addWidget(new QLabel("Parity:"));
    ui->toolBar->addWidget(parityComboBox_);

    //Set Toolbar Buttons Text
    openPort_->setText("Open");
    closePort_->setText("Close");
    portComboBox_->setMinimumWidth(200);

    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionAbout, &QAction::triggered, this, [this]()
    {
        QMessageBox::about(this, "About SerialForge", "SerialForge");
    });

    const auto setPage = [this](const QString& pageName)
    {
        ui->statusbar->showMessage(pageName);
    };


   for (auto& baud : serialforge::BAUDRATES_VALUES)
   {
       baudComboBox_->addItem(std::to_string(baud).data());
   }

    stopBitsComboBox_->addItem(std::to_string(serialforge::STOP_BITS_VALUES[0]).data());
    stopBitsComboBox_->addItem(std::to_string(serialforge::STOP_BITS_VALUES[1]).data());

    for (auto& dataBitCnt : serialforge::DATA_BITS_VALUES)
    {
        dataBitsComboBox_->addItem(std::to_string(dataBitCnt).data());
    }

    parityComboBox_->addItem(serialforge::PARITY_VALUES[0].data());
    parityComboBox_->addItem(serialforge::PARITY_VALUES[1].data());
    parityComboBox_->addItem(serialforge::PARITY_VALUES[2].data());

    connect(
        &controller_,
        &SerialController::stateChanged,
        this,
        &MainWindow::onStateChanged
    );

    connect(
        &controller_,
        &SerialController::portsChanged,
        this,
        &MainWindow::updatePortList
        );

}

MainWindow::~MainWindow()
{
    delete ui;
}
