#pragma once

#include "global.hpp"
#include <QPlainTextEdit>

class WTextFlowShow : public QPlainTextEdit {
    Q_OBJECT

private:
    long maxcount = 10000;
    QList<QString> texts;
    QString background;
    QString color;
    QString fontsize;

public:
    WTextFlowShow(QWidget* parent = nullptr)
        : QPlainTextEdit(parent)
    {
        background = G::config().value("TextFlowTheme/background").toString();
        color = G::config().value("TextFlowTheme/color").toString();
        fontsize = G::config().value("TextFlowTheme/font-size").toString();

        if (background.isEmpty())
            background = "transparent";
        if (fontsize.isEmpty())
            fontsize = "14px";

        setStyleSheet(QString(R"(
            QPlainTextEdit {
                %1;
                background: %2;
                font-size: %3;
            }
        )")
                .arg(color.isEmpty() ? "" : QString("color: %1").arg(color))
                .arg(background)
                .arg(fontsize));

        setPlainText("test");
    };

    long getMaxCount() { return maxcount; }
    void setMaxCount(long count) { maxcount = count; }
};