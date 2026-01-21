#include "ui_view_program.h"
#include "view_program.hpp"

ViewProgram::ViewProgram(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ViewProgram)
{
    ui->setupUi(this);
}

ViewProgram::~ViewProgram()
{
    delete ui;
}