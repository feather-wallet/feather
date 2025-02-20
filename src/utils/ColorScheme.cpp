// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2012 thomasv@gitorious
// SPDX-FileCopyrightText: The Monero Project

#include "ColorScheme.h"
#include <QDebug>

bool ColorScheme::darkScheme = false;
ColorSchemeItem ColorScheme::GREEN   = ColorSchemeItem("#117c11", "#8af296");
ColorSchemeItem ColorScheme::YELLOW  = ColorSchemeItem("#897b2a", "#ffff00");
ColorSchemeItem ColorScheme::RED     = ColorSchemeItem("#7c1111", "#f18c8c");
ColorSchemeItem ColorScheme::BLUE    = ColorSchemeItem("#123b7c", "#8cb3f2");
ColorSchemeItem ColorScheme::DEFAULT = ColorSchemeItem("black", "white");
ColorSchemeItem ColorScheme::GRAY    = ColorSchemeItem("gray", "gray");

bool ColorScheme::hasDarkBackground(QWidget *widget) {
    int r, g, b;
    widget->palette().color(QPalette::Window).getRgb(&r, &g, &b);
    auto brightness = r + g + b;
    return brightness < (255*3/2);
}

void ColorScheme::updateFromWidget(QWidget *widget, bool forceDark) {
    darkScheme = forceDark || ColorScheme::hasDarkBackground(widget);
}

QString ColorSchemeItem::asStylesheet(bool background) {
    auto cssPrefix = background ? "background-" : "";
    auto color = this->getColor(background);
    return QString("QWidget { %1color : %2; }").arg(cssPrefix, color);
}

QColor ColorSchemeItem::asColor(bool background) {
    auto color = this->getColor(background);
    return QColor(color);
}

QString ColorSchemeItem::getColor(bool background) {
    return m_colors[(int(background) + int(ColorScheme::darkScheme)) % 2];
}