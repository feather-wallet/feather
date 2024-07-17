// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PageMultisigMMSRecoveryInfo.h"
#include "ui_PageMultisigMMSRecoveryInfo.h"

#include "utils/Icons.h"
#include "utils/Utils.h"

#include "libwalletqt/MultisigMessageStore.h"

PageMultisigMMSRecoveryInfo::PageMultisigMMSRecoveryInfo(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageMultisigMMSRecoveryInfo)
        , m_fields(fields)
{
    ui->setupUi(this);
    this->setTitle("MMS Recovery Info");

    this->setFinalPage(true);
    this->setButtonText(QWizard::FinishButton, "Open wallet");

    ui->infoFrame->setInfo(icons()->icon("key.png"), "You will need this recovery info to reconnect to the messaging service if you need to restore your wallet.\n\nStore it safely alongside your seed.");

    connect(ui->check_saved, &QCheckBox::toggled, [this]{
        this->completeChanged();
    });
}

void PageMultisigMMSRecoveryInfo::initializePage() {
//    QJsonDocument doc;
//    QJsonObject obj;
//    obj["restore_height"] = QString::number(m_fields->wallet->getWalletCreationHeight()).toInt();
//    obj["message_daemon"] = m_fields->multisigService;
//    obj["setup_key"] = m_fields->multisigSetupKey;
//
//    QJsonObject me;
//    me["address"] = m_fields->originalPrimaryAddress;
//    me["viewkey"] = m_fields->originalSecretViewKey;
//    me["label"] = m_fields->multisigUsername;
//
//    QJsonArray signers;
//    signers.append(me);
//
//    for (int i = 1; i < m_fields->wallet->multisigSigners(); i++) {
//        QJsonObject signer;
//        auto info = m_fields->wallet->mmsStore()->getSignerInfo(i);
//
//        signer["address"] = info.address;
//        signer["label"] = info.label;
//
//        signers.append(signer);
//    }
//
//    obj["signers"] = signers;
//    doc.setObject(obj);
//
//    QString recoveryData = QString("MMS_RECOVERY:%1").arg(doc.toJson(QJsonDocument::Compact).toBase64());
    ui->mmsRecoveryInfo->setPlainText(m_fields->wallet->mmsStore()->getRecoveryInfo());
}

int PageMultisigMMSRecoveryInfo::nextId() const {
    return -1;
}

bool PageMultisigMMSRecoveryInfo::validatePage() {
    emit showWallet(m_fields->wallet);
    return true;
}

bool PageMultisigMMSRecoveryInfo::isComplete() const {
    return ui->check_saved->isChecked();
}
