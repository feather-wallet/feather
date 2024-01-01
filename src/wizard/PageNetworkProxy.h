// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_PageNetworkProxy_H
#define FEATHER_PageNetworkProxy_H

#include <QWizardPage>

namespace Ui {
    class PageNetworkProxy;
}

class PageNetworkProxy : public QWizardPage
{
Q_OBJECT

public:
    explicit PageNetworkProxy(QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;

signals:
    void initialNetworkConfigured();

private:
    Ui::PageNetworkProxy *ui;
};

#endif //FEATHER_PageNetworkProxy_H
