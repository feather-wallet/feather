// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "CCSWidget.h"
#include "ui_CCSWidget.h"

#include <QTableWidget>
#include <QJsonArray>

#include "CCSProgressDelegate.h"
#include "utils/Utils.h"
#include "utils/WebsocketNotifier.h"

CCSWidget::CCSWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::CSSWidget)
        , m_model(new CCSModel(this))
        , m_contextMenu(new QMenu(this))
{
    ui->setupUi(this);
    ui->treeView->setModel(m_model);

    m_contextMenu->addAction("View proposal", this, &CCSWidget::linkClicked);
    m_contextMenu->addAction("Donate", this, &CCSWidget::donateClicked);

    connect(ui->treeView, &QHeaderView::customContextMenuRequested, this, &CCSWidget::showContextMenu);
    connect(ui->treeView, &QTreeView::doubleClicked, this, &CCSWidget::linkClicked);

    ui->treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeView->header()->setSectionResizeMode(CCSModel::Title, QHeaderView::Stretch);

    connect(websocketNotifier(), &WebsocketNotifier::dataReceived, this, [this](const QString& type, const QJsonValue& json) {
        if (type == "ccs") {
            QJsonArray ccs_data = json.toArray();
            QList<QSharedPointer<CCSEntry>> l;

            for (const auto& entry: ccs_data) {
                auto obj = entry.toObject();
                auto c = QSharedPointer<CCSEntry>(new CCSEntry());

                if (obj.value("state").toString() != "FUNDING-REQUIRED")
                    continue;

                c->state = obj.value("state").toString();
                c->address = obj.value("address").toString();
                c->author = obj.value("author").toString();
                c->date = obj.value("date").toString();
                c->title = obj.value("title").toString();
                c->target_amount = obj.value("target_amount").toDouble();
                c->raised_amount = obj.value("raised_amount").toDouble();
                c->percentage_funded = obj.value("percentage_funded").toDouble();
                c->contributions = obj.value("contributions").toInt();
                c->organizer = obj.value("organizer").toString();
                c->currency = obj.value("currency").toString();

                QString urlpath = obj.value("urlpath").toString();
                if (c->organizer == "CCS") {
                    c->url = QString("https://ccs.getmonero.org/%1").arg(urlpath);
                }
                else if (c->organizer == "MAGIC") {
                    c->url = QString("https://monerofund.org/%1").arg(urlpath);
                }
                else {
                    continue;
                }

                l.append(c);
            }

            m_model->updateEntries(l);
        }
    });
}

void CCSWidget::linkClicked() {
    QModelIndex index = ui->treeView->currentIndex();
    auto entry = m_model->entry(index.row());

    if (entry) {
        Utils::externalLinkWarning(this, entry->url);
    }
}

void CCSWidget::donateClicked() {
    QModelIndex index = ui->treeView->currentIndex();
    auto entry = m_model->entry(index.row());

    if (entry) {
        emit fillSendTab(entry->address, QString("Donation to %1: %2").arg(entry->organizer, entry->title));
    }
}

void CCSWidget::showContextMenu(const QPoint &pos) {
    QModelIndex index = ui->treeView->indexAt(pos);
        if (!index.isValid()) {
        return;
    }

    m_contextMenu->exec(ui->treeView->viewport()->mapToGlobal(pos));
}

CCSWidget::~CCSWidget() = default;