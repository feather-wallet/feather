// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "TransactionHistory.h"
#include "utils/Utils.h"
#include "utils/AppData.h"
#include "utils/config.h"
#include "constants.h"
#include "Wallet.h"
#include "WalletManager.h"
#include "rows/Output.h"
#include "wallet/wallet2.h"

TransactionHistory::TransactionHistory(Wallet *wallet, tools::wallet2 *wallet2, QObject *parent)
    : QObject(parent)
    , m_wallet(wallet)
    , m_wallet2(wallet2)
    , m_locked(false)
{

}

QString description(tools::wallet2 *wallet2, const tools::wallet2::payment_details &pd)
{
    QString description = QString::fromStdString(wallet2->get_tx_note(pd.m_tx_hash));
    if (description.isEmpty()) {
        if (pd.m_coinbase) {
            description = "Coinbase";
        }
        else if (pd.m_subaddr_index.major == 0 && pd.m_subaddr_index.minor == 0) {
            description = "Primary address";
        }
        else {
            description = QString::fromStdString(wallet2->get_subaddress_label(pd.m_subaddr_index));
        }
    }
    return description;
}

void TransactionHistory::refresh()
{
    qDebug() << Q_FUNC_INFO;

    emit refreshStarted();

    {
        QWriteLocker locker(&m_lock);

        m_rows.clear();

        quint64 lastTxHeight = 0;
        bool hasFakePaymentId = m_wallet->isTrezor();
        m_locked = false;

        uint64_t min_height = 0;
        uint64_t max_height = (uint64_t)-1;
        uint64_t wallet_height = m_wallet->blockChainHeight();
        uint32_t account = m_wallet->currentSubaddressAccount();

        // transactions are stored in wallet2:
        // - confirmed_transfer_details   - out transfers
        // - unconfirmed_transfer_details - pending out transfers
        // - payment_details              - input transfers

        // payments are "input transactions";
        // one input transaction contains only one transfer. e.g. <transaction_id> - <100XMR>

        std::list<std::pair<crypto::hash, tools::wallet2::payment_details>> in_payments;
        m_wallet2->get_payments(in_payments, min_height, max_height);
        for (std::list<std::pair<crypto::hash, tools::wallet2::payment_details>>::const_iterator i = in_payments.begin(); i != in_payments.end(); ++i)
        {
            const tools::wallet2::payment_details &pd = i->second;
            if (pd.m_subaddr_index.major != account) {
                continue;
            }

            std::string payment_id = epee::string_tools::pod_to_hex(i->first);
            if (payment_id.substr(16).find_first_not_of('0') == std::string::npos)
                payment_id = payment_id.substr(0,16);

            TransactionRow t;
            t.paymentId = QString::fromStdString(payment_id);
            t.coinbase = pd.m_coinbase;
            t.amount = pd.m_amount;
            t.balanceDelta = pd.m_amount;
            t.fee = pd.m_fee;
            t.direction = TransactionRow::Direction_In;
            t.hash = QString::fromStdString(epee::string_tools::pod_to_hex(pd.m_tx_hash));
            t.blockHeight = pd.m_block_height;
            t.subaddrIndex = { pd.m_subaddr_index.minor };
            t.subaddrAccount = pd.m_subaddr_index.major;
            t.label = QString::fromStdString(m_wallet2->get_subaddress_label(pd.m_subaddr_index));
            t.timestamp = QDateTime::fromSecsSinceEpoch(pd.m_timestamp);
            t.confirmations = (wallet_height > pd.m_block_height) ? wallet_height - pd.m_block_height : 0;
            t.unlockTime = pd.m_unlock_time;
            t.description = description(m_wallet2, pd);

            m_rows.append(std::move(t));
        }

        // confirmed output transactions
        // one output transaction may contain more than one money transfer, e.g.
        // <transaction_id>:
        //    transfer1: 100XMR to <address_1>
        //    transfer2: 50XMR  to <address_2>
        //    fee: fee charged per transaction
        //

        std::list<std::pair<crypto::hash, tools::wallet2::confirmed_transfer_details>> out_payments;
        m_wallet2->get_payments_out(out_payments, min_height, max_height);

        for (std::list<std::pair<crypto::hash, tools::wallet2::confirmed_transfer_details>>::const_iterator i = out_payments.begin();
             i != out_payments.end(); ++i) {

            const crypto::hash &hash = i->first;
            const tools::wallet2::confirmed_transfer_details &pd = i->second;
            if (pd.m_subaddr_account != account) {
                continue;
            }

            uint64_t change = pd.m_change == (uint64_t)-1 ? 0 : pd.m_change; // change may not be known
            uint64_t fee = pd.m_amount_in - pd.m_amount_out;

            std::string payment_id = epee::string_tools::pod_to_hex(i->second.m_payment_id);
            if (payment_id.substr(16).find_first_not_of('0') == std::string::npos)
                payment_id = payment_id.substr(0,16);

            TransactionRow t;
            t.paymentId = QString::fromStdString(payment_id);

            t.amount = pd.m_amount_out - change;
            t.balanceDelta = change - pd.m_amount_in;
            t.fee = fee;

            t.direction = TransactionRow::Direction_Out;
            t.hash = QString::fromStdString(epee::string_tools::pod_to_hex(hash));
            t.blockHeight = pd.m_block_height;
            t.description = QString::fromStdString(m_wallet2->get_tx_note(hash));
            t.subaddrAccount = pd.m_subaddr_account;
            t.label = QString::fromStdString(pd.m_subaddr_indices.size() == 1 ? m_wallet2->get_subaddress_label({pd.m_subaddr_account, *pd.m_subaddr_indices.begin()}) : "");
            t.timestamp = QDateTime::fromSecsSinceEpoch(pd.m_timestamp);
            t.confirmations = (wallet_height > pd.m_block_height) ? wallet_height - pd.m_block_height : 0;

            for (uint32_t idx : t.subaddrIndex)
            {
                t.subaddrIndex.insert(idx);
            }

            // single output transaction might contain multiple transfers
            for (auto const &d: pd.m_dests)
            {
                t.transfers.emplace_back(
                    d.amount,
                    QString::fromStdString(d.address(m_wallet2->nettype(), pd.m_payment_id, !hasFakePaymentId)));
            }
            for (auto const &r: pd.m_rings)
            {
                t.rings.emplace_back(
                    QString::fromStdString(epee::string_tools::pod_to_hex(r.first)),
                    cryptonote::relative_output_offsets_to_absolute(r.second));
            }

            m_rows.append(std::move(t));
        }

        // unconfirmed output transactions
        std::list<std::pair<crypto::hash, tools::wallet2::unconfirmed_transfer_details>> upayments_out;
        m_wallet2->get_unconfirmed_payments_out(upayments_out);
        for (std::list<std::pair<crypto::hash, tools::wallet2::unconfirmed_transfer_details>>::const_iterator i = upayments_out.begin(); i != upayments_out.end(); ++i) {
            const tools::wallet2::unconfirmed_transfer_details &pd = i->second;
            if (pd.m_subaddr_account != account) {
                continue;
            }

            const crypto::hash &hash = i->first;
            uint64_t amount = pd.m_amount_in;
            uint64_t fee = amount - pd.m_amount_out;
            uint64_t change = pd.m_change == (uint64_t)-1 ? 0 : pd.m_change;
            std::string payment_id = epee::string_tools::pod_to_hex(i->second.m_payment_id);
            if (payment_id.substr(16).find_first_not_of('0') == std::string::npos)
                payment_id = payment_id.substr(0,16);
            bool is_failed = pd.m_state == tools::wallet2::unconfirmed_transfer_details::failed;

            TransactionRow t;
            t.paymentId = QString::fromStdString(payment_id);

            t.amount = pd.m_amount_out - change;
            t.balanceDelta = change - pd.m_amount_in;
            t.fee = fee;

            t.direction = TransactionRow::Direction_Out;
            t.failed = is_failed;
            t.pending = true;
            t.hash = QString::fromStdString(epee::string_tools::pod_to_hex(hash));
            t.description = QString::fromStdString(m_wallet2->get_tx_note(hash));
            t.subaddrAccount = pd.m_subaddr_account;
            t.label = QString::fromStdString(pd.m_subaddr_indices.size() == 1 ? m_wallet2->get_subaddress_label({pd.m_subaddr_account, *pd.m_subaddr_indices.begin()}) : "");
            t.timestamp = QDateTime::fromSecsSinceEpoch(pd.m_timestamp);
            t.confirmations = 0;
            for (uint32_t idx : t.subaddrIndex)
            {
                t.subaddrIndex.insert(idx);
            }

            for (auto const &d: pd.m_dests)
            {
                t.transfers.emplace_back(
                    d.amount,
                    QString::fromStdString(d.address(m_wallet2->nettype(), pd.m_payment_id, !hasFakePaymentId)));
            }
            for (auto const &r: pd.m_rings)
            {
                t.rings.emplace_back(
                    QString::fromStdString(epee::string_tools::pod_to_hex(r.first)),
                    cryptonote::relative_output_offsets_to_absolute(r.second));
            }

            m_rows.append(std::move(t));
        }


        // unconfirmed payments (tx pool)
        std::list<std::pair<crypto::hash, tools::wallet2::pool_payment_details>> upayments;
        m_wallet2->get_unconfirmed_payments(upayments);
        for (std::list<std::pair<crypto::hash, tools::wallet2::pool_payment_details>>::const_iterator i = upayments.begin(); i != upayments.end(); ++i) {
            const tools::wallet2::payment_details &pd = i->second.m_pd;
            if (pd.m_subaddr_index.major != account) {
                continue;
            }

            std::string payment_id = epee::string_tools::pod_to_hex(i->first);
            if (payment_id.substr(16).find_first_not_of('0') == std::string::npos)
                payment_id = payment_id.substr(0,16);

            TransactionRow t;

            t.paymentId = QString::fromStdString(payment_id);
            t.amount = pd.m_amount;
            t.balanceDelta = pd.m_amount;
            t.direction = TransactionRow::Direction_In;
            t.hash = QString::fromStdString(epee::string_tools::pod_to_hex(pd.m_tx_hash));
            t.blockHeight = pd.m_block_height;
            t.pending = true;
            t.subaddrIndex = { pd.m_subaddr_index.minor };
            t.subaddrAccount = pd.m_subaddr_index.major;
            t.label = QString::fromStdString(m_wallet2->get_subaddress_label(pd.m_subaddr_index));
            t.timestamp = QDateTime::fromSecsSinceEpoch(pd.m_timestamp);
            t.confirmations = 0;
            t.description = description(m_wallet2, pd);

            m_rows.append(std::move(t));

            LOG_PRINT_L1(__FUNCTION__ << ": Unconfirmed payment found " << pd.m_amount);
        }
    }

    emit refreshFinished();
}

