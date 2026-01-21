#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class ViewModbus;
}
QT_END_NAMESPACE

class ViewModbus : public QWidget {
    Q_OBJECT

public:
    explicit ViewModbus(QWidget* parent = nullptr);
    ~ViewModbus();

private:
    Ui::ViewModbus* ui;
};