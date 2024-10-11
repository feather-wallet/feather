// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "Utils.h"

#include <QApplication>
#include <QMessageBox>
#include <QClipboard>
#include <QDesktopServices>
#include <QPushButton>
#include <QFontDatabase>
#include <QTcpSocket>
#include <QFileDialog>
#include <QLayout>
#include <QLayoutItem>
#include <QJsonDocument>
#include <QThread>
#include <QStandardPaths>
#include <QProcess>

#include "constants.h"
#include "networktype.h"
#include "utils/ColorScheme.h"
#include "utils/config.h"
#include "utils/os/tails.h"
#include "utils/os/whonix.h"
#include "libwalletqt/Wallet.h"
#include "WindowManager.h"

namespace Utils {
bool fileExists(const QString &path) {
    QFileInfo check_file(path);
    return check_file.exists() && check_file.isFile();
}

QByteArray fileOpen(const QString &path) {
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return {};
    }

    QByteArray data = file.readAll();
    file.close();
    return data;
}

QByteArray fileOpenQRC(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "error: " << file.errorString();
        return {};
    }

    QByteArray data = file.readAll();
    file.close();
    return data;
}

QString loadQrc(const QString &qrc) {
    QFile file(qrc);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open resource:" << qrc;
        return QString();
    }

    QTextStream in(&file);
    return in.readAll();
}

bool fileWrite(const QString &path, const QString &data) {
    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream out(&file); out << data << Qt::endl;
        file.close();
        return true;
    }
    return false;
}

bool pixmapWrite(const QString &path, const QPixmap &pixmap) {
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

QStringList fileFind(const QRegularExpression &pattern, const QString &baseDir, int level, int depth, const int maxPerDir) {
    // like `find /foo -name -maxdepth 2 "*.jpg"`
    QStringList rtn;
    QDir dir(baseDir);
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks | QDir::NoDot | QDir::NoDotDot);

    int fileCount = 0;
    for (const auto &fileInfo: dir.entryInfoList({"*"})) {
        fileCount += 1;
        if(fileCount > maxPerDir) return rtn;
        if(!fileInfo.isReadable())
            continue;

        const auto fn = fileInfo.fileName();
        const auto path = fileInfo.filePath();

        QRegularExpression re(QRegularExpression::anchoredPattern(pattern.pattern()));
        QRegularExpressionMatch match = re.match(fn);

        if (fileInfo.isDir()) {
            if (level + 1 <= depth)
                rtn << fileFind(pattern, path, level + 1, depth, maxPerDir);
        }
        else if (match.hasMatch()) {
            rtn << path;
        }
    }
    return rtn;
}

QString getSaveFileName(QWidget* parent, const QString &caption, const QString &filename, const QString &filter) {
    QDir lastPath{conf()->get(Config::lastPath).toString()};
    QString fn = QFileDialog::getSaveFileName(parent, caption, lastPath.filePath(filename), filter);
    if (fn.isEmpty()) {
        return {};
    }
    QFileInfo fileInfo(fn);
    conf()->set(Config::lastPath, fileInfo.absolutePath());
    return fn;
}

QString getOpenFileName(QWidget* parent, const QString& caption, const QString& filter) {
    QString lastPath = conf()->get(Config::lastPath).toString();
    QString fn = QFileDialog::getOpenFileName(parent, caption, lastPath, filter);
    if (fn.isEmpty()) {
        return {};
    }
    QFileInfo fileInfo(fn);
    conf()->set(Config::lastPath, fileInfo.absolutePath());
    return fn;
}

bool dirExists(const QString &path) {
    QDir pathDir(path);
    return pathDir.exists();
}

QString portablePath() {
    return Utils::applicationPath() + "/feather_data";
}

bool isPortableMode() {
    return Utils::portableFileExists(Utils::applicationPath());
}

bool portableFileExists(const QString &dir) {
    QStringList portableFiles = {".portable", ".portable.txt", "portable.txt"};

    return std::find_if(portableFiles.begin(), portableFiles.end(), [dir](const QString &portableFile){
        return QFile::exists(dir + "/" + portableFile);
    }) != portableFiles.end();
}

