// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2012 thomasv@gitorious
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_COLORSCHEME_H
#define FEATHER_COLORSCHEME_H

#include <QString>
#include <QVector>
#include <QColor>
#include <QWidget>

class ColorSchemeItem {

public:
    explicit ColorSchemeItem(const QString &fgColor, const QString &bgColor)
            : m_colors({fgColor, bgColor}) {}

    QString asStylesheet(bool background = false);
    QColor asColor(bool background = false);

private:
    QString getColor(bool background);
    QVector<QString> m_colors;
};


class ColorScheme {
public:
    static bool darkScheme;

    static ColorSchemeItem GREEN;
    static ColorSchemeItem YELLOW;
    static ColorSchemeItem RED;
    static ColorSchemeItem BLUE;
    static ColorSchemeItem DEFAULT;
    static ColorSchemeItem GRAY;

    static bool hasDarkBackground(QWidget *widget);
    static void updateFromWidget(QWidget *widget, bool forceDark = false);
};


#endif //FEATHER_COLORSCHEME_H
