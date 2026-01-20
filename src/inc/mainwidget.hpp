#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWKWidgets/widgetwindowagent.h>
#include <QWidget>

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
    MainWidget(QWidget* parent = nullptr);
    ~MainWidget();
    bool eventFilter(QObject* obj, QEvent* event) override;
    void changeEvent(QEvent* event) override;
    void showEvent(QShowEvent* event) override;

    void setBgDark(bool dark = false);

public slots:
    void exit();
    void maximize();
    void minimize();
    void fixed();
    void dark();

private:
    Ui::MainWidget* ui;
    QWK::WidgetWindowAgent* agent;
    bool onTop = false; // 窗口置顶
    bool onDark = false; // 窗口暗黑模式
};
#endif // MAINWIDGET_H
