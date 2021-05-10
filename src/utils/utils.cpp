// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QScreen>
#include <QMessageBox>
#include <QtNetwork>
#include <QClipboard>
#include <QCompleter>
#include <QDesktopServices>
#include <QtWidgets/QStyle>
#include <QPushButton>

#include "utils.h"
#include "utils/config.h"
#include "utils/tails.h"
#include "utils/whonix.h"
#include "utils/ColorScheme.h"
#include "globals.h"

QByteArray Utils::fileGetContents(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly))
    {
        throw std::runtime_error(QString("failed to open %1").arg(path).toStdString());
    }

    QByteArray data;
    data.resize(file.size());
    if (file.read(data.data(), data.size()) != data.size())
    {
        throw std::runtime_error(QString("failed to read %1").arg(path).toStdString());
    }

    return data;
}

bool Utils::fileExists(const QString &path) {
    QFileInfo check_file(path);
    return check_file.exists() && check_file.isFile();
}

bool Utils::dirExists(const QString &path) {
    QDir pathDir(path);
    return pathDir.exists();
}

QByteArray Utils::fileOpen(const QString &path) {
    QFile file(path);
    if(!file.open(QFile::ReadOnly | QFile::Text)) {
        return QByteArray();
    }

    QByteArray data = file.readAll();
    file.close();
    return data;
}

QByteArray Utils::fileOpenQRC(const QString &path) {
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)) {
        qDebug() << "error: " << file.errorString();
    }

    QByteArray data = file.readAll();
    file.close();
    return data;
}

bool Utils::fileWrite(const QString &path, const QString &data) {
    QFile file(path);
    if(file.open(QIODevice::WriteOnly)){
        QTextStream out(&file); out << data << Qt::endl;
        file.close();
        return true;
    }
    return false;
}

bool Utils::validateJSON(const QByteArray &blob) {
    QJsonDocument doc = QJsonDocument::fromJson(blob);
    QString jsonString = doc.toJson(QJsonDocument::Indented);
    return !jsonString.isEmpty();
}

bool Utils::readJsonFile(QIODevice &device, QSettings::SettingsMap &map) {
    QJsonDocument json = QJsonDocument::fromJson(device.readAll());
    map = json.object().toVariantMap();
    return true;
}

bool Utils::writeJsonFile(QIODevice &device, const QSettings::SettingsMap &map) {
    device.write(QJsonDocument(QJsonObject::fromVariantMap(map)).toJson(QJsonDocument::Indented));
    return true;
}

void Utils::applicationLogHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    const QString fn = context.function ? QString::fromUtf8(context.function) : "";
    const QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString line;

    switch (type) {
        case QtDebugMsg:
            line = QString("[%1 D] %2(:%3) %4\n").arg(date).arg(fn).arg(context.line).arg(msg);
            fprintf(stderr, "%s", line.toLatin1().data());
            break;
        case QtInfoMsg:
            line = QString("[%1 I] %2\n").arg(date).arg(msg);
            fprintf(stdout, "%s", line.toLatin1().data());
            break;
        case QtWarningMsg:
            line = QString("[%1 W] %2(:%3) %4\n").arg(date).arg(fn).arg(context.line).arg(msg);
            fprintf(stdout, "%s", line.toLatin1().data());
            break;
        case QtCriticalMsg:
            line = QString("[%1 C] %2(:%3) %4\n").arg(date).arg(fn).arg(context.line).arg(msg);
            fprintf(stderr, "%s", line.toLatin1().data());
            break;
        case QtFatalMsg:
            line = QString("[%1 F] %2(:%3) %4\n").arg(date).arg(fn).arg(context.line).arg(msg);
            fprintf(stderr, "%s", line.toLatin1().data());
            break;
    }

    auto message = logMessage(type, line, fn);

    //emit applicationLogUpdated(message);
}

