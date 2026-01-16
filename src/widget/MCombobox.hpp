#ifndef __MCOMBOBOX_H__
#define __MCOMBOBOX_H__

#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class ComboPopup : public QWidget {
    Q_OBJECT
public:
    explicit ComboPopup(QWidget* parent = nullptr)
        : QWidget(parent) // 注意：parent 必须是 nullptr
    {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::NoDropShadowWindowHint);

        setAttribute(Qt::WA_ShowWithoutActivating);
        setFocusPolicy(Qt::NoFocus);
        setAttribute(Qt::WA_TranslucentBackground, true);
        setStyleSheet("QWidget { background-color: #f0f0f0; border-radius: 12px; padding: 0px; margin: 0px; }");
        // setFixedHeight(100);

        list = new QListWidget(this);

        connect(list, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
            emit itemSelected(item->text());
            hide();
        });
    }

    QListWidget* list;

signals:
    void itemSelected(const QString& text);
};

class CustomComboBox : public QWidget {
    Q_OBJECT
public:
    explicit CustomComboBox(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setFixedHeight(28);

        button = new QPushButton("", this);
        button->setObjectName("mbutton");
        button->setStyleSheet("QPushButton { width: 100px;}");

        auto layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(button);

        popup = new ComboPopup(this);

        connect(button, &QPushButton::clicked, this, &CustomComboBox::showPopup);
        connect(popup, &ComboPopup::itemSelected, this, &CustomComboBox::setCurrentText);
    }

    void addItem(const QString& text)
    {
        popup->list->addItem(text);
        if (button->text().isEmpty())
            button->setText(text);
    }

private slots:
    void showPopup()
    {
        if (popup->isVisible()) {
            popup->hide();
            return;
        }

        QPoint globalPos = mapToGlobal(QPoint(0, height()));
        popup->setFixedWidth(width());
        popup->move(globalPos);
        popup->show();
    }

    void setCurrentText(const QString& text)
    {
        button->setText(text);
    }

private:
    QPushButton* button;
    ComboPopup* popup;
};

#endif