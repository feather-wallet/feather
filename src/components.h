// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef COMP_H
#define COMP_H

#include <QPushButton>
#include <QHBoxLayout>
#include <QDialog>
#include <QLabel>
#include <QPlainTextEdit>
#include <QLineEdit>

class StatusBarButton : public QPushButton
{
    Q_OBJECT

public:
    explicit StatusBarButton(const QIcon &icon, const QString &tooltip, QWidget *parent = nullptr);
};

class WWLabel : public QLabel
{
    Q_OBJECT

public:
    explicit WWLabel(const QString& label="", QWidget *parent = nullptr);
};

class PasswordLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit PasswordLineEdit(QWidget *parent = nullptr);
};

class Buttons : public QHBoxLayout
{
    Q_OBJECT

public:
    explicit Buttons(QPushButton* arr[], int size, QWidget *parent = nullptr);
};


class CloseButton : public QPushButton
{
Q_OBJECT

public:
    explicit CloseButton(QDialog *dialog, QWidget *parent = nullptr);
};

class OkButton : public QPushButton
{
Q_OBJECT

public:
    explicit OkButton(QDialog *dialog, const QString& label, QWidget *parent = nullptr);
};


class ButtonsTextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    void setText(const QString &text);
    QString text();

    explicit ButtonsTextEdit(const QString &text = "");
};

class HelpLabel : public QLabel
{
    Q_OBJECT

public:
    QString help_text;
    QFont font;
    void setHelpText(const QString &text);

    explicit HelpLabel(QWidget * parent);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
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
#endif
