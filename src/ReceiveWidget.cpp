// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "ReceiveWidget.h"
#include "ui_ReceiveWidget.h"

#include <QFileDialog>
#include <QMenu>

#include "libwalletqt/rows/SubaddressRow.h"
#include "dialog/PaymentRequestDialog.h"
#include "dialog/QrCodeDialog.h"
#include "utils/config.h"
#include "utils/Icons.h"
#include "utils/Utils.h"

ReceiveWidget::ReceiveWidget(Wallet *wallet, QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::ReceiveWidget)
        , m_wallet(wallet)
{
    ui->setupUi(this);

    m_model = m_wallet->subaddressModel();
    m_proxyModel = new SubaddressProxyModel(this, m_wallet->subaddress());
    m_proxyModel->setSourceModel(m_model);

    ui->addresses->setModel(m_proxyModel);
    ui->addresses->setColumnHidden(SubaddressModel::isUsed, true);
    ui->addresses->header()->setSectionResizeMode(SubaddressModel::Index, QHeaderView::ResizeToContents);
    ui->addresses->header()->setSectionResizeMode(SubaddressModel::Address, QHeaderView::ResizeToContents);
    ui->addresses->header()->setSectionResizeMode(SubaddressModel::Label, QHeaderView::Stretch);

    connect(ui->addresses->selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection &selected, const QItemSelection &deselected){
        this->updateQrCode();
    });
    connect(m_model, &SubaddressModel::modelReset, [this](){
        this->updateQrCode();
    });

    ui->qrCode->setCursor(Qt::PointingHandCursor);

    // header context menu
    ui->addresses->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    m_headerMenu = new QMenu(this);
    auto subMenu = new QMenu(this);
    subMenu->setTitle("Columns");
    
    this->addOption(m_headerMenu, "Show used addresses", Config::showUsedAddresses, [this](bool show){
        m_proxyModel->invalidate();
    });
    this->addOption(m_headerMenu, "Show hidden addresses", Config::showHiddenAddresses, [this](bool show){
        m_proxyModel->invalidate();
    });
    this->addOption(m_headerMenu, "Show full addresses", Config::showFullAddresses, [this](bool show){
        m_proxyModel->invalidate();
    });
    this->addOption(m_headerMenu, "Show change address", Config::showChangeAddresses, [this](bool show){
        m_proxyModel->invalidate();
    });
    
    m_headerMenu->addMenu(subMenu);
    this->addOption(subMenu, "Show index", Config::showAddressIndex, [this](bool show){
        ui->addresses->setColumnHidden(0, !show);
    });
    this->addOption(subMenu, "Show labels", Config::showAddressLabels, [this](bool show){
        ui->addresses->setColumnHidden(2, !show);
    });

    connect(ui->addresses->header(), &QHeaderView::customContextMenuRequested, this, &ReceiveWidget::showHeaderMenu);
    ui->toolBtn_options->setMenu(m_headerMenu);

    // context menu
    ui->addresses->setContextMenuPolicy(Qt::CustomContextMenu);
    m_showTransactionsAction = new QAction("Show transactions", this);
    connect(m_showTransactionsAction, &QAction::triggered, this, &ReceiveWidget::onShowTransactions);
    connect(ui->addresses, &QTreeView::customContextMenuRequested, this, &ReceiveWidget::showContextMenu);
    connect(ui->addresses, &SubaddressView::copyAddress, this, &ReceiveWidget::copyAddress);

    connect(ui->qrCode, &ClickableLabel::clicked, this, &ReceiveWidget::showQrCodeDialog);
    connect(ui->search, &QLineEdit::textChanged, this, &ReceiveWidget::setSearchFilter);

    connect(ui->btn_generateSubaddress, &QPushButton::clicked, this, &ReceiveWidget::generateSubaddress);
    connect(ui->btn_createPaymentRequest, &QPushButton::clicked, this, &ReceiveWidget::createPaymentRequest);
    connect(ui->btn_exportSubAddresses, &QPushButton::clicked, this, &ReceiveWidget::exportSubaddresses);
}

