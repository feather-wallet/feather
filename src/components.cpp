// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "components.h"

#include <QtWidgets>

DoublePixmapLabel::DoublePixmapLabel(QWidget *parent)
        : QLabel(parent)
{}

void DoublePixmapLabel::setAssets(const QString &firstAsset, const QString &secondAsset)
{
    m_first.load(firstAsset);
    m_second.load(secondAsset);
    this->setPixmap(m_first);
}

void DoublePixmapLabel::setMode(bool mode) {
    if (mode != m_mode) {
        this->setPixmap(mode ? m_second : m_first);
    }
    m_mode = mode;
}

StatusBarButton::StatusBarButton(const QIcon &icon, const QString &tooltip, QWidget *parent) : QPushButton(parent) {
    setIcon(icon);
    setToolTip(tooltip);
    setFlat(true);
    setMaximumWidth(20);
    setIconSize(QSize(20,20));
    setCursor(QCursor(Qt::PointingHandCursor));
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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void HelpLabel::enterEvent(QEvent *event)
#else
void HelpLabel::enterEvent(QEnterEvent *event)
#endif
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

WindowModalDialog::WindowModalDialog(QWidget *parent)
    : QDialog(parent)
{
#ifndef Q_OS_MACOS
    this->setWindowModality(Qt::WindowModal);
#endif
}