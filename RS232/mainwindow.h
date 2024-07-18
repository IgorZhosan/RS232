#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QListWidget>
#include <QStatusBar>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_connectButton_clicked();
    void checkConnection();
    void readData();
    void handleError(QSerialPort::SerialPortError error);
    void updatePortsInfo();

private:
    void setupUi();
    void fillPortsInfo();
    void updateConnectionStatus(bool connected);

    QSerialPort *serialPort;
    QTimer *connectionCheckTimer;
    QTimer *portsUpdateTimer;

    QComboBox *portComboBox;
    QComboBox *baudRateComboBox;
    QPushButton *connectButton;
    QTextEdit *textEdit;
    QLabel *statusLabel;
    QLabel *developedByLabel;
    QLabel *availablePortsLabel;
    QLabel *usedPortsLabel;
    QListWidget *availablePortsList;
    QListWidget *usedPortsList;
    QStatusBar *statusBar;
};

#endif // MAINWINDOW_H
