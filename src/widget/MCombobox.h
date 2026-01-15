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
        : QWidget(nullptr) // 注意：parent 必须是 nullptr
    {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::NoDropShadowWindowHint);

        setAttribute(Qt::WA_ShowWithoutActivating);
        setFocusPolicy(Qt::NoFocus);

        auto layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);

        list = new QListWidget(this);
        layout->addWidget(list);

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

        label = new QLabel(this);
        button = new QPushButton("▼", this);

        label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        button->setFixedWidth(24);

        auto layout = new QHBoxLayout(this);
        layout->setContentsMargins(6, 0, 0, 0);
        layout->addWidget(label);
        layout->addWidget(button);

        popup = new ComboPopup();

        connect(button, &QPushButton::clicked, this, &CustomComboBox::showPopup);
        connect(popup, &ComboPopup::itemSelected, this, &CustomComboBox::setCurrentText);
    }

    void addItem(const QString& text)
    {
        popup->list->addItem(text);
        if (label->text().isEmpty())
            label->setText(text);
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
        label->setText(text);
    }

private:
    QLabel* label;
    QPushButton* button;
    ComboPopup* popup;
};

#endif