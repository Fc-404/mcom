#pragma once

#include <QAbstractButton>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QPointer>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QWidget>

namespace G {
/**
 * ICON
 */
struct TICON {
    QIcon light;
    QIcon dark;
};
inline static QMap<QString, TICON> ICONS = {
    { "close", { QIcon(":/img/icon/close.svg"), QIcon(":/img/icon/dark/close.svg") } },
    { "close_hover", { QIcon(":/img/icon/close_hover.svg"), QIcon(":/img/icon/dark/close_hover.svg") } },
    { "maximize", { QIcon(":/img/icon/maximize.svg"), QIcon(":/img/icon/dark/maximize.svg") } },
    { "remaximize", { QIcon(":/img/icon/remaximize.svg"), QIcon(":/img/icon/dark/remaximize.svg") } },
    { "minimize", { QIcon(":/img/icon/minimize.svg"), QIcon(":/img/icon/dark/minimize.svg") } },
    { "fixed", { QIcon(":/img/icon/fixed.svg"), QIcon(":/img/icon/dark/fixed.svg") } },
    { "enfixed", { QIcon(":/img/icon/enfixed.svg"), QIcon(":/img/icon/dark/enfixed.svg") } },
    { "dark", { QIcon(":/img/icon/dark.svg"), QIcon(":/img/icon/dark/endark.svg") } },
};

/**
 * variable
 */
inline bool ondark = false;

/**
 * function
 */
inline QSettings& config()
{
    static QSettings* ini = []() -> QSettings* {
        const QString localPath = QDir::current().filePath("mcom.ini");

        if (QFileInfo::exists(localPath)) {
            return new QSettings(localPath, QSettings::IniFormat);
        }

        const QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.xazh";
        QDir dir;
        dir.mkpath(homeDir);
        return new QSettings(homeDir + "/mcom.ini", QSettings::IniFormat);
    }();

    return *ini;
}

inline void loadQss(std::variant<QWidget*, QApplication*> w, QString f, bool allupdate = false)
{
    static QHash<std::variant<QWidget*, QApplication*>, QString> widgets;

    static auto loader = [](std::variant<QWidget*, QApplication*> w, QString f) {
        QFile qss(QString(":/qss/qss/%1.qss").arg(f));
        QSettings theme(":/qss/qss/theme.ini", QSettings::IniFormat);
        if (!qss.open(QFile::ReadOnly)) {
            return;
        }

        QString qssStr = qss.readAll();
        theme.beginGroup(G::ondark ? "dark" : "light");
        QStringList keys = theme.allKeys();

        for (const QString& key : keys) {
            qssStr.replace(
                QString(": %1;").arg(key),
                QString(": %1;").arg(theme.value(key).toString()));
        }
        theme.endGroup();

        std::visit(
            [&](auto* obj) {
                if (obj) {
                    obj->setStyleSheet(qssStr);
                    if constexpr (std::is_same_v<decltype(obj), QWidget*>) {
                        obj->repaint();
                    }
                }
            },
            w);

        qss.close();
    };

    if (allupdate) {
        QMutableHashIterator<std::variant<QWidget*, QApplication*>, QString> i(widgets);
        while (i.hasNext()) {
            i.next();

            // 如果对象为空，则删除
            if (std::holds_alternative<QWidget*>(i.key())) {
                if (qobject_cast<QWidget*>(std::get<QWidget*>(i.key())) == nullptr) {
                    i.remove();
                    continue;
                }
            } else if (std::holds_alternative<QApplication*>(i.key())) {
                if (qobject_cast<QApplication*>(std::get<QApplication*>(i.key())) == nullptr) {
                    i.remove();
                    continue;
                }
            }

            loader(i.key(), i.value());
        }
    } else if (!f.isEmpty()) {
        widgets[w] = f;
        loader(w, f);
    }
}

inline void loadSvg(QAbstractButton* w, QString n, bool allupdate = false)
{
    static QHash<QAbstractButton*, QString> widgets;

    if (allupdate) {
        QMutableHashIterator<QAbstractButton*, QString> i(widgets);
        while (i.hasNext()) {
            i.next();

            if (qobject_cast<QAbstractButton*>(i.key()) == nullptr) {
                i.remove();
                continue;
            }

            i.key()->setIcon(G::ondark ? ICONS[i.value()].dark : ICONS[i.value()].light);
        }
    } else if (qobject_cast<QAbstractButton*>(w) && ICONS.contains(n)) {
        widgets[w] = n;
        w->setIcon(G::ondark ? ICONS[n].dark : ICONS[n].light);
    }
}
}