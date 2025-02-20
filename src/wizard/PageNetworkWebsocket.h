// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_PAGENETWORKWEBSOCKET_H
#define FEATHER_PAGENETWORKWEBSOCKET_H

#include <QWizardPage>

namespace Ui {
    class PageNetworkWebsocket;
}

class PageNetworkWebsocket : public QWizardPage
{
Q_OBJECT

public:
    explicit PageNetworkWebsocket(QWidget *parent = nullptr);
    bool validatePage() override;
    int nextId() const override;

signals:
    void initialNetworkConfigured();

private:
    Ui::PageNetworkWebsocket *ui;
};

#endif //FEATHER_PAGENETWORKWEBSOCKET_H
