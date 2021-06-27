// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "TorInfoDialog.h"
#include "ui_TorInfoDialog.h"

#include <QPushButton>
#include <QDesktopServices>
#include <QMessageBox>
#include <QInputDialog>

#include "utils/TorManager.h"
#include "utils/os/tails.h"
#include "utils/Icons.h"

TorInfoDialog::TorInfoDialog(QSharedPointer<AppContext> ctx, QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::TorInfoDialog)
        , m_ctx(std::move(ctx))
{
    ui->setupUi(this);

    if (!torManager()->torConnected && !torManager()->errorMsg.isEmpty()) {
        ui->message->setText(torManager()->errorMsg);
    } else {
        ui->message->hide();
    }

    if (torManager()->isLocalTor()) {
        ui->frame_logs->setHidden(true);
    } else {
        ui->logs->setPlainText(torManager()->torLogs);
    }

    initConnectionSettings();
    initPrivacyLevel();
    onConnectionStatusChanged(torManager()->torConnected);

    connect(torManager(), &TorManager::connectionStateChanged, this, &TorInfoDialog::onConnectionStatusChanged);
    connect(torManager(), &TorManager::logsUpdated, this, &TorInfoDialog::onLogsUpdated);

    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &TorInfoDialog::onApplySettings);

    connect(ui->line_host, &QLineEdit::textEdited, this, &TorInfoDialog::onSettingsChanged);
    connect(ui->line_port, &QLineEdit::textEdited, this, &TorInfoDialog::onSettingsChanged);
    connect(ui->check_useLocalTor, &QCheckBox::stateChanged, this, &TorInfoDialog::onSettingsChanged);
    connect(ui->btnGroup_privacyLevel, &QButtonGroup::idToggled, this, &TorInfoDialog::onSettingsChanged);

    ui->label_changes->hide();

    ui->btn_configureInitSync->setIcon(icons()->icon("preferences.svg"));
    connect(ui->btn_configureInitSync, &QPushButton::clicked, this, &TorInfoDialog::onShowInitSyncConfigDialog);

#ifndef HAS_TOR_BIN
    ui->check_useLocalTor->setChecked(true);
    ui->check_useLocalTor->setEnabled(false);
    ui->check_useLocalTor->setToolTip("Feather was bundled without Tor");
#endif

    this->adjustSize();
}

void TorInfoDialog::onLogsUpdated() {
    ui->logs->setPlainText(torManager()->torLogs);
}

void TorInfoDialog::onConnectionStatusChanged(bool connected) {
    if (connected) {
        ui->icon_connectionStatus->setPixmap(QPixmap(":/assets/images/status_connected.png").scaledToWidth(16, Qt::SmoothTransformation));
        ui->label_testConnectionStatus->setText("Connected");
    } else {
        ui->icon_connectionStatus->setPixmap(QPixmap(":/assets/images/status_disconnected.png").scaledToWidth(16, Qt::SmoothTransformation));
        ui->label_testConnectionStatus->setText("Disconnected");
    }
}

void TorInfoDialog::onApplySettings() {
    config()->set(Config::socks5Host, ui->line_host->text());
    config()->set(Config::socks5Port, ui->line_port->text());

    int id = ui->btnGroup_privacyLevel->checkedId();
    config()->set(Config::torPrivacyLevel, id);

    ui->label_changes->hide();

    bool useLocalTor = ui->check_useLocalTor->isChecked();
    if (config()->get(Config::useLocalTor).toBool() && useLocalTor && torManager()->isStarted()) {
        QMessageBox::warning(this, "Warning", "Feather is running the bundled Tor daemon, "
                                              "but the option to never start a bundled Tor daemon was selected. "
                                              "A restart is required to apply the setting.");
    }
    config()->set(Config::useLocalTor, useLocalTor);

    ui->icon_connectionStatus->setPixmap(QPixmap(":/assets/images/status_lagging.png").scaledToWidth(16, Qt::SmoothTransformation));
    ui->label_testConnectionStatus->setText("Connecting");

    emit torSettingsChanged();
}

void TorInfoDialog::onSettingsChanged() {
    ui->label_changes->show();
}

void TorInfoDialog::initConnectionSettings() {
    bool localTor = torManager()->isLocalTor();
    ui->label_connectionSettingsMessage->setVisible(!localTor);
    ui->frame_connectionSettings->setVisible(localTor);

    ui->line_host->setText(config()->get(Config::socks5Host).toString());
    ui->line_port->setText(config()->get(Config::socks5Port).toString());

    ui->check_useLocalTor->setChecked(config()->get(Config::useLocalTor).toBool());
}

void TorInfoDialog::initPrivacyLevel() {
    ui->btnGroup_privacyLevel->setId(ui->radio_allTorExceptNode,  Config::allTorExceptNode);
    ui->btnGroup_privacyLevel->setId(ui->radio_allTorExceptInitSync, Config::allTorExceptInitSync);
    ui->btnGroup_privacyLevel->setId(ui->radio_allTor, Config::allTor);

    int privacyLevel = config()->get(Config::torPrivacyLevel).toInt();
    auto button = ui->btnGroup_privacyLevel->button(privacyLevel);
    if (button) {
        button->setChecked(true);
    }

    if (m_ctx->nodes->connection().isLocal()) {
        ui->label_notice->setText("You are connected to a local node. Traffic to node is not routed over Tor.");
    }
    else if (Utils::isTorsocks()) {
        ui->label_notice->setText("Feather was started with torsocks, all traffic is routed over Tor");
    }
    else if (WhonixOS::detect()) {
        ui->label_notice->setText("Feather is running on Whonix, all traffic is routed over Tor");
    }
    else if (TailsOS::detect()) {
        ui->label_notice->setText("Feather is running on Tails, all traffic is routed over Tor");
    }
    else {
        ui->frame_notice->hide();
    }

    QPixmap iconNoTor(":/assets/images/securityLevelStandardWhite.png");
    QPixmap iconNoSync(":/assets/images/securityLevelSaferWhite.png");
    QPixmap iconAllTor(":/assets/images/securityLevelSafestWhite.png");
    ui->icon_noTor->setPixmap(iconNoTor.scaledToHeight(16, Qt::SmoothTransformation));
    ui->icon_noSync->setPixmap(iconNoSync.scaledToHeight(16, Qt::SmoothTransformation));
    ui->icon_allTor->setPixmap(iconAllTor.scaledToHeight(16, Qt::SmoothTransformation));
}

void TorInfoDialog::onStopTor() {
    torManager()->stop();
}

void TorInfoDialog::onShowInitSyncConfigDialog() {

    int threshold = config()->get(Config::initSyncThreshold).toInt();

    bool ok;
    int newThreshold = QInputDialog::getInt(this, "Sync threshold",
                                            "Synchronize over clearnet if wallet is behind more than x blocks: ",
                                            threshold, 0, 10000, 10, &ok);

    if (ok) {
        config()->set(Config::initSyncThreshold, newThreshold);
    }
}

TorInfoDialog::~TorInfoDialog() = default;