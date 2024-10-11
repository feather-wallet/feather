// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_URLLISTCONFIGUREWIDGET_H
#define FEATHER_URLLISTCONFIGUREWIDGET_H

#include <QWidget>

#include "utils/config.h"

namespace Ui {
    class UrlListConfigureWidget;
}

class UrlListConfigureWidget : public QWidget
{
Q_OBJECT

public:
    explicit UrlListConfigureWidget(QWidget *parent = nullptr);
    ~UrlListConfigureWidget() override;

    void setup(const QString &what, Config::ConfigKey list, Config::ConfigKey preferred, const QStringList& keys);

private slots:
    void onConfigureClicked();
    void onUrlSelected(int index);

private:
    void setupComboBox();

    QScopedPointer<Ui::UrlListConfigureWidget> ui;

    QString m_what;
    Config::ConfigKey m_listKey;
    Config::ConfigKey m_preferredKey;
    QStringList m_keys;
    QStringList m_urls;
};

#endif //FEATHER_URLLISTCONFIGUREWIDGET_H
