// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "RevuoWidget.h"
#include "ui_RevuoWidget.h"

#include "utils/ColorScheme.h"
#include "Utils.h"
#include "utils/WebsocketNotifier.h"

RevuoWidget::RevuoWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::RevuoWidget)
        , m_contextMenu(new QMenu(this))
{
    ui->setupUi(this);

    ui->textBrowser->setOpenLinks(false);
    ui->textBrowser->document()->setDefaultStyleSheet("a {color: white; }");
    connect(ui->textBrowser, &QTextBrowser::anchorClicked, this, &RevuoWidget::onLinkActivated);

    ui->textBrowser->setText("<h4>No item selected</h4>");

    m_contextMenu->addAction("Open link", this, &RevuoWidget::onOpenLink);
    m_contextMenu->addAction("Donate to author", this, &RevuoWidget::onDonate);

    ui->splitter->setStretchFactor(1, 5);

    connect(ui->listWidget, &QListWidget::currentTextChanged, this, &RevuoWidget::onSelectItem);
    connect(ui->listWidget, &QListWidget::customContextMenuRequested, this, &RevuoWidget::showContextMenu);

    connect(websocketNotifier(), &WebsocketNotifier::dataReceived, this, [this](const QString& type, const QJsonValue& json) {
        if (type == "revuo") {
            QJsonArray revuo_data = json.toArray();
            QList<QSharedPointer<RevuoItem>> l;

            for (const auto &entry: revuo_data) {
                auto obj = entry.toObject();

                QStringList newsbytes;
                for (const auto &n : obj.value("newsbytes").toArray()) {
                    newsbytes.append(n.toString());
                }

                auto revuoItem = new RevuoItem(
                        obj.value("title").toString(),
                        obj.value("url").toString(),
                        newsbytes);

                QSharedPointer<RevuoItem> r = QSharedPointer<RevuoItem>(revuoItem);
                l.append(r);
            }

            this->updateItems(l);
        }
    });
}

void RevuoWidget::updateItems(const QList<QSharedPointer<RevuoItem>> &items) {
    QStringList titles;
    for (const auto &item : items) {
        titles << item->title;

        QString text = "<h3>Recent News</h3>\n";
        for (const auto &newsbyte : item->newsbytes) {
            text += "<p> • " + newsbyte + "</p>\n";
        }
        text += QString("<br>\nRead the whole issue in your <a href=\"%1\">browser</a>.").arg(item->url);
        text += "<br><br>\nEnjoy Revuo? Consider a <a href=\"feather://donate-revuo\">donation</a> to the author.";

        m_items[item->title] = text;
        m_links[item->title] = item->url;
    }

    ui->listWidget->clear();
    ui->listWidget->addItems(titles);
    ui->listWidget->setCurrentRow(0);
    ui->listWidget->setMinimumWidth(ui->listWidget->sizeHintForColumn(0) + 10);
}

void RevuoWidget::onSelectItem(const QString &item) {
    auto *currentItem = ui->listWidget->currentItem();
    if (currentItem == nullptr) {
        return;
    }
    QString title = currentItem->text();
    ui->textBrowser->setText(m_items[title]);
}

void RevuoWidget::onLinkActivated(const QUrl &link) {
    if (link.host() == "donate-revuo") {
        this->onDonate();
        return;
    }

    Utils::externalLinkWarning(this, link.toString());
}

void RevuoWidget::showContextMenu(const QPoint &pos) {
    m_contextMenu->exec(ui->listWidget->viewport()->mapToGlobal(pos));
}

void RevuoWidget::onOpenLink() {
    QString currentItem = ui->listWidget->currentItem()->text();
    Utils::externalLinkWarning(this, m_links[currentItem]);
}

void RevuoWidget::onDonate() {
    emit donate("89Esx7ZAoVcD9wiDw57gxgS7m52sFEEbQiFC4qq18YZy3CdcsXvJ67FYdcDFbmYEGK7xerxgmDptd1C2xLstCbgF3RUhSMT", "Donation to Revuo Monero");
}

void RevuoWidget::skinChanged() {
    QString color = "black";
    if (ColorScheme::hasDarkBackground(this)) {
        color = "white";
    }
    auto stylesheet = QString("a {color: %1; }").arg(color);

    ui->textBrowser->document()->setDefaultStyleSheet(stylesheet);
    this->onSelectItem("");
}

RevuoWidget::~RevuoWidget() = default;