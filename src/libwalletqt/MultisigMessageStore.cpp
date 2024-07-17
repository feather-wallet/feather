// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "MultisigMessageStore.h"

#include "Coins.h"
#include "rows/CoinsInfo.h"
#include <wallet/wallet2.h>
#include "rows/MultisigMessage.h"
#include "rows/TxProposal.h"
#include "libwalletqt/Input.h"
#include "utils/Utils.h"
#include "multisig/multisig_account.h"

MultisigMessageStore::MultisigMessageStore(Wallet *wallet, tools::wallet2 *wallet2, QObject *parent)
        : QObject(parent)
        , m_wallet(wallet)
        , m_wallet2(wallet2)
{

}

bool MultisigMessageStore::message(int index, std::function<void (MultisigMessage &)> callback)
{
    QReadLocker locker(&m_lock);

    if (index < 0 || index >= m_rows.size()) {
        qCritical("%s: no transaction info for index %d", __FUNCTION__, index);
        qCritical("%s: there's %lld transactions in backend", __FUNCTION__, m_rows.count());
        return false;
    }

    callback(*m_rows.value(index));
    return true;
}

bool MultisigMessageStore::txProposal(int index, std::function<void (TxProposal &)> callback)
{
    QReadLocker locker(&m_lock);

    if (index < 0 || index >= m_txProposals.size()) {
        qCritical("%s: no transaction info for index %d", __FUNCTION__, index);
        qCritical("%s: there's %lld transactions in backend", __FUNCTION__, m_txProposals.count());
        return false;
    }

    callback(*m_txProposals.value(index));
    return true;
}


MultisigMessage* MultisigMessageStore::message(int index)
{
    return m_rows.value(index);
}

bool MultisigMessageStore::signTx(quint32 id) {
    mms::message_store& ms = m_wallet2->get_message_store();
    mms::message m;
    bool valid_id = ms.get_message_by_id(id, m);
    if (!valid_id) {
        return false;
    }
    return signMultisigTx(m.content);
}

bool MultisigMessageStore::deleteMessage(quint32 id) {
    mms::message_store& ms = m_wallet2->get_message_store();
    mms::message m;
    bool valid_id = ms.get_message_by_id(id, m);
    if (!valid_id) {
        return false;
    }
    ms.delete_message(m.id);
    this->refresh(false);
    return true;
}

void MultisigMessageStore::deleteAllMessages() {
    mms::message_store& ms = m_wallet2->get_message_store();
    ms.delete_all_messages();
    this->refresh(false);
}

std::string MultisigMessageStore::exportMessage(quint32 id) {
    mms::message_store& ms = m_wallet2->get_message_store();
    mms::message m;
    bool valid_id = ms.get_message_by_id(id, m);
    if (!valid_id) {
        return "";
    }

    return m.content;
}

void MultisigMessageStore::receiveMessages()
{
    // Beware!
    // This function will always be called from the refresh thread

    qDebug() << "receiveMessages";

    std::vector<mms::message> new_messages;
    mms::message_store& ms = m_wallet2->get_message_store();

    ms.register_user();

    if (!ms.signer_keys_complete()) {
      uint32_t users;
      ms.get_channel_users(m_wallet2->get_multisig_wallet_state(), users);
      statusChanged(QString("Waiting for cosigners (%1/%2)").arg(QString::number(users), QString::number(ms.get_num_authorized_signers())), false);
      return;
    }

    bool avail;
    try {
        avail = ms.check_for_messages(m_wallet2->get_multisig_wallet_state(), new_messages);
    }
    catch (std::exception &e) {
        // TODO: set status
        return;
    }

    for (const auto &msg : new_messages) {
      if (msg.type == mms::message_type::multisig_sync_data) {
        this->processSyncData();
        break;
      }
    }

    // Send any messages that couldn't be sent due to connection issues
    this->sendReadyMessages();

    if (avail) {
      this->refresh();
    }
}