void ReceiveWidget::addOption(QMenu *menu, const QString &text, Config::ConfigKey key, const std::function<void(bool show)>& func) {
    // QMenu takes ownership of the returned QAction.
    QAction *action = menu->addAction(text, func);
    action->setCheckable(true);
    bool toggled = conf()->get(key).toBool();
    action->setChecked(toggled);
    func(toggled);
    connect(action, &QAction::toggled, [key](bool toggled){
        conf()->set(key, toggled);
    });
}

void ReceiveWidget::setSearchbarVisible(bool visible) {
    ui->frame_search->setVisible(visible);
}

void ReceiveWidget::focusSearchbar() {
    ui->search->setFocusPolicy(Qt::StrongFocus);
    ui->search->setFocus();
}

QString ReceiveWidget::getAddress(quint32 minorIndex) {
    bool ok;
    QString reason;
    QString address = m_wallet->getAddressSafe(m_wallet->currentSubaddressAccount(), minorIndex, ok, reason);

    if (!ok) {
        Utils::showError(this, "Unable to get address",
                         QString("Reason: %1\n\n"
                                 "WARNING!\n\n"
                                 "Potential wallet file corruption detected.\n\n"
                                 "To prevent LOSS OF FUNDS do NOT continue to use this wallet file.\n\n"
                                 "Restore your wallet from seed, keys, or device.\n\n"
                                 "Please report this incident to the Feather developers.\n\n"
                                 "WARNING!").arg(reason), {}, "report_an_issue");
        return {};
    }

    return address;
}

void ReceiveWidget::copyAddress() {
    QModelIndex index = getCurrentIndex();
    if (!index.isValid()) {
        return;
    }

    QString address = this->getAddress(index.row());
    Utils::copyToClipboard(address);
}

void ReceiveWidget::copyLabel() {
    QModelIndex index = ui->addresses->currentIndex();
    Utils::copyColumn(&index, SubaddressModel::Label);
}

void ReceiveWidget::editLabel() {
    QModelIndex index = ui->addresses->currentIndex().siblingAtColumn(SubaddressModel::ModelColumn::Label);
    ui->addresses->setCurrentIndex(index);
    ui->addresses->edit(index);
}

void ReceiveWidget::showContextMenu(const QPoint &point) {
    QModelIndex index = getCurrentIndex();
    if (!index.isValid()) {
        return;
    }
    auto& row = m_model->entryFromIndex(index);

    auto *menu = new QMenu(ui->addresses);

    menu->addAction("Copy address", this, &ReceiveWidget::copyAddress);
    menu->addAction("Copy label", this, &ReceiveWidget::copyLabel);
    menu->addAction("Edit label", this, &ReceiveWidget::editLabel);

    if (row.used) {
        menu->addAction(m_showTransactionsAction);
    }

    QAction *actionPin = menu->addAction("Pin address", [this](bool toggled){
        QModelIndex index = getCurrentIndex();
        if (!index.isValid()) {
            return;
        }
        auto& row = m_model->entryFromIndex(index);

        m_wallet->subaddress()->setPinned(row.address, toggled);
        m_proxyModel->invalidate();
    });
    actionPin->setCheckable(true);
    actionPin->setChecked(row.pinned);

    QAction *actionHide = menu->addAction("Hide address", [this](bool toggled){
        QModelIndex index = getCurrentIndex();
        if (!index.isValid()) {
            return;
        }
        auto& row = m_model->entryFromIndex(index);

        m_wallet->subaddress()->setHidden(row.address, toggled);
        m_proxyModel->invalidate();
    });
    actionHide->setCheckable(true);
    actionHide->setChecked(row.hidden);

    if (m_wallet->isHwBacked()) {
        menu->addAction("Show on device", this, &ReceiveWidget::showOnDevice);
    }

    menu->popup(ui->addresses->viewport()->mapToGlobal(point));
}

