// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_UTILS_H
#define FEATHER_UTILS_H

#include <QRegularExpression>
#include <QStandardItemModel>
#include <QApplication>
#include <QTextCharFormat>

#include "libwalletqt/Wallet.h"
#include "networktype.h"

namespace Utils
{
    bool fileExists(const QString &path);
    QByteArray fileOpen(const QString &path);
    QByteArray fileOpenQRC(const QString &path);
    bool fileWrite(const QString &path, const QString &data);
    bool pixmapWrite(const QString &path, const QPixmap &pixmap);
    QStringList fileFind(const QRegularExpression &pattern, const QString &baseDir, int level, int depth, int maxPerDir);

    QString portablePath();
    bool isPortableMode();
    bool portableFileExists(const QString &dir);
    QString ringDatabasePath();

    bool dirExists(const QString &path);
    QString defaultWalletDir();
    QString applicationPath();

    bool validateJSON(const QByteArray &blob);
    bool readJsonFile(QIODevice &device, QSettings::SettingsMap &map);
    bool writeJsonFile(QIODevice &device, const QSettings::SettingsMap &map);

    void copyToClipboard(const QString &string);
    QString copyFromClipboard();
    void copyColumn(QModelIndex * index, int column);

    QString xdgDesktopEntry();
    bool xdgDesktopEntryWrite(const QString &path);
    void xdgRefreshApplications();
    bool xdgDesktopEntryRegister();

    bool portOpen(const QString &hostname, quint16 port);
    quint16 getDefaultRpcPort(NetworkType::Type type);
    bool isTorsocks();

    double roundSignificant(double N, double n);
    int maxLength(const QVector<QString> &array);
    QString formatBytes(quint64 bytes);

    QLocale getCurrencyLocale(const QString &currencyCode);
    QString amountToCurrencyString(double amount, const QString &currencyCode);

    QStandardItem *qStandardItem(const QString &text);
    QStandardItem *qStandardItem(const QString &text, QFont &font);

    QString blockExplorerLink(const QString &blockExplorer, NetworkType::Type nettype, const QString &txid);
    void externalLinkWarning(QWidget *parent, const QString &url);

    void desktopNotify(const QString &title, const QString &message, int duration);

    QString displayAddress(const QString& address, int sections = 3, const QString & sep = " ");
    QTextCharFormat addressTextFormat(const SubaddressIndex &index, quint64 amount);

    QFont getMonospaceFont();
    QFont relativeFont(int delta);

    void applicationLogHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    QString barrayToString(const QByteArray &data);
    QString getAccountName();

    bool isLocalUrl(const QUrl &url);

    template<typename QEnum>
    QString QtEnumToString (QEnum value) {
        return QString::fromStdString(std::string(QMetaEnum::fromType<QEnum>().valueToKey(value)));
    }
}

#endif //FEATHER_UTILS_H
