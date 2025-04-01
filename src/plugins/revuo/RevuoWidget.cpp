// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "RevuoWidget.h"
#include "ui_RevuoWidget.h"

#include <QJsonArray>

#include "utils/ColorScheme.h"
#include "Utils.h"
#include "utils/Icons.h"
#include "utils/WebsocketNotifier.h"

RevuoWidget::RevuoWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::RevuoWidget)
{
    ui->setupUi(this);

    ui->textBrowser->setOpenLinks(false);
    ui->textBrowser->document()->setDefaultStyleSheet("a {color: white; }");
    ui->textBrowser->setText("<h4>No item selected</h4>");
    connect(ui->textBrowser, &QTextBrowser::anchorClicked, this, &RevuoWidget::onLinkActivated);

    ui->btn_openLink->setIcon(icons()->icon("external-link.svg"));
    connect(ui->btn_openLink, &QPushButton::clicked, this, &RevuoWidget::onOpenLink);

    connect(ui->combo_issue, &QComboBox::currentIndexChanged, this, &RevuoWidget::onSelectItem);

    connect(websocketNotifier(), &WebsocketNotifier::dataReceived, this, [this](const QString& type, const QJsonValue& json) {
        if (type == "revuo") {
            QJsonArray revuo_data = json.toArray();
            QList<QSharedPointer<RevuoItem>> l;

            for (const auto &entry: revuo_data) {
                auto obj = entry.toObject();

                QSharedPointer<RevuoItem> item = QSharedPointer<RevuoItem>(new RevuoItem(this));

                for (const auto &n : obj.value("newsbytes").toArray()) {
                    item->newsbytes.append(n.toString());
                }

                for (const auto &e : obj.value("events").toArray()) {
                    auto f = e.toObject();
                    item->events.append({f.value("date").toString(), f.value("description").toString()});
                }

                item->title = obj.value("title").toString();
                item->url = obj.value("url").toString();

                l.append(item);
            }

            this->updateItems(l);
        }
    });
}

void RevuoWidget::updateItems(const QList<QSharedPointer<RevuoItem>> &items) {
    m_items.clear();
    m_links.clear();
    ui->combo_issue->clear();

    QStringList titles;
    for (const auto &item : items) {
        titles << item->title;

        QString text = "<h3>Recent News</h3>\n";
        for (const auto &newsbyte : item->newsbytes) {
            text += "<p> • " + newsbyte + "</p>\n";
        }
        text += "╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍\n";
        text += "<h3>Upcoming Events</h3>\n";
        if (item->events.isEmpty()) {
            text += "<p>There are no upcoming events.</p>\n";
        }
        for (const auto &event : item->events) {
            text += "<h4>" + event.first + "</h4>\n";
            text += "<p>" + event.second + "</p>\n";
        }
        text += "<hr>";
        text += QString("Read the whole issue in your <a href=\"%1\">browser</a>.").arg(item->url);
        text += "<br><br>\nEnjoy Revuo? Consider a <a href=\"feather://donate-revuo\">donation</a> to the author.";

        m_items.append(text);
        m_links.append(item->url);
        ui->combo_issue->addItem(item->title);
    }

    ui->combo_issue->setCurrentIndex(0);

}

void RevuoWidget::onSelectItem(int index) {
    if (index >= m_items.length() || index < 0) {
        ui->textBrowser->setText("<h4>No item selected</h4>");
        return;
    }

    ui->textBrowser->setText(m_items[index]);
}

void RevuoWidget::onLinkActivated(const QUrl &link) {
    if (link.host() == "donate-revuo") {
        this->onDonate();
        return;
    }

    Utils::externalLinkWarning(this, link.toString());
}

void RevuoWidget::onOpenLink() {
    int currentItem = ui->combo_issue->currentIndex();
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
    this->onSelectItem(0);
}

RevuoWidget::~RevuoWidget() = default;
