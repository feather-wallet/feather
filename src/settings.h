// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QSettings>
#include <QAbstractButton>

#include "appcontext.h"
#include "widgets/nodewidget.h"

namespace Ui {
    class Settings;
}

class Settings : public QDialog
{
Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings() override;

signals:
    void closed();
    void preferredFiatCurrencyChanged(QString currency);
    void skinChanged(QString skinName);
    void showHomeCCS(bool);
    void blockExplorerChanged(QString blockExplorer);

public slots:
    void copyToClipboard();
    void checkboxExternalLinkWarn();
    void fiatCurrencySelected(int index);
    void comboBox_skinChanged(int pos);
    void comboBox_blockExplorerChanged(int post);

private:
    QStringList m_skins{"Native", "QDarkStyle", "Breeze/Dark", "Breeze/Light"};

private:
    void setupSkinCombobox();

    AppContext *m_ctx;
    Ui::Settings *ui;
};

#endif // SETTINGS_H