void ReceiveWidget::createPaymentRequest() {
    QModelIndex index = getCurrentIndex();
    if (!index.isValid()) {
        return;
    }

    QString address = this->getAddress(index.row());

    PaymentRequestDialog dialog{this, m_wallet, address};
    dialog.exec();
}

void ReceiveWidget::onShowTransactions() {
    QModelIndex index = getCurrentIndex();
    if (!index.isValid()) {
        return;
    }

    QString address = this->getAddress(index.row());
    emit showTransactions(address);
}

void ReceiveWidget::setSearchFilter(const QString &filter) {
    m_proxyModel->setSearchFilter(filter);
}

void ReceiveWidget::showHeaderMenu(const QPoint& position)
{
    Q_UNUSED(position)
    m_headerMenu->exec(QCursor::pos());
}

void ReceiveWidget::showOnDevice() {
    QModelIndex index = getCurrentIndex();
    if (!index.isValid()) {
        return;
    }
    m_wallet->deviceShowAddressAsync(m_wallet->currentSubaddressAccount(), index.row(), "");
}

void ReceiveWidget::generateSubaddress() {
    int numSubaddressesToGenerate = ui->spn_numBatchSubaddresses->value();

    for (int subAddressIndex = 0; subAddressIndex < numSubaddressesToGenerate; subAddressIndex++) {
        bool r = m_wallet->subaddress()->addRow(m_wallet->currentSubaddressAccount(), "");
        if (!r) {
            Utils::showError(this, "Failed to generate subaddress", m_wallet->subaddress()->getError());

            //Stop the loop on first error, to prevent multiple errors if we batch create more than 1 address.
            subAddressIndex = numSubaddressesToGenerate;
        }
    }
}

void ReceiveWidget::exportSubaddresses() {
    QFileDialog exportAddressesdialog;
    exportAddressesdialog.setAcceptMode(QFileDialog::AcceptSave);
    exportAddressesdialog.setDirectory(".");
    exportAddressesdialog.selectFile("Addresses.txt");

    if (exportAddressesdialog.exec()) {
        QString fileName = exportAddressesdialog.selectedFiles().first();
        QFile file(fileName);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            Utils::showError(this, "Failed to export subaddress", "Could not export subaddresses to file.");
        }

        QTextStream stream(&file);
        for (int i = 0; i < m_wallet->numSubaddresses(m_wallet->currentSubaddressAccount()); i++) {
            QString address = this->getAddress(i);
            stream << address << "\n";
        }
        file.close();
    }
}

void ReceiveWidget::updateQrCode(){
    QModelIndex index = getCurrentIndex();
    if (!index.isValid()) {
        ui->qrCode->clear();
        ui->btn_createPaymentRequest->hide();
        return;
    }

    QString address = this->getAddress(index.row());
    const QrCode qrc(address, QrCode::Version::AUTO, QrCode::ErrorCorrectionLevel::MEDIUM);

    int width = ui->qrCode->width() - 4;
    if (qrc.isValid()) {
        ui->qrCode->setPixmap(qrc.toPixmap(1).scaled(width, width, Qt::KeepAspectRatio));
        ui->btn_createPaymentRequest->show();
    }
}

void ReceiveWidget::showQrCodeDialog() {
    QModelIndex index = getCurrentIndex();
    if (!index.isValid()) {
        return;
    }

    QString address = this->getAddress(index.row());
    QrCode qr(address, QrCode::Version::AUTO, QrCode::ErrorCorrectionLevel::HIGH);
    QrCodeDialog dialog{this, &qr, "Address"};
    dialog.exec();
}

QModelIndex ReceiveWidget::getCurrentIndex()
{
    return m_proxyModel->mapToSource(ui->addresses->currentIndex());
}

ReceiveWidget::~ReceiveWidget() = default;