void MultisigMessageStore::refresh(bool next)
{
    qDebug() << "MultisigMessageStore::refresh";

    QReadLocker locker(&m_lock);

    emit refreshStarted();

    bool haveWaiting = false;

    {
        clearRows();
        mms::message_store& ms = m_wallet2->get_message_store();
        auto messages = ms.get_all_messages();

        for (const auto &message : messages)
        {
            auto msg = new MultisigMessage(this);
            msg->id = message.id;
            msg->type = QString::fromStdString(mms::message_store::message_type_to_string(message.type));
            msg->direction = QString::fromStdString(mms::message_store::message_direction_to_string(message.direction));
            msg->content = message.content;
            msg->created = QDateTime::fromSecsSinceEpoch(message.created);
            msg->modified = QDateTime::fromSecsSinceEpoch(message.modified);
            msg->sent = QDateTime::fromSecsSinceEpoch(message.sent);
            msg->signer_index = message.signer_index;
            msg->signer = QString::fromStdString(ms.signer_to_string(ms.get_signer(message.signer_index), 100));
            msg->state = QString::fromStdString(mms::message_store::message_state_to_string(message.state));
            msg->hash = QString::fromStdString(epee::string_tools::pod_to_hex(message.hash));
            msg->wallet_height = message.wallet_height;
            msg->round = message.round;
            msg->signature_count = message.signature_count;
            msg->transport_id = QString::fromStdString(message.transport_id);

            if (message.state == mms::message_state::waiting) {
                haveWaiting = true;
            }

            if (message.type == mms::message_type::partially_signed_tx && message.direction == mms::message_direction::in) {
                PendingTransaction * tx = m_wallet->restoreMultisigTransaction(message.content);

                QString prefixHash = tx->prefixHashes()[0];

                TxProposal *txProposal;
//                if (m_txProposalsIndex.contains(prefixHash)) {
//                    txProposal = m_txProposals.value(m_txProposalsIndex[prefixHash]);
//                } else {
//                    txProposal = new TxProposal(this);
//                }

                txProposal = new TxProposal(this);
                txProposal->txCount = tx->txCount();
                txProposal->txId = tx->txid()[0]; // TODO;
                txProposal->prefixHash = tx->prefixHashes()[0];
                txProposal->messageId = message.id;
                txProposal->timestamp = QDateTime::fromSecsSinceEpoch(message.created);

                txProposal->balanceDelta = tx->amount();
                txProposal->numSignatures = tx->signersKeys().size();

//                txProposal->from =

                // This doesn't work, we don't know the final txid yet
//                if (m_wallet->haveTransaction(txProposal->txId)) {
//                    txProposal->status = "Completed";
//                }

                txProposal->status = TxProposal::Status::Pending;

                // Decide if we can sign this transaction

                // Are any of the spent outputs frozen?
                // Do we have multisig_k for all LR values?


                tx->refresh();
                auto txx = tx->transaction(0);

                bool completed = false;
                bool cant_sign = false;
                bool any_frozen = false;
                bool double_spend = false;

                for (const auto& input : txx->inputs()) {
                    txProposal->spendsOutputs.append(input->pubKey().mid(0, 8));

                    crypto::public_key pk;
                    if (!epee::string_tools::hex_to_pod(input->pubKey().toStdString(), pk))
                    {
                        continue;
                    }

                    auto idx = m_wallet2->get_transfer_details(pk); // TODO: inefficient
                    auto td = m_wallet2->get_transfer_details(idx);

//                    if (td.m_multisig_k.size() == 0) {
//                        cant_sign = true;
//                    }
//
//                    for (const auto &k : td.m_multisig_k) { // weak check, we also need to check LR
//                        if (k == rct::zero()) {
//                            cant_sign = true;
//                            break;
//                        }
//                    }

                    if (td.m_spent) {
                        double_spend = true;
                    }

                    if (td.m_frozen) {
                        any_frozen = true;
                    }
                }


                crypto::hash prefix_hash;
                if (!epee::string_tools::hex_to_pod(txProposal->prefixHash.toStdString(), prefix_hash))
                {
                    continue;
                }

                completed = m_wallet2->have_tx_prefix(prefix_hash);

                if (tx->haveWeSigned()) {
                    txProposal->status = TxProposal::Status::Signed;
                }
                else if (!tx->canSign()) {
                    txProposal->status = TxProposal::Status::Cant_Sign;
                }
                else if (any_frozen) {
                    txProposal->status = TxProposal::Status::Frozen;
                }
                else if (double_spend) {
                    txProposal->status = TxProposal::Status::Double_Spend;
                }

                if (completed) {
                    txProposal->status = TxProposal::Status::Completed;
                }

                m_txProposals.push_back(txProposal);
                m_txProposalsIndex[prefixHash] = m_txProposals.length() - 1;
            }

            if (message.type == mms::message_type::partially_signed_tx && message.direction == mms::message_direction::out) {

            }

            m_rows.push_back(msg);
        }
    }

    if (haveWaiting && next) {
        this->next();
    }

    emit refreshFinished();
}

