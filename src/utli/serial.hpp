#pragma once

#include <QSerialPort>
#include <QSerialPortInfo>

class Com : public QSerialPort {
    Q_OBJECT

    Com() {
        
    };
};