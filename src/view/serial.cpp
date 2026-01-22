#include "WTextFlowShow.hpp"
#include "global.hpp"
#include "ui_view_serial.h"
#include "view_serial.hpp"
#include <QInputDialog>
#include <QTimer>
#include <thread>

ViewSerial::ViewSerial(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ViewSerial)
{
    ui->setupUi(this);
    G::loadQss(this, "view_serial");

    textflow = new WTextFlowShow(this, true);
    textflow->setObjectName("textflow");
    ui->show->addWidget(textflow);

    // QTimer* timer = new QTimer(nullptr);
    // static long i = 0;
    // connect(timer, &QTimer::timeout, [&]() {
    //     if (i == 10)
    //         textflow->onTimestamp(false);
    //     if (i == 20)
    //         textflow->onHex(true);
    //     textflow->append(QString("Hello world 你好世界 Hello world 你好世界 Hello world 你好世界 Hello world 你好世界 %1").arg(i++).toUtf8(), QTime::currentTime());
    // });
    // timer->start(100);
}

ViewSerial::~ViewSerial()
{
    delete ui;
}