// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_PAGENETWORKTOR_H
#define FEATHER_PAGENETWORKTOR_H

#include <QWizardPage>

#include "appcontext.h"

namespace Ui {
    class PageNetworkTor;
}

class PageNetworkTor : public QWizardPage
{
Q_OBJECT

public:
    explicit PageNetworkTor(QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;

signals:
    void initialNetworkConfigured();

private:
    Ui::PageNetworkTor *ui;
};

#endif //FEATHER_PAGENETWORKTOR_H
