// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "ui_receivewidget.h"
#include "receivewidget.h"
#include "qrcode/QrCode.h"
#include "model/ModelUtils.h"
#include "dialog/qrcodedialog.h"

#include <QMenu>
#include <QClipboard>

ReceiveWidget::ReceiveWidget(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ReceiveWidget)
{
    ui->setupUi(this);

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

    connect(ui->btn_generateSubaddress, &QPushButton::clicked, [=]() {
        emit generateSubaddress();
    });

    connect(ui->qrCode, &ClickableLabel::clicked, this, &ReceiveWidget::showQrCodeDialog);
    connect(ui->label_addressSearch, &QLineEdit::textChanged, this, &ReceiveWidget::setSearchFilter);
}

void ReceiveWidget::setModel(SubaddressModel * model, Subaddress * subaddress) {
    m_subaddress = subaddress;
    m_model = model;
    m_proxyModel = new SubaddressProxyModel(this, subaddress);
    m_proxyModel->setSourceModel(m_model);
    ui->addresses->setModel(m_proxyModel);

    ui->addresses->setColumnHidden(SubaddressModel::isUsed, true);
    ui->addresses->header()->setSectionResizeMode(SubaddressModel::Address, QHeaderView::Stretch);
    ui->addresses->header()->setSectionResizeMode(SubaddressModel::Label, QHeaderView::ResizeToContents);
    ui->addresses->header()->setMinimumSectionSize(200);

    connect(ui->addresses->selectionModel(), &QItemSelectionModel::currentRowChanged, [=](QModelIndex current, QModelIndex prev){
        if (current.isValid())
            this->setQrCode(current.model()->data(current.siblingAtColumn(SubaddressModel::Address), Qt::UserRole).toString());
        else
            ui->qrCode->clear();
    });
//    connect(m_model, &SubaddressModel::modelReset, [this](){
//        ui->btn_generateSubaddress->setEnabled(m_model->unusedLookahead() < SUBADDRESS_LOOKAHEAD_MINOR);
//    });
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
    QModelIndex index = ui->addresses->indexAt(point);
    if (!index.isValid()) {
        return;
    }

    auto *menu = new QMenu(ui->addresses);
    bool isUsed = index.model()->data(index.siblingAtColumn(SubaddressModel::isUsed), Qt::UserRole).toBool();

    menu->addAction(QIcon(":/assets/images/copy.png"), "Copy address", this, &ReceiveWidget::copyAddress);
    menu->addAction(QIcon(":/assets/images/copy.png"), "Copy label", this, &ReceiveWidget::copyLabel);
    menu->addAction(QIcon(":/assets/images/edit.png"), "Edit label", this, &ReceiveWidget::editLabel);

    if (isUsed) {
        menu->addAction(m_showTransactionsAction);
    }

    menu->popup(ui->addresses->viewport()->mapToGlobal(point));
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

void ReceiveWidget::setSearchFilter(const QString &filter) {
    if (!m_proxyModel) return;
    m_proxyModel->setSearchFilter(filter);
}

void ReceiveWidget::showHeaderMenu(const QPoint& position)
{
    m_showFullAddressesAction->setChecked(m_model->isShowFullAddresses());
    m_headerMenu->exec(QCursor::pos());
}

void ReceiveWidget::setQrCode(const QString &address){
    const QrCode qrc(address, QrCode::Version::AUTO, QrCode::ErrorCorrectionLevel::MEDIUM);

    int width = ui->qrCode->width() - 4;
    if (qrc.isValid()) {
        ui->qrCode->setPixmap(qrc.toPixmap(1).scaled(width, width, Qt::KeepAspectRatio));
    }
}

void ReceiveWidget::showQrCodeDialog() {
    QModelIndex index = ui->addresses->currentIndex();
    QString address = index.model()->data(index.siblingAtColumn(SubaddressModel::Address), Qt::UserRole).toString();
    auto *dialog = new QrCodeDialog(this, address, "Address");
    dialog->exec();
    dialog->deleteLater();
}

ReceiveWidget::~ReceiveWidget() {
    delete ui;
}
