// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.
// Copyright (c) 2012 thomasv@gitorious

#include "PayToEdit.h"

#include <QtGlobal>
#include <QScrollBar>

#include "libwalletqt/WalletManager.h"
#include "model/ModelUtils.h"

PayToEdit::PayToEdit(QWidget *parent) : QPlainTextEdit(parent)
{
    this->setFont(ModelUtils::getMonospaceFont());

    connect(this->document(), &QTextDocument::contentsChanged, this, &PayToEdit::updateSize);
    connect(this, &QPlainTextEdit::textChanged, this, &PayToEdit::checkText);

    this->updateSize();
}

void PayToEdit::setNetType(NetworkType::Type netType) {
    m_netType = netType;
}

void PayToEdit::setText(const QString &text) {
    this->setPlainText(text);
}

QString PayToEdit::text() {
    return this->toPlainText();
}

QVector<PayToLineError> PayToEdit::getErrors() {
    return m_errors;
}

QVector<PartialTxOutput> PayToEdit::getOutputs() {
    return m_outputs;
}

quint64 PayToEdit::getTotal() {
    return m_total;
}

QStringList PayToEdit::lines() {
    return this->toPlainText().split("\n");
}

bool PayToEdit::isMultiline() {
    return this->lines().size() > 1;
}

void PayToEdit::payToMany() {
    this->setPlainText("\n\n\n");
    this->updateSize();
}

bool PayToEdit::isOpenAlias() {
    if (this->isMultiline()) {
        return false;
    }
    auto text = this->toPlainText().trimmed();
    if (!(text.contains('.') and (!text.contains(' ')))) {
        return false;
    }
    auto parts = text.split(',');
    if (parts.size() > 0 and WalletManager::addressValid(parts[0], m_netType)) {
        return false;
    }
    return true;
}

void PayToEdit::checkText() {
    m_errors.clear();
    m_outputs.clear();

    // filter out empty lines
    QStringList lines;
    for (auto &l : this->lines()) {
        if (!l.isEmpty()) {
            lines.push_back(l);
        }
    }

    this->parseAsMultiline(lines);
}

void PayToEdit::updateSize() {
    qreal lineHeight = QFontMetrics(this->document()->defaultFont()).height();
    qreal docHeight = this->document()->size().height();
    int h = int(docHeight * lineHeight + 11);
    h = qMin(qMax(h, m_heightMin), m_heightMax);
    this->setMinimumHeight(h);
    this->setMaximumHeight(h);
    this->verticalScrollBar()->hide();
}

PartialTxOutput PayToEdit::parseAddressAndAmount(const QString &line) {
    QStringList x = line.split(",");
    if (x.size() != 2) {
        return PartialTxOutput();
    }

    QString address = this->parseAddress(x[0]);
    quint64 amount = this->parseAmount(x[1]);

    return PartialTxOutput(address, amount);
}

quint64 PayToEdit::parseAmount(QString amount) {
    amount.replace(',', '.');
    if (amount.isEmpty()) return 0;

    return WalletManager::amountFromString(amount.trimmed());
}

QString PayToEdit::parseAddress(QString address) {
    if (!WalletManager::addressValid(address.trimmed(), m_netType)) {
        return "";
    }
    return address;
}

void PayToEdit::parseAsMultiline(const QStringList &lines) {
    m_outputs.clear();
    m_total = 0;

    int i = 0;
    for (auto &line : lines) {
        PartialTxOutput output = this->parseAddressAndAmount(line);
        if (output.address.isEmpty() && output.amount == 0) {
            m_errors.append(PayToLineError(line, "Expected two comma-separated values: (address, amount)", i, true));
            continue;
        } else if (output.address.isEmpty()) {
            m_errors.append(PayToLineError(line, "Invalid address", i, true));
            continue;
        } else if (output.amount == 0) {
            m_errors.append(PayToLineError(line, "Invalid amount", i, true));
            continue;
        }

        m_outputs.append(output);
        m_total += output.amount;
        i += 1;
    }
}