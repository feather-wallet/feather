// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#include "PageNetwork.h"
#include "ui_PageNetwork.h"

#include <QtConcurrent/QtConcurrent>

#include "constants.h"
#include "Utils.h"
#include "WalletWizard.h"

PageNetwork::PageNetwork(QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::PageNetwork)
    , m_portOpenWatcher(new QFutureWatcher<QPair<bool, QString>>(this))
{
    ui->setupUi(this);
    this->setTitle("Welcome to Feather");

    ui->frame_nodeDetected->hide();
    ui->frame_customNode->hide();

    ui->btnGroup_network->setId(ui->radio_autoConnect, Button::AUTO);
    ui->btnGroup_network->setId(ui->radio_custom, Button::CUSTOM);

    QPixmap infoIcon = QPixmap(":/assets/images/info2.svg");
    ui->infoIcon->setPixmap(infoIcon.scaledToWidth(32, Qt::SmoothTransformation));

    connect(ui->btnGroup_network, &QButtonGroup::idClicked, [this](int id) {
        ui->frame_customNode->setVisible(id == Button::CUSTOM);
    });
    connect(ui->line_customNode, &QLineEdit::textEdited, [this]{
        this->completeChanged();
    });

    connect(m_portOpenWatcher, &QFutureWatcher<QPair<bool, QString>>::finished, [this](){
        auto res = m_portOpenWatcher->result();
        bool nodeFound = res.first;
        if (nodeFound) {
            ui->frame_nodeDetected->show();
            ui->label_nodeDetected->setText(QString("Feather detected a local node on %1").arg(res.second));

            ui->btnGroup_network->button(Button::CUSTOM)->click();
            ui->line_customNode->setText(res.second);
        }
    });

    QFuture<QPair<bool, QString>> portOpen = QtConcurrent::run([]{
        QString localhost = "127.0.0.1";
        quint16 port = Utils::getDefaultRpcPort(constants::networkType);
        return QPair<bool, QString>{Utils::portOpen(localhost, port), QString("%1:%2").arg(localhost, QString::number(port))};
    });
    m_portOpenWatcher->setFuture(portOpen);
}

int PageNetwork::nextId() const {
    return WalletWizard::Page_NetworkTor;
}

bool PageNetwork::validatePage() {
    int id = ui->btnGroup_network->checkedId();
    config()->set(Config::nodeSource, id);

    if (id == Button::CUSTOM) {
        NodeList nodeList;
        nodeList.addNode(ui->line_customNode->text(), constants::networkType, NodeList::Type::custom);
    }

    return true;
}

bool PageNetwork::isComplete() const {
    if (ui->btnGroup_network->checkedId() == Button::AUTO) {
        return true;
    }

    FeatherNode node{ui->line_customNode->text()};
    return node.isValid();
}