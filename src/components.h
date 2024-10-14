// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_COMPONENTS_H
#define FEATHER_COMPONENTS_H

#include <QCompleter>
#include <QPushButton>
#include <QHBoxLayout>
#include <QDialog>
#include <QLabel>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QListWidget>

class DoublePixmapLabel : public QLabel
{
Q_OBJECT

public:
    explicit DoublePixmapLabel(QWidget *parent = nullptr);

public slots:
    void setAssets(const QString &firstAsset, const QString &secondAsset);
    void setMode(bool mode);

private:
    bool m_mode = false;
    QPixmap m_first;
    QPixmap m_second;
};

class StatusBarButton : public QPushButton
{
    Q_OBJECT

public:
    explicit StatusBarButton(const QIcon &icon, const QString &tooltip, QWidget *parent = nullptr);
};

class HelpLabel : public QLabel
{
    Q_OBJECT

public:
    QFont font;
    void setHelpText(const QString &text, const QString &informativeText, const QString &doc = "");

    explicit HelpLabel(QWidget * parent);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QString m_text;
    QString m_informativeText;
    QString m_doc;
};

class ClickableLabel : public QLabel {
Q_OBJECT

public:
    explicit ClickableLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~ClickableLabel() override;

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event) override;

};

class WindowModalDialog : public QDialog {
    Q_OBJECT

public:
    explicit WindowModalDialog(QWidget *parent);
};

class InfoFrame : public QFrame {
    Q_OBJECT

public:
    explicit InfoFrame(QWidget *parent);
    void setInfo(const QIcon &icon, const QString &text);
    void setText(const QString &text);

private:
    QPushButton *m_icon;
    QLabel *m_infoLabel;
};

class U32Validator : public QValidator {
public:
    U32Validator(QObject *parent = nullptr) : QValidator(parent) {}

    QValidator::State validate(QString &input, int &pos) const override {
        if (input.isEmpty()) {
            return QValidator::Intermediate;
        }

        bool ok;
        qint64 value = input.toLongLong(&ok);

        if (!ok) {
            return QValidator::Invalid;
        }

        if (value < 0 || value > UINT32_MAX) {
            return QValidator::Invalid;
        }

        return QValidator::Acceptable;
    }
};

class SpaceCompleter : public QCompleter {
protected:
    QString pathFromIndex(const QModelIndex &index) const override
    {
        QString completion = QCompleter::pathFromIndex(index);
        return completion + ' ';
    }
};

#endif //FEATHER_COMPONENTS_H
