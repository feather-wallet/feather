// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include <QDialog>

#include "components.h"

#ifndef FEATHER_SEEDDICEDIALOG_H
#define FEATHER_SEEDDICEDIALOG_H

namespace Ui {
    class SeedDiceDialog;
}

class SeedDiceDialog : public WindowModalDialog
{
    Q_OBJECT

public:
    explicit SeedDiceDialog(QWidget *parent);
    ~SeedDiceDialog() override;

    const char* getSecret();

    const QString& getMnemonic();

private:
    void addFlip(bool heads);
    void addRoll(const QString &roll);
    double entropyPerRoll();
    bool validateRollEntry();

    void update();
    bool updateEntropy();
    void updateRolls();
    void updateRollsLeft();
    void setEnableMethodSelection(bool enabled);

    QScopedPointer<Ui::SeedDiceDialog> ui;
    QStringList m_rolls;
    QByteArray m_key;
    int entropyNeeded = 152; // Polyseed requests 19 bytes of random data
    QString m_mnemonic;
};


#endif //FEATHER_SEEDDICEDIALOG_H
