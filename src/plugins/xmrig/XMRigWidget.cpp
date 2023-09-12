// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "XMRigWidget.h"
#include "ui_XMRigWidget.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QInputDialog>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QTableWidget>

#include "utils/Icons.h"
#include "utils/Utils.h"

XMRigWidget::XMRigWidget(Wallet *wallet, QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::XMRigWidget)
        , m_wallet(wallet)
        , m_XMRig(new XmRig(Config::defaultConfigDir().path()))
        , m_model(new QStandardItemModel(this))
        , m_contextMenu(new QMenu(this))
{
    ui->setupUi(this);

    connect(m_XMRig, &XmRig::stateChanged, this, &XMRigWidget::onXMRigStateChanged);
    connect(m_XMRig, &XmRig::output, this, &XMRigWidget::onProcessOutput);
    connect(m_XMRig, &XmRig::error, this, &XMRigWidget::onProcessError);
    connect(m_XMRig, &XmRig::hashrate, this, &XMRigWidget::onHashrate);

    // [Downloads] tab
    ui->tableView->setModel(m_model);
    m_contextMenu->addAction(icons()->icon("network.png"), "Download file", this, &XMRigWidget::linkClicked);
    connect(ui->tableView, &QHeaderView::customContextMenuRequested, this, &XMRigWidget::showContextMenu);
    connect(ui->tableView, &QTableView::doubleClicked, this, &XMRigWidget::linkClicked);

    // [Settings] tab
    ui->poolFrame->show();
    ui->soloFrame->hide();

    // XMRig executable
    connect(ui->btn_browse, &QPushButton::clicked, this, &XMRigWidget::onBrowseClicked);
    ui->lineEdit_path->setText(conf()->get(Config::xmrigPath).toString());

    // Run as admin/root
    bool elevated = conf()->get(Config::xmrigElevated).toBool();
    if (elevated) {
        ui->radio_elevateYes->setChecked(true);
    } else {
        ui->radio_elevateNo->setChecked(true);
    }
    connect(ui->radio_elevateYes, &QRadioButton::toggled, this, &XMRigWidget::onXMRigElevationChanged);
#if defined(Q_OS_WIN)
    ui->radio_elevateYes->setToolTip("Not supported on Windows, yet.");
    ui->radio_elevateYes->setEnabled(false);
    ui->radio_elevateNo->setChecked(true);
#endif

    // CPU threads
    ui->threadSlider->setMinimum(1);
    ui->threadSlider->setMaximum(QThread::idealThreadCount());

    int threads = conf()->get(Config::xmrigThreads).toInt();
    ui->threadSlider->setValue(threads);
    ui->label_threads->setText(QString("CPU threads: %1").arg(threads));

    connect(ui->threadSlider, &QSlider::valueChanged, this, &XMRigWidget::onThreadsValueChanged);

    // Mining mode
    connect(ui->combo_miningMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XMRigWidget::onMiningModeChanged);
    ui->combo_miningMode->setCurrentIndex(conf()->get(Config::miningMode).toInt());

    // Pool/node address
    this->updatePools();
    connect(ui->combo_pools, &QComboBox::currentTextChanged, this, &XMRigWidget::onPoolChanged);

    connect(ui->btn_poolConfig, &QPushButton::clicked, [this]{
        QStringList pools = conf()->get(Config::pools).toStringList();
        bool ok;
        QString poolStr = QInputDialog::getMultiLineText(this, "Pool addresses", "Set pool addresses (one per line):", pools.join("\n"), &ok);
        if (!ok) {
            return;
        }
        QStringList newPools = poolStr.split("\n");
        newPools.removeAll("");
        newPools.removeDuplicates();
        conf()->set(Config::pools, newPools);
        this->updatePools();
    });

    ui->lineEdit_solo->setText(conf()->get(Config::xmrigDaemon).toString());
    connect(ui->lineEdit_solo, &QLineEdit::textChanged, [this](){
        conf()->set(Config::xmrigDaemon, ui->lineEdit_solo->text());
    });

    // Network settings
    connect(ui->check_tls, &QCheckBox::toggled, this, &XMRigWidget::onNetworkTLSToggled);
    connect(ui->relayTor, &QCheckBox::toggled, this, &XMRigWidget::onNetworkTorToggled);
    ui->check_tls->setChecked(conf()->get(Config::xmrigNetworkTLS).toBool());
    ui->relayTor->setChecked(conf()->get(Config::xmrigNetworkTor).toBool());

    // Receiving address
    auto username = m_wallet->getCacheAttribute("feather.xmrig_username");
    if (!username.isEmpty()) {
        ui->lineEdit_address->setText(username);
    }
    connect(ui->lineEdit_address, &QLineEdit::textChanged, [=]() {
        m_wallet->setCacheAttribute("feather.xmrig_username", ui->lineEdit_address->text());
    });
    connect(ui->btn_fillPrimaryAddress, &QPushButton::clicked, this, &XMRigWidget::onUsePrimaryAddressClicked);

    // Password
    auto password = m_wallet->getCacheAttribute("feather.xmrig_password");
    ui->lineEdit_password->setText(password);
    connect(ui->lineEdit_password, &QLineEdit::textChanged, [=]() {
        m_wallet->setCacheAttribute("feather.xmrig_password", ui->lineEdit_password->text());
    });

    // [Status] tab
    connect(ui->btn_start, &QPushButton::clicked, this, &XMRigWidget::onStartClicked);
    connect(ui->btn_stop, &QPushButton::clicked, this, &XMRigWidget::onStopClicked);
    connect(ui->btn_clear, &QPushButton::clicked, this, &XMRigWidget::onClearClicked);

    ui->btn_stop->setEnabled(false);
    ui->check_autoscroll->setChecked(true);
    ui->label_status->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ui->label_status->hide();

    this->printConsoleInfo();
}

