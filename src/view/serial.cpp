#include "WTextFlowShow.hpp"
#include "global.hpp"
#include "ui_view_serial.h"
#include "view_serial.hpp"
#include <cstdint>
#include <qcombobox.h>
#include <qcontainerfwd.h>
#include <qdebug.h>
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
  });

  // 处理串口线程错误信号
  connect(G::com, &Com::serialError, this, &ViewSerial::handleComError);
  // 处理串口数据信号
  connect(G::com, &Com::serialData, this, &ViewSerial::handleComData);

  // 处理打开按钮信号
  connect(ui->openPort, &QPushButton::clicked, [this]() {
    if (isComOpen) {
      closeCom();
    } else {
      openCom();
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
  int ct_timeout = G::config().value("ComCtl/timeout").toInt();
  int ct_autosend = G::config().value("ComCtl/autosend").toInt();
  if (ct_timeout > 0) {
    ui->v_timeout->setValue(ct_timeout);
  }
  if (ct_autosend > 0) {
    ui->v_timer->setValue(ct_autosend);
  }

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
    G::config().setValue("TextFlow/onhexsend", s);
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
    QMessageBox::critical(this, "错误", "串口打开失败");
    qDebug() << "串口打开失败" << error;
    break;
  case Com::ErrorCode::ParamError:
    closeCom();
    QMessageBox::critical(this, "错误", "参数错误");
    qDebug() << "参数错误" << error;
    break;
  case Com::ErrorCode::PermissionError:
    closeCom();
    QMessageBox::critical(this, "错误", "串口拒绝访问");
    qDebug() << "串口拒绝访问" << error;
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
  //   emit G::com->send(txbyte);
  int64_t result = ComCallB(&Com::sendByteArray, txbyte);
  if (result > 0) {
    textflow->append(txbyte, QTime::currentTime(), WTextFlowShow::Tx);
    G::mainwidget->addtx(txbyte.size());
  }
}