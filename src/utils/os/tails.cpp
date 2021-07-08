// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QRegExp>
#include <QMessageBox>

#include "tails.h"
#include "Utils.h"

bool TailsOS::detected = false;
bool TailsOS::isTails = false;
const QString TailsOS::tailsPathData = QString("/live/persistence/TailsData_unlocked/");

bool TailsOS::detect()
{
    if (detected) {
        return TailsOS::isTails;
    }

    if (!Utils::fileExists("/etc/os-release"))
        return false;

    QByteArray data = Utils::fileOpen("/etc/os-release");
    QRegularExpression re("TAILS_PRODUCT_NAME=\"Tails\"");
    QRegularExpressionMatch os_match = re.match(data);
    bool matched = os_match.hasMatch();

    if (matched)
        qDebug() << "Tails OS detected";

    TailsOS::detected = true;
    TailsOS::isTails = matched;

    return matched;
}

bool TailsOS::detectDataPersistence()
{
    return QDir(QDir::homePath() + "/Persistent").exists();
}

bool TailsOS::detectDotPersistence()
{
    return QDir(tailsPathData + "dotfiles").exists();
}

QString TailsOS::version()
{
    if (!Utils::fileExists("/etc/os-release"))
        return "";

    QByteArray data = Utils::fileOpen("/etc/os-release");
    QRegExp re(R"(TAILS_VERSION_ID="(\d+.\d+))");
    int pos = re.indexIn(data);
    if (pos >= 0) {
        return re.cap(1);
    }
    return "";
}

void TailsOS::showDataPersistenceDisabledWarning()
{
    QMessageBox msgBox;
    msgBox.setText(QObject::tr("Warning: persistence disabled"));
    msgBox.setWindowTitle(QObject::tr("Warning: persistence disabled"));
    msgBox.setInformativeText(
        QObject::tr("Feather has detected that Tails persistence is "
                     "currently disabled. Any configurations and wallets you make inside "
                     "Feather will not be permanently saved."
                     "\n\n"
                     "Make sure to not save your wallet on the "
                     "filesystem, as it will be lost at shutdown."
                     "\n\n"
                     "To enable Tails persistence, setup an encrypted volume "
                     "and restart Tails. To gain a startup menu item, "
                     "enable dotfiles persistence."));

    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIconPixmap(QPixmap(":/images/tails-grey.png"));
    msgBox.exec();
}

void TailsOS::persistXdgMime(const QString &filePath, const QString &data)
{
    QFileInfo file(filePath);
    QString tailsPath = tailsPathData + "dotfiles/.local/share/applications/";

    // write to persistent volume
#ifdef QT_DEBUG
    qDebug() << "Writing xdg mime: " << tailsPath + file.fileName();
#endif

    QDir().mkpath(tailsPath);  // ensure directory exists
    Utils::fileWrite(tailsPath + file.fileName(), data);

    // write to current session
#ifdef QT_DEBUG
    qDebug() << "Writing xdg mime: " << file.filePath();
#endif

    QDir().mkpath(file.path());  // ensure directory exists
    Utils::fileWrite(file.filePath(), data);
}
