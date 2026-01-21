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
    explicit ViewSerial(QWidget* parent = nullptr);
    ~ViewSerial();

private:
    Ui::ViewSerial* ui;
    WTextFlowShow* textflow;
};