void Utils::desktopNotify(const QString &title, const QString &message, int duration) {
    QStringList notify_send = QStringList() << title << message << "-t" << QString::number(duration);
    QStringList kdialog = QStringList() << title << message;
    QStringList macos = QStringList() << "-e" << QString(R"(display notification "%1" with title "%2")").arg(message).arg(title);
#if defined(Q_OS_LINUX)
    QProcess process;
    if (Utils::fileExists("/usr/bin/kdialog"))
        process.start("/usr/bin/kdialog", kdialog);
    else if (Utils::fileExists("/usr/bin/notify-send"))
        process.start("/usr/bin/notify-send", notify_send);
    process.waitForFinished(-1);
    QString stdout = process.readAllStandardOutput();
    QString stderr = process.readAllStandardError();
#elif defined(Q_OS_MACOS)
    QProcess process;
    // @TODO: need to escape special chars with "\"
    process.start("osascript", macos);
    process.waitForFinished(-1);
    QString stdout = process.readAllStandardOutput();
    QString stderr = process.readAllStandardError();
#endif
}

bool Utils::portOpen(const QString &hostname, quint16 port){
    QTcpSocket socket;
    socket.connectToHost(hostname, port);
    return socket.waitForConnected(600);
}

QString Utils::barrayToString(const QByteArray &data) {
    return QString(QTextCodec::codecForMib(106)->toUnicode(data));
}

void Utils::externalLinkWarning(QWidget *parent, const QString &url){
    if(!config()->get(Config::warnOnExternalLink).toBool()) {
        QDesktopServices::openUrl(QUrl(url));
        return;
    }

    QString body = "You are about to open the following link:\n\n";
    body += QString("%1").arg(url);

    if (!(TailsOS::detect() || WhonixOS::detect()))
        body += "\n\nYou will NOT be using Tor.";


    QMessageBox linkWarning(parent);
    linkWarning.setWindowTitle("External link warning");
    linkWarning.setText(body);
    QPushButton *copy = linkWarning.addButton("Copy link", QMessageBox::HelpRole);
    linkWarning.addButton(QMessageBox::Cancel);
    linkWarning.addButton(QMessageBox::Ok);
    linkWarning.setDefaultButton(QMessageBox::Ok);

    linkWarning.exec();

    if (linkWarning.clickedButton() == copy) {
        Utils::copyToClipboard(url);
    } else if (linkWarning.result() == QMessageBox::Ok) {
        QDesktopServices::openUrl(QUrl(url));
    }
}

QStringList Utils::fileFind(const QRegExp &pattern, const QString &baseDir, int level, int depth, const int maxPerDir) {
    // like `find /foo -name -maxdepth 2 "*.jpg"`
    QStringList rtn;
    QDir dir(baseDir);
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks | QDir::NoDot | QDir::NoDotDot);

    int fileCount = 0;
    for(const auto &fileInfo: dir.entryInfoList({"*"})) {
        fileCount += 1;
        if(fileCount > maxPerDir) return rtn;
        if(!fileInfo.isReadable())
            continue;

        const auto fn = fileInfo.fileName();
        const auto path = fileInfo.filePath();

        if (fileInfo.isDir()) {
            if (level + 1 <= depth)
                rtn << Utils::fileFind(pattern, path, level + 1, depth, maxPerDir);
        }
        else if (pattern.exactMatch(fn))
            rtn << path;
    }
    return rtn;
}

void Utils::copyToClipboard(const QString &string){
    QClipboard * clipboard = QApplication::clipboard();
    if (!clipboard) {
        qWarning() << "Unable to access clipboard";
        return;
    }
    clipboard->setText(string, QClipboard::Clipboard);
    if (clipboard->supportsSelection())
        clipboard->setText(string, QClipboard::Selection);

#if defined(Q_OS_LINUX)
    QThread::msleep(1);
#endif
}

QString Utils::copyFromClipboard() {
    QClipboard * clipboard = QApplication::clipboard();
    if (!clipboard) {
        qWarning() << "Unable to access clipboard";
        return "";
    }
    return clipboard->text();
}

