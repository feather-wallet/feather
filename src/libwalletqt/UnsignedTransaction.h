// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef UNSIGNEDTRANSACTION_H
#define UNSIGNEDTRANSACTION_H

#include <QObject>

#include <wallet/api/wallet2_api.h>
#include "libwalletqt/PendingTransactionInfo.h"

class UnsignedTransaction : public QObject
{
    Q_OBJECT

public:
    enum Status {
        Status_Ok       = Monero::UnsignedTransaction::Status_Ok,
        Status_Error    = Monero::UnsignedTransaction::Status_Error,
        Status_Critical    = Monero::UnsignedTransaction::Status_Critical
    };
    Q_ENUM(Status)

    Status status() const;
    QString errorString() const;
    quint64 amount(size_t index) const;
    quint64 fee(size_t index) const;
    quint64 mixin(size_t index) const;
    QStringList recipientAddress() const;
    QStringList paymentId() const;
    quint64 txCount() const;
    QString confirmationMessage() const;
    quint64 minMixinCount() const;
    bool sign(const QString &fileName) const;
    bool signToStr(std::string &data) const;
    
    void setFilename(const QString &fileName);
    void refresh();

    ConstructionInfo * constructionInfo(int index) const;

private:
    explicit UnsignedTransaction(Monero::UnsignedTransaction * pt, Monero::Wallet *walletImpl, QObject *parent = nullptr);
    ~UnsignedTransaction();
private:
    friend class Wallet;
    Monero::UnsignedTransaction * m_pimpl;
    QString m_fileName;
    Monero::Wallet * m_walletImpl;
    mutable QList<ConstructionInfo*> m_construction_info;
};

#endif // UNSIGNEDTRANSACTION_H
