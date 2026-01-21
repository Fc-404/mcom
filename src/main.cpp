#include "global.hpp"
#include "mainwidget.hpp"
#include <QApplication>
#include <QFile>

int main(int argc, char* argv[])
{
    QGuiApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    QApplication a(argc, argv);
    MainWidget w;

    G::mainwidget = &w;
    G::loadQss(&a, "global");

    w.show();
    return a.exec();
}
