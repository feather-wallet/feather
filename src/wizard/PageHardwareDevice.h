// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_PAGEHARDWAREDEVICE_H
#define FEATHER_PAGEHARDWAREDEVICE_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>
#include <QDir>

#include "WalletWizard.h"

namespace Ui {
    class PageHardwareDevice;
}

class PageHardwareDevice : public QWizardPage
{
Q_OBJECT

public:
    explicit PageHardwareDevice(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;
    bool isComplete() const override;

private:
    void onOptionsClicked();

    Ui::PageHardwareDevice *ui;
    WizardFields *m_fields;
};


#endif //FEATHER_PAGEHARDWAREDEVICE_H
