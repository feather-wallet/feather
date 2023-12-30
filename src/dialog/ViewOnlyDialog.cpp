// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "ViewOnlyDialog.h"
#include "ui_ViewOnlyDialog.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

#include "URDialog.h"
#include "utils/Utils.h"
#include "WalletManager.h"
#include "qrcode/QrCode.h"
#include "dialog/PasswordSetDialog.h"
#include "dialog/QrCodeDialog.h"

ViewOnlyDialog::ViewOnlyDialog(Wallet *wallet, QWidget *parent)
    : WindowModalDialog(parent)
    , ui(new Ui::ViewOnlyDialog)
    , m_wallet(wallet)
{
    ui->setupUi(this);

    ui->label_restoreHeight->setText(QString::number(m_wallet->getWalletCreationHeight()));
    ui->label_primaryAddress->setText(m_wallet->address(0, 0));
    ui->label_secretViewKey->setText(m_wallet->getSecretViewKey());

    connect(ui->btn_Copy, &QPushButton::clicked, this, &ViewOnlyDialog::copyToClipboard);
    connect(ui->btn_Save, &QPushButton::clicked, this, &ViewOnlyDialog::onWriteViewOnlyWallet);
    connect(ui->btn_transmitOverUR, &QPushButton::clicked, [this] {
        QrCode qr(this->toJsonString(), QrCode::Version::AUTO, QrCode::ErrorCorrectionLevel::HIGH);
        QrCodeDialog dialog{this, &qr, "View-Only details"};
        dialog.exec();
    });

    if (m_wallet->viewOnly()) {
        ui->btn_Save->setEnabled(false);
        ui->btn_Save->setToolTip("Wallet is already view-only");
    }

    this->adjustSize();
}

void ViewOnlyDialog::onWriteViewOnlyWallet(){
    QDir walletDir = QDir(Utils::defaultWalletDir());
    QString fn = QFileDialog::getSaveFileName(this, "Save .keys wallet file", walletDir.filePath(QString("%1_view_only").arg(m_wallet->walletName())), "Monero wallet (*.keys)");
    if (fn.isEmpty()) {
        return;
    }
    if (!fn.endsWith(".keys")) {
        fn += ".keys";
    }

    PasswordSetDialog dialog("Set a password for the view-only wallet", this);
    dialog.exec();
    QString password = dialog.password();

    m_wallet->createViewOnly(fn, password);

    QMessageBox::information(this, "Information", "View-only wallet successfully written to disk.");
}

QString ViewOnlyDialog::toString() {
    QString text;
    text += QString("Secret view key: %1\n").arg(ui->label_secretViewKey->text());
    text += QString("Address: %1\n").arg(ui->label_primaryAddress->text());
    text += QString("Restore height: %1\n").arg(ui->label_restoreHeight->text());
    text += QString("Wallet name: %1\n").arg(m_wallet->walletName());
    return text;
}

QString ViewOnlyDialog::toJsonString() {
    QVariantMap data;
    data["version"] = 0,
    data["primaryAddress"] = m_wallet->address(0, 0);
    data["privateViewKey"] = m_wallet->getSecretViewKey();
    data["restoreHeight"] = m_wallet->getWalletCreationHeight();
    data["walletName"] = m_wallet->walletName();

    auto obj = QJsonDocument::fromVariant(data);
    return obj.toJson(QJsonDocument::Compact);
}

void ViewOnlyDialog::copyToClipboard() {
    Utils::copyToClipboard(this->toString());
}

ViewOnlyDialog::~ViewOnlyDialog() = default;
