// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_ICONS_H
#define FEATHER_ICONS_H

#include <QIcon>
#include <QHash>
#include <QString>

class Icons {
public:
    QIcon icon(const QString& name);

    static Icons* instance();

private:
    Icons();

    static Icons* m_instance;

    QHash<QString, QIcon> m_iconCache;

    Q_DISABLE_COPY(Icons)
};

inline Icons* icons()
{
    return Icons::instance();
}

#endif //FEATHER_ICONS_H