bool MultisigMessageStore::processSyncData() {
    qDebug() << tr("import_multisig_info");
    QWriteLocker locker(&lock);

    mms::message_store& ms = m_wallet2->get_message_store();

    std::vector<uint32_t> messages;
    ms.process_sync_data(messages);

    std::vector<cryptonote::blobdata> infos;
    for (size_t i = 0; i < messages.size(); ++i)
    {
        if (messages[i] == 0) {
            continue;
        }

        mms::message m = ms.get_message_by_id(messages[i]);
        infos.push_back(m.content);
    }

    bool success = importMultisig(infos);

    this->refresh();

    return success;
}

bool MultisigMessageStore::sendPendingTransaction(quint32 id, quint32 cosigner) {
    qDebug() << tr("sendPendingTransaction");

    mms::message_store& ms = m_wallet2->get_message_store();
    mms::message m = ms.get_message_by_id(id);

    if (cosigner == 0) {
        // Send to all cosigners except me
        for (int i = 1; i < m_wallet2->get_multisig_signers(); i++) {
            ms.add_message(m_wallet2->get_multisig_wallet_state(), i, m.type, mms::message_direction::out, m.content);
        }
    } else {
        ms.add_message(m_wallet2->get_multisig_wallet_state(), cosigner, m.type, mms::message_direction::out, m.content);
    }

    sendReadyMessages();
    this->refresh();
    return true;
}

