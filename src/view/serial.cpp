#include "WTextFlowShow.hpp"
#include "global.hpp"
#include "ui_view_serial.h"
#include "view_serial.hpp"
#include <QTimer>

ViewSerial::ViewSerial(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ViewSerial)
{
    ui->setupUi(this);
    G::loadQss(this, "view_serial");

    textflow = new WTextFlowShow(this);
    textflow->setObjectName("textflow");
    ui->verticalLayout->addWidget(textflow);

}

ViewSerial::~ViewSerial()
{
    delete ui;
}