QString defaultWalletDir() {
    if (Utils::isPortableMode()) {
        return Utils::portablePath() + "/wallets";
    }

    if (TailsOS::detect()) {
        QString path = []{
            // Starting in 1.1.0 the wallet and config directory were moved from ./.feather to ./feather_data
            // A user might accidentally delete the folder containing the file hidden folder after moving the AppImage
            // We return the old path if it still exists

            QString appImagePath = qgetenv("APPIMAGE");
            if (appImagePath.isEmpty()) {
                qDebug() << "Not an appimage, using currentPath()";
                if (QDir(QDir::currentPath() + "/.feather").exists()) {
                    return QDir::currentPath() + "/.feather/Monero/wallets";
                }
                return QDir::currentPath() + "/feather_data/wallets";
            }

            QFileInfo appImageDir(appImagePath);
            QString absolutePath = appImageDir.absoluteDir().path();
            if (QDir(absolutePath + "/.feather").exists()) {
                return absolutePath + "/.feather/Monero/wallets";
            }
            return absolutePath + "/feather_data/wallets";
        }();

        return path;
    }

#if defined(Q_OS_LINUX) or defined(Q_OS_MAC)
    return QString("%1/Monero/wallets").arg(QDir::homePath());
#elif defined(Q_OS_WIN)
    return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Monero/wallets";
#endif
}

QString ringDatabasePath() {
    if (Utils::isPortableMode()) {
        QString suffix = "";
        if (constants::networkType != NetworkType::Type::MAINNET) {
            suffix = "-" + Utils::QtEnumToString(constants::networkType);
        }
        return Utils::portablePath() + "/ringdb" + suffix;
    }
    return ""; // Use libwallet default
}

QString applicationPath() {
    QString applicationPath = QCoreApplication::applicationDirPath();
    QDir appDir(applicationPath);

#ifdef Q_OS_MACOS
    // applicationDirPath will be inside the app bundle

    if (applicationPath.endsWith("Contents/MacOS")) {
        appDir.cd("../../..");
    }
    return appDir.absolutePath();
#endif

    QString appimagePath = qgetenv("APPIMAGE");
    if (!appimagePath.isEmpty()) {
        return QFileInfo(appimagePath).absoluteDir().path();
    }

    return applicationPath;
}

QString applicationFilePath() {
    QString appimagePath = qgetenv("APPIMAGE");
    if (!appimagePath.isEmpty()) {
        return appimagePath;
    }

    return QApplication::applicationFilePath();
}

bool validateJSON(const QByteArray &blob) {
    QJsonDocument doc = QJsonDocument::fromJson(blob);
    QString jsonString = doc.toJson(QJsonDocument::Indented);
    return !jsonString.isEmpty();
}

bool readJsonFile(QIODevice &device, QSettings::SettingsMap &map) {
    QJsonDocument json = QJsonDocument::fromJson(device.readAll());
    map = json.object().toVariantMap();
    return true;
}

bool writeJsonFile(QIODevice &device, const QSettings::SettingsMap &map) {
    device.write(QJsonDocument(QJsonObject::fromVariantMap(map)).toJson(QJsonDocument::Indented));
    return true;
}

void copyToClipboard(const QString &string){
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

QString copyFromClipboard() {
    QClipboard * clipboard = QApplication::clipboard();
    if (!clipboard) {
        qWarning() << "Unable to access clipboard";
        return "";
    }
    return clipboard->text();
}

void copyColumn(QModelIndex *index, int column) {
    QString string(index->model()->data(index->siblingAtColumn(column), Qt::UserRole).toString());
    QClipboard * clipboard = QApplication::clipboard();
    if (!clipboard) {
        qWarning() << "Unable to access clipboard";
        return;
    }
    clipboard->setText(string);
}

QString xdgDesktopEntry(){
    return QString(
            "[Desktop Entry]\n"
            "Name=Feather Wallet\n"
            "Comment=A free Monero desktop wallet\n"
            "Keywords=Monero;\n"
            "Exec=\"%1\"\n"
            "Terminal=false\n"
            "Type=Application\n"
            "Icon=feather\n"
            "Categories=Network;\n"
            "StartupNotify=false\n"
    ).arg(applicationFilePath());
}

bool xdgDesktopEntryWrite(const QString &path){
    QString mime = xdgDesktopEntry();
    QFileInfo file(path);
    QDir().mkpath(file.path());
    qDebug() << "Writing xdg desktop entry: " << path;
    return fileWrite(path, mime);
}

void xdgRefreshApplications(){
    QStringList args = {QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation)};
    QProcess::startDetached("update-desktop-database", args);
}

