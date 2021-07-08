// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "ContactsWidget.h"
#include "ui_ContactsWidget.h"

#include <QMessageBox>

#include "dialog/ContactsDialog.h"
#include "libwalletqt/AddressBook.h"
#include "model/ModelUtils.h"
#include "utils/Icons.h"

ContactsWidget::ContactsWidget(QSharedPointer<AppContext> ctx, QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::ContactsWidget)
        , m_ctx(std::move(ctx))
{
    ui->setupUi(this);

    m_model = m_ctx->wallet->addressBookModel();
    m_proxyModel = new AddressBookProxyModel;
    m_proxyModel->setSourceModel(m_model);
    ui->contacts->setModel(m_proxyModel);

    ui->contacts->setSortingEnabled(true);
    ui->contacts->header()->setSectionResizeMode(AddressBookModel::Address, QHeaderView::Stretch);
    ui->contacts->header()->setSectionResizeMode(AddressBookModel::Description, QHeaderView::ResizeToContents);
    ui->contacts->header()->setMinimumSectionSize(200);

    // header context menu
    ui->contacts->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    m_headerMenu = new QMenu(this);
    m_showFullAddressesAction = m_headerMenu->addAction("Show full addresses", this, &ContactsWidget::setShowFullAddresses);
    m_showFullAddressesAction->setCheckable(true);

    connect(ui->contacts->header(), &QHeaderView::customContextMenuRequested, this, &ContactsWidget::showHeaderMenu);

    // context menu
    ui->contacts->setContextMenuPolicy(Qt::CustomContextMenu);
    m_contextMenu = new QMenu(ui->contacts);
    m_contextMenu->addAction(icons()->icon("person.svg"), "New contact", [this]{
        this->newContact();
    });

    // row context menu
    m_rowMenu = new QMenu(ui->contacts);
    m_rowMenu->addAction(icons()->icon("copy.png"), "Copy address", this, &ContactsWidget::copyAddress);
    m_rowMenu->addAction(icons()->icon("copy.png"), "Copy name", this, &ContactsWidget::copyName);
    m_rowMenu->addAction("Pay to", this, &ContactsWidget::payTo);
    m_rowMenu->addAction("Delete", this, &ContactsWidget::deleteContact);

    connect(ui->contacts, &QTreeView::customContextMenuRequested, [=](const QPoint & point){
        QModelIndex index = ui->contacts->indexAt(point);
        if (index.isValid()) {
            m_rowMenu->exec(ui->contacts->viewport()->mapToGlobal(point));
        }
        else {
            m_contextMenu->exec(ui->contacts->viewport()->mapToGlobal(point));
        }
    });

    connect(ui->search, &QLineEdit::textChanged, this, &ContactsWidget::setSearchFilter);
}

void ContactsWidget::setSearchbarVisible(bool visible) {
    ui->search->setVisible(visible);
}

void ContactsWidget::focusSearchbar() {
    ui->search->setFocusPolicy(Qt::StrongFocus);
    ui->search->setFocus();
}

void ContactsWidget::copyAddress() {
    QModelIndex index = ui->contacts->currentIndex();
    ModelUtils::copyColumn(&index, AddressBookModel::Address);
}

void ContactsWidget::copyName() {
    QModelIndex index = ui->contacts->currentIndex();
    ModelUtils::copyColumn(&index, AddressBookModel::Description);
}

void ContactsWidget::payTo() {
    QModelIndex index = ui->contacts->currentIndex();
    QString address = index.model()->data(index.siblingAtColumn(AddressBookModel::Address), Qt::UserRole).toString();
    emit fillAddress(address);
}

void ContactsWidget::setShowFullAddresses(bool show) {
    m_model->setShowFullAddresses(show);
}

void ContactsWidget::setSearchFilter(const QString &filter) {
    if(!m_proxyModel) return;
    m_proxyModel->setSearchFilter(filter);
}

void ContactsWidget::showHeaderMenu(const QPoint& position)
{
    m_showFullAddressesAction->setChecked(m_model->isShowFullAddresses());
    m_headerMenu->exec(QCursor::pos());
}

void ContactsWidget::newContact(QString address, QString name)
{
    ContactsDialog dialog{this, address, name};
    int ret = dialog.exec();
    if (ret != QDialog::Accepted) {
        return;
    }

    address = dialog.getAddress();
    name = dialog.getName();

    bool addressValid = WalletManager::addressValid(address, m_ctx->wallet->nettype());
    if (!addressValid) {
        QMessageBox::warning(this, "Invalid address", "Invalid address");
        return;
    }

    int num_addresses = m_ctx->wallet->addressBook()->count();
    QString address_entry;
    QString name_entry;
    for (int i=0; i<num_addresses; i++) {
        m_ctx->wallet->addressBook()->getRow(i, [&address_entry, &name_entry](const AddressBookInfo &entry){
            address_entry = entry.address();
            name_entry = entry.description();
        });

        if (address == address_entry) {
            QMessageBox::warning(this, "Unable to add contact", "Duplicate address");
            ui->contacts->setCurrentIndex(m_model->index(i,0)); // Highlight duplicate address
            return;
        }
        if (name == name_entry) {
            QMessageBox::warning(this, "Unable to add contact", "Duplicate label");
            this->newContact(address, name);
            return;
        }
    }

    m_ctx->wallet->addressBook()->addRow(address, "", name);
}

void ContactsWidget::deleteContact()
{
    QModelIndex index = ui->contacts->currentIndex();
    m_model->deleteRow(m_proxyModel->mapToSource(index).row());
}

ContactsWidget::~ContactsWidget() = default;
