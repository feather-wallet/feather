// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "UrlListConfigureWidget.h"
#include "ui_UrlListConfigureWidget.h"

#include <QComboBox>
#include <QWidget>
#include <QInputDialog>

#include "dialog/MultiLineInputDialog.h"
#include "utils/config.h"
#include "utils/Utils.h"

UrlListConfigureWidget::UrlListConfigureWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::UrlListConfigureWidget)
{
    ui->setupUi(this);
}

void UrlListConfigureWidget::setup(const QString &what, Config::ConfigKey list, Config::ConfigKey preferred, const QStringList &keys) {
    m_what = what;
    m_listKey = list;
    m_preferredKey = preferred;
    m_keys = keys;

    this->setupComboBox();

    connect(ui->configure, &QPushButton::clicked, this, &UrlListConfigureWidget::onConfigureClicked);
    connect(ui->comboBox, &QComboBox::currentIndexChanged, this, &UrlListConfigureWidget::onUrlSelected);
}

void UrlListConfigureWidget::onConfigureClicked() {
    QStringList list = conf()->get(m_listKey).toStringList();
    QStringList newList;

    while (true) {
        auto input = MultiLineInputDialog(this, m_what, QString("Set %1 (one per line):").arg(m_what.toLower()), list);
        auto status = input.exec();

        if (status == QDialog::Rejected) {
            break;
        }

        newList = input.getList();
        newList.removeAll("");
        newList.removeDuplicates();

        bool error = false;
        for (const auto& item : newList) {
            auto url = QUrl::fromUserInput(item);
            qDebug() << url.scheme();
            if (url.scheme() != "http" && url.scheme() != "https") {
                Utils::showError(this, QString("Invalid %1 entered").arg(m_what.toLower()), QString("Invalid URL: %1").arg(item));
                error = true;
                break;
            }

            for (const auto& key : m_keys) {
                if (!item.contains(key)) {
                    Utils::showError(this, QString("Invalid %1 entered").arg(m_what.toLower()), QString("Key %1 missing from URL: %2").arg(key, item));
                    error = true;
                    break;
                }
            }
        }
        if (error) {
            list = newList;
            continue;
        }

        conf()->set(m_listKey, newList);
        this->setupComboBox();
        break;
    }
}

void UrlListConfigureWidget::setupComboBox() {
    m_urls = conf()->get(m_listKey).toStringList();

    QStringList cleanList;
    for (const auto &item : m_urls) {
        QUrl url(item);
        cleanList << url.host();
    }

    ui->comboBox->clear();
    ui->comboBox->insertItems(0, cleanList);

    if (m_urls.empty()) {
        return;
    }

    QString preferred = conf()->get(m_preferredKey).toString();
    if (m_urls.contains(preferred)) {
        ui->comboBox->setCurrentIndex(m_urls.indexOf(preferred));
    }
    else {
        conf()->set(m_preferredKey, m_urls.at(0));
    }
}

void UrlListConfigureWidget::onUrlSelected(int index) {
    if (index < 0 || index >= m_urls.length()) {
        return;
    }

    conf()->set(m_preferredKey, m_urls.at(index));
}

UrlListConfigureWidget::~UrlListConfigureWidget() = default;
