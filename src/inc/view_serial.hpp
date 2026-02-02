#pragma once

#include "WTextFlowShow.hpp"
#include <QWidget>
#include <cstdint>
#include <qcontainerfwd.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class ViewSerial;
}
QT_END_NAMESPACE

class ViewSerial : public QWidget {
  Q_OBJECT

public:
  explicit ViewSerial(QWidget *parent = nullptr);
  ~ViewSerial();
  bool eventFilter(QObject *obj, QEvent *event) override;
  void openCom();
  void closeCom();

public slots:
  void handleComError(Com::ErrorCode code, QString error);
  void handleComData(QByteArray data);
  void handleTxCheck();
  void handleTxSend();

private:
  Ui::ViewSerial *ui;
  WTextFlowShow *textflow;
  bool isComOpen = false;
  QString currentCom;
  QByteArray txbyte; // tx发送缓冲区
  QTimer *txtimer;   // tx发送定时器
  uint64_t rxcount = 0;
  uint64_t txcount = 0;
};