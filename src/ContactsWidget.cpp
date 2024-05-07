// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "ContactsWidget.h"
#include "ui_ContactsWidget.h"

#include <QMessageBox>

#include "dialog/ContactsDialog.h"
#include "libwalletqt/AddressBook.h"
#include "libwalletqt/WalletManager.h"
#include "utils/Icons.h"
#include "utils/Utils.h"

ContactsWidget::ContactsWidget(Wallet *wallet, QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::ContactsWidget)
        , m_wallet(wallet)
{
    ui->setupUi(this);

    m_btn_addContact = new QPushButton(this);
    m_btn_addContact->setIcon(icons()->icon("register.svg"));
    ui->searchLayout->addWidget(m_btn_addContact, 0, Qt::AlignRight);
    connect(m_btn_addContact, &QPushButton::clicked, [this]{this->newContact();});

    m_model = m_wallet->addressBookModel();
    m_proxyModel = new AddressBookProxyModel;
    m_proxyModel->setSourceModel(m_model);
    ui->contacts->setModel(m_proxyModel);

    ui->contacts->setSortingEnabled(true);
    ui->contacts->header()->setSectionResizeMode(AddressBookModel::Address, QHeaderView::ResizeToContents);
    ui->contacts->header()->setSectionResizeMode(AddressBookModel::Description, QHeaderView::Stretch);
    ui->contacts->header()->setMinimumSectionSize(200);

    // header context menu
    ui->contacts->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    m_headerMenu = new QMenu(this);
    m_showFullAddressesAction = m_headerMenu->addAction("Show full addresses", this, &ContactsWidget::setShowFullAddresses);
    m_showFullAddressesAction->setCheckable(true);

    connect(ui->contacts->header(), &QHeaderView::customContextMenuRequested, this, &ContactsWidget::showHeaderMenu);

    connect(ui->contacts, &QTreeView::doubleClicked, [this](QModelIndex index){
        if (!(m_model->flags(index) & Qt::ItemIsEditable)) {
            this->payTo();
        }
    });

    // context menu
    ui->contacts->setContextMenuPolicy(Qt::CustomContextMenu);
    m_contextMenu = new QMenu(ui->contacts);
    m_contextMenu->addAction("New contact", [this]{
        this->newContact();
    });

    // row context menu
    m_rowMenu = new QMenu(ui->contacts);
    m_rowMenu->addAction("Copy address", this, &ContactsWidget::copyAddress);
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
    Utils::copyColumn(&index, AddressBookModel::Address);
}

void ContactsWidget::copyName() {
    QModelIndex index = ui->contacts->currentIndex();
    Utils::copyColumn(&index, AddressBookModel::Description);
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

    bool addressValid = WalletManager::addressValid(address, m_wallet->nettype());
    bool nameIsAddress = WalletManager::addressValid(name, m_wallet->nettype());

    if (addressValid && nameIsAddress) {
        Utils::showError(this, "Unable to add contact", "Name can not be an address", {}, "add_contact");
        return;
    }

    if (!addressValid && !nameIsAddress) {
        Utils::showError(this, "Unable to add contact", "Invalid address", {"Use 'Tools -> Address checker' to check if the address is valid."}, "add_contact");
        return;
    }

    if (!addressValid && nameIsAddress) {
        // User accidentally swapped name and address, allow it
        std::swap(address, name);
    }

    int num_addresses = m_wallet->addressBook()->count();
    QString address_entry;
    QString name_entry;
    for (int i=0; i<num_addresses; i++) {
        m_wallet->addressBook()->getRow(i, [&address_entry, &name_entry](const ContactRow &entry){
            address_entry = entry.getAddress();
            name_entry = entry.getLabel();
        });

        if (address == address_entry) {
            Utils::showError(this, "Unable to add contact", "Address already exists in contacts", {}, "add_contact");
            ui->contacts->setCurrentIndex(m_model->index(i,0)); // Highlight duplicate address
            return;
        }
        if (name == name_entry) {
            Utils::showError(this, "Unable to add contact", "Label already exists in contacts", {}, "add_contact");
            this->newContact(address, name);
            return;
        }
    }

    m_wallet->addressBook()->addRow(address, name);
}

void ContactsWidget::deleteContact()
{
    QModelIndex index = ui->contacts->currentIndex();
    m_model->deleteRow(m_proxyModel->mapToSource(index).row());
}

ContactsWidget::~ContactsWidget() = default;
