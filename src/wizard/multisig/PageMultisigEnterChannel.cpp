// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PageMultisigEnterChannel.h"
#include "ui_PageMultisigEnterChannel.h"

#include <QFileDialog>

#include "utils/Icons.h"
#include "utils/Utils.h"

#include "ringct/rctOps.h"
#include "string_tools.h"
#include "libwalletqt/MultisigMessageStore.h"

PageMultisigEnterChannel::PageMultisigEnterChannel(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageMultisigEnterChannel)
        , m_fields(fields)
{
    ui->setupUi(this);
    this->setTitle("Create setup key (2/3)");

    ui->infoFrame->setInfo(icons()->icon("mail.png"), "Enter the URL for the messaging service that all participants will use.\n\nMake sure everyone agrees to use this service.");

    connect(ui->check_authRequired, &QCheckBox::toggled, [this](bool checked){
      ui->frame_auth->setVisible(checked);
    });

    connect(ui->line_service, &QLineEdit::textChanged, [this] {
        m_channelRegistered = false;
    });

    connect(ui->radio_no, &QRadioButton::toggled, [this] {
        completeChanged();
    });
    connect(ui->radio_yes, &QRadioButton::toggled, [this] {
        completeChanged();
    });
}

void PageMultisigEnterChannel::registerChannel() {
    QString serviceUrl = ui->line_service->text();
    QString serviceLogin = ui->check_authRequired->isChecked() ? ui->line_apiKey->text() : "";

    m_fields->wallet->mmsStore()->setServiceDetails(serviceUrl, serviceLogin);

    // TODO: make async
    QString channel;
    bool success = m_fields->wallet->mmsStore()->registerChannel(channel, m_fields->multisigSigners);
    if (success) {
        m_fields->multisigChannel = channel;
        m_channelRegistered = true;
        completeChanged();
    } else {
        QString errorString = m_fields->wallet->mmsStore()->errorString();

        if (errorString.contains("authentication")) {
            ui->check_authRequired->setChecked(true);
        }

        Utils::showError(this, "Unable to register channel", m_fields->wallet->mmsStore()->errorString());
    }
}

void PageMultisigEnterChannel::initializePage() {
    ui->frame_auth->hide();
    ui->frame_confirm->hide();
    ui->check_authRequired->setChecked(false);

    if (!m_fields->multisigInitiator) {
        this->setTitle("Messaging service");
        ui->check_authRequired->hide();
        ui->line_service->setText(m_fields->multisigService);
        ui->line_service->setReadOnly(true);
        ui->infoFrame->setText("The initiator has chosen the following messaging service.");
        ui->frame_confirm->show();

        m_channelRegistered = true;
        completeChanged();
    }
}

int PageMultisigEnterChannel::nextId() const {
    if (m_fields->mode == WizardMode::CreateMultisig) {
        if (m_fields->multisigInitiator) {
          return WalletWizard::Page_MultisigSignerConfig;
        }
        else {
          return WalletWizard::Page_MultisigEnterName;
        }
    }
    if (m_fields->mode == WizardMode::RestoreMultisig) {
        return WalletWizard::Page_SetRestoreHeight;
    }

    return -1;
}

bool PageMultisigEnterChannel::validatePage() {
    if (!m_channelRegistered) {
        this->registerChannel();
    }

    if (!m_channelRegistered) {
        return false;
    }

    m_fields->multisigService = ui->line_service->text();

    return true;
}

bool PageMultisigEnterChannel::isComplete() const {
    if (!m_fields->multisigInitiator) {
        return ui->radio_yes->isChecked();
    }

    return true;
}