void MultisigMessageStore::next(bool forceSync, bool calledFromRefresh)
{
    qDebug() << "MultisigMessageStore::next";

    mms::message_store& ms = m_wallet2->get_message_store();

    bool avail = false;
    std::vector<mms::processing_data> data_list; // List of processable message ids
    uint32_t choice = 0;
    {
        std::string wait_reason;
        {
            avail = ms.get_processable_messages(m_wallet2->get_multisig_wallet_state(), forceSync, data_list, wait_reason);
        }
        if (!wait_reason.empty())
        {
            QString waitReason = QString::fromStdString(wait_reason);
            if (!waitReason.contains("Wallet can't")) {
                emit statusChanged(QString::fromStdString(wait_reason), false);
            }

            qDebug() << "No next step: " << wait_reason;
        }
    }

    if (avail)
    {
        qDebug() << "Processing available messages";

        mms::processing_data data = data_list[choice];
        bool command_successful = false;
        switch(data.processing)
        {
            case mms::message_processing::add_auto_config_data: {
                qDebug() << "Add signer config data";
//                ms.add_auto_config_data_messages(m_wallet2->get_multisig_wallet_state());
                command_successful = true;
                break;
            }

            case mms::message_processing::process_auto_config_data:
            {
                qDebug() << tr("Process auto config data");
                size_t num_auto_config_data = data.message_ids.size();

//                emit statusChanged(QString("Waiting for signer info (%1/%2)").arg(QString::number(data.message_ids.size()+1), QString::number(ms.get_num_authorized_signers())), false);
//                if (num_auto_config_data < (ms.get_num_authorized_signers()-1)) {
//                    break;
//                }

                for (size_t i = 0; i < num_auto_config_data; ++i) {
                  ms.process_auto_config_data_message(data.message_ids[i]);
                }

                ms.stop_auto_config();
                command_successful = prepareMultisig();
                emit signersUpdated();
                break;
            }

            case mms::message_processing::prepare_multisig:
                qDebug() << tr("prepare_multisig");
                command_successful = prepareMultisig();
                break;

            case mms::message_processing::make_multisig:
            {
                qDebug() << tr("make_multisig");
                size_t number_of_key_sets = data.message_ids.size();
                std::vector<std::string> sig_args(number_of_key_sets);
                for (size_t i = 0; i < number_of_key_sets; ++i)
                {
                    mms::message m = ms.get_message_by_id(data.message_ids[i]);
                    sig_args[i] = m.content;
                }
                command_successful = makeMultisig(ms.get_num_required_signers(), sig_args);
                break;
            }

            case mms::message_processing::exchange_multisig_keys:
            {
                qDebug() << tr("exchange_multisig_keys");
                size_t number_of_key_sets = data.message_ids.size();
                // Other than "make_multisig" only the key sets as parameters, no num_required_signers
                std::vector<std::string> sig_args(number_of_key_sets);
                for (size_t i = 0; i < number_of_key_sets; ++i)
                {
                    mms::message m = ms.get_message_by_id(data.message_ids[i]);
                    sig_args[i] = m.content;
                }
                // todo: update mms to enable 'key exchange force updating'
                command_successful = exchangeMultisig(sig_args);
                break;
            }

            case mms::message_processing::create_sync_data:
            {
//                qDebug() << tr("export_multisig_info");
//                command_successful = exportMultisig();
                break;
            }

            case mms::message_processing::process_sync_data:
            {
//                qDebug() << tr("import_multisig_info");
//                std::vector<cryptonote::blobdata> infos;
//                for (size_t i = 0; i < data.message_ids.size(); ++i)
//                {
//                    mms::message m = ms.get_message_by_id(data.message_ids[i]);
//                    infos.push_back(m.content);
//                }
//                command_successful = importMultisig(infos);
                break;
            }

            case mms::message_processing::sign_tx:
            {
                qDebug() << tr("sign_multisig");
//                mms::message m = ms.get_message_by_id(data.message_ids[0]);
//                command_successful = signMultisigTx(m.content);
                break;
            }

            case mms::message_processing::submit_tx:
            {
                qDebug() << tr("submit_multisig");
//                mms::message m = ms.get_message_by_id(data.message_ids[0]);
//                command_successful = submitMultisigTx(m.content);
                break;
            }

            case mms::message_processing::process_signer_config:
            {
                qDebug() << tr("Process signer config");
//                LOCK_IDLE_SCOPE();
//                mms::message m = ms.get_message_by_id(data.message_ids[0]);
//                mms::authorized_signer me = ms.get_signer(0);
//                mms::multisig_wallet_state state = get_multisig_wallet_state();
//                if (!me.auto_config_running)
//                {
//                    // If no auto-config is running, the config sent may be unsolicited or problematic
//                    // so show what arrived and ask for confirmation before taking it in
//                    std::vector<mms::authorized_signer> signers;
//                    ms.unpack_signer_config(state, m.content, signers);
//                    list_signers(signers);
//                    if (!user_confirms(tr("Replace current signer config with the one displayed above?")))
//                    {
//                        break;
//                    }
//                    if (!user_confirms_auto_config())
//                    {
//                        message_writer() << tr("You can use the \"mms delete\" command to delete any unwanted message");
//                        break;
//                    }
//                }
//                ms.process_signer_config(state, m.content);
//                ms.stop_auto_config();
//                list_signers(ms.get_all_signers());
//                command_successful = true;
                break;
            }

            default:
                qDebug() << tr("Nothing ready to process");
                break;
        }

        if (command_successful)
        {
            {
//                LOCK_IDLE_SCOPE();
               ms.set_messages_processed(data);
               sendReadyMessages();
               this->refresh();
            }
        }
    }
}

