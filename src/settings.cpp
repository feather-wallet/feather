// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "settings.h"
#include "ui_settings.h"
#include "appcontext.h"
#include "utils/config.h"
#include "mainwindow.h"

#include <QFileDialog>

Settings::Settings(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::Settings)
{
    ui->setupUi(this);
    m_ctx = MainWindow::getContext();

    this->setWindowIcon(QIcon("://assets/images/appicons/64x64.png"));

    ui->tabWidget->setTabVisible(2, false);
    ui->tabWidget->setTabVisible(4, false);

    connect(ui->btnCopyToClipboard, &QPushButton::clicked, this, &Settings::copyToClipboard);
    connect(ui->checkBox_externalLink, &QCheckBox::clicked, this, &Settings::checkboxExternalLinkWarn);
    connect(ui->checkBox_hideBalance, &QCheckBox::toggled, [this](bool toggled){
        config()->set(Config::hideBalance, toggled);
        m_ctx->updateBalance();
    });

    connect(ui->closeButton, &QDialogButtonBox::accepted, this, &Settings::close);

    // nodes
    ui->nodeWidget->setupUI(m_ctx);
    connect(ui->nodeWidget, &NodeWidget::nodeSourceChanged, m_ctx->nodes, &Nodes::onNodeSourceChanged);
    connect(ui->nodeWidget, &NodeWidget::connectToNode, m_ctx->nodes, QOverload<const FeatherNode&>::of(&Nodes::connectToNode));

    // setup checkboxes
    ui->checkBox_externalLink->setChecked(config()->get(Config::warnOnExternalLink).toBool());
    ui->checkBox_hideBalance->setChecked(config()->get(Config::hideBalance).toBool());

    // setup comboboxes
    this->setupSkinCombobox();
    auto settingsSkin = config()->get(Config::skin).toString();
    if (m_skins.contains(settingsSkin))
        ui->comboBox_skin->setCurrentIndex(m_skins.indexOf(settingsSkin));

    connect(ui->comboBox_skin, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Settings::comboBox_skinChanged);
    connect(ui->comboBox_blockExplorer, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Settings::comboBox_blockExplorerChanged);

    // setup preferred fiat currency combobox
    QStringList fiatCurrencies;
    for (int index = 0; index < ui->comboBox_fiatCurrency->count(); index++)
        fiatCurrencies << ui->comboBox_fiatCurrency->itemText(index);

    auto preferredFiatCurrency = config()->get(Config::preferredFiatCurrency).toString();
    if(!preferredFiatCurrency.isEmpty())
        if(fiatCurrencies.contains(preferredFiatCurrency))
            ui->comboBox_fiatCurrency->setCurrentText(preferredFiatCurrency);

    connect(ui->comboBox_fiatCurrency, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Settings::fiatCurrencySelected);

    // setup paths tab
    this->updatePaths();
    connect(ui->btn_browseDefaultWalletDir, &QPushButton::clicked, [this]{
        QString walletDir = QFileDialog::getExistingDirectory(this, "Select wallet directory ", m_ctx->defaultWalletDir, QFileDialog::ShowDirsOnly);
        if (walletDir.isEmpty()) return;
        m_ctx->defaultWalletDir = walletDir;
        m_ctx->defaultWalletDirRoot = walletDir;
        config()->set(Config::walletDirectory, walletDir);
        ui->lineEdit_defaultWalletDir->setText(m_ctx->defaultWalletDir);
    });

    this->adjustSize();
}

void Settings::updatePaths() {
    ui->lineEdit_defaultWalletDir->setText(m_ctx->defaultWalletDir);
    ui->lineEdit_configDir->setText(m_ctx->configDirectory);
    ui->lineEdit_applicationDir->setText(m_ctx->applicationPath);
}

void Settings::fiatCurrencySelected(int index) {
    QString selection = ui->comboBox_fiatCurrency->itemText(index);
    config()->set(Config::preferredFiatCurrency, selection);
    emit preferredFiatCurrencyChanged(selection);
}

void Settings::comboBox_skinChanged(int pos) {
    emit skinChanged(m_skins.at(pos));
}

void Settings::comboBox_blockExplorerChanged(int pos) {
    QString blockExplorer = ui->comboBox_blockExplorer->currentText();
    config()->set(Config::blockExplorer, blockExplorer);
    emit blockExplorerChanged(blockExplorer);
}

void Settings::copyToClipboard() {
    ui->textLogs->copy();
}

void Settings::checkboxExternalLinkWarn() {
    bool state = ui->checkBox_externalLink->isChecked();
    config()->set(Config::warnOnExternalLink, state);
}

void Settings::setupSkinCombobox() {
#if defined(Q_OS_WIN)
    m_skins.removeOne("Breeze/Dark");
    m_skins.removeOne("Breeze/Light");
#elif defined(Q_OS_MAC)
    m_skins.removeOne("QDarkStyle");
#endif

    ui->comboBox_skin->insertItems(0, m_skins);
}

Settings::~Settings() {
    delete ui;
}
