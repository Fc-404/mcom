#include "WTextFlowShow.hpp"
#include "byteverify.hpp"
#include "global.hpp"
#include "ui_view_serial.h"
#include "view_serial.hpp"
#include <cstdint>
#include <qcombobox.h>
#include <qcontainerfwd.h>
#include <qdebug.h>
#include <qendian.h>
#include <qinputdialog.h>
#include <qlogging.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qoverload.h>
#include <qplaintextedit.h>
#include <qspinbox.h>
#include <qtextcursor.h>
#include <qtimer.h>
#include <qtmetamacros.h>
#include <type_traits>

ViewSerial::ViewSerial(QWidget *parent)
    : QWidget(parent), ui(new Ui::ViewSerial), txtimer(new QTimer(this)) {
  ui->setupUi(this);
  G::loadQss(this, "view_serial");

  textflow = new WTextFlowShow(this, true);
  textflow->setObjectName("textflow");
  ui->show->addWidget(textflow);

  // 处理串口列表信号
  ui->box_ports->installEventFilter(this);
  connect(G::com, &Com::scanResult, [this](const QStringList &ports) {
    ui->box_ports->clear();
    ui->box_ports->addItems(ports);
    if (ports.contains(currentCom)) {
      ui->box_ports->setCurrentText(currentCom);
    }
    G::Ok("串口列表更新完成！");
  });

  // 处理串口线程错误信号
  connect(G::com, &Com::serialError, this, &ViewSerial::handleComError);
  // 处理串口数据信号
  connect(G::com, &Com::serialData, this, &ViewSerial::handleComData);

  // 处理打开按钮信号
  connect(ui->openPort, &QPushButton::clicked, [this]() {
    if (isComOpen) {
      closeCom();
      G::Ok("关闭串口" + ui->box_ports->currentText() + "！");
    } else {
      openCom();
      G::Ok("打开串口" + ui->box_ports->currentText() + "！");
    }
  });

  // 初始化显示板功能
  txbyte = G::config().value("ComCtl/txbuf").toByteArray();
  if (G::config().value("TextFlow/ontimestamp").toBool()) {
    ui->on_timestamp->setChecked(true);
  } else {
    ui->on_timestamp->setChecked(false);
  }
  if (G::config().value("TextFlow/onhex").toBool()) {
    ui->on_hexshow->setChecked(true);
  } else {
    ui->on_hexshow->setChecked(false);
  }
  if (G::config().value("TextFlow/onutf-8").toBool()) {
    ui->on_utf8show->setChecked(true);
  } else {
    ui->on_utf8show->setChecked(false);
  }
  if (G::config().value("ComCtl/onhexsend").toBool()) {
    ui->on_hexsend->setChecked(true);
    ui->in_tx->setPlainText(txbyte.toHex(' ').toUpper());
  } else {
    ui->on_hexsend->setChecked(false);
    ui->in_tx->setPlainText(QString::fromUtf8(txbyte));
  }
  if (G::config().value("ComCtl/onverify").toBool()) {
    onVerify = true;
    ui->on_crc->setChecked(true);
  } else {
    onVerify = false;
    ui->on_crc->setChecked(false);
  }
  if (G::config().value("ComCtl/onvbig").toBool()) {
    onVBig = true;
    ui->on_crcbig->setChecked(true);
  } else {
    onVBig = false;
    ui->on_crcbig->setChecked(false);
  }
  int ct_timeout = G::config().value("ComCtl/timeout").toInt();
  int ct_autosend = G::config().value("ComCtl/autosend").toInt();
  int ct_crci = G::config().value("ComCtl/vcrci").toInt();
  int ct_crcs = G::config().value("ComCtl/vcrcs").toInt();
  int ct_crce = G::config().value("ComCtl/vcrce").toInt();
  QString ct_crct = G::config().value("ComCtl/vcrct").toString();
  if (ct_timeout > 0) {
    ui->v_timeout->setValue(ct_timeout);
  }
  if (ct_autosend > 0) {
    ui->v_timer->setValue(ct_autosend);
  }
  ui->v_crci->setValue(ct_crci);
  ui->v_crcs->setValue(ct_crcs);
  ui->v_crce->setValue(ct_crce);
  ui->box_crc->setCurrentText(ct_crct);

  // 处理显示板功能
  connect(ui->on_timestamp, &QCheckBox::checkStateChanged, [this](int state) {
    bool s = (state == Qt::Checked);
    textflow->onTimestamp(s);
    G::config().setValue("TextFlow/ontimestamp", s);
  });
  connect(ui->on_hexshow, &QCheckBox::checkStateChanged, [this](int state) {
    bool s = (state == Qt::Checked);
    textflow->onHex(s);
    G::config().setValue("TextFlow/onhex", s);
  });
  connect(ui->on_utf8show, &QCheckBox::checkStateChanged, [this](int state) {
    bool s = (state == Qt::Checked);
    textflow->onUtf_8(s);
    G::config().setValue("TextFlow/onutf-8", s);
  });
  connect(ui->v_timeout, &QSpinBox::valueChanged,
          [this](int value) { ComCall(&Com::setTimeout, value); });
  connect(ui->on_hexno, &QCheckBox::checkStateChanged,
          [this](int state) { textflow->onHexNo(state == Qt::Checked); });
  connect(ui->clearWTF, &QPushButton::clicked, [this]() {
    textflow->clearall();
    G::mainwidget->resetrtx();
  });

  // 处理发送功能
  connect(ui->txSend, &QPushButton::clicked, this, &ViewSerial::handleTxSend);
  connect(ui->in_tx, &QPlainTextEdit::textChanged, this,
          &ViewSerial::handleTxCheck);
  ui->in_tx->installEventFilter(this);
  connect(ui->on_hexsend, &QCheckBox::checkStateChanged, [this](int state) {
    bool s = (state == Qt::Checked);
    if (s) {
      ui->in_tx->setPlainText(txbyte.toHex(' ').toUpper());
    } else {
      ui->in_tx->setPlainText(QString::fromUtf8(txbyte));
    }
    G::config().setValue("ComCtl/onhexsend", s);
  });
  connect(ui->v_timeout, &QSpinBox::valueChanged,
          [this](int value) { G::config().setValue("ComCtl/timeout", value); });
  connect(ui->v_timer, &QSpinBox::valueChanged, [this](int value) {
    G::config().setValue("ComCtl/autosend", value);
  });

  // 处理自动发送
  txtimer->setTimerType(Qt::PreciseTimer);
  connect(txtimer, &QTimer::timeout, this, &ViewSerial::handleTxSend);
  connect(ui->on_timer, &QCheckBox::checkStateChanged, [this](int state) {
    bool s = (state == Qt::Checked);
    if (s) {
      txtimer->start(ui->v_timer->text().toInt());
    } else {
      txtimer->stop();
    }
  });

  // 处理检验码
  connect(ui->on_crc, &QCheckBox::checkStateChanged, [this](int state) {
    bool s = (state == Qt::Checked);
    G::config().setValue("ComCtl/onverify", s);
    onVerify = s;
  });
  connect(ui->on_crcbig, &QCheckBox::checkStateChanged, [this](int state) {
    bool s = (state == Qt::Checked);
    G::config().setValue("ComCtl/onvbig", s);
    onVBig = s;
  });
  connect(
      ui->box_crc, &QComboBox::currentTextChanged,
      [this](const QString &s) { G::config().setValue("ComCtl/vcrct", s); });
  connect(ui->v_crci, &QSpinBox::valueChanged,
          [this](int value) { G::config().setValue("ComCtl/vcrci", value); });
  connect(ui->v_crcs, &QSpinBox::valueChanged,
          [this](int value) { G::config().setValue("ComCtl/vcrcs", value); });
  connect(ui->v_crce, &QSpinBox::valueChanged,
          [this](int value) { G::config().setValue("ComCtl/vcrce", value); });

  // 刷新串口列表
  emit G::com->scanPorts();
}