bool xdgDesktopEntryRegister() {
#if defined(Q_OS_MACOS)
    return false;
#endif
#if defined(Q_OS_WIN)
    // @TODO: implement
    return false;
#endif

    QPixmap appIcon(":assets/images/appicons/64x64.png");
    QString pathIcon = QString("%1/.local/share/icons/feather.png").arg(QDir::homePath());
    if (!fileExists(pathIcon)) {
        pixmapWrite(pathIcon, appIcon);
    }
    xdgDesktopEntryWrite(QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + "/feather-wallet.desktop");

    xdgRefreshApplications();
    return true;
}

bool portOpen(const QString &hostname, quint16 port) { // TODO: this call should be async
    if (conf()->get(Config::offlineMode).toBool()) {
        return false;
    }

    QTcpSocket socket;
    socket.connectToHost(hostname, port);
    return socket.waitForConnected(600);
}

quint16 getDefaultRpcPort(NetworkType::Type type) {
    switch (type) {
        case NetworkType::Type::MAINNET:
            return 18081;
        case NetworkType::Type::TESTNET:
            return 28081;
        case NetworkType::Type::STAGENET:
            return 38081;
    }
    return 18081;
}

bool isTorsocks() {
#if defined(Q_OS_MAC)
    return qgetenv("DYLD_INSERT_LIBRARIES").indexOf("libtorsocks") >= 0;
#elif defined(Q_OS_LINUX)
    return qgetenv("LD_PRELOAD").indexOf("libtorsocks") >= 0;
#else
    return false;
#endif
}

double roundSignificant(double N, double n)
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

int maxLength(const QVector<QString> &array) {
    int maxLength = 0;
    for (const auto &str: array) {
        auto length = str.length();
        if (length > maxLength) {
            maxLength = length;
        }
    }
    return maxLength;
}

QString formatBytes(quint64 bytes)
{
    QVector<QString> sizes = { "B", "KB", "MB", "GB", "TB" };

    int i;
    double _data = bytes;
    for (i = 0; i < sizes.count() && bytes >= 10000; i++, bytes /= 1000)
        _data = bytes / 1000.0;

    if (_data < 0)
        _data = 0;

    // unrealistic
    if (_data > 10000)
        _data = 0;

    return QString("%1 %2").arg(QString::number(_data, 'f', 1), sizes[i]);
}

QMap<QString, QLocale> localeCache = {};
QLocale getCurrencyLocale(const QString &currencyCode) {
    QLocale locale;
    if (localeCache.contains(currencyCode)) {
        locale = localeCache[currencyCode];
    } else {
        QList<QLocale> allLocales = QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry);
        for (const auto& locale_: allLocales) {
            if (locale_.currencySymbol(QLocale::CurrencyIsoCode) == currencyCode) {
                locale = locale_;
                break;
            }
        }
        localeCache[currencyCode] = locale;
    }
    return locale;
}

QString amountToCurrencyString(double amount, const QString &currencyCode) {
    QLocale locale = getCurrencyLocale(currencyCode);

    // \xC2\xA0 = UTF-8 non-breaking space, it looks off.
    if (currencyCode == "USD")
        return locale.toCurrencyString(amount, "$").remove("\xC2\xA0");

    return QString("%1%2").arg(locale.currencySymbol(), QString::number(amount, 'f', 2));
}