bool XMRigWidget::isMining() {
    return m_isMining;
}

void XMRigWidget::setDownloadsTabEnabled(bool enabled) {
    ui->tabWidget->setTabVisible(2, enabled);
}

void XMRigWidget::onWalletClosed() {
    this->onStopClicked();
}

void XMRigWidget::onThreadsValueChanged(int threads) {
    conf()->set(Config::xmrigThreads, threads);
    ui->label_threads->setText(QString("CPU threads: %1").arg(threads));
}

void XMRigWidget::onPoolChanged(const QString &pool) {
    if (!pool.isEmpty()) {
        conf()->set(Config::xmrigPool, pool);
    }
}

void XMRigWidget::onXMRigElevationChanged(bool elevated) {
    conf()->set(Config::xmrigElevated, elevated);
}

void XMRigWidget::onBrowseClicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "Path to XMRig executable", QDir::homePath());
    if (fileName.isEmpty()) {
        return;
    }
    conf()->set(Config::xmrigPath, fileName);
    ui->lineEdit_path->setText(fileName);
}

void XMRigWidget::onClearClicked() {
    ui->console->clear();
}

void XMRigWidget::onUsePrimaryAddressClicked() {
    ui->lineEdit_address->setText(m_wallet->address(0, 0));
}

void XMRigWidget::onStartClicked() {
    QString xmrigPath = conf()->get(Config::xmrigPath).toString();
    if (!this->checkXMRigPath()) {
        return;
    }

    QString address = [this](){
        if (ui->combo_miningMode->currentIndex() == Config::MiningMode::Pool) {
            return conf()->get(Config::xmrigPool).toString();
        } else {
            return ui->lineEdit_solo->text().trimmed();
        }
    }();
    if (address.isEmpty()) {
        ui->console->appendPlainText("No pool or node address set. Please configure on the Settings tab.");
        return;
    }

    // username is receiving address usually
    auto username = m_wallet->getCacheAttribute("feather.xmrig_username");
    auto password = m_wallet->getCacheAttribute("feather.xmrig_password");

    if (username.isEmpty()) {
        ui->console->appendPlainText("Please specify a receiving address on the Settings screen.");
        return;
    }

    if (address.contains("cryptonote.social") && !username.contains(".")) {
        // cryptonote social requires <addr>.<username>, we'll just grab a few chars from primary addy
        username = QString("%1.%2").arg(username, m_wallet->address(0, 0).mid(0, 6));
    }

    int threads = ui->threadSlider->value();
    bool solo = (ui->combo_miningMode->currentIndex() == Config::MiningMode::Solo);
    QStringList extraOptions = ui->lineEdit_extraOptions->text().split(" ");

    m_XMRig->start(xmrigPath, threads, address, username, password, ui->relayTor->isChecked(), ui->check_tls->isChecked(),
                   ui->radio_elevateYes->isChecked(), solo, extraOptions);
}

void XMRigWidget::onStopClicked() {
    m_XMRig->stop();
}

void XMRigWidget::onProcessOutput(const QByteArray &data) {
    auto output = Utils::barrayToString(data);
    if(output.endsWith("\n"))
        output = output.trimmed();

    ui->console->appendPlainText(output);

    if(ui->check_autoscroll->isChecked())
        ui->console->verticalScrollBar()->setValue(ui->console->verticalScrollBar()->maximum());
}

void XMRigWidget::onProcessError(const QString &msg) {
    ui->console->appendPlainText("\n" + msg);
    ui->btn_start->setEnabled(true);
    ui->btn_stop->setEnabled(false);
    this->setMiningStopped();
}

void XMRigWidget::onHashrate(const QString &hashrate) {
    ui->label_status->show();
    ui->label_status->setText(QString("Mining at %1").arg(hashrate));
}