ViewSerial::~ViewSerial() { delete ui; }

bool ViewSerial::eventFilter(QObject *obj, QEvent *event) {
  if (obj == ui->box_ports) {
    if (isComOpen == false && event->type() == QEvent::MouseButtonPress) {
      currentCom = ui->box_ports->currentText();
      emit G::com->scanPorts();
    }
  }
  if (obj == ui->in_tx) {
    switch (event->type()) {
    case QEvent::FocusOut:
      G::config().setValue("ComCtl/txbuf", txbyte);
      break;
    default:
      break;
    }
  }

  return QObject::eventFilter(obj, event);
}

void ViewSerial::openCom() {
  emit G::com->open(
      ui->box_ports->currentText(), ui->box_baudrate->currentText().toInt(),
      ui->box_databits->currentIndex(), ui->box_stopbits->currentIndex(),
      ui->box_parity->currentIndex(), ui->box_flowcontrol->currentIndex());

  ui->openPort->setText("关闭串口");
  G::setProperty(ui->openPort, "status", "open");

  ui->box_baudrate->setEnabled(false);
  ui->box_databits->setEnabled(false);
  ui->box_stopbits->setEnabled(false);
  ui->box_parity->setEnabled(false);
  ui->box_flowcontrol->setEnabled(false);
  ui->box_ports->setEnabled(false);

  isComOpen = true;
}

