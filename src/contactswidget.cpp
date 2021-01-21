// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "contactswidget.h"
#include "ui_contactswidget.h"
#include "dialog/contactsdialog.h"
#include "model/ModelUtils.h"
#include "mainwindow.h"
#include "libwalletqt/AddressBook.h"

#include <QMessageBox>

ContactsWidget::ContactsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ContactsWidget)
{
    ui->setupUi(this);
    m_ctx = MainWindow::getContext();

    // header context menu
    ui->contacts->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    m_headerMenu = new QMenu(this);
    m_showFullAddressesAction = m_headerMenu->addAction("Show full addresses", this, &ContactsWidget::setShowFullAddresses);
    m_showFullAddressesAction->setCheckable(true);

    connect(ui->contacts->header(), &QHeaderView::customContextMenuRequested, this, &ContactsWidget::showHeaderMenu);

    // context menu
    ui->contacts->setContextMenuPolicy(Qt::CustomContextMenu);
    m_contextMenu = new QMenu(ui->contacts);
    m_contextMenu->addAction(QIcon(":/assets/images/person.svg"), "New contact", [this]{
        this->newContact();
    });

    // row context menu
    m_rowMenu = new QMenu(ui->contacts);
    m_rowMenu->addAction(QIcon(":/assets/images/copy.png"), "Copy address", this, &ContactsWidget::copyAddress);
    m_rowMenu->addAction(QIcon(":/assets/images/copy.png"), "Copy name", this, &ContactsWidget::copyName);
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

void ContactsWidget::setModel(AddressBookModel *model)
{
    m_model = model;
    m_proxyModel = new AddressBookProxyModel;
    m_proxyModel->setSourceModel(m_model);
    ui->contacts->setModel(m_proxyModel);

    ui->contacts->setSortingEnabled(true);
    ui->contacts->header()->setSectionResizeMode(AddressBookModel::Address, QHeaderView::Stretch);
    ui->contacts->header()->setSectionResizeMode(AddressBookModel::Description, QHeaderView::ResizeToContents);
    ui->contacts->header()->setMinimumSectionSize(200);
}

void ContactsWidget::setShowFullAddresses(bool show) {
    m_model->setShowFullAddresses(show);
}

void ContactsWidget::setSearchFilter(const QString &filter) {
    if(!m_proxyModel) return;
    m_proxyModel->setSearchFilter(filter);
}

void ContactsWidget::resetModel()
{
    ui->contacts->setModel(nullptr);
}

void ContactsWidget::showHeaderMenu(const QPoint& position)
{
    m_showFullAddressesAction->setChecked(m_model->isShowFullAddresses());
    m_headerMenu->exec(QCursor::pos());
}

void ContactsWidget::newContact(QString address, QString name)
{
    auto * dialog = new ContactsDialog(this, address, name);
    int ret = dialog->exec();
    if (!ret) return;

    address = dialog->getAddress();
    name = dialog->getName();

    bool addressValid = WalletManager::addressValid(address, m_ctx->currentWallet->nettype());
    if (!addressValid) {
        QMessageBox::warning(this, "Invalid address", "Invalid address");
        return;
    }

    int num_addresses = m_ctx->currentWallet->addressBook()->count();
    QString address_entry;
    QString name_entry;
    for (int i=0; i<num_addresses; i++) {
        m_ctx->currentWallet->addressBook()->getRow(i, [&address_entry, &name_entry](const AddressBookInfo &entry){
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

    m_ctx->currentWallet->addressBook()->addRow(address, "", name);
}

void ContactsWidget::deleteContact()
{
    QModelIndex index = ui->contacts->currentIndex();
    m_model->deleteRow(m_proxyModel->mapToSource(index).row());
}

ContactsWidget::~ContactsWidget()
{
    delete ui;
}
