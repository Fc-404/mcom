#include "mainwidget.hpp"
#include <QApplication>
#include <QFile>

int main(int argc, char* argv[])
{
    QGuiApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    QApplication a(argc, argv);
    MainWidget w;

    QFile qssFile(":/qss/qss/global.qss");
    if (qssFile.open(QFile::ReadOnly)) {
        a.setStyleSheet(qssFile.readAll());
        qssFile.close();
    }

    w.show();
    return a.exec();
}
