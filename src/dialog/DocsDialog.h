// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_DOCSDIALOG_H
#define FEATHER_DOCSDIALOG_H

#include <QDialog>
#include <QStringListModel>
#include <QTreeWidgetItem>

#include "components.h"

namespace Ui {
    class DocsDialog;
}

class DocsDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit DocsDialog(QWidget *parent = nullptr);
    ~DocsDialog() override;

    void filterIndex(const QString &text);
    void showDoc(const QString &doc, const QString& highlight = "");
    void updateHighlights(const QString &highlight, bool scrollToCursor = false);
    bool wordMatch(QString &search);

private:
    QScopedPointer<Ui::DocsDialog> ui;

    QString m_currentSource = "";

    QMap<QString, QString> m_docs;
    QMap<QString, QStringList> m_categoryIndex;
    QMap<QString, QString> m_navTitleIndex;

    QMap<QString, QTreeWidgetItem *> m_items;
};

#endif //FEATHER_DOCSDIALOG_H
