// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "ReceiveWidget.h"
#include "ui_ReceiveWidget.h"

#include <QMenu>
#include <QMessageBox>

#include "dialog/PaymentRequestDialog.h"
#include "dialog/QrCodeDialog.h"
#include "model/ModelUtils.h"
#include "utils/Icons.h"

ReceiveWidget::ReceiveWidget(QSharedPointer<AppContext> ctx, QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::ReceiveWidget)
        , m_ctx(std::move(ctx))
{
    ui->setupUi(this);

    m_model = m_ctx->wallet->subaddressModel();
    m_proxyModel = new SubaddressProxyModel(this, m_ctx->wallet->subaddress());
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setHiddenAddresses(this->getHiddenAddresses());

    ui->addresses->setModel(m_proxyModel);
    ui->addresses->setColumnHidden(SubaddressModel::isUsed, true);
    ui->addresses->header()->setSectionResizeMode(SubaddressModel::Address, QHeaderView::Stretch);
    ui->addresses->header()->setSectionResizeMode(SubaddressModel::Label, QHeaderView::ResizeToContents);
    ui->addresses->header()->setMinimumSectionSize(200);

    connect(ui->addresses->selectionModel(), &QItemSelectionModel::currentChanged, [=](QModelIndex current, QModelIndex prev){
        this->updateQrCode();
    });
    connect(m_model, &SubaddressModel::modelReset, [this](){
        this->updateQrCode();
    });

    ui->qrCode->setCursor(Qt::PointingHandCursor);

    // header context menu
    ui->addresses->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    m_headerMenu = new QMenu(this);
    m_showFullAddressesAction = m_headerMenu->addAction("Show full addresses", this, &ReceiveWidget::setShowFullAddresses);
    m_showFullAddressesAction->setCheckable(true);
    m_showUsedAddressesAction = m_headerMenu->addAction("Show used addresses", this, &ReceiveWidget::setShowUsedAddresses);
    m_showUsedAddressesAction->setCheckable(true);
    connect(ui->addresses->header(), &QHeaderView::customContextMenuRequested, this, &ReceiveWidget::showHeaderMenu);

    // context menu
    ui->addresses->setContextMenuPolicy(Qt::CustomContextMenu);
    m_showTransactionsAction = new QAction("Show transactions");
    connect(m_showTransactionsAction, &QAction::triggered, this, &ReceiveWidget::onShowTransactions);
    connect(ui->addresses, &QTreeView::customContextMenuRequested, this, &ReceiveWidget::showContextMenu);

    connect(ui->btn_generateSubaddress, &QPushButton::clicked, this, &ReceiveWidget::generateSubaddress);

    connect(ui->qrCode, &ClickableLabel::clicked, this, &ReceiveWidget::showQrCodeDialog);
    connect(ui->search, &QLineEdit::textChanged, this, &ReceiveWidget::setSearchFilter);

    connect(ui->check_showUsed, &QCheckBox::clicked, this, &ReceiveWidget::setShowUsedAddresses);
    connect(ui->check_showHidden, &QCheckBox::clicked, this, &ReceiveWidget::setShowHiddenAddresses);

    connect(ui->btn_createPaymentRequest, &QPushButton::clicked, this, &ReceiveWidget::createPaymentRequest);
}

void ReceiveWidget::setSearchbarVisible(bool visible) {
    ui->search->setVisible(visible);
}

void ReceiveWidget::focusSearchbar() {
    ui->search->setFocusPolicy(Qt::StrongFocus);
    ui->search->setFocus();
}

void ReceiveWidget::copyAddress() {
    QModelIndex index = ui->addresses->currentIndex();
    ModelUtils::copyColumn(&index, SubaddressModel::Address);
}

void ReceiveWidget::copyLabel() {
    QModelIndex index = ui->addresses->currentIndex();
    ModelUtils::copyColumn(&index, SubaddressModel::Label);
}

void ReceiveWidget::editLabel() {
    QModelIndex index = ui->addresses->currentIndex().siblingAtColumn(m_model->ModelColumn::Label);
    ui->addresses->setCurrentIndex(index);
    ui->addresses->edit(index);
}

void ReceiveWidget::showContextMenu(const QPoint &point) {
    Monero::SubaddressRow* row = this->currentEntry();
    if (!row) return;

    QString address = QString::fromStdString(row->getAddress());
    bool isUsed = row->isUsed();

    auto *menu = new QMenu(ui->addresses);

    menu->addAction(icons()->icon("copy.png"), "Copy address", this, &ReceiveWidget::copyAddress);
    menu->addAction(icons()->icon("copy.png"), "Copy label", this, &ReceiveWidget::copyLabel);
    menu->addAction(icons()->icon("edit.png"), "Edit label", this, &ReceiveWidget::editLabel);

    if (isUsed) {
        menu->addAction(m_showTransactionsAction);
    }

    QStringList hiddenAddresses = this->getHiddenAddresses();
    if (hiddenAddresses.contains(address)) {
        menu->addAction("Show address", this, &ReceiveWidget::showAddress);
    } else {
        menu->addAction("Hide address", this, &ReceiveWidget::hideAddress);
    }

    if (m_ctx->wallet->isHwBacked()) {
        menu->addAction("Show on device", this, &ReceiveWidget::showOnDevice);
    }

    menu->popup(ui->addresses->viewport()->mapToGlobal(point));
}

