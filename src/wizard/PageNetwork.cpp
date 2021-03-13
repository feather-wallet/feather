// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "PageNetwork.h"
#include "ui_PageNetwork.h"

#include <QFileDialog>

// Unused for now
PageNetwork::PageNetwork(AppContext *ctx, QWidget *parent) :
        QWizardPage(parent),
        ui(new Ui::PageNetwork),
        m_ctx(ctx) {
    ui->setupUi(this);
    this->setTitle("Welcome to Feather!");

    ui->customFrame->hide();

    ui->label_eg->setText("Examples:\n- http://127.0.0.1:18089\n- my.node.com\n- my.node.com:18089\n- user:pass@my.node.com:18089");

    auto nodeSourceUInt = config()->get(Config::nodeSource).toUInt();
    auto nodeSource = static_cast<NodeSource>(nodeSourceUInt);
    if(nodeSource == NodeSource::websocket){
        ui->radioRemote->setChecked(true);
        ui->customFrame->hide();
        ui->remoteFrame->show();
    } else if(nodeSource == NodeSource::custom) {
        ui->radioCustom->setChecked(true);
        ui->remoteFrame->hide();
        ui->customFrame->show();
    }

    connect(ui->networkBtnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), [=](QAbstractButton *button) {
        auto name = button->objectName();
        if(name == "radioRemote") {
            ui->customFrame->hide();
            ui->remoteFrame->show();
        } else if(name == "radioCustom") {
            ui->remoteFrame->hide();
            ui->customFrame->show();
        }
    });
}

int PageNetwork::nextId() const {
    return 0;
}

bool PageNetwork::validatePage() {
    auto cfg = config()->get(Config::nodeSource);
    if(ui->radioRemote->isChecked()) {
        if(cfg != NodeSource::websocket)
            m_ctx->nodeSourceChanged(NodeSource::websocket);
    } else if (ui->radioCustom->isChecked()) {
        if(cfg != NodeSource::custom)
            m_ctx->nodeSourceChanged(NodeSource::custom);
        auto nodeText = ui->lineEdit_customNode->text().trimmed();
        if(!nodeText.isEmpty()) {
            auto customNodes = m_ctx->nodes->customNodes();
            auto node = FeatherNode(nodeText);
            customNodes.append(node);
            m_ctx->setCustomNodes(customNodes);
        }

    }
    return true;
}