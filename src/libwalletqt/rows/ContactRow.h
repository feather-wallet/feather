// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_CONTACTROW_H
#define FEATHER_CONTACTROW_H

#include <QObject>

class ContactRow : public QObject
{
    Q_OBJECT

public:
    ContactRow(QObject *parent, qsizetype row, const QString& address, const QString &label)
            : QObject(parent)
            , m_row(row)
            , m_address(address)
            , m_label(label) {}

    qsizetype getRow() const;
    const QString& getAddress() const;
    const QString& getLabel() const;

private:
    qsizetype m_row;
    QString m_address;
    QString m_label;
};


#endif //FEATHER_CONTACTROW_H