void MultisigMessageStore::sendReadyMessages() {
    qDebug() << "sendReadyMessages";
    mms::message_store& ms = m_wallet2->get_message_store();
    std::vector<mms::message> ready_messages;
    const std::vector<mms::message> &messages = ms.get_all_messages();
    for (size_t i = 0; i < messages.size(); ++i)
    {
        const mms::message &m = messages[i];
        if (m.state == mms::message_state::ready_to_send)
        {
            ready_messages.push_back(m);
        }
    }

    mms::multisig_wallet_state state = m_wallet2->get_multisig_wallet_state();
    for (size_t i = 0; i < ready_messages.size(); ++i)
    {
        try {
            ms.send_message(state, ready_messages[i].id);
        }
        catch (std::exception &e) {
            emit connectionError();
            qWarning() << "Unable to send MMS message";
            return;
        }

        ms.set_message_processed_or_sent(ready_messages[i].id);
    }
    qDebug() << "Queued for sending";
}

bool MultisigMessageStore::prepareMultisig() {
    std::string multisig_info = m_wallet2->get_multisig_first_kex_msg();
    mms::message_store& ms = m_wallet2->get_message_store();
    std::vector<uint32_t> message_ids;
    ms.process_wallet_created_data(m_wallet2->get_multisig_wallet_state(), mms::message_type::key_set, multisig_info, message_ids);

    uint32_t rounds = multisig::multisig_setup_rounds_required(ms.get_num_authorized_signers(), ms.get_num_required_signers());
    emit statusChanged(QString("Exchanging keys (1/%1)").arg(QString::number(rounds)), false);
    return true;
}

QString MultisigMessageStore::errorString() {
    return m_errorString;
}

void MultisigMessageStore::setErrorString(const QString &errorString) {
    m_errorString = errorString;
}

void MultisigMessageStore::clearStatus() {
    m_errorString = "";
}

void MultisigMessageStore::setServiceDetails(const QString &serviceUrl, const QString &serviceLogin) {
    // TODO: make sure we respect "only allow connections to .onion services"

    mms::message_store& ms = m_wallet2->get_message_store();
    ms.set_service_details(serviceUrl.toStdString(), serviceLogin.toStdString());
}

bool MultisigMessageStore::registerChannel(QString &channel, quint32 user_limit) {
    mms::message_store& ms = m_wallet2->get_message_store();
    bool success;

    clearStatus();

    try {
        std::string channel_std;
        success = ms.register_channel(channel_std, user_limit);
        channel = QString::fromStdString(channel_std);
    }
    catch (const std::exception &e) {
        this->setErrorString(e.what());
        return false;
    }

    return success;
}

bool MultisigMessageStore::makeMultisig(quint32 threshold, const std::vector<std::string> &kexMessages) {
    try
    {
        std::string multisig_extra_info = m_wallet2->make_multisig("", kexMessages, threshold);
        bool ready;
        m_wallet2->multisig(&ready);
        if (!ready)
        {
            std::vector<uint32_t> message_ids;
            m_wallet2->get_message_store().process_wallet_created_data(m_wallet2->get_multisig_wallet_state(), mms::message_type::additional_key_set, multisig_extra_info, message_ids);
            emit statusChanged(QString("Exchanging keys (2/%2)").arg(QString::number(m_wallet2->get_multisig_setup_rounds_required())), false);
            return true;
        }
    }
    catch (const std::exception &e)
    {
        qDebug() << tr("Error creating multisig: ") << e.what();
        return false;
    }

    uint32_t total;
    if (!m_wallet2->multisig(NULL, &threshold, &total))
    {
        qDebug() << tr("Error creating multisig: new wallet is not multisig");
        return false;
    }
    qDebug() << std::to_string(threshold) << "/" << total << tr(" multisig address: ")
                         << m_wallet2->get_account().get_public_address_str(m_wallet2->nettype());


    return true;
}

