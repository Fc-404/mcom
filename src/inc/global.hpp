#pragma once

#include "com.hpp"
#include "mainwidget.hpp"
#include <QAbstractButton>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QObject>
#include <QPointer>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QStyle>
#include <QWidget>

namespace G {
/**
 * ICON自动化配置
 * 分别设置浅色和深色图标
 */
struct TICON {
  QIcon light;
  QIcon dark;
};
inline static QMap<QString, TICON> ICONS = {
    // { "close", { QIcon(":/img/icon/close.svg"),
    // QIcon(":/img/icon/dark/close.svg") } },
};

// 深色模式
inline bool ondark = false;
// 禁止在构造函数内使用，QTimer::singleShot(0, this, 这里可以使用);
inline MainWidget *mainwidget = nullptr;
// 全局异步串口类，用于连接信号槽
inline Com *com = nullptr;
inline QThread *comthread = nullptr;

/**
 * 获取配置文件配置项
 * 如果程序目录没有文件，则在用户目录创建打开
 */
inline QSettings &config() {
  static QSettings *ini = []() -> QSettings * {
    const QString localPath = QDir::current().filePath("mcom.ini");

    if (QFileInfo::exists(localPath)) {
      return new QSettings(localPath, QSettings::IniFormat);
    }

    const QString homeDir =
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +
        "/.xazh";
    QDir dir;
    dir.mkpath(homeDir);
    return new QSettings(homeDir + "/mcom.ini", QSettings::IniFormat);
  }();

  return *ini;
}

/**
 * 获取程序主题颜色
 * @param name 颜色名称 color-name-[0-9]
 */
inline QString getColor(QString name, QString section = "default") {
  QSettings theme(":/qss/qss/theme.ini", QSettings::IniFormat);
  QString colorName(name);
  QString themeColor = config().value("Theme/themeColor").toString();
  colorName.replace("color-theme-", themeColor.isEmpty()
                                        ? "color-gray-"
                                        : ("color-" + themeColor + "-"));
  int num = colorName.right(1).toInt();
  ondark ? num = 9 - num : num;
  colorName = colorName.left(colorName.length() - 1) + QString::number(num);

  return theme.value(section + "/" + colorName).toString();
}

/**
 * 为QWidget加载样式表，也可以更新所有注册过的QWidget的样式表
 * @param w QWidget对象
 * @param f qss文件名，不需要后缀和前缀
 * @param allupdate 是否更新所有注册过的QWidget的样式表
 */
inline void loadQss(std::variant<QWidget *, QApplication *> w, QString f,
                    bool allupdate = false) {
  static QHash<std::variant<QWidget *, QApplication *>, QString> widgets;

  static auto loader = [](std::variant<QWidget *, QApplication *> w,
                          QString f) {
    QFile qss(QString(":/qss/qss/%1.qss").arg(f));
    QSettings theme(":/qss/qss/theme.ini", QSettings::IniFormat);
    if (!qss.open(QFile::ReadOnly)) {
      return;
    }

    QString qssStr = qss.readAll();

    // 替换为主题颜色，主题色遵循浅色不变，深色为浅色反序
    QString themeColor = config().value("Theme/themeColor").toString();
    for (int i = 0; i < 10; i++) {
      QString str = QString(" color-%1-%2")
                        .arg(themeColor.isEmpty() ? "gray" : themeColor)
                        .arg(G::ondark ? 9 - i : i);
      qssStr.replace(QString(" color-theme-%1").arg(i), str);
      qssStr.replace(QString(" color-%1").arg(i),
                     QString(" color-%1").arg(G::ondark ? 9 - i : i));
    }

    // 设置通用颜色
    theme.beginGroup("default");
    for (const QString &key : theme.allKeys()) {
      qssStr.replace(QString(" %1").arg(key),
                     QString(" %1").arg(theme.value(key).toString()));
    }
    theme.endGroup();

    // 设置专有颜色
    theme.beginGroup(G::ondark ? "dark" : "light");
    for (const QString &key : theme.allKeys()) {
      qssStr.replace(QString(" %1").arg(key),
                     QString(" %1").arg(theme.value(key).toString()));
    }
    theme.endGroup();

    std::visit(
        [&](auto *obj) {
          if (obj) {
            obj->setStyleSheet(qssStr);
            if constexpr (std::is_same_v<decltype(obj), QWidget *>) {
              obj->repaint();
            }
          }
        },
        w);

    qss.close();
  };

  if (allupdate) {
    QMutableHashIterator<std::variant<QWidget *, QApplication *>, QString> i(
        widgets);
    while (i.hasNext()) {
      i.next();

      // 如果对象为空，则删除
      if (std::holds_alternative<QWidget *>(i.key())) {
        if (qobject_cast<QWidget *>(std::get<QWidget *>(i.key())) == nullptr) {
          i.remove();
          continue;
        }
      } else if (std::holds_alternative<QApplication *>(i.key())) {
        if (qobject_cast<QApplication *>(std::get<QApplication *>(i.key())) ==
            nullptr) {
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

/**
 * 加载图标，同时可以更新所有注册过的按钮的图标
 * @param w QAbstractButton按钮对象
 * @param n 图标名称，不需要前缀和后缀
 * @param allupdate 是否更新所有注册过的按钮的图标
 */
inline void loadSvg(QAbstractButton *w, QString n, bool allupdate = false) {
  static QHash<QAbstractButton *, QString> widgets;

  if (allupdate) {
    QMutableHashIterator<QAbstractButton *, QString> i(widgets);
    while (i.hasNext()) {
      i.next();

      if (qobject_cast<QAbstractButton *>(i.key()) == nullptr) {
        i.remove();
        continue;
      }

      i.key()->setIcon(G::ondark ? ICONS[i.value()].dark
                                 : ICONS[i.value()].light);
    }
  } else if (qobject_cast<QAbstractButton *>(w)) {
    if (!ICONS.contains(n)) {
      ICONS[n] = {QIcon(QString(":/img/icon/%1.svg").arg(n)),
                  QIcon(QString(":/img/icon/dark/%1.svg").arg(n))};
    }
    widgets[w] = n;
    w->setIcon(G::ondark ? ICONS[n].dark : ICONS[n].light);
  }
}

/**
 * 设置属性并刷新样式
 * @param obj QWidget对象
 * @param name 属性名称
 * @param value 属性值
 */
inline void setProperty(QWidget *obj, const char *name, QVariant &&value) {
  obj->setProperty(name, value);
  obj->style()->unpolish(obj);
  obj->style()->polish(obj);
}
} // namespace G

/**
 * Com 线程的阻塞调用
 */
template <typename Func, typename... Args>
auto ComCallB(Func func, Args... args) {
  using R = std::invoke_result_t<Func, Com *, Args...>;
  R result;
  if (!G::com)
    return result;
  QMetaObject::invokeMethod(
      G::com, [&]() { result = (G::com->*func)(std::forward<Args>(args)...); },
      Qt::BlockingQueuedConnection);
  return result;
}
/**
 * Com 线程非阻塞调用
 */
template <typename Func, typename... Args>
void ComCall(Func func, Args... args) {
  if (!G::com)
    return;
  QMetaObject::invokeMethod(
      G::com,
      [func, args...]() mutable {
        (G::com->*func)(std::forward<Args>(args)...);
      },
      Qt::QueuedConnection);
}