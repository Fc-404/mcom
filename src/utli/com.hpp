#pragma once

#include <QObject>
#include <QOverload>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QThread>
#include <QTimer>
#include <cstdint>
#include <qcontainerfwd.h>
#include <qdebug.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qoverload.h>
#include <qtmetamacros.h>
#include <qtypes.h>

class Com : public QObject {
  Q_OBJECT
public:
  enum ErrorCode {
    ParamError,

    NoError,
    NotFoundError,
    PermissionError,
    OpenError,
    WriteError,
    ReadError,
    ResourceError,
    NotOpenError,
    UnknownError,
  };

private:
  QSerialPort *serial;
  QByteArray buffer;
  QTimer *rxtimer;
  uint16_t maxlen = 1024;
  uint16_t timeout = 10;

public:
  explicit Com(QObject *parent = nullptr) : QObject(parent) {};
  Com(const Com &) = delete;
  Com &operator=(const Com &) = delete;
  ~Com() {
    if (serial)
      delete serial;
    if (rxtimer)
      delete rxtimer;
  }
  void init() {
    serial = new QSerialPort();
    rxtimer = new QTimer();
    rxtimer->setSingleShot(true);
    connect(rxtimer, &QTimer::timeout, [this]() {
      emit serialData(buffer);
      buffer.clear();
    });

    connect(serial, &QSerialPort::readyRead, [this]() {
      if (buffer.size() >= maxlen) {
        emit serialData(buffer);
        buffer.clear();
        return;
      }
      buffer.append(serial->readAll());
      rxtimer->start(timeout);
    });

    connect(serial, &QSerialPort::errorOccurred,
            [this](QSerialPort::SerialPortError error) {
              ErrorCode code;
              switch (error) {
              case QSerialPort::NoError:
                code = NoError;
                break;
              case QSerialPort::DeviceNotFoundError:
                code = NotFoundError;
                break;
              case QSerialPort::PermissionError:
                code = PermissionError;
                break;
              case QSerialPort::OpenError:
                code = OpenError;
                break;
              case QSerialPort::WriteError:
                code = WriteError;
                break;
              case QSerialPort::ReadError:
                code = ReadError;
                break;
              case QSerialPort::ResourceError:
                code = ResourceError;
                break;
              case QSerialPort::NotOpenError:
                code = NotOpenError;
                break;
              default:
                code = UnknownError;
              }
              emit serialError(code, serial->errorString());
            });

    connect(this, &Com::send, this, &Com::sendByteArray);
    connect(this, &Com::sendStr, this, &Com::sendString);
    connect(this, &Com::sendFile, this, &Com::sendFile_);
    connect(this, &Com::open, this, &Com::openPortL);
    connect(this, &Com::close, this, &Com::closePort);
  }

signals:
  void scanResult(const QStringList &ports);
  void serialError(ErrorCode code, QString error = "");
  void serialData(QByteArray data);
  void send(QByteArray data);
  void sendStr(QString data);
  void sendFile(QString filename);
  void open(QString, uint32_t, uint8_t, uint8_t, uint8_t, uint8_t);
  void close();

public slots:
  // 设置串口最大接收长度
  uint16_t setMaxLen(uint16_t len) {
    maxlen = len;
    return maxlen;
  }
  // 设置串口接收超时
  uint16_t setTimeout(uint16_t ms) {
    timeout = ms;
    return timeout;
  }
  // 获取串口状态
  bool isOpen() { return serial->isOpen(); }
  // 关闭串口
  bool closePort() {
    serial->close();
    return isOpen();
  }
  /**
   * 扫描串口
   */
  void scanPorts() {
    QStringList ports;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
      ports.append(info.portName());
    }
    emit scanResult(ports);
  }
  /**
   * 打开串口
   * @param portName 串口名称
   * @param baudRate 波特率
   * @param dataBits 数据位
   * @param parity 校验位
   * @param stopBits 停止位
   * @param flowControl 流控制
   */
  void openPort(QString portName, uint32_t baudRate,
                QSerialPort::DataBits dataBits, QSerialPort::StopBits stopBits,
                QSerialPort::Parity parity,
                QSerialPort::FlowControl flowControl) {
    if (portName.isEmpty() || baudRate == 0) {
      emit serialError(ParamError, "串口名称和波特率不能为空");
      return;
    }
    if (serial->isOpen())
      serial->close();
    serial->setPortName(portName);
    serial->setBaudRate(baudRate);
    serial->setDataBits(dataBits);
    serial->setParity(parity);
    serial->setStopBits(stopBits);
    serial->setFlowControl(flowControl);
    serial->open(QIODevice::ReadWrite);
  }
  /**
   * Qt Combobox参数版本打开串口
   */
  void openPortL(QString portName, uint32_t baudRate, uint8_t dataBits,
                 uint8_t stopBits, uint8_t parity, uint8_t flowControl) {
    QSerialPort::DataBits d;
    QSerialPort::StopBits s;
    QSerialPort::Parity p;
    QSerialPort::FlowControl f;
    switch (dataBits) {
    case 0:
      d = QSerialPort::Data5;
      break;
    case 1:
      d = QSerialPort::Data6;
      break;
    case 2:
      d = QSerialPort::Data7;
      break;
    case 3:
      d = QSerialPort::Data8;
      break;
    default:
      d = QSerialPort::Data8;
    }
    switch (stopBits) {
    case 0:
      s = QSerialPort::OneStop;
      break;
    case 1:
      s = QSerialPort::OneAndHalfStop;
      break;
    case 2:
      s = QSerialPort::TwoStop;
      break;
    default:
      s = QSerialPort::OneStop;
    }
    switch (parity) {
    case 0:
      p = QSerialPort::NoParity;
      break;
    case 1:
      p = QSerialPort::EvenParity;
      break;
    case 2:
      p = QSerialPort::OddParity;
      break;
    case 3:
      p = QSerialPort::SpaceParity;
      break;
    case 4:
      p = QSerialPort::MarkParity;
      break;
    default:
      p = QSerialPort::NoParity;
    }
    switch (flowControl) {
    case 0:
      f = QSerialPort::NoFlowControl;
      break;
    case 1:
      f = QSerialPort::HardwareControl;
      break;
    case 2:
      f = QSerialPort::SoftwareControl;
      break;
    default:
      f = QSerialPort::NoFlowControl;
    }
    openPort(portName, baudRate, d, s, p, f);
  }

  // 发送QString数据
  int64_t sendString(QString data) { return serial->write(data.toUtf8()); }
  // 发送QByteArray数据
  int64_t sendByteArray(QByteArray data) { return serial->write(data); }
  // 发送文件
  int64_t sendFile_(QString filename) {
    // todo
    return 0;
  }
};