QStandardItem *qStandardItem(const QString& text) {
    auto font = QApplication::font();
    return Utils::qStandardItem(text, font);
}

QStandardItem *qStandardItem(const QString& text, QFont &font) {
    // stupid Qt doesnt set font sizes correctly on OSX
    // @TODO: memleak
    auto item = new QStandardItem(text);
    item->setFont(font);
    return item;
}

QString blockExplorerLink(const QString &txid) {
    QString link = conf()->get(Config::blockExplorer).toString();

    QUrl url(link);
    if (url.scheme() != "http" && url.scheme() != "https") {
        return {};
    }

    if (!link.contains("%txid%")) {
        return {};
    }

    return link.replace("%txid%", txid);
}

void externalLinkWarning(QWidget *parent, const QString &url){
    if (!conf()->get(Config::warnOnExternalLink).toBool()) {
        QDesktopServices::openUrl(QUrl(url));
        return;
    }

    QMessageBox linkWarning(parent);
    linkWarning.setWindowTitle("External link warning");
    linkWarning.setText("You are about to open the following link:");
    linkWarning.setInformativeText(url);
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

QString displayAddress(const QString& address, int sections, const QString& sep) {
    QStringList list;
    if (sections < 1) sections = 1;
    for (int i = 0; i < sections; i += 1) {
        list << address.mid(i*5, 5);
    }
    list << "…"; // utf-8 Horizontal Ellipsis
    for (int i = sections; i > 0; i -= 1) {
        list << address.mid(address.length() - i * 5, 5);
    }
    return list.join(sep);
}

QTextCharFormat addressTextFormat(const SubaddressIndex &index, quint64 amount) {
    QTextCharFormat rec;
    if (index.isPrimary()) {
        rec.setBackground(QBrush(ColorScheme::YELLOW.asColor(true)));
        rec.setToolTip("Wallet change/primary address");
    }
    else if (index.isValid()) {
        rec.setBackground(QBrush(ColorScheme::GREEN.asColor(true)));
        rec.setToolTip("Wallet receive address");
    }
    else if (amount == 0) {
        rec.setBackground(QBrush(ColorScheme::GRAY.asColor(true)));
        rec.setToolTip("Dummy output (Min. 2 outs consensus rule)");
    }
    return rec;
}

void applicationLogHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    const QString fn = context.function ? QString::fromUtf8(context.function) : "";
    const QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString line;

    switch (type) {
        case QtDebugMsg:
            line = QString("[%1 D] %2(:%3) %4\n").arg(date, fn, QString::number(context.line), msg);
            fprintf(stderr, "%s", line.toLatin1().data());
            break;
        case QtInfoMsg:
            line = QString("[%1 I] %2\n").arg(date, msg);
            fprintf(stdout, "%s", line.toLatin1().data());
            break;
        case QtWarningMsg:
            line = QString("[%1 W] %2(:%3) %4\n").arg(date, fn, QString::number(context.line), msg);
            fprintf(stdout, "%s", line.toLatin1().data());
            break;
        case QtCriticalMsg:
            line = QString("[%1 C] %2(:%3) %4\n").arg(date, fn, QString::number(context.line), msg);
            fprintf(stderr, "%s", line.toLatin1().data());
            break;
        case QtFatalMsg:
            line = QString("[%1 F] %2(:%3) %4\n").arg(date, fn, QString::number(context.line), msg);
            fprintf(stderr, "%s", line.toLatin1().data());
            break;
    }
}

QString barrayToString(const QByteArray &data) {
    return QString::fromUtf8(data);
}

QFont getMonospaceFont()
{
    if (QFontInfo(QApplication::font()).fixedPitch()) {
        return QApplication::font();
    }
    return QFontDatabase::systemFont(QFontDatabase::FixedFont);
}

QFont relativeFont(int delta) {
    auto font = QApplication::font();
    font.setPointSize(font.pointSize() + delta);
    return font;
}

