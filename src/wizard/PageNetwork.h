// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_WIZARDNETWORK_H
#define FEATHER_WIZARDNETWORK_H

#include <QWizardPage>
#include <QFutureWatcher>

namespace Ui {
    class PageNetwork;
}

class PageNetwork : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageNetwork(QWidget *parent = nullptr);
    bool validatePage() override;
    int nextId() const override;
    bool isComplete() const override;

private:
    enum Button {
        AUTO=0,
        CUSTOM
    };

    Ui::PageNetwork *ui;
    QFutureWatcher<QPair<bool, QString>> *m_portOpenWatcher;
};

#endif //FEATHER_WIZARDNETWORK_H
