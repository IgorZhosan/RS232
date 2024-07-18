#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <QListWidgetItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , serialPort(new QSerialPort(this))
    , connectionCheckTimer(new QTimer(this))
    , portsUpdateTimer(new QTimer(this)) // Инициализация нового таймера
{
    setupUi();
    fillPortsInfo();

    connect(serialPort, &QSerialPort::readyRead, this, &MainWindow::readData);
    connect(serialPort, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
    connect(connectionCheckTimer, &QTimer::timeout, this, &MainWindow::checkConnection);
    connect(portsUpdateTimer, &QTimer::timeout, this, &MainWindow::updatePortsInfo); // Подключение таймера обновления портов
    portsUpdateTimer->start(2000); // Обновляем информацию о портах каждые 2 секунды
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    this->resize(600, 400);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout *comboLayout = new QHBoxLayout();

    portComboBox = new QComboBox(centralWidget);
    comboLayout->addWidget(portComboBox);

    baudRateComboBox = new QComboBox(centralWidget);
    baudRateComboBox->addItems({"9600", "19200", "38400", "57600", "115200"});
    comboLayout->addWidget(baudRateComboBox);

    connectButton = new QPushButton("Подключиться", centralWidget);
    comboLayout->addWidget(connectButton);
    connect(connectButton, &QPushButton::clicked, this, &MainWindow::on_connectButton_clicked);

    mainLayout->addLayout(comboLayout);

    QHBoxLayout *portsLayout = new QHBoxLayout();

    QVBoxLayout *availablePortsLayout = new QVBoxLayout();
    availablePortsLabel = new QLabel("Доступные порты", centralWidget);
    availablePortsList = new QListWidget(centralWidget);
    availablePortsList->setFixedSize(200, 50);
    availablePortsLayout->addWidget(availablePortsLabel);
    availablePortsLayout->addWidget(availablePortsList);

    QVBoxLayout *usedPortsLayout = new QVBoxLayout();
    usedPortsLabel = new QLabel("Недоступные порты", centralWidget);
    usedPortsList = new QListWidget(centralWidget);
    usedPortsList->setFixedSize(200, 50);
    usedPortsLayout->addWidget(usedPortsLabel);
    usedPortsLayout->addWidget(usedPortsList);

    portsLayout->addLayout(availablePortsLayout);
    portsLayout->addLayout(usedPortsLayout);

    mainLayout->addLayout(portsLayout);

    textEdit = new QTextEdit(centralWidget);
    textEdit->setReadOnly(true);
    mainLayout->addWidget(textEdit);

    statusLabel = new QLabel("Отключено", centralWidget);
    statusLabel->setStyleSheet("QLabel { color : red; }");
    mainLayout->addWidget(statusLabel);

    developedByLabel = new QLabel("Разработано: Игорь Жосан", centralWidget);
    developedByLabel->setStyleSheet("QLabel { font-size: 10px; color: gray; }");
    mainLayout->addWidget(developedByLabel);

    statusBar = new QStatusBar(this);
    this->setStatusBar(statusBar);

    this->setCentralWidget(centralWidget);
}

void MainWindow::fillPortsInfo()
{
    portComboBox->clear();
    availablePortsList->clear();
    usedPortsList->clear();

    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        QString portName = info.portName();
        QString itemText = portName;
        QSerialPort testPort;
        testPort.setPort(info);
        if (testPort.open(QIODevice::ReadWrite)) {
            itemText += " (Свободен)";
            QListWidgetItem *item = new QListWidgetItem(portName, availablePortsList);
            item->setForeground(QColor("#008000")); // Темно-зеленый цвет
            testPort.close();
        } else {
            itemText += " (Занят)";
            QString baudRate = QString::number(testPort.baudRate());
            QListWidgetItem *item = new QListWidgetItem(portName + " " + baudRate + " (Занят)", usedPortsList);
            item->setForeground(Qt::red);
        }
        portComboBox->addItem(itemText, portName);
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
        serialPort->setPortName(portComboBox->currentData().toString());
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

void MainWindow::updatePortsInfo()
{
    static QList<QString> previousPorts;

    const auto infos = QSerialPortInfo::availablePorts();
    QList<QString> currentPorts;
    for (const QSerialPortInfo &info : infos) {
        currentPorts.append(info.portName());
    }

    if (currentPorts != previousPorts) {
        fillPortsInfo();
        previousPorts = currentPorts;
    }
}
