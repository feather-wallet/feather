// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "URSettingsDialog.h"
#include "ui_URSettingsDialog.h"

#include "utils/config.h"
#include "utils/Utils.h"

URSettingsDialog::URSettingsDialog(QWidget *parent)
        : WindowModalDialog(parent)
        , ui(new Ui::URSettingsDialog)
{
    ui->setupUi(this);
    
    ui->spin_fragmentLength->setValue(conf()->get(Config::URfragmentLength).toInt());
    ui->spin_speed->setValue(conf()->get(Config::URmsPerFragment).toInt());
    ui->check_fountainCode->setChecked(conf()->get(Config::URfountainCode).toBool());
    
    connect(ui->spin_fragmentLength, &QSpinBox::valueChanged, [](int value){
        conf()->set(Config::URfragmentLength, value);
    });
    connect(ui->spin_speed, &QSpinBox::valueChanged, [](int value){
        conf()->set(Config::URmsPerFragment, value);
    });
    connect(ui->check_fountainCode, &QCheckBox::toggled, [](bool toggled){
        conf()->set(Config::URfountainCode, toggled);
    });
    
    connect(ui->btn_reset, &QPushButton::clicked, [this]{
        ui->spin_speed->setValue(100);
        ui->spin_fragmentLength->setValue(100);
        ui->check_fountainCode->setChecked(false);
    });
   
    this->adjustSize();
}

URSettingsDialog::~URSettingsDialog() = default;