QString Utils::blockExplorerLink(const QString &blockExplorer, NetworkType::Type nettype, const QString &txid) {
    if (blockExplorer == "exploremonero.com") {
        if (nettype == NetworkType::MAINNET) {
            return QString("https://exploremonero.com/transaction/%1").arg(txid);
        }
    }
    else if (blockExplorer == "moneroblocks.info") {
        if (nettype == NetworkType::MAINNET) {
            return QString("https://moneroblocks.info/tx/%1").arg(txid);
        }
    }
    else if (blockExplorer == "blockchair.com") {
        if (nettype == NetworkType::MAINNET) {
            return QString("https://blockchair.com/monero/transaction/%1").arg(txid);
        }
    }

    switch (nettype) {
        case NetworkType::MAINNET:
            return QString("https://xmrchain.net/tx/%1").arg(txid);
        case NetworkType::STAGENET:
            return QString("https://stagenet.xmrchain.net/tx/%1").arg(txid);
        case NetworkType::TESTNET:
            return QString("https://testnet.xmrchain.net/tx/%1").arg(txid);
    }

    return QString("");
}

QStandardItem *Utils::qStandardItem(const QString& text) {
    auto font = QApplication::font();
    return Utils::qStandardItem(text, font);
}

QStandardItem *Utils::qStandardItem(const QString& text, QFont &font) {
    // stupid Qt doesnt set font sizes correctly on OSX
    // @TODO: memleak
    auto item = new QStandardItem(text);
    item->setFont(font);
    return item;
}

QString Utils::getUnixAccountName() {
    QString accountName = qgetenv("USER"); // mac/linux
    if (accountName.isEmpty())
        accountName = qgetenv("USERNAME"); // Windows
    if (accountName.isEmpty())
        throw std::runtime_error("Could derive system account name from env vars: USER or USERNAME");
    return accountName;
}

QString Utils::xdgDesktopEntry(){
    return QString(
        "[Desktop Entry]\n"
        "Name=Feather\n"
        "GenericName=Feather\n"
        "X-GNOME-FullName=Feather\n"
        "Comment=a free Monero desktop wallet\n"
        "Keywords=Monero;\n"
        "Exec=\"%1\" %u\n"
        "Terminal=false\n"
        "Type=Application\n"
        "Icon=monero\n"
        "Categories=Network;GNOME;Qt;\n"
        "StartupNotify=true\n"
        "X-GNOME-Bugzilla-Bugzilla=GNOME\n"
        "X-GNOME-UsesNotifications=true\n"
    ).arg(QApplication::applicationFilePath());
}

bool Utils::xdgDesktopEntryWrite(const QString &path){
    QString mime = xdgDesktopEntry();
    QFileInfo file(path);
    QDir().mkpath(file.path());
    qDebug() << "Writing xdg desktop entry: " << path;
    return Utils::fileWrite(path, mime);
}

void Utils::xdgRefreshApplications(){
    QStringList args = {QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation)};
    QProcess process;
    process.start("update-desktop-database", args);
    process.waitForFinished(2500);
    process.close();
}

bool Utils::xdgDesktopEntryRegister() {
#if defined(Q_OS_MACOS)
    return false;
#endif
#if defined(Q_OS_WIN)
    // @TODO: implement
    return false;
#endif
    // no support for Tails here
    if(TailsOS::detect()) return false;

    QString writeLocations = "Write locations:\n";
    writeLocations += QString("- %1\n").arg(xdgPaths.pathApp);
    writeLocations += QString("- %1\n").arg(xdgPaths.pathIcon);

    QPixmap appIcon(":assets/images/feather.png");
    if (!Utils::fileExists(xdgPaths.pathIcon))
        Utils::pixmapWrite(xdgPaths.pathIcon, appIcon);
    Utils::xdgDesktopEntryWrite(xdgPaths.pathApp);
    Utils::xdgRefreshApplications();
    return true;
}

bool Utils::pixmapWrite(const QString &path, const QPixmap &pixmap) {
    qDebug() << "Writing xdg icon: " << path;
    QFile file(path);
    QFileInfo iconInfo(file);
    QDir().mkpath(iconInfo.path());
    if(file.open(QIODevice::WriteOnly)){
        pixmap.save(&file, "PNG");
        file.close();
        return true;
    }
    return false;
}

QFont Utils::relativeFont(int delta) {
    auto font = QApplication::font();
    font.setPointSize(font.pointSize() + delta);
    return font;
}

