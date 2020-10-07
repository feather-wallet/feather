// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_WIZARDMENU_H
#define FEATHER_WIZARDMENU_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>

#include "appcontext.h"

namespace Ui {
    class MenuPage;
}

class MenuPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit MenuPage(AppContext *ctx, QWidget *parent = nullptr);
    bool validatePage() override;
    int nextId() const override;

private:
    AppContext *m_ctx;
    QLabel *topLabel;
    Ui::MenuPage *ui;
};

#endif //FEATHER_WIZARDMENU_H