void ViewSerial::closeCom() {
  emit G::com->close();
  ui->openPort->setText("打开串口");
  G::setProperty(ui->openPort, "status", "close");

  ui->box_baudrate->setEnabled(true);
  ui->box_databits->setEnabled(true);
  ui->box_stopbits->setEnabled(true);
  ui->box_parity->setEnabled(true);
  ui->box_flowcontrol->setEnabled(true);
  ui->box_ports->setEnabled(true);

  isComOpen = false;
}

void ViewSerial::handleComError(Com::ErrorCode code, QString error) {
  switch (code) {
  case Com::ErrorCode::OpenError:
    closeCom();
    G::Err("串口打开失败！");
    break;
  case Com::ErrorCode::ParamError:
    closeCom();
    G::Err("串口参数错误！");
    break;
  case Com::ErrorCode::PermissionError:
    closeCom();
    G::Err(QString("串口%1拒绝访问！").arg(ui->box_ports->currentText()));
  default:
    break;
  }
}

void ViewSerial::handleComData(QByteArray data) {
  G::mainwidget->addrx(data.size());
  textflow->append(data);
}

void ViewSerial::handleTxCheck() {
  QString txbuf = ui->in_tx->toPlainText();
  QTextCursor tc = ui->in_tx->textCursor();
  int ci = tc.position();

  if (ui->on_hexsend->isChecked()) {
    for (uint32_t i = 0; i < txbuf.size(); i++) {
      if (i % 3 == 2) {
        if (txbuf[i] != ' ') {
          txbuf.insert(i, ' ');
          ci++;
        }
        continue;
      }
      if (txbuf[i] >= '0' && txbuf[i] <= '9' ||
          txbuf[i] >= 'A' && txbuf[i] <= 'F')
        continue;
      if (txbuf[i] >= 'a' && txbuf[i] <= 'f') {
        txbuf[i] = txbuf[i].toUpper();
      } else {
        txbuf.remove(i, 1);
        i--;
        ci--;
      }
    }
    txbyte = QByteArray::fromHex(txbuf.toUtf8());
  } else {
    txbyte = txbuf.toUtf8();
  }

  ui->in_tx->blockSignals(true);
  ui->in_tx->setPlainText(txbuf);
  tc.setPosition(ci);
  ui->in_tx->setTextCursor(tc);
  ui->in_tx->blockSignals(false);
}