bool MultisigMessageStore::exchangeMultisig(const std::vector<std::string> &kexMessages) {
    try
    {
        std::string multisig_extra_info = m_wallet2->exchange_multisig_keys("", kexMessages, false);
        bool ready;
        m_wallet2->multisig(&ready);
        if (!ready)
        {
            auto state = m_wallet2->get_multisig_wallet_state();
            emit statusChanged(QString("Exchanging keys (%1/%2)").arg(QString::number(state.multisig_rounds_passed + 1), QString::number(m_wallet2->get_multisig_setup_rounds_required())), false);

            std::vector<uint32_t> message_ids;
            m_wallet2->get_message_store().process_wallet_created_data(m_wallet2->get_multisig_wallet_state(), mms::message_type::additional_key_set, multisig_extra_info, message_ids);

            return true;
        } else {
            uint32_t threshold, total;
            m_wallet2->multisig(NULL, &threshold, &total);
            emit statusChanged("Multisig wallet has been created", true);
            emit multisigWalletCreated(QString::fromStdString(m_wallet2->get_account().get_public_address_str(m_wallet2->nettype())));
            qDebug() << tr("Multisig wallet has been successfully created. Current wallet type: ") << threshold << "/" << total;
            qDebug() << tr("Multisig address: ") << m_wallet2->get_account().get_public_address_str(m_wallet2->nettype());
        }
    }
    catch (const std::exception &e)
    {
        qDebug() << tr("Failed to perform multisig keys exchange: ") << e.what();
        return false;
    }

    return true;
}

bool MultisigMessageStore::exportMultisig() {
    bool ready;
    if (!m_wallet2->multisig(&ready)) {
        qWarning() << "This wallet is not multisig";
        return false;
    }
    if (!ready) {
        qWarning() << "This multisig wallet is not yet finalized";
        return false;
    }

    qDebug() << "exportMultisig: current thread: " << QThread::currentThread();

    QWriteLocker locker(&lock);

    // Calling this function will reset any nonces recorded by the previous call to this function. Doing so will
    // invalidate any in-progress signing attempts that rely on the previous output of this function.

    // This function should be called if:
    // - There are any transfer details with m_key_image_known == false

    // std::vector<multisig_info> info; (one multisig_info per owned output)
    //
    // multisig_info:
    //   crypto::public_key m_signer = get_multisig_signer_public_key();  (our public spend key)
    //   std::vector<LR> m_LR;                                            (nlr * 2 times)
    //   std::vector<crypto::key_image> m_partial_key_images;             (one for each m_multisig_keys)
    //
    // LR:
    //     rct::key m_L;             = k * G                              (nonce public key)
    //     rct::key m_R;             = k * H(output public key)           (nonce key image)


    // The following state is affected:
    //
    // transfer_details: (m_transfers)
    //  std::vector<rct::key> m_multisig_k;                               (nlr * 2 nonces (k))

    try
    {
        cryptonote::blobdata ciphertext = m_wallet2->export_multisig();
        std::vector<uint32_t> message_ids;
        m_wallet2->get_message_store().process_wallet_created_data(m_wallet2->get_multisig_wallet_state(), mms::message_type::multisig_sync_data, ciphertext, message_ids);
    }
    catch (const std::exception &e)
    {
        qWarning() << tr("Error exporting multisig info: ") << e.what();
        return false;
    }

    sendReadyMessages();
    this->refresh();

    emit multisigInfoExported();

    qDebug() << "Multisig info exported to MMS";
    return true;
}

bool MultisigMessageStore::importMultisig(const std::vector<cryptonote::blobdata> info) {
    bool ready;
    uint32_t threshold, total;
    if (!m_wallet2->multisig(&ready, &threshold, &total)) {
        qWarning() << "This wallet is not multisig";
        return false;
    }
    if (!ready) {
        qWarning() << "This multisig wallet is not yet finalized";
        return false;
    }

    qDebug() << "importMultisig: current thread: " << QThread::currentThread();

    // The following state is affected:
    //
    // transfer_details: (m_transfers)
    //   std::vector<multisig_info> m_multisig_info;   (one multisig_info per other participant)
    //   crypto::key_image m_key_image;                (calculated using m_partial_key_images in m_multisig_info)
    //   bool m_key_image_known = true;
    //   bool m_key_image_request = false;
    //   bool m_key_image_partial = false;

    try
    {
        // TODO: need to be synced here;
        size_t n_outputs = m_wallet2->import_multisig(info);
    }
    catch (const std::exception &e)
    {
        qWarning() << tr("Failed to import multisig info: ") << e.what();
        return true;
    }

    try
    {
        m_wallet2->rescan_spent();
    }
    catch (const std::exception &e)
    {
        qWarning() << tr("Failed to update spent status after importing multisig info: ") << e.what();
        return false;
    }

    emit multisigInfoImported();

    return true;
}

