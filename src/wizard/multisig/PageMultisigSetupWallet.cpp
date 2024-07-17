// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PageMultisigSetupWallet.h"
#include "ui_PageMultisigSetupWallet.h"

#include "libwalletqt/MultisigMessageStore.h"
#include "utils/Icons.h"
#include "utils/Utils.h"

PageMultisigSetupWallet::PageMultisigSetupWallet(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageMultisigSetupWallet)
        , m_fields(fields)
{
    ui->setupUi(this);
    this->setTitle("Setting up multisig wallet");

    ui->infoFrame->setInfo(icons()->icon("status_waiting.svg"), "Wait for the multisig wallet setup to complete.");

    ui->tree_cosigners->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tree_cosigners->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tree_cosigners->header()->setMinimumSectionSize(100);

    // Don't allow returning to this page.
    this->setCommitPage(true);
}

void PageMultisigSetupWallet::initializePage() {
    this->setStatus("Connecting to messaging service");

    // We have reached the next stage of the setup,
    // set a cache attribute so that when we exit now we return to this page.
    m_fields->wallet->setCacheAttribute("feather.multisig_setup", "configured");

    // Since we can't go back, add a button here to copy the setup key in case the initiator forgot.
    if (!m_fields->multisigInitiator) {
        ui->btn_copySetupKey->hide();
    }

    // Don't show the signer configuration table until we have something to show
    ui->frame_signerConfiguration->hide();

    // Use a fast refresh interval to speed up wallet setup
    m_fields->wallet->setRefreshInterval(2);

    // TODO: move elsewhere?
    m_fields->wallet->mmsStore()->sendReadyMessages();

    // Begin checking for new messages and processing them in the refresh thread
    m_fields->wallet->setMMSRefreshEnabled(true);

    // Usually it's not a good idea to connect in initializePage, because re-entering from the previous page would
    // duplicate the connection. However, we can't go back on this page, so it doesn't matter.
    connect(m_fields->wallet->mmsStore(), &MultisigMessageStore::statusChanged, this, &PageMultisigSetupWallet::setStatus);

    // Use Qt::QueuedConnection to make sure updateSignerConfig is executed in the GUI thread, and not the refresh thread.
    connect(m_fields->wallet->mmsStore(), &MultisigMessageStore::signersUpdated, this, &PageMultisigSetupWallet::updateSignerConfig, Qt::QueuedConnection);

    connect(m_fields->wallet->mmsStore(), &MultisigMessageStore::multisigWalletCreated, this, &PageMultisigSetupWallet::onWalletCreated);
}

void PageMultisigSetupWallet::setStatus(const QString &status, bool finished) {
    ui->label_status->setText(QString("<b>Status</b>: %1").arg(status));

    if (finished) {
        ui->infoFrame->setInfo(icons()->icon("arrow.svg"), "Proceed to the next step");
        m_fields->wallet->setRefreshInterval(10);
    }
}

void PageMultisigSetupWallet::updateSignerConfig() {
    qDebug() << "updateSignerConfig";
    ui->tree_cosigners->clear();

    // Get all signer info
    auto signerInfo = m_fields->wallet->mmsStore()->getSignerInfo();
    if (signerInfo.isEmpty()) {
        qDebug() << "Signer info was empty";
        return;
    }

    ui->frame_signerConfiguration->show();

    if (signerInfo.size() < m_fields->multisigSigners) {
        this->setStatus(QString("Waiting for signer info (%1/%2)").arg(QString::number(signerInfo.size()), QString::number(m_fields->multisigSigners)));
    }

    // Sort signers by label
    std::sort(signerInfo.begin(), signerInfo.end(), [](const MultisigMessageStore::SignerInfo &a, const MultisigMessageStore::SignerInfo &b){
        return a.label < b.label;
    });

    for (const auto &info : signerInfo) {
        auto *sItem = new QTreeWidgetItem(ui->tree_cosigners);
        sItem->setText(0, info.label);
        sItem->setText(1, info.publicKey);
        sItem->setFont(1, Utils::getMonospaceFont());
    }
}

void PageMultisigSetupWallet::onWalletCreated() {
    m_fields->wallet->setCacheAttribute("feather.multisig_setup", "verify");
    m_created = true;
    completeChanged();
}

int PageMultisigSetupWallet::nextId() const {
    return WalletWizard::Page_MultisigVerifyAddress;
}

bool PageMultisigSetupWallet::validatePage() {
    return true;
}

bool PageMultisigSetupWallet::isComplete() const {
    if (!m_created) {
        return false;
    }

    return true;
}
