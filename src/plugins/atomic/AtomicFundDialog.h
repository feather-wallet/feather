//
// Created by dev on 7/8/24.
//

#ifndef FEATHER_ATOMICFUNDDIALOG_H
#define FEATHER_ATOMICFUNDDIALOG_H

#include <QDialog>
#include "components.h"
#include "qrcode/QrCode.h"
#include "widgets/QrCodeWidget.h"


namespace Ui {
    class AtomicFundDialog;
}


class AtomicFundDialog : public WindowModalDialog {
    Q_OBJECT

public:
    explicit AtomicFundDialog(QWidget *parent, const QString &title = "Qr Code", const QString &btc_address = "Error Restart swap");
    ~AtomicFundDialog() override;
signals:
    void cleanProcs();
private:
    void copyImage();
    void saveImage();
    void copyAddress();
    QScopedPointer<Ui::AtomicFundDialog> ui;
    QPixmap m_pixmap;
    QString address;
    QrCode qrCode;
};


#endif //FEATHER_ATOMICFUNDDIALOG_H