bool MultisigMessageStore::signMultisigTx(const cryptonote::blobdata &data) {
    PendingTransaction * tx = m_wallet->restoreMultisigTransaction(data);
    emit askToSign(tx);

//    std::vector<crypto::hash> txids;
//    uint32_t signers = 0;
//    try
//    {
//        tools::wallet2::multisig_tx_set exported_txs;
//        std::string ciphertext;
//        bool r = m_wallet2->load_multisig_tx(data, exported_txs, [&](const tools::wallet2::multisig_tx_set &tx){
//            signers = tx.m_signers.size();
//            return true;
//        });
//        if (r)
//        {
//            r = m_wallet2->sign_multisig_tx(exported_txs, txids);
//        }
//        if (r)
//        {
//            ciphertext = m_wallet2->save_multisig_tx(exported_txs);
//            if (ciphertext.empty())
//            {
//                r = false;
//            }
//        }
//        if (r)
//        {
//            mms::message_type message_type = mms::message_type::fully_signed_tx;
//            if (txids.empty())
//            {
//                message_type = mms::message_type::partially_signed_tx;
//            }
//            m_wallet2->get_message_store().process_wallet_created_data(m_wallet2->get_multisig_wallet_state(), message_type, ciphertext);
////            filename = "MMS";   // for the messages below
//        }
//        else
//        {
//            qWarning() << tr("Failed to sign multisig transaction");
//            return false;
//        }
//    }ye
//    catch (const tools::error::multisig_export_needed& e)
//    {
//        qWarning() << tr("Multisig error: ") << e.what();
//        return false;
//    }
//    catch (const std::exception &e)
//    {
//        qWarning() << tr("Failed to sign multisig transaction: ") << e.what();
//        return false;
//    }

//    if (txids.empty())
//    {
//        uint32_t threshold;
//        m_wallet2->multisig(NULL, &threshold);
//        uint32_t signers_needed = threshold - signers - 1;
//        success_msg_writer(true) << tr("Transaction successfully signed to file ") << filename << ", "
//                                 << signers_needed << " more signer(s) needed";
//        return true;
//    }
//    else
//    {
//        std::string txids_as_text;
//        for (const auto &txid: txids)
//        {
//            if (!txids_as_text.empty())
//                txids_as_text += (", ");
//            txids_as_text += epee::string_tools::pod_to_hex(txid);
//        }
//        success_msg_writer(true) << tr("Transaction successfully signed to file ") << filename << ", txid " << txids_as_text;
//        success_msg_writer(true) << tr("It may be relayed to the network with submit_multisig");
//    }
    return true;
}

bool MultisigMessageStore::submitMultisigTx(const cryptonote::blobdata &data) {
    bool ready;
    uint32_t threshold;
    if (!m_wallet2->multisig(&ready, &threshold))
    {
        qDebug() << tr("This is not a multisig wallet");
        return false;
    }

    try
    {
        tools::wallet2::multisig_tx_set txs;

        bool r = m_wallet2->load_multisig_tx(data, txs, [&](const tools::wallet2::multisig_tx_set &tx){ return true; });
        if (!r)
        {
            qDebug() << tr("Failed to load multisig transaction from MMS");
            return false;
        }


        if (txs.m_signers.size() < threshold)
        {
            qDebug() << QString("Multisig transaction signed by only %1 signers, needs %2 more signatures").arg(QString::number(txs.m_signers.size()), QString::number(threshold - txs.m_signers.size()));
            return false;
        }

        // actually commit the transactions
        for (auto &ptx: txs.m_ptx)
        {
            m_wallet2->commit_tx(ptx);
            qDebug() << tr("Transaction successfully submitted, transaction ");
        }
    }
    catch (const std::exception &e)
    {
        qWarning() << "Something terrible happened";
//        handle_transfer_exception(std::current_exception(), m_wallet2->is_trusted_daemon());
    }
    catch (...)
    {
        LOG_ERROR("unknown error");
        qDebug() << tr("unknown error");
        return false;
    }

    return true;
}

