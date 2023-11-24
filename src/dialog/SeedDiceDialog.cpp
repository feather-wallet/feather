// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "SeedDiceDialog.h"
#include "ui_SeedDiceDialog.h"

#include <cmath>
#include <algorithm>

#include <QPasswordDigestor>

#include "utils/Seed.h"

SeedDiceDialog::SeedDiceDialog(QWidget *parent)
        : WindowModalDialog(parent)
        , ui(new Ui::SeedDiceDialog)
{
    ui->setupUi(this);

    ui->frame_dice->hide();
    ui->frame_coinflip->hide();

    connect(ui->radio_dice, &QRadioButton::toggled, [this](bool toggled){
        ui->frame_dice->setVisible(toggled);
        this->updateRollsLeft();
        ui->label_rollsLeft2->setText("Rolls left:");
        ui->label_rolls->setText("Rolls:");
    });

    connect(ui->radio_coinflip, &QRadioButton::toggled, [this](bool toggled){
        ui->frame_coinflip->setVisible(toggled);
        this->updateRollsLeft();
        ui->label_rollsLeft2->setText("Flips left:");
        ui->label_rolls->setText("Flips:");
    });

    connect(ui->spin_sides, &QSpinBox::valueChanged, [this](int value){
       if (!ui->radio_dice->isChecked()) {
           return;
       }
       this->updateRollsLeft();
    });

    connect(ui->line_roll, &QLineEdit::textChanged, this, &SeedDiceDialog::validateRollEntry);

    connect(ui->btn_next, &QPushButton::clicked, [this]{
        this->setEnableMethodSelection(false);

        if (!this->validateRollEntry()) {
            return;
        }

        QStringList rolls = ui->line_roll->text().simplified().split(" ");
        for (const auto &roll : rolls) {
            this->addRoll(roll);
        }

        ui->line_roll->clear();
        ui->line_roll->setFocus();
    });

    connect(ui->btn_heads, &QPushButton::clicked, [this]{
        this->setEnableMethodSelection(false);
        this->addFlip(true);
    });

    connect(ui->btn_tails, &QPushButton::clicked, [this]{
        this->setEnableMethodSelection(false);
        this->addFlip(false);
    });

    connect(ui->btn_reset, &QPushButton::clicked, [this]{
       m_rolls.clear();
       this->update();
       this->setEnableMethodSelection(true);
       ui->btn_createPolyseed->setEnabled(false);
    });

    connect(ui->btn_createPolyseed, &QPushButton::clicked, [this]{
        QByteArray salt = "POLYSEED";
        QByteArray data = m_rolls.join(" ").toUtf8();

        // We already have enough entropy assuming unbiased throws, but a few extra rounds can't hurt
        // Polyseed requests 19 bytes of random data and discards two bits (for a total of 150 bits)
        m_key = QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha256, data, salt, 2048, 19);

        this->accept();
    });

    connect(ui->btn_cancel, &QPushButton::clicked, [this]{
        this->reject();
    });

    ui->radio_dice->setChecked(true);

    this->update();
    this->adjustSize();
}

void SeedDiceDialog::addFlip(bool heads) {
    m_rolls << (heads ? "H" : "T");
    this->update();
}

void SeedDiceDialog::addRoll(const QString &roll) {
    if (roll.isEmpty()) {
        return;
    }

    m_rolls << roll;
    this->update();
}

bool SeedDiceDialog::validateRollEntry() {
    ui->line_roll->setStyleSheet("");

    QString errStyle = "QLineEdit{border: 1px solid red;}";
    QStringList rolls = ui->line_roll->text().simplified().split(" ");

    for (const auto &rollstr : rolls) {
        if (rollstr.isEmpty()) {
            continue;
        }

        bool ok;
        int roll = rollstr.toInt(&ok);
        if (!ok || roll < 1 || roll > ui->spin_sides->value()) {
            ui->line_roll->setStyleSheet(errStyle);
            return false;
        }
    }

    return true;
}

void SeedDiceDialog::update() {
    this->updateRollsLeft();
    this->updateRolls();

    if (this->updateEntropy()) {
        ui->btn_createPolyseed->setEnabled(true);
    }
}

bool SeedDiceDialog::updateEntropy() {
    double entropy = entropyPerRoll() * m_rolls.length();
    ui->label_entropy->setText(QString("%1 / %2 bits").arg(QString::number(entropy, 'f', 2), QString::number(entropyNeeded)));

    return entropy > entropyNeeded;
}

void SeedDiceDialog::updateRolls() {
    ui->rolls->setPlainText(m_rolls.join(" "));
}

double SeedDiceDialog::entropyPerRoll() {
    if (ui->radio_dice->isChecked()) {
        return log(ui->spin_sides->value()) / log(2);
    } else {
        return 1;
    }
}

void SeedDiceDialog::updateRollsLeft() {
    int rollsLeft = std::max((int)(ceil((entropyNeeded - (this->entropyPerRoll() * m_rolls.length())) / this->entropyPerRoll())), 0);
    ui->label_rollsLeft->setText(QString::number(rollsLeft));
}

void SeedDiceDialog::setEnableMethodSelection(bool enabled) {
    ui->radio_dice->setEnabled(enabled);
    ui->radio_coinflip->setEnabled(enabled);
    ui->spin_sides->setEnabled(enabled);
}

const char* SeedDiceDialog::getSecret() {
    return m_key.data();
}

const QString& SeedDiceDialog::getMnemonic() {
    return m_mnemonic;
}

SeedDiceDialog::~SeedDiceDialog() = default;