bool isLocalUrl(const QUrl &url) {
    QRegularExpression localNetwork(R"((^127\.)|(^10\.)|(^172\.1[6-9]\.)|(^172\.2[0-9]\.)|(^172\.3[0-1]\.)|(^192\.168\.))");
    return (localNetwork.match(url.host()).hasMatch() || url.host() == "localhost");
}

void showError(QWidget *parent, const QString &title, const QString &description, const QStringList &helpItems, const QString &doc, const QString &highlight, const QString &link) {
    showMsg(parent, QMessageBox::Warning, "Error", title, description, helpItems, doc, highlight, link);
}

void showInfo(QWidget *parent, const QString &title, const QString &description, const QStringList &helpItems, const QString &doc, const QString &highlight, const QString &link) {
    showMsg(parent, QMessageBox::Information,"Info", title, description, helpItems, doc, highlight, link);
}

void showWarning(QWidget *parent, const QString &title, const QString &description, const QStringList &helpItems, const QString &doc, const QString &highlight, const QString &link) {
    showMsg(parent, QMessageBox::Warning, "Warning", title, description, helpItems, doc, highlight, link);
}

void showMsg(QWidget *parent, QMessageBox::Icon icon, const QString &windowTitle, const QString &title, const QString &description, const QStringList &helpItems, const QString &doc, const QString &highlight, const QString &link) {
    QString informativeText = description;
    if (!helpItems.isEmpty()) {
        informativeText += "\n";
    }
    for (const auto &item : helpItems) {
        informativeText += QString("\n• %1").arg(item);
    }

    auto *msgBox = new QMessageBox(parent);
    msgBox->setText(QString("<b>%1</b>").arg(title));
    msgBox->setInformativeText(informativeText);
    msgBox->setIcon(icon);
    msgBox->setWindowTitle(windowTitle);
    msgBox->setStandardButtons(doc.isEmpty() ? QMessageBox::Ok : (QMessageBox::Ok | QMessageBox::Help));
    msgBox->setDefaultButton(QMessageBox::Ok);

    QPushButton *openLinkButton = nullptr;
    if (!link.isEmpty()) {
        openLinkButton = msgBox->addButton("Open link", QMessageBox::ActionRole);
    }

    msgBox->exec();

    if (openLinkButton && msgBox->clickedButton() == openLinkButton) {
        Utils::externalLinkWarning(parent, link);
    }

    if (msgBox->result() == QMessageBox::Help) {
        windowManager()->showDocs(parent, doc);
        windowManager()->setDocsHighlight(highlight);
    }

    msgBox->deleteLater();
}

void showMsg(const Message &m) {
    showMsg(m.parent, QMessageBox::Warning, "Error", m.title, m.description, m.helpItems, m.doc, m.highlight);
}

void openDir(QWidget *parent, const QString &message, const QString &dir) {
    QMessageBox openDir{parent};
    openDir.setWindowTitle("Info");
    openDir.setText(message);
    QPushButton *copy = openDir.addButton("Open directory", QMessageBox::HelpRole);
    openDir.addButton(QMessageBox::Ok);
    openDir.setDefaultButton(QMessageBox::Ok);
    openDir.exec();

    if (openDir.clickedButton() == copy) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
    }
}

QWindow *windowForQObject(QObject* object) {
    while (object) {
        if (auto widget = qobject_cast<QWidget*>(object)) {
            if (widget->windowHandle()) {
                return widget->windowHandle();
            }
        }
        object = object->parent();
    }
    return nullptr;
}

void clearLayout(QLayout* layout, bool deleteWidgets)
{
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (deleteWidgets) {
            if (QWidget *widget = item->widget()) {
                widget->deleteLater();
            }
        }
        if (QLayout *childLayout = item->layout()) {
            clearLayout(childLayout, deleteWidgets);
        }
        delete item;
    }
}

QString formatSyncStatus(quint64 height, quint64 target, bool daemonSync) {
    if (height < (target - 1)) {
        QString blocks = (target >= height) ? QString::number(target - height) : "?";
        QString type = daemonSync ? "Blockchain" : "Wallet";
        return QString("%1 sync: %2 blocks remaining").arg(type, blocks);
    }

    return "Synchronized";
}
}
