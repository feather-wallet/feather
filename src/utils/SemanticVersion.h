// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_SEMANTICVERSION_H
#define FEATHER_SEMANTICVERSION_H

#include <QObject>

struct SemanticVersion
{
    explicit SemanticVersion(int major=0, int minor=0, int patch=0, int release=0)
            : patch(patch), release(release)
    {
        this->major = major;
        this->minor = minor;
    }

    friend bool operator== (const SemanticVersion &v1, const SemanticVersion &v2) {
        return (v1.major == v2.major &&
                v1.minor == v2.minor &&
                v1.patch == v2.patch &&
                v1.release == v2.release);
    }

    friend bool operator!= (const SemanticVersion &v1, const SemanticVersion &v2) {
        return !(v1 == v2);
    }

    friend bool operator> (const SemanticVersion &v1, const SemanticVersion &v2) {
        if (v1.major != v2.major)
            return v1.major > v2.major;
        if (v1.minor != v2.minor)
            return v1.minor > v2.minor;
        if (v1.patch != v2.patch)
            return v1.patch > v2.patch;
        if (v1.release != v2.release)
            return v1.release > v2.release;
        return false;
    }

    friend bool operator< (const SemanticVersion &v1, const SemanticVersion &v2) {
        if (v1 == v2)
            return false;
        return !(v1 > v2);
    }

    friend bool operator <= (const SemanticVersion &v1, const SemanticVersion &v2) {
        if (v1 == v2)
            return true;
        return v1 < v2;
    }

    friend bool operator >= (const SemanticVersion &v1, const SemanticVersion &v2) {
        if (v1 == v2)
            return true;
        return v1 > v2;
    }

    QString toString() const {
        return QString("%1.%2.%3.%4").arg(QString::number(major), QString::number(minor),
                                          QString::number(patch), QString::number(release));
    }

    static SemanticVersion fromString(const QString &ver) {
        SemanticVersion version;

        if (ver.contains("beta")) {
            QRegularExpression verRe("beta-(?<minor>\\d+)");
            QRegularExpressionMatch match = verRe.match(ver);
            version.minor = match.captured("minor").toInt();
            return version;
        }

        QRegularExpression re(R"((?<major>\d+)\.(?<minor>\d+)\.(?<patch>\d+)(\.(?<release>\d+))?)");
        QRegularExpressionMatch match = re.match(ver);

        version.major = match.captured("major").toInt();
        version.minor = match.captured("minor").toInt();
        version.patch = match.captured("patch").toInt();
        version.release = match.captured("release").toInt();
        return version;
    }

    static bool isValid(const SemanticVersion &v) {
        return v != SemanticVersion();
    }

    int major = 0;
    int minor = 0;
    int patch = 0;
    int release = 0;
};

#endif //FEATHER_SEMANTICVERSION_H
