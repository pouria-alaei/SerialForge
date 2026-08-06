//
// Created by Pooria Alaei on 8/6/2026 AD.
//

#ifndef SERIALFORGE_MAINWINDOW_H
#define SERIALFORGE_MAINWINDOW_H

#include <QWidget>


QT_BEGIN_NAMESPACE

namespace Ui
{
    class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow* ui;
};


#endif //SERIALFORGE_MAINWINDOW_H
