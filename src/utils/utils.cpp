// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include <QtCore>
#include <QtGlobal>
#include <QObject>
#include <QScreen>
#include <QMessageBox>
#include <QtNetwork>
#include <QTcpSocket>
#include <QClipboard>
#include <QDesktopWidget>
#include <QProcess>
#include <QCompleter>
#include <QDesktopServices>
#include <QApplication>
#include <QMainWindow>
#include <QTextStream>
#include <QtWidgets/QStyle>
#include <QVector>

#include "utils.h"
#include "utils/config.h"
#include "utils/tails.h"

// Application log for current session
QVector<logMessage> applicationLog = QVector<logMessage>(); // todo: replace with ring buffer
QMutex logMutex;

void Utils::openWindow(QWidget *w) {
    auto first_screen = QApplication::screens()[0];
    w->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, w->size(), first_screen->availableGeometry()));
    w->show();
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
        QTextStream out(&file); out << data << endl;
        file.close();
        return true;
    }
    return false;
}

QString Utils::systemAccountName(){
    QString accountName = qgetenv("USER"); // mac/linux
    if (accountName.isEmpty())
        return qgetenv("USERNAME"); // Windows
    if (accountName.isEmpty())
        qDebug() << "accountName was empty";

    return "";
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

QStringList Utils::readJsonStringToQStringList(const QString &input) {
    QStringList data;

    QJsonDocument doc = QJsonDocument::fromJson(input.toUtf8());
    QJsonObject object = doc.object();
    QJsonArray array = doc.array();

    for(auto &&entry: array)
        data << entry.toString();
    return data;
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

    {
        QMutexLocker locker(&logMutex);
        applicationLog.append(message);
    }


    //emit applicationLogUpdated(message);
}

QByteArray Utils::zipExtract(const QString &path, const QString& destination) {
    Q_UNUSED(path)
    Q_UNUSED(destination)
    return QByteArray();
}

bool Utils::isDigit(const QString& inp) {
    for (auto &&i : inp) {
        if(!i.isDigit()) return false;
    }
    return true;
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
    // @TODO: need to escape special chars with \
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

QByteArray Utils::readSocket(QTcpSocket &socket, int buffer_size) {
    QByteArray data;
    if(!socket.waitForReadyRead(6000))
        return data;

    while(buffer_size > 0 && socket.bytesAvailable() > 0){
        QByteArray _data = socket.read(buffer_size);
        buffer_size -= _data.size();
        data += _data;
    }

    return data;
}

bool Utils::testSocks5(const QString &host, quint16 port){
    // synchronous socks5 tester
    QByteArray data;
    QTcpSocket socket;
    socket.connectToHost(host, port);
    if (!socket.waitForConnected(1000)) {
        qDebug() << QString("could not connect to %1 %2")
                .arg(host).arg(port);
        socket.close();
        return false;
    }

    // latest & greatest
    socket.write(QByteArray("\x05\x02", 2));
    socket.flush();

    // no auth pl0x
    socket.write(QByteArray("\x00\x01", 2));
    socket.flush();

    // cool story
    if(Utils::readSocket(socket, 2).isEmpty()){
        qDebug() << "socks response timeout";
        socket.close();
        return false;
    }

    // pls sir
    socket.write(QByteArray("\x05\x01\x00\x03", 4));

    // here we go!!
    socket.write(QByteArray("\x16", 1));  // len
    socket.write(QByteArray("kebjllr47c2ouoly.onion"));
    socket.write(QByteArray("\x00\x50", 2)); // port
    socket.flush();

    // fingers crossed
    auto result = Utils::readSocket(socket, 10);
    qDebug() << result;
    if(result.length() != 10 || result.at(1) != 0) {
        qDebug() << "bad socks response";
        socket.close();
        return false;
    }

    // can haz?
    QByteArray http("GET /api/v1/ping HTTP/1.1\r\nHost: kebjllr47c2ouoly.onion\r\nConnection: close\r\n\r\n");
    socket.write(http);

    auto resp = Utils::readSocket(socket, 555);
    QRegularExpression re(R"(^HTTP\/\d.\d 200 OK)");
    QRegularExpressionMatch match = re.match(resp);
    if(match.hasMatch()){
        socket.close();
        return true;
    }

    qDebug() << resp;

    socket.close();
    return false;
}

void Utils::externalLinkWarning(const QString &url){
    if(!config()->get(Config::warnOnExternalLink).toBool()) {
        QDesktopServices::openUrl(QUrl(url));
        return;
    }

    QString body = "You are about to open the following link:\n\n";
    body += QString("%1\n\n").arg(url);
    body += "You will NOT be using Tor.";

    switch (Utils::showMessageBox("External link warning", body, true)) {
        case QMessageBox::Cancel:
            break;
        default:
            QDesktopServices::openUrl(QUrl(url));
            break;
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

bool Utils::walletExists(QString name, const QString &path) {
    name = name.replace(".keys", "");
    auto walletPath = QDir(path).filePath(name + ".keys");
    return Utils::fileExists(walletPath);
}

int Utils::showMessageBox(const QString &windowTitle, const QString &body, bool warning){
    QMessageBox msgBox(QApplication::activeWindow());
    msgBox.setWindowTitle(windowTitle);
    msgBox.setText(body);
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);

    if(warning) {
        QPixmap iconWarning = QPixmap(":/assets/images/ghost_icon.png");
        msgBox.setIconPixmap(iconWarning);
    }
    else {
        QPixmap iconInfo = QPixmap(":/assets/images/info.png")
                .scaled(QSize(48,48), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        msgBox.setIconPixmap(iconInfo);
    }

    return msgBox.exec();
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

QList<processStruct> Utils::procList() {
    // windows maybe: https://stackoverflow.com/a/13635377/2054778
    QList<processStruct> rtn;
    QProcess process;
#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
#if defined(Q_OS_MAC)
    process.start("ps", QStringList() << "-wwaxo" << "pid,command");
#elif defined(Q_OS_LINUX)
    process.start("ps", QStringList() << "-wwaxo" << "pid,command");
#endif
    process.waitForFinished(-1);

    QString stdout = process.readAllStandardOutput();
    QString stderr = process.readAllStandardError();

    if(stdout.isEmpty())
        return rtn;

    QStringList spl = stdout.split("\n");
    if(spl.count() >= 1)
        spl.removeAt(0);

    for (auto& line: spl) {
        line = line.trimmed();
        if(line.isEmpty())
           continue;

        QStringList _spl = line.split(" ");
        processStruct ps;
        if(_spl.length() >= 2) {
            ps.pid = _spl.at(0).toInt();
            ps.command = _spl.at(1);
            rtn.append(ps);
        }
    }
#endif
    return rtn;
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
    double l, a, b, c, d, e, i, j, m, f, g;
    b = N;
    c = floor(N);

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