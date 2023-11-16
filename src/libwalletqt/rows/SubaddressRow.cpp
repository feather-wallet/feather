// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "SubaddressRow.h"

bool SubaddressRow::setHidden(bool hidden) {
    m_hidden = hidden;
}

bool SubaddressRow::setUsed(bool used) {
    m_used = used;
}

bool SubaddressRow::setPinned(bool pinned) {
    m_used = pinned;
}

qsizetype SubaddressRow::getRow() const {
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