void ViewSerial::handleTxSend() {
  if (!ComCallB(&Com::isOpen)) {
    G::Warn("串口未打开！请检查连接！");
    return;
  }
  QByteArray data = handleVerify();
  int64_t result = ComCallB(&Com::sendByteArray, data);
  if (result > 0) {
    textflow->append(data, QTime::currentTime(), WTextFlowShow::Tx);
    G::mainwidget->addtx(data.size());
  } else {
    G::Err("串口发送失败！请检查连接！");
  }
}

QByteArray ViewSerial::handleVerify() {
  static auto insert = [](QByteArray &data, int i, auto code,
                          bool big = false) {
    using codet = std::decay_t<decltype(code)>;
    int size = sizeof(codet);
    QByteArray bytes;
    bytes.resize(size);

    if (big) {
      qToBigEndian(code, bytes.data());
    } else {
      qToLittleEndian(code, bytes.data());
    }

    return data.insert(i, bytes);
  };

  if (!onVerify)
    return txbyte;
  QByteArray data = txbyte;
  int l = data.size();
  int s = ui->v_crcs->text().toInt();
  int e = ui->v_crce->text().toInt();
  int i = ui->v_crci->text().toInt();

  if (s >= l) {
    G::Err("校验起始位置错误！超出数据长度！");
    return data;
  }
  if (e < 0) {
    e = l + e;
  }
  if (e >= l) {
    G::Err("校验结束位置错误！超出数据长度！");
    return data;
  }
  if (s >= e) {
    G::Err("校验起始结束位置错误！起始超出结束位置！");
    return data;
  }
  if (i >= l) {
    G::Err("校验插入位置错误！超出数据长度！");
    return data;
  }
  if (i < 0) {
    i = l + i + 1;
  }

  QByteArray vcode;
  QByteArray vdata = data.sliced(s, e - s + 1);
  QString vtype = ui->box_crc->currentText();

  if (vtype == "CheckSum_8") {
    insert(data, i, ByteVerify::checksum8(vdata), onVBig);
  } else if (vtype == "CheckSum_16") {
    insert(data, i, ByteVerify::checksum16(vdata), onVBig);
  } else if (vtype == "LRC") {
    insert(data, i, ByteVerify::calcLRC(vdata), onVBig);
  } else if (vtype == "BCC") {
    insert(data, i, ByteVerify::calcBCC(vdata), onVBig);
  } else if (vtype == "CRC16_IBM") {
    insert(data, i, ByteVerify::crc16_ibm(vdata), onVBig);
  } else if (vtype == "CRC16_MAXIM") {
    insert(data, i, ByteVerify::crc16_maxim(vdata), onVBig);
  } else if (vtype == "CRC16_USB") {
    insert(data, i, ByteVerify::crc16_usb(vdata), onVBig);
  } else if (vtype == "CRC16_MODBUS") {
    insert(data, i, ByteVerify::crc16_modbus(vdata), onVBig);
  } else if (vtype == "CRC16_CCITT") {
    insert(data, i, ByteVerify::crc16_ccitt(vdata), onVBig);
  } else if (vtype == "CRC16_CCITT_FALSE") {
    insert(data, i, ByteVerify::crc16_ccitt_false(vdata), onVBig);
  } else if (vtype == "CRC16_X25") {
    insert(data, i, ByteVerify::crc16_x25(vdata), onVBig);
  } else if (vtype == "CRC16_XMODEM") {
    insert(data, i, ByteVerify::crc16_xmodem(vdata), onVBig);
  } else if (vtype == "CRC16_DNP") {
    insert(data, i, ByteVerify::crc16_dnp(vdata), onVBig);
  } else if (vtype == "CRC32") {
    insert(data, i, ByteVerify::crc32(vdata), onVBig);
  } else if (vtype == "CRC32_MPEG2") {
    insert(data, i, ByteVerify::crc32_mpeg2(vdata), onVBig);
  }

  return data;
}