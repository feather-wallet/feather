// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "TransactionHistory.h"
#include "utils/Utils.h"
#include "utils/AppData.h"
#include "utils/config.h"
#include "constants.h"
#include "WalletManager.h"
#include "Transfer.h"
#include "Ring.h"
#include "wallet/wallet2.h"

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

bool TransactionHistory::transaction(int index, std::function<void (TransactionRow &)> callback)
{
    QReadLocker locker(&m_lock);

    if (index < 0 || index >= m_rows.size()) {
        qCritical("%s: no transaction info for index %d", __FUNCTION__, index);
        qCritical("%s: there's %d transactions in backend", __FUNCTION__, this->count());
        return false;
    }

    callback(*m_rows.value(index));
    return true;
}

TransactionRow* TransactionHistory::transaction(const QString &id)
{
    QReadLocker locker(&m_lock);

    auto itr = std::find_if(m_rows.begin(), m_rows.end(),
            [&](const TransactionRow * ti) {
        return ti->hash() == id;
    });
    return itr != m_rows.end() ? *itr : nullptr;
}

TransactionRow* TransactionHistory::transaction(int index)
{
    if (index < 0 || index >= m_rows.size()) {
        return nullptr;
    }

    return m_rows[index];
}

void TransactionHistory::refresh()
{
    qDebug() << Q_FUNC_INFO;

    QDateTime firstDateTime = QDate(2014, 4, 18).startOfDay();
    QDateTime lastDateTime  = QDateTime::currentDateTime().addDays(1); // tomorrow (guard against jitter and timezones)

    emit refreshStarted();

    {
        QWriteLocker locker(&m_lock);

        clearRows();

        quint64 lastTxHeight = 0;
        bool hasFakePaymentId = m_wallet->isTrezor();
        m_locked = false;
        m_minutesToUnlock = 0;

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

            auto* t = new TransactionRow(this);
            t->m_paymentId = QString::fromStdString(payment_id);
            t->m_coinbase = pd.m_coinbase;
            t->m_amount = pd.m_amount;
            t->m_balanceDelta = pd.m_amount;
            t->m_fee = pd.m_fee;
            t->m_direction = TransactionRow::Direction_In;
            t->m_hash = QString::fromStdString(epee::string_tools::pod_to_hex(pd.m_tx_hash));
            t->m_blockHeight = pd.m_block_height;
            t->m_subaddrIndex = { pd.m_subaddr_index.minor };
            t->m_subaddrAccount = pd.m_subaddr_index.major;
            t->m_label = QString::fromStdString(m_wallet2->get_subaddress_label(pd.m_subaddr_index));
            t->m_timestamp = QDateTime::fromSecsSinceEpoch(pd.m_timestamp);
            t->m_confirmations = (wallet_height > pd.m_block_height) ? wallet_height - pd.m_block_height : 0;
            t->m_unlockTime = pd.m_unlock_time;
            t->m_description = description(m_wallet2, pd);

            m_rows.append(t);
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

            auto* t = new TransactionRow(this);
            t->m_paymentId = QString::fromStdString(payment_id);

            t->m_amount = pd.m_amount_out - change;
            t->m_balanceDelta = change - pd.m_amount_in;
            t->m_fee = fee;

            t->m_direction = TransactionRow::Direction_Out;
            t->m_hash = QString::fromStdString(epee::string_tools::pod_to_hex(hash));
            t->m_blockHeight = pd.m_block_height;
            t->m_description = QString::fromStdString(m_wallet2->get_tx_note(hash));
            t->m_subaddrAccount = pd.m_subaddr_account;
            t->m_label = QString::fromStdString(pd.m_subaddr_indices.size() == 1 ? m_wallet2->get_subaddress_label({pd.m_subaddr_account, *pd.m_subaddr_indices.begin()}) : "");
            t->m_timestamp = QDateTime::fromSecsSinceEpoch(pd.m_timestamp);
            t->m_confirmations = (wallet_height > pd.m_block_height) ? wallet_height - pd.m_block_height : 0;

            for (uint32_t idx : t->subaddrIndex())
            {
                t->m_subaddrIndex.insert(idx);
            }

            // single output transaction might contain multiple transfers
            for (auto const &d: pd.m_dests)
            {
                Transfer *transfer = new Transfer(d.amount, QString::fromStdString(d.address(m_wallet2->nettype(), pd.m_payment_id, !hasFakePaymentId)), this);
                t->m_transfers.append(transfer);
            }
            for (auto const &r: pd.m_rings)
            {
                Ring *ring = new Ring(QString::fromStdString(epee::string_tools::pod_to_hex(r.first)), cryptonote::relative_output_offsets_to_absolute(r.second), this);
                t->m_rings.append(ring);
            }

            m_rows.append(t);
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

            auto *t = new TransactionRow(this);
            t->m_paymentId = QString::fromStdString(payment_id);

            t->m_amount = pd.m_amount_out - change;
            t->m_balanceDelta = change - pd.m_amount_in;
            t->m_fee = fee;

            t->m_direction = TransactionRow::Direction_Out;
            t->m_failed = is_failed;
            t->m_pending = true;
            t->m_hash = QString::fromStdString(epee::string_tools::pod_to_hex(hash));
            t->m_description = QString::fromStdString(m_wallet2->get_tx_note(hash));
            t->m_subaddrAccount = pd.m_subaddr_account;
            t->m_label = QString::fromStdString(pd.m_subaddr_indices.size() == 1 ? m_wallet2->get_subaddress_label({pd.m_subaddr_account, *pd.m_subaddr_indices.begin()}) : "");
            t->m_timestamp = QDateTime::fromSecsSinceEpoch(pd.m_timestamp);
            t->m_confirmations = 0;
            for (uint32_t idx : t->subaddrIndex())
            {
                t->m_subaddrIndex.insert(idx);
            }

            for (auto const &d: pd.m_dests)
            {
                Transfer *transfer = new Transfer(d.amount, QString::fromStdString(d.address(m_wallet2->nettype(), pd.m_payment_id, !hasFakePaymentId)), this);
                t->m_transfers.append(transfer);
            }
            for (auto const &r: pd.m_rings)
            {
                Ring *ring = new Ring(QString::fromStdString(epee::string_tools::pod_to_hex(r.first)), cryptonote::relative_output_offsets_to_absolute(r.second), this);
                t->m_rings.append(ring);
            }

            m_rows.append(t);
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
            auto *t = new TransactionRow(this);
            t->m_paymentId = QString::fromStdString(payment_id);
            t->m_amount = pd.m_amount;
            t->m_balanceDelta = pd.m_amount;
            t->m_direction = TransactionRow::Direction_In;
            t->m_hash = QString::fromStdString(epee::string_tools::pod_to_hex(pd.m_tx_hash));
            t->m_blockHeight = pd.m_block_height;
            t->m_pending = true;
            t->m_subaddrIndex = { pd.m_subaddr_index.minor };
            t->m_subaddrAccount = pd.m_subaddr_index.major;
            t->m_label = QString::fromStdString(m_wallet2->get_subaddress_label(pd.m_subaddr_index));
            t->m_timestamp = QDateTime::fromSecsSinceEpoch(pd.m_timestamp);
            t->m_confirmations = 0;
            t->m_description = description(m_wallet2, pd);

            m_rows.append(t);

            LOG_PRINT_L1(__FUNCTION__ << ": Unconfirmed payment found " << pd.m_amount);
        }
    }

    emit refreshFinished();
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

quint64 TransactionHistory::count() const
{
    QReadLocker locker(&m_lock);

    return m_rows.length();
}

QDateTime TransactionHistory::firstDateTime() const
{
    return m_firstDateTime;
}

QDateTime TransactionHistory::lastDateTime() const
{
    return m_lastDateTime;
}

quint64 TransactionHistory::minutesToUnlock() const
{
    return m_minutesToUnlock;
}

bool TransactionHistory::TransactionHistory::locked() const
{
    return m_locked;
}


TransactionHistory::TransactionHistory(Wallet *wallet, tools::wallet2 *wallet2, QObject *parent)
    : QObject(parent)
    , m_wallet(wallet)
    , m_wallet2(wallet2)
    , m_minutesToUnlock(0)
    , m_locked(false)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    m_firstDateTime = QDate(2014, 4, 18).startOfDay();
#else
    m_firstDateTime  = QDateTime(QDate(2014, 4, 18)); // the genesis block
#endif
    m_lastDateTime = QDateTime::currentDateTime().addDays(1); // tomorrow (guard against jitter and timezones)
}

void TransactionHistory::clearRows() {
    qDeleteAll(m_rows);
    m_rows.clear();
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
