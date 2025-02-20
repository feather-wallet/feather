// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef PAGEPLUGINS_H
#define PAGEPLUGINS_H

#include <QWizardPage>

namespace Ui {
    class PagePlugins;
}

class PagePlugins : public QWizardPage
{
    Q_OBJECT

public:
    explicit PagePlugins(QWidget *parent = nullptr);
    bool validatePage() override;
    int nextId() const override;
    bool isComplete() const override;

private:
    Ui::PagePlugins *ui;
};

#endif //PAGEPLUGINS_H
