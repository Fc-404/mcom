#include "WTextFlowShow.hpp"
#include "global.hpp"
#include "ui_view_serial.h"
#include "view_serial.hpp"
#include <qcombobox.h>
#include <qdebug.h>
#include <qinputdialog.h>
#include <qlogging.h>
#include <qmessagebox.h>
#include <qoverload.h>
#include <qspinbox.h>
#include <qtimer.h>
#include <qtmetamacros.h>

ViewSerial::ViewSerial(QWidget *parent)
    : QWidget(parent), ui(new Ui::ViewSerial) {
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

  // 处理显示板功能
  connect(ui->on_timestamp, &QCheckBox::checkStateChanged,
          [this](int state) { textflow->onTimestamp(state == Qt::Checked); });
  connect(ui->on_hexshow, &QCheckBox::checkStateChanged,
          [this](int state) { textflow->onHex(state == Qt::Checked); });
  connect(ui->on_utf8show, &QCheckBox::checkStateChanged,
          [this](int state) { textflow->onUtf_8(state == Qt::Checked); });
  connect(ui->v_timeout, &QSpinBox::valueChanged,
          [this](int value) { ComCall(&Com::setTimeout, value); });
  connect(ui->on_hexno, &QCheckBox::checkStateChanged,
          [this](int state) { textflow->onHexNo(state == Qt::Checked); });

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
  case Com::ErrorCode::OpenFailed:
    closeCom();
    QMessageBox::critical(this, "错误", "串口打开失败");
    qDebug() << "串口打开失败" << error;
    break;
  case Com::ErrorCode::ParamError:
    QMessageBox::critical(this, "错误", "参数错误");
    qDebug() << "参数错误" << error;
    break;
  }
}

void ViewSerial::handleComData(QByteArray data) { textflow->append(data); }