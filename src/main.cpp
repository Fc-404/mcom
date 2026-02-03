#include "global.hpp"
#include "mainwidget.hpp"
#include <QApplication>
#include <QFile>
#include <QObject>
#include <QThread>

int main(int argc, char *argv[]) {
  QGuiApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
  QApplication a(argc, argv);

  // 初始化串口线程
  G::comthread = new QThread();
  G::com = new Com();
  G::com->moveToThread(G::comthread);
  QObject::connect(G::comthread, &QThread::started, G::com, &Com::init);
  G::comthread->start();

  // 存储主widget
  MainWidget w;
  G::mainwidget = &w;

  // 给窗口加载样式
  G::loadQss(&a, "global");

  w.setWindowTitle("MCOM");

  w.show();
  return a.exec();
}