void ReceiveWidget::createPaymentRequest() {
    QModelIndex index = ui->addresses->currentIndex();
    if (!index.isValid()) {
        return;
    }

    QString address = index.model()->data(index.siblingAtColumn(SubaddressModel::Address), Qt::UserRole).toString();

    PaymentRequestDialog dialog{this, m_ctx, address};
    dialog.exec();
}

void ReceiveWidget::onShowTransactions() {
    QModelIndex index = ui->addresses->currentIndex();
    if (!index.isValid()) {
        return;
    }

    QString address = index.model()->data(index.siblingAtColumn(SubaddressModel::Address), Qt::UserRole).toString();
    emit showTransactions(address);
}

void ReceiveWidget::setShowFullAddresses(bool show) {
    if (!m_model) return;
    m_model->setShowFullAddresses(show);
}

void ReceiveWidget::setShowUsedAddresses(bool show) {
    if (!m_proxyModel) return;
    m_proxyModel->setShowUsed(show);
}

void ReceiveWidget::setShowHiddenAddresses(bool show) {
    if (!m_proxyModel) return;
    m_proxyModel->setShowHidden(show);
}

void ReceiveWidget::setSearchFilter(const QString &filter) {
    if (!m_proxyModel) return;
    m_proxyModel->setSearchFilter(filter);
}

void ReceiveWidget::showHeaderMenu(const QPoint& position)
{
    Q_UNUSED(position);
    m_showFullAddressesAction->setChecked(m_model->isShowFullAddresses());
    m_headerMenu->exec(QCursor::pos());
}

void ReceiveWidget::hideAddress()
{
    Monero::SubaddressRow* row = this->currentEntry();
    if (!row) return;
    QString address = QString::fromStdString(row->getAddress());
    this->addHiddenAddress(address);
    m_proxyModel->setHiddenAddresses(this->getHiddenAddresses());
}

void ReceiveWidget::showAddress()
{
    Monero::SubaddressRow* row = this->currentEntry();
    if (!row) return;
    QString address = QString::fromStdString(row->getAddress());
    this->removeHiddenAddress(address);
    m_proxyModel->setHiddenAddresses(this->getHiddenAddresses());
}

void ReceiveWidget::showOnDevice() {
    Monero::SubaddressRow* row = this->currentEntry();
    if (!row) return;
    m_ctx->wallet->deviceShowAddressAsync(m_ctx->wallet->currentSubaddressAccount(), row->getRowId(), "");
}

void ReceiveWidget::generateSubaddress() {
    bool r = m_ctx->wallet->subaddress()->addRow(m_ctx->wallet->currentSubaddressAccount(), "");
    if (!r) {
        QMessageBox::warning(this, "Warning", QString("Failed to generate subaddress:\n\n%1").arg(m_ctx->wallet->subaddress()->errorString()));
    }
}

void ReceiveWidget::updateQrCode(){
    QModelIndex index = ui->addresses->currentIndex();
    if (!index.isValid()) {
        ui->qrCode->clear();
        ui->btn_createPaymentRequest->hide();
        return;
    }

    QString address = index.model()->data(index.siblingAtColumn(SubaddressModel::Address), Qt::UserRole).toString();
    const QrCode qrc(address, QrCode::Version::AUTO, QrCode::ErrorCorrectionLevel::MEDIUM);

    int width = ui->qrCode->width() - 4;
    if (qrc.isValid()) {
        ui->qrCode->setPixmap(qrc.toPixmap(1).scaled(width, width, Qt::KeepAspectRatio));
        ui->btn_createPaymentRequest->show();
    }
}

void ReceiveWidget::showQrCodeDialog() {
    QModelIndex index = ui->addresses->currentIndex();
    if (!index.isValid()) {
        return;
    }
    QString address = index.model()->data(index.siblingAtColumn(SubaddressModel::Address), Qt::UserRole).toString();
    QrCode qr(address, QrCode::Version::AUTO, QrCode::ErrorCorrectionLevel::HIGH);
    QrCodeDialog dialog{this, &qr, "Address"};
    dialog.exec();
}

QStringList ReceiveWidget::getHiddenAddresses() {
    QString data = m_ctx->wallet->getCacheAttribute("feather.hiddenaddresses");
    return data.split(",");
}

void ReceiveWidget::addHiddenAddress(const QString& address) {
    QStringList hiddenAddresses = this->getHiddenAddresses();
    if (!hiddenAddresses.contains(address)) {
        hiddenAddresses.append(address);
    }
    QString data = hiddenAddresses.join(",");
    m_ctx->wallet->setCacheAttribute("feather.hiddenaddresses", data);
}

void ReceiveWidget::removeHiddenAddress(const QString &address) {
    QStringList hiddenAddresses = this->getHiddenAddresses();
    hiddenAddresses.removeAll(address);
    QString data = hiddenAddresses.join(",");
    m_ctx->wallet->setCacheAttribute("feather.hiddenaddresses", data);
}

Monero::SubaddressRow* ReceiveWidget::currentEntry() {
    QModelIndexList list = ui->addresses->selectionModel()->selectedRows();
    if (list.size() == 1) {
        return m_model->entryFromIndex(m_proxyModel->mapToSource(list.first()));
    } else {
        return nullptr;
    }
}

ReceiveWidget::~ReceiveWidget() = default;