// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

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
    explicit Settings(QSharedPointer<AppContext> ctx, QWidget *parent = nullptr);
    ~Settings() override;

signals:
    void closed();
    void preferredFiatCurrencyChanged(QString currency);
    void skinChanged(QString skinName);
    void showHomeCCS(bool);
    void blockExplorerChanged(QString blockExplorer);
    void amountPrecisionChanged(int precision);

public slots:
    void updatePaths();
    void copyToClipboard();
    void checkboxExternalLinkWarn();
    void fiatCurrencySelected(int index);
    void comboBox_skinChanged(int pos);
    void comboBox_amountPrecisionChanged(int pos);
    void comboBox_dateFormatChanged(int pos);
    void comboBox_timeFormatChanged(int pos);

    void comboBox_blockExplorerChanged(int pos);
    void comboBox_redditFrontendChanged(int pos);
    void comboBox_localMoneroFrontendChanged(int pos);

private:
    void setupSkinCombobox();
    void setupLocalMoneroFrontendCombobox();

    Ui::Settings *ui;
    QSharedPointer<AppContext> m_ctx;

    QStringList m_skins{"Native", "QDarkStyle", "Breeze/Dark", "Breeze/Light"};
    QStringList m_dateFormats{"yyyy-MM-dd", "MM-dd-yyyy", "dd-MM-yyyy"};
    QStringList m_timeFormats{"hh:mm", "hh:mm ap"};
};

#endif // SETTINGS_H
