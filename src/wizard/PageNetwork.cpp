// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "PageNetwork.h"
#include "ui_PageNetwork.h"
#include "WalletWizard.h"

PageNetwork::PageNetwork(AppContext *ctx, QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::PageNetwork)
    , m_ctx(ctx)
{
    ui->setupUi(this);
    this->setTitle("Welcome to Feather");

    ui->frame_customNode->hide();

    ui->btnGroup_network->setId(ui->radio_autoConnect, 0);
    ui->btnGroup_network->setId(ui->radio_custom, 1);

    connect(ui->btnGroup_network, &QButtonGroup::idClicked, [this](int id) {
        ui->frame_customNode->setVisible(id == 1);
    });
    connect(ui->line_customNode, &QLineEdit::textEdited, [this]{
        this->completeChanged();
    });
}

int PageNetwork::nextId() const {
    return WalletWizard::Page_NetworkTor;
}

bool PageNetwork::validatePage() {
    int id = ui->btnGroup_network->checkedId();
    config()->set(Config::nodeSource, id);

    if (id == 1) {
        QList<FeatherNode> nodes;
        FeatherNode node{ui->line_customNode->text()};
        nodes.append(node);
        m_ctx->nodes->setCustomNodes(nodes);
    }

    return true;
}

bool PageNetwork::isComplete() const {
    if (ui->btnGroup_network->checkedId() == 0) {
        return true;
    }

    FeatherNode node{ui->line_customNode->text()};
    return node.isValid();
}