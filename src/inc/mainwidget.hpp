#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWKWidgets/widgetwindowagent.h>
#include <QWidget>
#include <cstdint>

#ifdef Q_OS_WIN
#include <dwmapi.h>
#include <windows.h>
#include <windowsx.h>
#endif

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWidget;
}
QT_END_NAMESPACE

class MainWidget : public QWidget {
  Q_OBJECT

public:
  MainWidget(QWidget *parent = nullptr);
  ~MainWidget();
  void setBgDark(bool dark = false);
  void addrx(uint32_t len);
  void addtx(uint32_t len);
  void resetrx();
  void resettx();
  void resetrtx();

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;
  void changeEvent(QEvent *event) override;
  void showEvent(QShowEvent *event) override;

public slots:
  void exit();
  void maximize();
  void minimize();
  void fixed();
  void dark();

public:
  QWK::WidgetWindowAgent *agent;

private:
  Ui::MainWidget *ui;
  bool onTop = false;  // 窗口置顶
  bool onDark = false; // 窗口暗黑模式

  uint64_t rxcount = 0;
  uint64_t txcount = 0;
};
#endif // MAINWIDGET_H
