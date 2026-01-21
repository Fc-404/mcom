#include "ui_view_modbus.h"
#include "view_modbus.hpp"

ViewModbus::ViewModbus(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ViewModbus)
{
    ui->setupUi(this);
}

ViewModbus::~ViewModbus()
{
    delete ui;
}