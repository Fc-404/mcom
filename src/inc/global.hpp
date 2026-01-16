#pragma once

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QString>

namespace G {
/**
 * variable
 */
bool ondark = false;

/**
 * function
 */
QSettings& config()
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
}