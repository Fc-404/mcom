#include "mainwidget.hpp"
#include "MCombobox.hpp"
#include "global.hpp"
#include "ui_mainwidget.h"
#include <QAbstractItemView>
#include <QFile>
#include <QListView>
#include <QOperatingSystemVersion>

MainWidget::MainWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    QFile qssFile(":/qss/qss/mainwidget.qss");
    if (qssFile.open(QFile::ReadOnly)) {
        this->setStyleSheet(qssFile.readAll());
        qssFile.close();
    }

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
    setBgDark(G::config().value("Theme/ondark").toBool());

    // 设置 标题栏 功能按钮icon
    ui->header_close->setIcon(icon_close);
    ui->header_close->setIconSize(QSize(16, 16));
    ui->header_max->setIcon(icon_maximize);
    ui->header_max->setIconSize(QSize(14, 14));
    ui->header_min->setIcon(icon_minimize);
    ui->header_min->setIconSize(QSize(16, 16));
    ui->header_fixed->setIcon(icon_fixed);
    ui->header_fixed->setIconSize(QSize(16, 16));
    ui->header_dark->setIcon(icon_dark);

    auto combo = new CustomComboBox(this);
    combo->addItem("1");
    combo->addItem("2");
    combo->addItem("3");

    ui->c->addWidget(combo);

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

    ui->header_fixed->setIcon(onTop ? icon_enfixed : icon_fixed);
}

void MainWidget::dark()
{
    onDark = !onDark;
    ui->header_dark->setIcon(onDark ? icon_endark : icon_dark);
}

bool MainWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == ui->header_close) {
        if (event->type() == QEvent::Enter) {
            ui->header_close->setIcon(icon_close_hover);
        } else if (event->type() == QEvent::Leave) {
            ui->header_close->setIcon(icon_close);
        }
        ui->header_close->setIconSize(QSize(16, 16));
    }
    return QWidget::eventFilter(obj, event);
}

void MainWidget::setBgDark(bool dark)
{
    static const auto isWindows11 = []() -> bool {
        auto v = QOperatingSystemVersion::current();
        return v.type() == QOperatingSystemVersionBase::Windows
            && v.majorVersion() == 10
            && v.microVersion() >= 22000; // Windows 11
    };

    bool isblur = G::config().value("Theme/onblur").toBool();

    if (dark) {
        if (isblur && isWindows11()) {
            // 深色mica
            setAttribute(Qt::WA_TranslucentBackground, true);
            agent->setWindowAttribute("mica-alt", true);
            HWND hwnd = (HWND)this->winId();
            BOOL useDarkMode = TRUE;
            DwmSetWindowAttribute(hwnd, 20, &useDarkMode, sizeof(useDarkMode));

        } else {
            // 深色普通
            setAttribute(Qt::WA_TranslucentBackground, false);
        }
        G::ondark = true;
    } else {
        if (isblur && isWindows11()) {
            // 浅色mica
            setAttribute(Qt::WA_TranslucentBackground, true);
            agent->setWindowAttribute("mica-alt", true);
        } else {
            // 浅色普通
            setAttribute(Qt::WA_TranslucentBackground, false);
        }
        G::ondark = false;
    }
}

void MainWidget::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange) {
        // 这里更新全屏icon可以避免win+up键放大后icon不更新
        if (isMaximized()) {
            ui->header_max->setIcon(icon_remaximize);
        } else {
            ui->header_max->setIcon(icon_maximize);
        }
    }
    QWidget::changeEvent(event);
}
