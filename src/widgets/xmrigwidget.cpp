// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QStandardItemModel>
#include <QTableWidget>
#include <QMessageBox>
#include <QDesktopServices>
#include <QScrollBar>
#include <QFileDialog>

#include "xmrigwidget.h"
#include "ui_xmrigwidget.h"
#include "utils/Icons.h"

XMRigWidget::XMRigWidget(AppContext *ctx, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::XMRigWidget)
    , m_ctx(ctx)
    , m_XMRig(new XmRig(Config::defaultConfigDir().path()))
    , m_model(new QStandardItemModel(this))
    , m_contextMenu(new QMenu(this))
{
    ui->setupUi(this);

    QPixmap p(":assets/images/xmrig.svg");
    ui->lbl_logo->setPixmap(p.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    connect(m_XMRig, &XmRig::output, this, &XMRigWidget::onProcessOutput);
    connect(m_XMRig, &XmRig::error, this, &XMRigWidget::onProcessError);
    connect(m_XMRig, &XmRig::hashrate, this, &XMRigWidget::onHashrate);

    connect(m_ctx, &AppContext::walletClosed, this, &XMRigWidget::onWalletClosed);
    connect(m_ctx, &AppContext::walletOpened, this, &XMRigWidget::onWalletOpened);

    // table
    ui->tableView->setModel(this->m_model);
    m_contextMenu->addAction(icons()->icon("network.png"), "Download file", this, &XMRigWidget::linkClicked);
    connect(ui->tableView, &QHeaderView::customContextMenuRequested, this, &XMRigWidget::showContextMenu);
    connect(ui->tableView, &QTableView::doubleClicked, this, &XMRigWidget::linkClicked);

    // threads
    ui->threadSlider->setMinimum(1);
    int threads = QThread::idealThreadCount();
    m_threads = threads / 2;
    ui->threadSlider->setMaximum(threads);
    ui->threadSlider->setValue(m_threads);
    ui->label_threads->setText(QString("CPU threads: %1").arg(m_threads));
    connect(ui->threadSlider, &QSlider::valueChanged, this, &XMRigWidget::onThreadsValueChanged);

    // buttons
    connect(ui->btn_start, &QPushButton::clicked, this, &XMRigWidget::onStartClicked);
    connect(ui->btn_stop, &QPushButton::clicked, this, &XMRigWidget::onStopClicked);
    connect(ui->btn_browse, &QPushButton::clicked, this, &XMRigWidget::onBrowseClicked);
    connect(ui->btn_clear, &QPushButton::clicked, this, &XMRigWidget::onClearClicked);

    // defaults
    ui->btn_stop->setEnabled(false);
    ui->check_autoscroll->setChecked(true);
    ui->relayTor->setChecked(false);
    ui->check_tls->setChecked(true);
    ui->label_status->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ui->label_status->hide();
    ui->soloFrame->hide();
    ui->poolFrame->hide();

    // XMRig binary
    auto path = config()->get(Config::xmrigPath).toString();
    if(!path.isEmpty()) {
        ui->lineEdit_path->setText(path);
    }

    // pools
    ui->poolFrame->show();
    ui->combo_pools->insertItems(0, m_pools);
    auto preferredPool = config()->get(Config::xmrigPool).toString();
    if (m_pools.contains(preferredPool))
        ui->combo_pools->setCurrentIndex(m_pools.indexOf(preferredPool));
    else {
        preferredPool = m_pools.at(0);
        config()->set(Config::xmrigPool, preferredPool);
    }
    connect(ui->combo_pools, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XMRigWidget::onPoolChanged);

    // info
    ui->console->appendPlainText(QString("Detected %1 CPU threads.").arg(threads));
    if(!path.isEmpty() && !Utils::fileExists(path))
        ui->console->appendPlainText("Invalid path to XMRig binary detected. Please reconfigure on the Settings tab.");
    else
        ui->console->appendPlainText(QString("XMRig path set to %1").arg(path));

    ui->console->appendPlainText("Ready to mine.");

    // username/password
    connect(ui->lineEdit_password, &QLineEdit::editingFinished, [=]() {
        m_ctx->currentWallet->setCacheAttribute("feather.xmrig_password", ui->lineEdit_password->text());
        m_ctx->storeWallet();
    });
    connect(ui->lineEdit_address, &QLineEdit::editingFinished, [=]() {
        m_ctx->currentWallet->setCacheAttribute("feather.xmrig_username", ui->lineEdit_address->text());
        m_ctx->storeWallet();
    });

    // checkbox connects
    connect(ui->check_solo, &QCheckBox::stateChanged, this, &XMRigWidget::onSoloChecked);
}

void XMRigWidget::onWalletClosed() {
    this->onStopClicked();
    this->onClearClicked();
    ui->lineEdit_password->setText("");
    ui->lineEdit_address->setText("");
}

void XMRigWidget::onWalletOpened(){
    // Xmrig username
    auto username = m_ctx->currentWallet->getCacheAttribute("feather.xmrig_username");
    if(!username.isEmpty())
        ui->lineEdit_address->setText(username);

    // Xmrig passwd
    auto password = m_ctx->currentWallet->getCacheAttribute("feather.xmrig_password");
    if(!password.isEmpty()) {
        ui->lineEdit_password->setText(password);
    } else {
        ui->lineEdit_password->setText("featherwallet");
        m_ctx->currentWallet->setCacheAttribute("feather.xmrig_password", ui->lineEdit_password->text());
    }
}

void XMRigWidget::onThreadsValueChanged(int threads) {
    m_threads = threads;
    ui->label_threads->setText(QString("CPU threads: %1").arg(m_threads));
}

void XMRigWidget::onPoolChanged(int pos) {
    config()->set(Config::xmrigPool, m_pools.at(pos));
}

void XMRigWidget::onBrowseClicked() {
    QString fileName = QFileDialog::getOpenFileName(
            this, "Path to XMRig executable", QDir::homePath());
    if (fileName.isEmpty()) return;
    config()->set(Config::xmrigPath, fileName);
    ui->lineEdit_path->setText(fileName);
}

void XMRigWidget::onClearClicked() {
    ui->console->clear();
}

void XMRigWidget::onStartClicked() {
    QString xmrigPath;
    bool solo = ui->check_solo->isChecked();
    xmrigPath = config()->get(Config::xmrigPath).toString();

    // username is receiving address usually
    auto username = m_ctx->currentWallet->getCacheAttribute("feather.xmrig_username");
    auto password = m_ctx->currentWallet->getCacheAttribute("feather.xmrig_password");

    if(username.isEmpty()) {
        QString err = "Please specify a receiving address on the Settings screen";
        ui->console->appendPlainText(err);
        QMessageBox::warning(this, "Error", err);
        return;
    }

    QString address;
    if(solo)
        address = ui->lineEdit_solo->text().trimmed();
    else
        address = config()->get(Config::xmrigPool).toString();

    if(address.contains("cryptonote.social") && !username.contains(".")) {
        // cryptonote social requires <addr>.<username>, we'll just grab a few chars from primary addy
        username = QString("%1.%2").arg(username, m_ctx->currentWallet->address(0, 0).mid(0, 6));
    }

    m_XMRig->start(xmrigPath, m_threads, address, username, password, ui->relayTor->isChecked(), ui->check_tls->isChecked());
    ui->btn_start->setEnabled(false);
    ui->btn_stop->setEnabled(true);
    emit miningStarted();
}

void XMRigWidget::onStopClicked() {
    m_XMRig->stop();
    ui->btn_start->setEnabled(true);
    ui->btn_stop->setEnabled(false);
    ui->label_status->hide();
    emit miningEnded();
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
    ui->label_status->hide();
    emit miningEnded();
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

void XMRigWidget::onSoloChecked(int state) {
    if(state == 2) {
        ui->poolFrame->hide();
        ui->soloFrame->show();
        ui->check_tls->setChecked(false);
    }
    else {
        ui->poolFrame->show();
        ui->soloFrame->hide();
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

XMRigWidget::~XMRigWidget() {
    delete ui;
}
