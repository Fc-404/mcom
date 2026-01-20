#include "mainwidget.hpp"
#include "global.hpp"
#include "ui_mainwidget.h"
#include <QAbstractItemView>
#include <QFile>
#include <QListView>
#include <QOperatingSystemVersion>
#include <QTimer>
#include <QWindow>

MainWidget::MainWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    G::loadQss(this, "mainwidget");

    // 注册 WindowKit 代理，管理窗口
    agent = new QWK::WidgetWindowAgent(this);
    agent->setup(this);
    agent->setTitleBar(ui->header);
    agent->setHitTestVisible(ui->header_funcs);
    agent->setHitTestVisible(ui->header_dark);
    agent->setHitTestVisible(ui->header_fixed);
    agent->setSystemButton(QWK::WindowAgentBase::Minimize, ui->header_min);
    agent->setSystemButton(QWK::WindowAgentBase::Maximize, ui->header_max);
    agent->setSystemButton(QWK::WindowAgentBase::Close, ui->header_close);

    // 设置背景特效
    setBgDark(false);

    // 设置 标题栏 功能按钮icon
    G::loadSvg(ui->header_close, "close");
    ui->header_close->setIconSize(QSize(16, 16));
    G::loadSvg(ui->header_max, "maximize");
    ui->header_max->setIconSize(QSize(14, 14));
    G::loadSvg(ui->header_min, "minimize");
    ui->header_min->setIconSize(QSize(16, 16));
    G::loadSvg(ui->header_fixed, "fixed");
    ui->header_fixed->setIconSize(QSize(16, 16));
    G::loadSvg(ui->header_dark, "dark");
    ui->header_dark->setIconSize(QSize(16, 16));

    // 处理标题栏按钮
    connect(ui->header_close, &QPushButton::clicked, this, &MainWidget::exit);
    ui->header_close->installEventFilter(this);
    connect(ui->header_max, &QPushButton::clicked, this, &MainWidget::maximize);
    connect(ui->header_min, &QPushButton::clicked, this, &MainWidget::minimize);
    connect(ui->header_fixed, &QPushButton::clicked, this, &MainWidget::fixed);
    connect(ui->header_dark, &QPushButton::clicked, this, &MainWidget::dark);
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::exit()
{
    QApplication::quit();
}

void MainWidget::maximize()
{
    if (isMaximized()) {
        showNormal();
    } else {
        showMaximized();
    }
}

void MainWidget::minimize()
{
    showMinimized();
}

void MainWidget::fixed()
{
    onTop = !onTop;
#ifdef Q_OS_WIN
    HWND h = (HWND)winId();
    SetWindowPos(h, onTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
#else
    setWindowFlag(Qt::WindowStaysOnTopHint, onTop);
#endif

    G::loadSvg(ui->header_fixed, onTop ? "enfixed" : "fixed");
}

void MainWidget::dark()
{
    onDark = !onDark;
    // G::loadSvg(ui->header_dark, onDark ? "endark" : "dark");
    setBgDark(onDark);
}

bool MainWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == ui->header_close) {
        if (event->type() == QEvent::Enter) {
            G::loadSvg(ui->header_close, "close_hover");
        } else if (event->type() == QEvent::Leave) {
            G::loadSvg(ui->header_close, "close");
        }
        ui->header_close->setIconSize(QSize(16, 16));
    }
    return QWidget::eventFilter(obj, event);
}

void MainWidget::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange) {
        // 这里更新全屏icon可以避免win+up键放大后icon不更新
        if (isMaximized()) {
            G::loadSvg(ui->header_max, "remaximize");
        } else {
            G::loadSvg(ui->header_max, "maximize");
        }
    }
    QWidget::changeEvent(event);
}

void MainWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    static bool afterInit = true;
    if (afterInit) {
        afterInit = false;

        /**
         * 一个奇怪的Bug
         * 使用windowkit代理后，不能够使用组件winId获取窗口句柄
         * 否则窗口绘制会出现问题，可能是代理自己管理句柄，结果winId立马创建了句柄
         * 使用组件的windowHandle->winId()可以获取到窗口句柄而不创建
         * 但是该方法只能在组件构造后使用，所以迁移到这里执行一次
         * 同时必须在构造时主动windowHandle()一次
         * 否则依旧出现绘制问题，不清楚这个问题
         */
        bool dark = G::config().value("Theme/ondark").toBool();
        onDark = dark;
        setBgDark(dark);
    }
}

void MainWidget::setBgDark(bool dark)
{
    static const auto isWindows11 = []() -> bool {
        auto v = QOperatingSystemVersion::current();
        return v.type() == QOperatingSystemVersionBase::Windows
            && v.majorVersion() == 10
            && v.microVersion() >= 22000; // Windows 11
    };

    bool isblur = false;
    if (isWindows11())
        isblur = G::config().value("Theme/onblur").toBool();

    if (isblur) {
#ifdef Q_OS_WIN
        if (auto* win = this->windowHandle()) {
            HWND hwnd = reinterpret_cast<HWND>(win->winId());
            BOOL useDarkMode = dark ? TRUE : FALSE;

            if (!dark) {
                // 深色mica
                DwmSetWindowAttribute(hwnd, 20, &useDarkMode, sizeof(useDarkMode));
            } else {
                // 浅色mica
                DwmSetWindowAttribute(hwnd, 20, &useDarkMode, sizeof(useDarkMode));
            }
        }
        setAttribute(Qt::WA_TranslucentBackground, true);
        agent->setWindowAttribute("mica-alt", true);
#endif
    } else {
        setAttribute(Qt::WA_TranslucentBackground, false);
    }

    G::ondark = dark;
    G::loadQss(this, nullptr, true);
    G::loadSvg(nullptr, nullptr, true);
}
