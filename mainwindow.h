#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "arserialport.h"
#include "coloranalyzerminolta.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_debugToolConnectButton_clicked();

    void on_debugToolDisconnectButton_clicked();

    void on_minoltaConnectButton_clicked();

    void on_minoltaDisconnectButton_clicked();

    //void on_comboBox_debugTool_activated(const QString &arg1);

    void on_start_button_clicked();

    //void on_pushButton_clicked();

    void on_actionactionRefresh_Ports_2_triggered();

    //void on_start_button_clicked(bool checked);

private:
    Ui::MainWindow *ui;
    ArSerialPort * serialPort;
    ColorAnalyzerMinolta * minolta;
};
#endif // MAINWINDOW_H
