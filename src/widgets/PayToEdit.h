// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.
// Copyright (c) 2012 thomasv@gitorious

#ifndef FEATHER_PAYTOEDIT_H
#define FEATHER_PAYTOEDIT_H

#include <QObject>
#include <QPlainTextEdit>

#include "utils/Utils.h"

struct PartialTxOutput {
    explicit PartialTxOutput(QString address = "", quint64 amount = 0)
        : address(address), amount(amount) {}

    QString address;
    quint64 amount;
};

struct PayToLineError {
    explicit PayToLineError(QString lineContent, QString error, int idx = 0, bool isMultiline = false)
        : lineContent(lineContent), error(error), idx(idx), isMultiline(isMultiline) {}

    QString lineContent;
    QString error;
    int idx;
    bool isMultiline;
};

class PayToEdit : public QPlainTextEdit
{
Q_OBJECT

public:
    explicit PayToEdit(QWidget *parent = nullptr);

    void setNetType(NetworkType::Type netType);
    void setText(const QString &text);
    QString text();

    QVector<PayToLineError> getErrors();
    QVector<PartialTxOutput> getOutputs();
    quint64 getTotal();

    QStringList lines();
    bool isMultiline();
    void payToMany();
    bool isOpenAlias();

private:
    void checkText();
    void updateSize();

    PartialTxOutput parseAddressAndAmount(const QString &line);
    quint64 parseAmount(QString amount);
    QString parseAddress(QString address);

    void parseAsMultiline(const QStringList &lines);

    int m_heightMin = 0;
    int m_heightMax = 150;
    quint64 m_total = 0;
    NetworkType::Type m_netType = NetworkType::Type::MAINNET;

    QVector<PayToLineError> m_errors;
    QVector<PartialTxOutput> m_outputs;
};

#endif //FEATHER_PAYTOEDIT_H
