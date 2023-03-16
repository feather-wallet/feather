// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "NetworkProxyWidget.h"
#include "ui_NetworkProxyWidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QWidget>

#include "utils/config.h"
#include "utils/os/Prestium.h"

NetworkProxyWidget::NetworkProxyWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::NetworkProxyWidget)
        , m_torInfoDialog(new TorInfoDialog(this))
{
    ui->setupUi(this);

    ui->comboBox_proxy->setCurrentIndex(config()->get(Config::proxy).toInt());
    connect(ui->comboBox_proxy, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index){
        this->onProxySettingsChanged();
        ui->frame_proxy->setVisible(index != Config::Proxy::None);
        ui->groupBox_proxySettings->setTitle(QString("%1 settings").arg(ui->comboBox_proxy->currentText()));
        ui->frame_tor->setVisible(index == Config::Proxy::Tor);
        this->updatePort();
    });

    int proxy = config()->get(Config::proxy).toInt();
    ui->frame_proxy->setVisible(proxy != Config::Proxy::None);
    ui->frame_tor->setVisible(proxy == Config::Proxy::Tor);
    ui->groupBox_proxySettings->setTitle(QString("%1 settings").arg(ui->comboBox_proxy->currentText()));

    // [Host]
    ui->line_host->setText(config()->get(Config::socks5Host).toString());
    connect(ui->line_host, &QLineEdit::textChanged, this, &NetworkProxyWidget::onProxySettingsChanged);

    // [Port]
    auto *portValidator = new QRegularExpressionValidator{QRegularExpression("[0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5]")};
    ui->line_port->setValidator(portValidator);
    ui->line_port->setText(config()->get(Config::socks5Port).toString());
    connect(ui->line_port, &QLineEdit::textChanged, this, &NetworkProxyWidget::onProxySettingsChanged);

    // [Tor settings]
    // [Let Feather start and manage a Tor daemon]
#if !defined(HAS_TOR_BIN) && !defined(PLATFORM_INSTALLER)
    ui->checkBox_torManaged->setChecked(false);
    ui->checkBox_torManaged->setEnabled(false);
    ui->checkBox_torManaged->setToolTip("Feather was bundled without Tor");
#else
    ui->checkBox_torManaged->setChecked(!config()->get(Config::useLocalTor).toBool());
    connect(ui->checkBox_torManaged, &QCheckBox::toggled, [this](bool toggled){
        this->updatePort();
        this->onProxySettingsChanged();
        if (!m_disableTorLogs) {
            ui->frame_torShowLogs->setVisible(toggled);
        }
    });
#endif

    // [Only allow connections to onion services]
    ui->checkBox_torOnlyAllowOnion->setChecked(config()->get(Config::torOnlyAllowOnion).toBool());
    connect(ui->checkBox_torOnlyAllowOnion, &QCheckBox::toggled, this, &NetworkProxyWidget::onProxySettingsChanged);

    // [Node traffic]
    ui->comboBox_torNodeTraffic->setCurrentIndex(config()->get(Config::torPrivacyLevel).toInt());
    connect(ui->comboBox_torNodeTraffic, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NetworkProxyWidget::onProxySettingsChanged);

    // [Show Tor logs]
    ui->frame_torShowLogs->setVisible(!config()->get(Config::useLocalTor).toBool());
#if !defined(HAS_TOR_BIN) && !defined(PLATFORM_INSTALLER)
    ui->frame_torShowLogs->setVisible(false);
#endif
    connect(ui->btn_torShowLogs, &QPushButton::clicked, [this]{
        m_torInfoDialog->show();
    });

    ui->frame_notice->hide();
}

void NetworkProxyWidget::onProxySettingsChanged() {
    if (!m_proxySettingsChanged) {
        emit proxySettingsChanged();
    }

    m_proxySettingsChanged = true;
}

void NetworkProxyWidget::updatePort() {
    int socks5port;
    switch (ui->comboBox_proxy->currentIndex()) {
        case Config::Proxy::Tor: {
            socks5port = 9050;
            break;
        }
        case Config::Proxy::i2p: {
            if (Prestium::detect()) {
                socks5port = 4448;
            } else {
                socks5port = 4447;
            }
            break;
        }
        default: {
            socks5port = 9050;
        }
    }
    ui->line_port->setText(QString::number(socks5port));
}

void NetworkProxyWidget::setProxySettings() {
    config()->set(Config::proxy, ui->comboBox_proxy->currentIndex());
    config()->set(Config::socks5Host, ui->line_host->text());
    config()->set(Config::socks5Port, ui->line_port->text());
    config()->set(Config::useLocalTor, !ui->checkBox_torManaged->isChecked());
    config()->set(Config::torOnlyAllowOnion, ui->checkBox_torOnlyAllowOnion->isChecked());
    config()->set(Config::torPrivacyLevel, ui->comboBox_torNodeTraffic->currentIndex());
    m_proxySettingsChanged = false;
}

void NetworkProxyWidget::setDisableTorLogs() {
    m_disableTorLogs = true;
    ui->frame_torShowLogs->hide();
}

NetworkProxyWidget::~NetworkProxyWidget() = default;