quint64 TransactionHistory::count() const
{
    QReadLocker locker(&m_lock);

    return m_rows.length();
}

const TransactionRow& TransactionHistory::transaction(int index)
{
    if (index < 0 || index >= m_rows.size()) {
        throw std::out_of_range("Index out of range");
    }
    return m_rows[index];
}

const QList<TransactionRow>& TransactionHistory::getRows()
{
    return m_rows;
}

void TransactionHistory::setTxNote(const QString &txid, const QString &note)
{
    cryptonote::blobdata txid_data;
    if (!epee::string_tools::parse_hexstr_to_binbuff(txid.toStdString(), txid_data) || txid_data.size() != sizeof(crypto::hash)) {
        qDebug() << Q_FUNC_INFO << "invalid txid";
        return;
    }

    const crypto::hash htxid = *reinterpret_cast<const crypto::hash*>(txid_data.data());

    m_wallet2->set_tx_note(htxid, note.toStdString());
    emit txNoteChanged();
}

bool TransactionHistory::locked() const
{
    return m_locked;
}

QStringList parseCSVLine(const QString &line) {
    QStringList result;
    QString currentField;
    bool inQuotes = false;

    for (int i = 0; i < line.length(); ++i) {
        QChar currentChar = line[i];

        if (currentChar == '"') {
            if (inQuotes && i + 1 < line.length() && line[i + 1] == '"') {
                currentField.append('"');
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (currentChar == ',' && !inQuotes) {
            result.append(currentField.trimmed());
            currentField.clear();
        } else {
            currentField.append(currentChar);
        }
    }

    result.append(currentField.trimmed());
    return result;
}

QString TransactionHistory::importLabelsFromCSV(const QString &fileName) {
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString("Could not open file: %1").arg(fileName);
    }

    QTextStream in(&file);

    QList<QStringList> fields;
    while (!in.atEnd()) {
        QString line = in.readLine();
        fields.append(parseCSVLine(line));
    }

    if (fields.empty()) {
        return "CSV file appears to be empty";
    }

    qint64 txidField = -1;
    qint64 descriptionField = -1;

    QStringList header = fields[0];
    for (int i = 0; i < header.length(); i++) {
        if (header[i] == "txid") {
            txidField = i;
            continue;
        }
        if (header[i] == "description") {
            descriptionField = i;
        }
    }

    if (txidField < 0) {
        return "'txid' field not found in CSV header";
    }
    if (descriptionField < 0) {
        return "'description' field not found in CSV header";
    }
    qint64 maxIndex = std::max(txidField, descriptionField);

    QList<QPair<QString, QString>> descriptions;

    for (int i = 1; i < fields.length(); i++) {
        const auto& row = fields[i];
        if (maxIndex >= row.length()) {
            qDebug() << "Row with invalid length in CSV";
            continue;
        }

        if (row[txidField].isEmpty()) {
            continue;
        }

        if (row[descriptionField].isEmpty()) {
            continue;
        }

        descriptions.push_back({row[txidField], row[descriptionField]});
    }

    for (const auto& description : descriptions) {
        qDebug() << "Setting note for tx:" << description.first << "description:" << description.second;
        this->setTxNote(description.first, description.second);
    }

    this->refresh();

    return {};
}
