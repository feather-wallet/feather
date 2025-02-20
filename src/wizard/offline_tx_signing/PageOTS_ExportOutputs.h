// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_PAGEOTS_EXPORTOUTPUTS_H
#define FEATHER_PAGEOTS_EXPORTOUTPUTS_H

#include <QWizardPage>
#include <QCheckBox>
#include "Wallet.h"

namespace Ui {
    class PageOTS_Export;
}

class PageOTS_ExportOutputs : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageOTS_ExportOutputs(QWidget *parent, Wallet *wallet);
    void initializePage() override;
    [[nodiscard]] int nextId() const override;

private slots:
    void exportOutputs();

private:
    void setupUR(bool all);
    
    Ui::PageOTS_Export *ui;
    QCheckBox *m_check_exportAll;
    Wallet *m_wallet;
};


#endif //FEATHER_PAGEOTS_EXPORTOUTPUTS_H
