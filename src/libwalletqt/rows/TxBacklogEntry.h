// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_TXBACKLOGENTRY_H
#define FEATHER_TXBACKLOGENTRY_H

struct TxBacklogEntry {
    quint64 weight;
    quint64 fee;
    quint64 timeInPool;
};

#endif //FEATHER_TXBACKLOGENTRY_H