double Utils::roundSignificant(double N, double n)
{
    int h;
    double b, d, e, i, j, m, f;
    b = N;

    for (i = 0; b >= 1; ++i)
        b = b / 10;

    d = n - i;
    b = N;
    b = b * pow(10, d);
    e = b + 0.5;
    if ((float)e == (float)ceil(b)) {
        f = (ceil(b));
        h = f - 2;
        if (h % 2 != 0) {
            e = e - 1;
        }
    }
    j = floor(e);
    m = pow(10, d);
    j = j / m;
    return j;
}

QString Utils::formatBytes(quint64 bytes)
{
    QVector<QString> sizes = { "B", "KB", "MB", "GB", "TB" };

    int i;
    double _data;
    for (i = 0; i < sizes.count() && bytes >= 10000; i++, bytes /= 1000)
        _data = bytes / 1000.0;

    if (_data < 0)
        _data = 0;

    // unrealistic
    if (_data > 10000)
        _data = 0;

    return QString("%1 %2").arg(QString::number(_data, 'f', 1), sizes[i]);
}


QMap<QString, QLocale> Utils::localeCache = {};

QLocale Utils::getCurrencyLocale(const QString &currencyCode) {
    QLocale locale;
    if (localeCache.contains(currencyCode)) {
        locale = localeCache[currencyCode];
    } else {
        QList<QLocale> allLocales = QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry);
        for (const auto& locale_: allLocales) {
            if (locale_.currencySymbol(QLocale::CurrencyIsoCode) == currencyCode) {
                locale = locale_;
            }
        }
        localeCache[currencyCode] = locale;
    }
    return locale;
}

QString Utils::amountToCurrencyString(double amount, const QString &currencyCode) {
    QLocale locale = getCurrencyLocale(currencyCode);

    // \xC2\xA0 = UTF-8 non-breaking space, it looks off.
    if (currencyCode == "USD")
        return locale.toCurrencyString(amount, "$").remove("\xC2\xA0");

    return locale.toCurrencyString(amount).remove("\xC2\xA0");
}

int Utils::maxLength(const QVector<QString> &array) {
    int maxLength = 0;
    for (const auto &str: array) {
        auto length = str.length();
        if (length > maxLength) {
            maxLength = length;
        }
    }
    return maxLength;
}

QString Utils::balanceFormat(quint64 balance) {
    QString str = QString::number(balance / globals::cdiv, 'f', 4);

    str.remove(QRegExp("0+$"));
    str.remove(QRegExp("\\.$"));

    return str;
}

QTextCharFormat Utils::addressTextFormat(const SubaddressIndex &index) {
    if (index.isPrimary()) {
        QTextCharFormat rec;
        rec.setBackground(QBrush(ColorScheme::YELLOW.asColor(true)));
        rec.setToolTip("Wallet change/primary address");
        return rec;
    }
    if (index.isValid()) {
        QTextCharFormat rec;
        rec.setBackground(QBrush(ColorScheme::GREEN.asColor(true)));
        rec.setToolTip("Wallet receive address");
        return rec;
    }
    return QTextCharFormat();
}

bool Utils::isTorsocks() {
#if defined(Q_OS_MAC)
    return qgetenv("DYLD_INSERT_LIBRARIES").indexOf("libtorsocks") >= 0;
#elif defined(Q_OS_LINUX)
    return qgetenv("LD_PRELOAD").indexOf("libtorsocks") >= 0;
#else
    return false;
#endif
}

QString Utils::defaultWalletDir() {
    QString portablePath = QCoreApplication::applicationDirPath().append("/%1");
    if (QFile::exists(portablePath.arg(".portable"))) {
        return portablePath.arg("feather_data/wallets");
    }

    if (TailsOS::detect()) {
        QString path = []{
            QString appImagePath = qgetenv("APPIMAGE");
            if (appImagePath.isEmpty()) {
                qDebug() << "Not an appimage, using currentPath()";
                return QDir::currentPath() + "/.feather/Monero/wallets";
            }

            QFileInfo appImageDir(appImagePath);
            return appImageDir.absoluteDir().path() + "/.feather/Monero/wallets";
        }();

        return path;
    }

#if defined(Q_OS_LINUX) or defined(Q_OS_MAC)
    return QString("%1/Monero/wallets").arg(QDir::homePath());
#elif defined(Q_OS_WIN)
    return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Monero/wallets";
#endif
}