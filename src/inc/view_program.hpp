#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class ViewProgram;
}
QT_END_NAMESPACE

class ViewProgram : public QWidget {
    Q_OBJECT

public:
    explicit ViewProgram(QWidget* parent = nullptr);
    ~ViewProgram();

private:
    Ui::ViewProgram* ui;
};