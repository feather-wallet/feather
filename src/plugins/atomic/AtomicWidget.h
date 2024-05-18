// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_ATOMICWIDGET_H
#define FEATHER_ATOMICWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QTimer>

namespace Ui {
    class AtomicWidget;
}

class AtomicWidget : public QWidget
{
Q_OBJECT

public:
    explicit AtomicWidget(QWidget *parent = nullptr);
    ~AtomicWidget() override;

public slots:
    void skinChanged();

private slots:
    void initComboBox();
    void showAtomicConfigureDialog();
    void onPricesReceived();

private:
    void convert(bool reverse);
    void setupComboBox(QComboBox *comboBox, const QStringList &crypto, const QStringList &fiat);
    void updateStatus();

    QScopedPointer<Ui::AtomicWidget> ui;
    bool m_comboBoxInit = false;
    QTimer m_statusTimer;
};

#endif // FEATHER_ATOMICWIDGET_H
