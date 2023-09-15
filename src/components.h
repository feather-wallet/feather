// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_COMPONENTS_H
#define FEATHER_COMPONENTS_H

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

#endif //FEATHER_COMPONENTS_H
