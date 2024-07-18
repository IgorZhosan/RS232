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
    , portsUpdateTimer(new QTimer(this))
    , currentFontSize(10) // Инициализация начального размера шрифта
{
    setupUi();
    fillPortsInfo();

    connect(serialPort, &QSerialPort::readyRead, this, &MainWindow::readData);
    connect(serialPort, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
    connect(connectionCheckTimer, &QTimer::timeout, this, &MainWindow::checkConnection);
    connect(portsUpdateTimer, &QTimer::timeout, this, &MainWindow::updatePortsInfo);
    portsUpdateTimer->start(2000); // Обновляем информацию о портах каждые 2 секунды
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    this->resize(600, 500);

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
    availablePortsLabel->setStyleSheet("QLabel { font-size: 14px; }"); // Увеличение размера шрифта
    availablePortsList = new QListWidget(centralWidget);
    availablePortsList->setFixedSize(200, 50);
    availablePortsLayout->addWidget(availablePortsLabel);
    availablePortsLayout->addWidget(availablePortsList);

    QVBoxLayout *usedPortsLayout = new QVBoxLayout();
    usedPortsLabel = new QLabel("Недоступные порты", centralWidget);
    usedPortsLabel->setStyleSheet("QLabel { font-size: 14px; }"); // Увеличение размера шрифта
    usedPortsList = new QListWidget(centralWidget);
    usedPortsList->setFixedSize(200, 50);
    usedPortsLayout->addWidget(usedPortsLabel);
    usedPortsLayout->addWidget(usedPortsList);

    portsLayout->addLayout(availablePortsLayout);
    portsLayout->addLayout(usedPortsLayout);

    mainLayout->addLayout(portsLayout);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();

    QPushButton *binaryButton = new QPushButton("Binary", centralWidget);
    connect(binaryButton, &QPushButton::clicked, this, &MainWindow::showBinary);
    buttonsLayout->addWidget(binaryButton);

    QPushButton *octalButton = new QPushButton("Octal", centralWidget);
    connect(octalButton, &QPushButton::clicked, this, &MainWindow::showOctal);
    buttonsLayout->addWidget(octalButton);

    QPushButton *decimalButton = new QPushButton("Decimal", centralWidget);
    connect(decimalButton, &QPushButton::clicked, this, &MainWindow::showDecimal);
    buttonsLayout->addWidget(decimalButton);

    QPushButton *hexButton = new QPushButton("Hex", centralWidget);
    connect(hexButton, &QPushButton::clicked, this, &MainWindow::showHex);
    buttonsLayout->addWidget(hexButton);

    mainLayout->addLayout(buttonsLayout);

    textEdit = new QTextEdit(centralWidget);
    textEdit->setReadOnly(true);
    mainLayout->addWidget(textEdit);

    QHBoxLayout *fontButtonsLayout = new QHBoxLayout();

    QPushButton *increaseFontSizeButton = new QPushButton("+", centralWidget);
    connect(increaseFontSizeButton, &QPushButton::clicked, this, &MainWindow::increaseFontSize);
    fontButtonsLayout->addWidget(increaseFontSizeButton);

    QPushButton *decreaseFontSizeButton = new QPushButton("-", centralWidget);
    connect(decreaseFontSizeButton, &QPushButton::clicked, this, &MainWindow::decreaseFontSize);
    fontButtonsLayout->addWidget(decreaseFontSizeButton);

    mainLayout->addLayout(fontButtonsLayout);

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
            QListWidgetItem *item = new QListWidgetItem(portName + " (9600)", usedPortsList); // Пример добавления скорости порта
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
    textData.append(data); // Сохраняем прочитанные данные для конвертации
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

void MainWindow::increaseFontSize()
{
    if (currentFontSize < 20) { // Задаем максимальный размер шрифта
        currentFontSize++;
        QFont font = textEdit->font();
        font.setPointSize(currentFontSize);
        textEdit->setFont(font);
    }
}

void MainWindow::decreaseFontSize()
{
    if (currentFontSize > 6) { // Задаем минимальный размер шрифта
        currentFontSize--;
        QFont font = textEdit->font();
        font.setPointSize(currentFontSize);
        textEdit->setFont(font);
    }
}

void MainWindow::showBinary()
{
    QString binaryData;
    for (char byte : textData) {
        binaryData.append(QString("%1 ").arg(static_cast<unsigned char>(byte), 8, 2, QChar('0')));
    }
    textEdit->setText(binaryData);
}

void MainWindow::showOctal()
{
    QString octalData;
    for (char byte : textData) {
        octalData.append(QString("%1 ").arg(static_cast<unsigned char>(byte), 3, 8, QChar('0')));
    }
    textEdit->setText(octalData);
}

void MainWindow::showDecimal()
{
    QString decimalData;
    for (char byte : textData) {
        decimalData.append(QString::number(static_cast<unsigned char>(byte)) + " ");
    }
    textEdit->setText(decimalData);
}

void MainWindow::showHex()
{
    QString hexData = textData.toHex().toUpper(); // Конвертируем данные в 16-ричную систему счисления
    QString formattedHexData;
    for (int i = 0; i < hexData.length(); i += 2) {
        formattedHexData += hexData.mid(i, 2) + " ";
    }
    textEdit->setText(formattedHexData.trimmed());
}
