#pragma once

#include "global.hpp"
#include <QPlainTextEdit>
#include <chrono>
#include <deque>

class WTextFlowShow : public QPlainTextEdit {
    Q_OBJECT

private:
    // 最大行数
    long maxcount = 10000;
    // 行容器
    std::deque<std::pair<std::array<char, 13>, QByteArray>> texts;
    // 是否开启utf-8
    bool onutf_8 = true;
    // 是否显示时间戳
    bool ontimestamp = true;
    // 是否以十六进制显示
    bool onhex = false;
    // 切换功能后是否重绘
    bool redraw = false;

    // 背景颜色
    QString background;
    // 文本颜色
    QString color;
    // 字体大小
    QString fontsize;
    // 边框
    QString border;

public:
    // 行类型
    enum TextType {
        Tip = 0, // 提示
        Rx, // 接收
        Tx, // 发送
    };

public:
    WTextFlowShow(QWidget* parent = nullptr)
        : QPlainTextEdit(parent)
    {
        setReadOnly(true);
        setUndoRedoEnabled(false);
        setMaximumBlockCount(maxcount);
        setContextMenuPolicy(Qt::NoContextMenu);
    };
    /**
     * 使用配置文件初始化
     * @param parent
     * @param useconfig 是否使用配置文件
     */
    WTextFlowShow(QWidget* parent, bool useconfig)
        : QPlainTextEdit(parent)
    {
        background = G::config().value("TextFlow/background").toString();
        color = G::config().value("TextFlow/color").toString();
        fontsize = G::config().value("TextFlow/font-size").toString();

        maxcount = G::config().value("TextFlow/maxcount").toInt();
        ontimestamp = G::config().value("TextFlow/ontimestamp").toBool();
        onutf_8 = G::config().value("TextFlow/onutf-8").toBool();
        onhex = G::config().value("TextFlow/onhex").toBool();
        redraw = G::config().value("TextFlow/redraw").toBool();
        border = G::config().value("TextFlow/border").toString();

        background = background.isEmpty() ? "transparent" : background;

        setReadOnly(true);
        setUndoRedoEnabled(false);
        setMaximumBlockCount(maxcount);
        setContextMenuPolicy(Qt::NoContextMenu);
        updateStyle();
    }

    /**
     * 获取最大行数
     */
    long getMaxCount() { return maxcount; }
    /**
     * 设置最大行数
     * @param count 最大行数
     */
    void setMaxCount(long count)
    {
        maxcount = count;
        setMaximumBlockCount(count);
    }

    /**
     * 设置背景颜色
     * @param bg 背景颜色
     */
    void setBackground(QString bg = "transparent")
    {
        background = bg;
        updateStyle();
    }
    /**
     * 设置文本颜色
     * @param c 文本颜色
     */
    void setColor(QString c = "")
    {
        color = c;
        updateStyle();
    }
    /**
     * 设置字体大小
     * @param s 字体大小
     */
    void setFontSize(QString s = "12px")
    {
        fontsize = s;
        updateStyle();
    }

    /**
     * 获取是否显示时间戳
     */
    bool onTimestamp() { return ontimestamp; }
    /**
     * 设置是否显示时间戳
     * @param on 是否显示时间戳
     */
    void onTimestamp(bool on)
    {
        ontimestamp = on;
        if (redraw)
            reshow();
    }
    /**
     * 获取是否以utf-8格式显示
     */
    bool onUtf_8() { return onutf_8; }
    /**
     * 设置是否以utf-8格式显示
     * @param on 是否以utf-8格式显示
     */
    void onUtf_8(bool on)
    {
        onutf_8 = on;
        if (redraw)
            reshow();
    }
    /**
     * 获取是否以十六进制显示
     */
    bool onHex() { return onhex; }
    /**
     * 设置是否以十六进制显示
     * @param on 是否以十六进制显示
     */
    void onHex(bool on)
    {
        onhex = on;
        if (redraw)
            reshow();
    }

    /**
     * 添加行
     * @param data 数据
     * @param time 时间
     * @param type 类型
     */
    void append(QByteArray data, QTime time, TextType type = Rx)
    {
        int offset = texts.size() - maxcount + 1;
        if (offset > 0)
            texts.erase(texts.begin(), texts.begin() + offset);

        std::array<char, 13> buf;
        memcpy(buf.data(), time.toString("hh:mm:ss.zzz").toUtf8(), 12);
        buf[buf.size() - 1] = type;

        texts.push_back(std::make_pair(buf, data));
        appendOne(texts.back());
    }

    /**
     * 清空所有行
     */
    void clearall()
    {
        texts.clear();
        clear();
    }

private:
    /**
     * 更新样式
     */
    void updateStyle()
    {
        setStyleSheet(QString(R"(
            QPlainTextEdit {
                %1 %2
                background: %3;
                font-size: %4;
            }
        )")
                .arg(color.isEmpty() ? "" : "color: " + color + ";")
                .arg(border.isEmpty() ? "" : "border: " + border + ";")
                .arg(background)
                .arg(fontsize));
        repaint();
    }
    /**
     * 添加一行
     * @param text 行数据
     */
    void appendOne(std::pair<std::array<char, 13>, QByteArray> text)
    {
        QString str;
        if (ontimestamp) {
            str += "[";
            switch (text.first[12]) {
            case Tip:
                str += "Tip: ";
                break;
            case Rx:
                str += "Rx: ";
                break;
            case Tx:
                str += "Tx: ";
                break;
            }
            str += std::string_view(text.first.data(), 12) + "] ";
        }
        if (onhex) {
            str += text.second.toHex(' ').toUpper();
        } else {
            if (onutf_8)
                str += QString::fromUtf8(text.second.data(), text.second.size());
            else
                str += QString::fromLatin1(text.second.data(), text.second.size());
        }
        appendPlainText(str);
    }
    /**
     * 重新显示所有行
     */
    void reshow()
    {
        clear();
        for (auto& text : texts) {
            appendOne(text);
        }
    }
};