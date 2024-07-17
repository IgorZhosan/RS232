#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>

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

private:
    QSerialPort *serialPort;
    QComboBox *portComboBox;
    QComboBox *baudRateComboBox;
    QPushButton *connectButton;
    QTextEdit *textEdit;
    QStatusBar *statusBar;
    QLabel *statusLabel;
    QLabel *developedByLabel;
    QTimer *connectionCheckTimer;

    void setupUi();
    void fillPortsInfo();
    void updateConnectionStatus(bool connected);
};

#endif // MAINWINDOW_H
