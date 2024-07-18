#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QStatusBar>
#include <QByteArray>

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
    void increaseFontSize();
    void decreaseFontSize();
    void showBinary();
    void showOctal();
    void showDecimal();
    void showHex();

private:
    void setupUi();
    void fillPortsInfo();
    void updateConnectionStatus(bool connected);

    QSerialPort *serialPort;
    QTimer *connectionCheckTimer;
    QTimer *portsUpdateTimer;
    QTextEdit *textEdit;
    QComboBox *portComboBox;
    QComboBox *baudRateComboBox;
    QPushButton *connectButton;
    QListWidget *availablePortsList;
    QListWidget *usedPortsList;
    QLabel *statusLabel;
    QLabel *availablePortsLabel;
    QLabel *usedPortsLabel;
    QLabel *developedByLabel;
    QStatusBar *statusBar;
    QByteArray textData;
    int currentFontSize;
};

#endif // MAINWINDOW_H
