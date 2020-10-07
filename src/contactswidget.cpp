// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "contactswidget.h"
#include "ui_contactswidget.h"
#include "dialog/contactsdialog.h"
#include "model/ModelUtils.h"
#include "utils/utils.h"

#include <QClipboard>
#include <QDebug>
#include <QKeyEvent>

ContactsWidget::ContactsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ContactsWidget)
{
    ui->setupUi(this);

    // header context menu
    ui->contacts->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    m_headerMenu = new QMenu(this);
    m_showFullAddressesAction = m_headerMenu->addAction("Show full addresses", this, &ContactsWidget::setShowFullAddresses);
    m_showFullAddressesAction->setCheckable(true);

    connect(ui->contacts->header(), &QHeaderView::customContextMenuRequested, this, &ContactsWidget::showHeaderMenu);

    // context menu
    ui->contacts->setContextMenuPolicy(Qt::CustomContextMenu);
    m_contextMenu = new QMenu(ui->contacts);
    m_contextMenu->addAction(QIcon(":/assets/images/person.svg"), "New contact", this, &ContactsWidget::newContact);


    // row context menu
    m_rowMenu = new QMenu(ui->contacts);
    m_rowMenu->addAction(QIcon(":/assets/images/copy.png"), "Copy address", this, &ContactsWidget::copyAddress);
    m_rowMenu->addAction(QIcon(":/assets/images/copy.png"), "Copy name", this, &ContactsWidget::copyName);
    m_rowMenu->addAction(QIcon(":/assets/images/appicons/128x128.png"), "Pay to", this, &ContactsWidget::payTo);
    m_deleteEntryAction = m_rowMenu->addAction("Delete", this, &ContactsWidget::deleteContact);

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

void ContactsWidget::showHeaderMenu(const QPoint& position)
{
    m_showFullAddressesAction->setChecked(m_model->isShowFullAddresses());
    m_headerMenu->exec(QCursor::pos());
}

void ContactsWidget::newContact()
{
    auto * dialog = new ContactsDialog(this);
    int ret = dialog->exec();
    if (!ret) return;

    QString address = dialog->getAddress();
    QString name = dialog->getName();

    emit addContact(address, name);
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
