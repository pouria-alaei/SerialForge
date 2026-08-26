//
// Created by Pooria Alaei on 8/6/2026 AD.
//

#ifndef SERIALFORGE_MAINWINDOW_H
#define SERIALFORGE_MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>


class QComboBox;
class SerialController;
namespace serialforge
{
    enum class SerialConnectionState;
}
QT_BEGIN_NAMESPACE

namespace Ui
{
    class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
        explicit MainWindow(
        SerialController& controller_,
        QWidget* parent = nullptr
    );
    ~MainWindow() override;
    void updatePortList(const std::vector<std::string>& ports);
private:
    Ui::MainWindow* ui;
    void onStateChanged(serialforge::SerialConnectionState state);
    QComboBox* portComboBox_;
    QComboBox* parityComboBox_;
    QComboBox* stopBitsComboBox_;
    QComboBox* baudComboBox_;
    QComboBox* dataBitsComboBox_;
    QPushButton* openPort_;
    QPushButton* closePort_;
};


#endif //SERIALFORGE_MAINWINDOW_H
