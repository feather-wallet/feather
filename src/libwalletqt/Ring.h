// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_RINGS_H
#define FEATHER_RINGS_H

#include <wallet/api/wallet2_api.h>
#include <QObject>
#include <QList>
#include <utility>

class Ring : public QObject
{
Q_OBJECT
    Q_PROPERTY(QString keyImage READ keyImage)
    Q_PROPERTY(std::vector<uint64_t> ringMembers READ ringMembers)
private:
    explicit Ring(QString _keyImage, std::vector<uint64_t> _ringMembers, QObject *parent = nullptr): QObject(parent), m_keyImage(std::move(_keyImage)), m_ringMembers(std::move(_ringMembers)) {};
private:
    friend class TransactionInfo;
    QString m_keyImage;
    std::vector<uint64_t> m_ringMembers;
public:
    QString keyImage() const { return m_keyImage; }
    std::vector<uint64_t> ringMembers() const { return m_ringMembers; }
};

#endif //FEATHER_RINGS_H