bool MultisigMessageStore::havePartiallySignedTxWaiting() {
    for (const auto & m : m_rows) {
        if (m->type == "partially signed tx" && m->state == "waiting") {
            return true;
        }
    }
    return false;
}

QString MultisigMessageStore::createSetupKey(quint32 threshold, quint32 signers, const QString &service, const QString &channel, SetupMode mode) {
    mms::message_store& ms = m_wallet2->get_message_store();

    QString setupKey;
    try {
        std::string key = ms.create_setup_key(threshold, signers, service.toStdString(), channel.toStdString(), static_cast<mms::setup_mode>(mode));
        setupKey = QString::fromStdString(key);
    }
    catch (const std::exception &e) {
        return "";
    }

    return setupKey;
}

bool MultisigMessageStore::checkSetupKey(const QString &setupKeyStr, SetupKey &setupKey) {
    mms::message_store& ms = m_wallet2->get_message_store();

    std::string adjusted_token;
    mms::setup_key key;
    bool tokenValid = ms.check_auto_config_token(setupKeyStr.toStdString(), key);

    if (tokenValid) {
      setupKey.threshold = key.threshold;
      setupKey.participants = key.participants;
      setupKey.service = QString::fromStdString(key.service_url);
      setupKey.mode = static_cast<SetupMode>(key.mode);
    }

    return tokenValid;
}

void MultisigMessageStore::init(const QString &setupKey, const QString &ownLabel) {
    mms::message_store& ms = m_wallet2->get_message_store();
    ms.init_from_setup_key(m_wallet2->get_multisig_wallet_state(), setupKey.toStdString(), ownLabel.toStdString());
}

void MultisigMessageStore::setSigner(quint32 index, const QString& label, const QString& address) {
    mms::message_store& ms = m_wallet2->get_message_store();

    // TODO: do sanity checks

    cryptonote::address_parse_info info;
    if (!get_account_address_from_str(info, static_cast<cryptonote::network_type>(m_wallet2->nettype()), address.toStdString())) {
        return;
    }

    ms.set_signer(m_wallet2->get_multisig_wallet_state(), index, label.toStdString(), {});
}

QVector<MultisigMessageStore::SignerInfo> MultisigMessageStore::getSignerInfo() {
    QWriteLocker locker(&lock);
    mms::message_store& ms = m_wallet2->get_message_store();

    QVector<SignerInfo> signerInfo;

    for (size_t i = 0; i < ms.get_num_authorized_signers(); i++) {
        mms::authorized_signer signer = ms.get_signer(i);

        if (!signer.public_key_known) {
            continue;
        }

        SignerInfo info;
        info.label = QString::fromStdString(signer.label);
        info.publicKey = QString::fromStdString(epee::string_tools::pod_to_hex(signer.public_key));
        signerInfo.push_back(info);
    }

    return signerInfo;
}

QString MultisigMessageStore::getRecoveryInfo() {
    QWriteLocker locker(&lock);
    mms::message_store& ms = m_wallet2->get_message_store();

    std::string info = ms.get_recovery_info(m_wallet2->get_multisig_wallet_state(), m_wallet2->get_refresh_from_block_height());
    return QString::fromStdString(info);
}

quint64 MultisigMessageStore::count() const
{
    QReadLocker locker(&m_lock);

    return m_rows.length();
}

quint64 MultisigMessageStore::txProposalCount() const {
    QReadLocker locker(&m_lock);

    return m_txProposals.size();
}


void MultisigMessageStore::clearRows() {
    qDeleteAll(m_rows);
    m_rows.clear();

    qDeleteAll(m_txProposals);
    m_txProposals.clear();
}

MultisigMessageStore::SignerInfo MultisigMessageStore::getSignerInfo(quint32 index) {
    SignerInfo info;
    mms::message_store& ms = m_wallet2->get_message_store();
    mms::authorized_signer signer = ms.get_signer(index);

    info.label = QString::fromStdString(signer.label);
    info.publicKey = QString::fromStdString(epee::string_tools::pod_to_hex(signer.public_key));

    return info;
}
