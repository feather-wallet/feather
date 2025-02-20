// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "DocsDialog.h"
#include "ui_DocsDialog.h"

#include <QScrollBar>
#include <QDirIterator>

#include "utils/Utils.h"
#include "ColorScheme.h"

DocsDialog::DocsDialog(QWidget *parent)
        : WindowModalDialog(parent)
        , ui(new Ui::DocsDialog)
{
    ui->setupUi(this);

    ui->toolButton_backward->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    ui->toolButton_forward->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));

    ui->splitter->setStretchFactor(1, 8);

    QRegularExpression navTitleRe{R"(\[nav_title\]: # \((.+?)\)\n)"};
    QRegularExpression categoryRe{R"(\[category\]: # \((.+?)\)\n)"};

    QSet<QString> categories;

    QDirIterator it(":/docs/", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString resource = it.next();
        QString docString = Utils::loadQrc(resource);

        resource = "qrc" + resource;

        // Extract navigation title from metadata
        QRegularExpressionMatch navTitleMatch = navTitleRe.match(docString);
        if (!navTitleMatch.hasMatch()) {
            continue;
        }
        QString navTitle = navTitleMatch.captured(1);
        navTitle.replace("\\(", "(").replace("\\)", ")");

        // Extract category from metadata
        QRegularExpressionMatch categoryMatch = categoryRe.match(docString);
        if (!categoryMatch.hasMatch()) {
            continue;
        }
        QString category = categoryMatch.captured(1);

        m_categoryIndex[category].append(resource);
        m_navTitleIndex[resource] = navTitle;

        categories += category;

        m_docs[resource] = docString.toLower();
    }

    // Add categories and docs to index
    QList<QTreeWidgetItem *> items;
    for (const auto& category : categories) {
        auto *categoryWidget = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), {category});

        QList<QTreeWidgetItem *> subItems;
        for (const auto& resource : m_categoryIndex[category]) {
            auto *docWidget = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), {m_navTitleIndex[resource]});
            docWidget->setData(0, Qt::UserRole,resource);
            subItems.append(docWidget);
            m_items[resource] = docWidget;
        }

        categoryWidget->addChildren(subItems);
        items.append(categoryWidget);
    }
    ui->index->addTopLevelItems(items);
    ui->index->sortItems(0, Qt::SortOrder::AscendingOrder);

    connect(ui->index, &QTreeWidget::itemClicked, [this](QTreeWidgetItem *current, int column){
        if (ui->index->indexOfTopLevelItem(current) != -1) {
            current->setExpanded(!current->isExpanded());
            return;
        }

        QString resource = current->data(0, Qt::UserRole).toString();
        this->showDoc(resource);
    });

    connect(ui->textBrowser, &QTextBrowser::anchorClicked, [this](const QUrl &url){
        QString doc = url.toString();

        if (!doc.startsWith("qrc")) {
            Utils::externalLinkWarning(this, doc);
            this->showDoc(m_currentSource);
            return;
        }

        doc.replace("-", "_");
        doc += ".md";
        this->showDoc(doc);
    });

    connect(ui->textBrowser, &QTextBrowser::sourceChanged, [this](const QUrl& source){
        this->updateHighlights(ui->search->text());

        QString newSource = source.toString();
        if (m_currentSource == newSource) {
            return;
        }
        m_currentSource = newSource;

        if (m_items.contains(m_currentSource)) {
            ui->index->setCurrentItem(m_items[m_currentSource]);
        }
    });

    connect(ui->textBrowser, &QTextBrowser::backwardAvailable, [this](bool available){
       ui->toolButton_backward->setEnabled(available);
    });

    connect(ui->textBrowser, &QTextBrowser::forwardAvailable, [this](bool available){
       ui->toolButton_forward->setEnabled(available);
    });

    connect(ui->toolButton_backward, &QToolButton::clicked, [this]{
       ui->textBrowser->backward();
    });

    connect(ui->toolButton_forward, &QToolButton::clicked, [this]{
        ui->textBrowser->forward();
    });

    connect(ui->search, &QLineEdit::textEdited, [this](const QString &text){
        this->filterIndex(text);
        this->updateHighlights(ui->search->text());
    });

    // Pressing 'enter' in the search box shouldn't close the dialog
    QPushButton *closeButton = ui->buttonBox->button(QDialogButtonBox::Close);
    if (closeButton) {
        closeButton->setAutoDefault(false);
    }

    this->showDoc("report_an_issue");
}

void DocsDialog::filterIndex(const QString &text) {
    QTreeWidgetItemIterator it(ui->index);
    while (*it) {
        QString resource = (*it)->data(0, Qt::UserRole).toString();
        bool docContainsText = m_docs[resource].contains(text, Qt::CaseInsensitive);
        bool titleContainsText = (*it)->text(0).contains(text, Qt::CaseInsensitive);

        if (titleContainsText && !text.isEmpty()) {
            ColorScheme::updateFromWidget(this);
            (*it)->setBackground(0, ColorScheme::YELLOW.asColor(true));
        } else {
            (*it)->setBackground(0, Qt::transparent);
        }

        if (docContainsText || titleContainsText) {
            (*it)->setHidden(false);

            QTreeWidgetItem *parent = (*it)->parent();
            if (parent) {
                parent->setHidden(false);
                parent->setExpanded(true);
            }
        } else {
            (*it)->setHidden(true);
        }
        ++it;
    }
}

void DocsDialog::showDoc(const QString &doc, const QString &highlight) {
    ColorScheme::updateFromWidget(this);
    QPalette p = qApp->palette();
    p.setBrush(QPalette::Link, ColorScheme::darkScheme ? ColorScheme::BLUE.asColor() : QColor("blue"));
    qApp->setPalette(p);

    QString resource = doc;

    if (!resource.startsWith("qrc")) {
        resource = "qrc:/docs/" + doc + ".md";
    }

    if (m_items.contains(resource)) {
        ui->index->setCurrentItem(m_items[doc]);
    }

    QString file = resource;
    file.remove("qrc");
    if (!QFile::exists(file)) {
        Utils::showError(this, "Unable to load document", "File does not exist");
        ui->textBrowser->setSource(m_currentSource);
        return;
    }

    ui->textBrowser->setSource(QUrl(resource));
}

void DocsDialog::updateHighlights(const QString &searchString, bool scrollToCursor) {
    QTextDocument *document = ui->textBrowser->document();
    QTextCursor cursor(document);

    // Clear highlighting
    QTextCursor cursor2(document);
    cursor2.select(QTextCursor::Document);
    QTextCharFormat defaultFormat;
    defaultFormat.setBackground(Qt::transparent);
    cursor2.mergeCharFormat(defaultFormat);

    bool firstFind = true;
    if (!searchString.isEmpty()) {
        while (!cursor.isNull() && !cursor.atEnd()) {
            cursor = document->find(searchString, cursor);

            if (!cursor.isNull()) {
                if (firstFind) {
                    // Scroll to first match
                    QRect cursorRect = ui->textBrowser->cursorRect(cursor);
                    int positionToScroll = ui->textBrowser->verticalScrollBar()->value() + cursorRect.top();
                    ui->textBrowser->verticalScrollBar()->setValue(positionToScroll);

                    firstFind = false;
                }

                ColorScheme::updateFromWidget(this);
                QTextCharFormat colorFormat(cursor.charFormat());
                colorFormat.setBackground(ColorScheme::YELLOW.asColor(true));
                cursor.mergeCharFormat(colorFormat);
            }
        }
    }
}

DocsDialog::~DocsDialog() = default;