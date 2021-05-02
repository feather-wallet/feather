// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

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
    explicit PageNetworkTor(AppContext *ctx, QWidget *parent = nullptr);
    bool validatePage() override;
    int nextId() const override;

signals:
    void initialNetworkConfigured();

private:
    AppContext *m_ctx;
    Ui::PageNetworkTor *ui;
};

#endif //FEATHER_PAGENETWORKTOR_H
