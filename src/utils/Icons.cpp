// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "Icons.h"

Icons* Icons::m_instance(nullptr);

Icons::Icons()
= default;

QIcon Icons::icon(const QString& name)
{
    QIcon icon = m_iconCache.value(name);

    if (!icon.isNull()) {
        return icon;
    }

    icon = QIcon{":/assets/images/" + name};

    m_iconCache.insert(name, icon);
    return icon;
}

Icons* Icons::instance()
{
    if (!m_instance) {
        m_instance = new Icons();
    }

    return m_instance;
}
