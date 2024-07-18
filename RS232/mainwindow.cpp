#include "mainwindow.h"
#include <QTextCodec>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , serialPort(new QSerialPort(this))
    , connectionCheckTimer(new QTimer(this))
{
    setupUi();
    fillPortsInfo();

    connect(serialPort, &QSerialPort::readyRead, this, &MainWindow::readData);
    connect(serialPort, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
    connect(connectionCheckTimer, &QTimer::timeout, this, &MainWindow::checkConnection);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    this->resize(400, 300);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    portComboBox = new QComboBox(centralWidget);
    layout->addWidget(portComboBox);

    baudRateComboBox = new QComboBox(centralWidget);
    baudRateComboBox->addItems({"9600", "19200", "38400", "57600", "115200"});
    layout->addWidget(baudRateComboBox);

    connectButton = new QPushButton("Подключиться", centralWidget);
    layout->addWidget(connectButton);
    connect(connectButton, &QPushButton::clicked, this, &MainWindow::on_connectButton_clicked);

    textEdit = new QTextEdit(centralWidget);
    textEdit->setReadOnly(true);
    layout->addWidget(textEdit);

    statusLabel = new QLabel("Отключено", centralWidget);
    statusLabel->setStyleSheet("QLabel { color : red; }");
    layout->addWidget(statusLabel);

    developedByLabel = new QLabel("Разработано: ЗАО МНИТИ", centralWidget);
    developedByLabel->setStyleSheet("QLabel { font-size: 10px; color: gray; }");
    layout->addWidget(developedByLabel);

    statusBar = new QStatusBar(this);
    this->setStatusBar(statusBar);

    this->setCentralWidget(centralWidget);
}

void MainWindow::fillPortsInfo()
{
    portComboBox->clear();
    // Добавление статических портов COM1-COM10
    for (int i = 1; i <= 10; ++i) {
        portComboBox->addItem("COM" + QString::number(i));
    }

    // Опционально: можете добавить динамическое обнаружение доступных портов
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        if (!portComboBox->findText(info.portName())) {
            portComboBox->addItem(info.portName());
        }
    }
}

void MainWindow::on_connectButton_clicked()
{
    if (serialPort->isOpen()) {
        serialPort->close();
        connectButton->setText("Подключиться");
        statusBar->showMessage("Отключено");
        updateConnectionStatus(false);
        connectionCheckTimer->stop();
    } else {
        serialPort->setPortName(portComboBox->currentText());
        serialPort->setBaudRate(baudRateComboBox->currentText().toInt());
        serialPort->setDataBits(QSerialPort::Data8);
        serialPort->setParity(QSerialPort::NoParity);
        serialPort->setStopBits(QSerialPort::OneStop);
        serialPort->setFlowControl(QSerialPort::NoFlowControl);

        if (serialPort->open(QIODevice::ReadOnly)) {
            connectButton->setText("Отключиться");
            statusBar->showMessage("Подключено к " + serialPort->portName());
            updateConnectionStatus(true);
            connectionCheckTimer->start(1000);
        } else {
            QString errorMsg = "Не удалось подключиться к " + serialPort->portName() + "\nОшибка: " + serialPort->errorString();
            qDebug() << errorMsg;  // Логируем ошибку для отладки
            statusBar->showMessage("Ошибка: " + serialPort->errorString());
            QMessageBox::critical(this, "Ошибка подключения", errorMsg);
            updateConnectionStatus(false);
        }
    }
}

void MainWindow::checkConnection()
{
    if (serialPort->isOpen()) {
        bool portFound = false;
        const auto infos = QSerialPortInfo::availablePorts();
        for (const QSerialPortInfo &info : infos) {
            if (info.portName() == serialPort->portName()) {
                portFound = true;
                break;
            }
        }

        if (!portFound) {
            serialPort->close();
            connectButton->setText("Подключиться");
            statusBar->showMessage("Отключено");
            updateConnectionStatus(false);
            connectionCheckTimer->stop();
            QMessageBox::critical(this, "Соединение потеряно", "Соединение с " + serialPort->portName() + " потеряно.");
        }
    }
}

void MainWindow::readData()
{
    const QByteArray data = serialPort->readAll();
    QString hexData = data.toHex().toUpper(); // Конвертируем данные в 16-ричную систему счисления
    QString formattedHexData;
    for (int i = 0; i < hexData.length(); i += 2) {
        formattedHexData += hexData.mid(i, 2) + " ";
    }
    textEdit->append(formattedHexData.trimmed());
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        statusBar->showMessage("Критическая ошибка: " + serialPort->errorString());
        QMessageBox::critical(this, "Критическая ошибка", "Произошла критическая ошибка: " + serialPort->errorString());
        serialPort->close();
        updateConnectionStatus(false);
        connectionCheckTimer->stop();
    }
}

void MainWindow::updateConnectionStatus(bool connected)
{
    if (connected) {
        statusLabel->setText("Подключено");
        statusLabel->setStyleSheet("QLabel { color : green; }");
    } else {
        statusLabel->setText("Отключено");
        statusLabel->setStyleSheet("QLabel { color : red; }");
    }
}
