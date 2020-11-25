// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "components.h"

#include <QtWidgets>

StatusBarButton::StatusBarButton(const QIcon &icon, const QString &tooltip, QWidget *parent) : QPushButton(parent) {
    setIcon(icon);
    setToolTip(tooltip);
    setFlat(true);
    setMaximumWidth(20);
    setIconSize(QSize(20,20));
    setCursor(QCursor(Qt::PointingHandCursor));
}

WWLabel::WWLabel(const QString& text, QWidget *parent) : QLabel(text, parent){
    setWordWrap(true);
    setTextInteractionFlags(Qt::TextSelectableByMouse);
}

PasswordLineEdit::PasswordLineEdit(QWidget *parent) : QLineEdit(parent) {
    setEchoMode(Password);
}

Buttons::Buttons(QPushButton* arr[], int size, QWidget *parent) : QHBoxLayout(parent) {
    addStretch();
    for (int n=0; n < size; n++) {
        addWidget(arr[n]);
    }
}

OkButton::OkButton(QDialog *dialog, const QString& label = nullptr, QWidget *parent) : QPushButton(parent){
    setText(label == nullptr ? label : "OK");
    connect(this, &OkButton::clicked, dialog, &QDialog::accept);
    setDefault(true);
}

CloseButton::CloseButton(QDialog *dialog, QWidget *parent) : QPushButton(parent) {
    setText("Close");
    connect(this, &CloseButton::clicked, dialog, &QDialog::close);
    setDefault(true);
}

ButtonsTextEdit::ButtonsTextEdit(const QString &text) : QPlainTextEdit(text) {

}

void ButtonsTextEdit::setText(const QString &text) {
    this->setPlainText(text);
}

QString ButtonsTextEdit::text(){
    return this->toPlainText();
}

void HelpLabel::setHelpText(const QString &text)
{
    this->help_text = text;
}

HelpLabel::HelpLabel(QWidget *parent) : QLabel(parent)
{
    this->help_text = "help_text";
    this->font = QFont();
}

void HelpLabel::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    QMessageBox::information(this, "Help", this->help_text);
}

void HelpLabel::enterEvent(QEvent *event)
{
   font.setUnderline(true);
   setFont(font);
   QApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor));
   return QLabel::enterEvent(event);
}

void HelpLabel::leaveEvent(QEvent *event)
{
    font.setUnderline(false);
    setFont(font);
    QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
    return QLabel::leaveEvent(event);
}

ClickableLabel::ClickableLabel(QWidget* parent, Qt::WindowFlags f)
        : QLabel(parent) {
}

ClickableLabel::~ClickableLabel() = default;

void ClickableLabel::mousePressEvent(QMouseEvent* event) {
    emit clicked();
}