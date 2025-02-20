// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "SubaddressRow.h"

quint32 SubaddressRow::getRow() const {
    return m_row;
}

const QString& SubaddressRow::getAddress() const {
    return m_address;
}

const QString& SubaddressRow::getLabel() const {
    return m_label;
}

bool SubaddressRow::isUsed() const {
    return m_used;
}

bool SubaddressRow::isPinned() const {
    return m_pinned;
}

bool SubaddressRow::isHidden() const {
    return m_hidden;
}