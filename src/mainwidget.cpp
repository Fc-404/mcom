#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QFile>
#include <QWKWidgets/widgetwindowagent.h>

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
    auto agent = new QWK::WidgetWindowAgent(this);
    agent->setup(this);
    agent->setTitleBar(ui->header);
    agent->setSystemButton(QWK::WindowAgentBase::Help, ui->header_fixed);
    agent->setSystemButton(QWK::WindowAgentBase::Minimize, ui->header_min);
    agent->setSystemButton(QWK::WindowAgentBase::Maximize, ui->header_max);
    agent->setSystemButton(QWK::WindowAgentBase::Close, ui->header_close);

    // 设置窗口透明，支持窗口特效
    setAttribute(Qt::WA_TranslucentBackground, true);
    agent->setWindowAttribute("mica-alt", true);

    // 设置 标题栏 功能按钮icon
    QIcon icon_close(":/img/img/close.svg");
    QIcon icon_close_hover(":/img/img/close_hover.svg");
    QIcon icon_max(":/img/img/maximize.svg");
    QIcon icon_min(":/img/img/minimize.svg");
    QIcon icon_fixed(":/img/img/fixed.svg");
    ui->header_close->setIcon(icon_close);
    ui->header_close->setIconSize(QSize(16, 16));
    ui->header_max->setIcon(icon_max);
    ui->header_max->setIconSize(QSize(14, 14));
    ui->header_min->setIcon(icon_min);
    ui->header_min->setIconSize(QSize(16, 16));
    ui->header_fixed->setIcon(icon_fixed);
    ui->header_fixed->setIconSize(QSize(15, 15));
    
}

MainWidget::~MainWidget()
{
    delete ui;
}