void XMRigWidget::onDownloads(const QJsonObject &data) {
    // For the downloads table we'll manually update the table
    // with items once, as opposed to creating a class in
    // src/models/. Saves effort; full-blown model
    // is unnecessary in this case.

    m_model->clear();
    m_urls.clear();

    auto version = data.value("version").toString();
    ui->label_latest_version->setText(QString("Latest version: %1").arg(version));
    QJsonObject assets = data.value("assets").toObject();

    const auto _linux = assets.value("linux").toArray();
    const auto macos = assets.value("macos").toArray();
    const auto windows = assets.value("windows").toArray();

    auto info = QSysInfo::productType();
    QJsonArray *os_assets;
    if(info == "osx") {
        os_assets = const_cast<QJsonArray *>(&macos);
    } else if (info == "windows") {
        os_assets = const_cast<QJsonArray *>(&windows);
    } else {
        // assume linux
        os_assets = const_cast<QJsonArray *>(&_linux);
    }

    int i = 0;
    for(const auto &entry: *os_assets) {
        auto _obj = entry.toObject();
        auto _name = _obj.value("name").toString();
        auto _url = _obj.value("url").toString();
        auto _created_at = _obj.value("created_at").toString();

        m_urls.append(_url);
        auto download_count = _obj.value("download_count").toInt();

        m_model->setItem(i, 0, Utils::qStandardItem(_name));
        m_model->setItem(i, 1, Utils::qStandardItem(_created_at));
        m_model->setItem(i, 2, Utils::qStandardItem(QString::number(download_count)));
        i++;
    }

    m_model->setHeaderData(0, Qt::Horizontal, tr("Filename"), Qt::DisplayRole);
    m_model->setHeaderData(1, Qt::Horizontal, tr("Date"), Qt::DisplayRole);
    m_model->setHeaderData(2, Qt::Horizontal, tr("Downloads"), Qt::DisplayRole);

    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableView->setColumnWidth(2, 100);
}

void XMRigWidget::showContextMenu(const QPoint &pos) {
    QModelIndex index = ui->tableView->indexAt(pos);
        if (!index.isValid()) {
        return;
    }
    m_contextMenu->exec(ui->tableView->viewport()->mapToGlobal(pos));
}

void XMRigWidget::updatePools() {
    QStringList pools = conf()->get(Config::pools).toStringList();
    if (pools.isEmpty()) {
        pools = m_defaultPools;
        conf()->set(Config::pools, pools);
    }
    ui->combo_pools->clear();
    ui->combo_pools->insertItems(0, pools);

    QString preferredPool = conf()->get(Config::xmrigPool).toString();
    if (pools.contains(preferredPool)) {
        ui->combo_pools->setCurrentIndex(pools.indexOf(preferredPool));
    } else {
        preferredPool = pools.at(0);
        conf()->set(Config::xmrigPool, preferredPool);
    }
}

void XMRigWidget::printConsoleInfo() {
    ui->console->appendPlainText(QString("Detected %1 CPU threads.").arg(QThread::idealThreadCount()));
    if (this->checkXMRigPath()) {
        QString path = conf()->get(Config::xmrigPath).toString();
        ui->console->appendPlainText(QString("XMRig path set to %1").arg(path));
    }
}

void XMRigWidget::onMiningModeChanged(int mode) {
    conf()->set(Config::miningMode, mode);

    if (mode == Config::MiningMode::Pool) {
        ui->poolFrame->show();
        ui->soloFrame->hide();
        ui->label_poolNodeAddress->setText("Pool address:");
        ui->check_tls->setChecked(true);
    } else { // Solo mining
        ui->poolFrame->hide();
        ui->soloFrame->show();
        ui->label_poolNodeAddress->setText("Node address:");
        ui->check_tls->setChecked(false);
    }
}

void XMRigWidget::onNetworkTLSToggled(bool checked) {
    conf()->set(Config::xmrigNetworkTLS, checked);
}

void XMRigWidget::onNetworkTorToggled(bool checked) {
    conf()->set(Config::xmrigNetworkTor, checked);
}

void XMRigWidget::onXMRigStateChanged(QProcess::ProcessState state) {
    if (state == QProcess::ProcessState::Starting) {
        ui->btn_start->setEnabled(false);
        ui->btn_stop->setEnabled(false);
        this->setMiningStarted();
    }
    else if (state == QProcess::ProcessState::Running) {
        ui->btn_start->setEnabled(false);
        ui->btn_stop->setEnabled(true);
        this->setMiningStarted();
    }
    else if (state == QProcess::ProcessState::NotRunning) {
        ui->btn_start->setEnabled(true); // todo
        ui->btn_stop->setEnabled(false);
        ui->label_status->hide();
        this->setMiningStopped();
    }
}

void XMRigWidget::setMiningStopped() {
    m_isMining = false;
    emit miningEnded();
}

void XMRigWidget::setMiningStarted() {
    m_isMining = true;
    emit miningStarted();
}

bool XMRigWidget::checkXMRigPath() {
    QString path = conf()->get(Config::xmrigPath).toString();

    if (path.isEmpty()) {
        ui->console->appendPlainText("No XMRig executable is set. Please configure on the Settings tab.");
        return false;
    } else if (!Utils::fileExists(path)) {
        ui->console->appendPlainText("Invalid path to XMRig executable detected. Please reconfigure on the Settings tab.");
        return false;
    } else {
        return true;
    }
}

void XMRigWidget::linkClicked() {
    QModelIndex index = ui->tableView->currentIndex();
    auto download_link = m_urls.at(index.row());
    Utils::externalLinkWarning(this, download_link);
}

QStandardItemModel *XMRigWidget::model() {
    return m_model;
}

XMRigWidget::~XMRigWidget() = default;