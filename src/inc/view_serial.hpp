#pragma once

#include "WTextFlowShow.hpp"
#include <QWidget>

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

private:
  Ui::ViewSerial *ui;
  WTextFlowShow *textflow;
  bool isComOpen = false;
  QString currentCom;
};