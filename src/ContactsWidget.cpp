// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "ContactsWidget.h"
#include "ui_ContactsWidget.h"

#include <QMessageBox>
#include <QFileDialog>

#include "dialog/ContactsDialog.h"
#include "model/AddressBookModel.h"
#include "model/AddressBookProxyModel.h"
#include "libwalletqt/AddressBook.h"
#include "libwalletqt/Wallet.h"
#include "libwalletqt/WalletManager.h"
#include "utils/Icons.h"
#include "utils/Utils.h"

ContactsWidget::ContactsWidget(Wallet *wallet, QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::ContactsWidget)
        , m_wallet(wallet)
{
    ui->setupUi(this);

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
    m_headerMenu->addAction("New contact", [this] {
        this->newContact();
    });
    m_headerMenu->addAction("Import CSV", this, &ContactsWidget::importCSV);
    m_headerMenu->addAction("Export CSV", this, &ContactsWidget::exportCSV);
    m_headerMenu->addSeparator();
    m_showFullAddressesAction = m_headerMenu->addAction("Show full addresses", this, &ContactsWidget::setShowFullAddresses);
    m_showFullAddressesAction->setCheckable(true);

    ui->btn_options->setMenu(m_headerMenu);

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
    ui->frame_search->setVisible(visible);
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
    QString description = index.model()->data(index.siblingAtColumn(AddressBookModel::Description), Qt::UserRole).toString();
    emit fill(address, description);
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

void ContactsWidget::importCSV() {
    const QString targetFile = QFileDialog::getOpenFileName(this, "Import CSV file", QDir::homePath(), "CSV Files (*.csv)");
    if(targetFile.isEmpty()) return;

    auto *model = m_wallet->addressBookModel();
    QMapIterator<QString, QString> i(model->readCSV(targetFile));
    int inserts = 0;
    while (i.hasNext()) {
        i.next();
        bool addressValid = WalletManager::addressValid(i.value(), m_wallet->nettype());
        if(addressValid) {
            m_wallet->addressBook()->addRow(i.value(), i.key());
            inserts++;
        }
    }

    Utils::showInfo(this, "Contacts imported", QString("Total contacts imported: %1").arg(inserts));
}

void ContactsWidget::exportCSV() {
    auto *model = m_wallet->addressBookModel();
    if (model->rowCount() <= 0){
        Utils::showInfo(this, "Unable to export contacts", "No contacts to export");
        return;
    }

    const QString targetDir = QFileDialog::getExistingDirectory(this, "Select CSV output directory ", QDir::homePath(), QFileDialog::ShowDirsOnly);
    if(targetDir.isEmpty()) return;

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    QString fn = QString("%1/monero-contacts_%2.csv").arg(targetDir, QString::number(now / 1000));
    if (model->writeCSV(fn)) {
        Utils::showInfo(this, "Contacts exported successfully", QString("Exported to: %1").arg(fn));
    }
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

    auto& rows = m_wallet->addressBook()->getRows();
    for (int i = 0; i < rows.size(); i++) {
        const ContactRow& row = rows[i];

        if (address == row.address) {
            Utils::showError(this, "Unable to add contact", "Address already exists in contacts", {}, "add_contact");
            QModelIndex sourceIndex = m_model->index(i, 0);
            ui->contacts->setCurrentIndex(m_proxyModel->mapFromSource(sourceIndex)); // Highlight duplicate address
            return;
        }

        if (name == row.label) {
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
