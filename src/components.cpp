// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "components.h"

#include <QtWidgets>

#include "Utils.h"

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

void HelpLabel::setHelpText(const QString &text, const QString &informativeText, const QString &doc)
{
    m_text = text;
    m_informativeText = informativeText;
    m_doc = doc;
}

HelpLabel::HelpLabel(QWidget *parent) : QLabel(parent)
{
    this->font = QFont();
}

void HelpLabel::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    Utils::showInfo(this, m_text, m_informativeText, {}, m_doc);
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

InfoFrame::InfoFrame(QWidget *parent)
    : QFrame(parent)
{
  auto *layout = new QHBoxLayout(this);

  m_icon = new QPushButton(this);
  m_icon->setFlat(true);
  m_icon->setIconSize(QSize(32, 32));
  m_icon->setMaximumSize(32, 32);
  m_icon->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
  layout->addWidget(m_icon);

  auto *spacer = new QSpacerItem(5, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
  layout->addSpacerItem(spacer);

  m_infoLabel = new QLabel(this);
  m_infoLabel->setWordWrap(true);
  m_infoLabel->setTextFormat(Qt::MarkdownText);
  m_infoLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
  layout->addWidget(m_infoLabel);
}

void InfoFrame::setInfo(const QIcon &icon, const QString &text) {
  m_icon->setIcon(icon);
  m_infoLabel->setText(text);

#ifdef Q_OS_MACOS
  this->setFrameShape(QFrame::Box);
#else
  this->setFrameShape(QFrame::StyledPanel);
#endif
}

void InfoFrame::setText(const QString &text) {
  m_infoLabel->setText(text);
}