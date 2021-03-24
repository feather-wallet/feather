// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "HistoryView.h"

#include "TransactionHistoryProxyModel.h"
#include "libwalletqt/TransactionInfo.h"

#include <QHeaderView>
#include <QMenu>

HistoryView::HistoryView(QWidget *parent)
    : QTreeView(parent)
    , m_model(nullptr)
    , m_headerMenu(new QMenu(this))
{
    setUniformRowHeights(true);
    setRootIsDecorated(false);
    setAlternatingRowColors(true);
    setDragEnabled(true);
    setSortingEnabled(true);

    header()->setStretchLastSection(false);
    header()->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(header(), SIGNAL(customContextMenuRequested(QPoint)), SLOT(showHeaderMenu(QPoint)));
}

void HistoryView::setHistoryModel(TransactionHistoryProxyModel *model) {
    m_model = model;
    m_model->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_model->setSortRole(Qt::UserRole);
    m_model->setDynamicSortFilter(true);
    m_model->setSortLocaleAware(true);

    QTreeView::setModel(m_model);
    resetViewToDefaults();

    m_headerMenu->clear();

    // Actions to toggle column visibility, each carrying the corresponding
    // column index as data
    m_columnActions = new QActionGroup(this);
    m_columnActions->setExclusive(false);
    for (int visualIndex = 1; visualIndex < header()->count(); ++visualIndex) {
        int logicalIndex = header()->logicalIndex(visualIndex);
        QString caption = m_model->headerData(logicalIndex, Qt::Horizontal, Qt::DisplayRole).toString();
        if (caption.isEmpty()) {
            caption = m_model->headerData(logicalIndex, Qt::Horizontal, Qt::ToolTipRole).toString();
        }

        auto action = m_headerMenu->addAction(caption);
        action->setCheckable(true);
        action->setData(logicalIndex);
        m_columnActions->addAction(action);
    }
    connect(m_columnActions, &QActionGroup::triggered, this, &HistoryView::toggleColumnVisibility);

    m_headerMenu->addSeparator();
    m_headerMenu->addAction(tr("Fit to window"), this, &HistoryView::fitColumnsToWindow);
    m_headerMenu->addAction(tr("Fit to contents"), this, &HistoryView::fitColumnsToContents);
    m_headerMenu->addSeparator();
    m_headerMenu->addAction(tr("Reset to defaults"), this, &HistoryView::resetViewToDefaults);

    fitColumnsToWindow();
}

TransactionHistoryModel* HistoryView::sourceModel()
{
    return dynamic_cast<TransactionHistoryModel *>(m_model->sourceModel());
}

TransactionInfo* HistoryView::currentEntry()
{
    QModelIndexList list = selectionModel()->selectedRows();
    if (list.size() == 1) {
        return this->sourceModel()->entryFromIndex(m_model->mapToSource(list.first()));
    } else {
        return nullptr;
    }
}

void HistoryView::setSearchMode(bool mode) {
    m_inSearchMode = mode;

    if (mode) {
        header()->showSection(TransactionHistoryModel::TxID);
    } else {
        header()->hideSection(TransactionHistoryModel::TxID);
    }
}

QByteArray HistoryView::viewState() const
{
    return header()->saveState();
}

bool HistoryView::setViewState(const QByteArray& state)
{
    // Reset to unsorted first (https://bugreports.qt.io/browse/QTBUG-86694)
    header()->setSortIndicator(-1, Qt::AscendingOrder);
    bool status = header()->restoreState(state);
    m_columnsNeedRelayout = state.isEmpty();

    m_showTxidColumn = !header()->isSectionHidden(TransactionHistoryModel::TxID);
    return status;
}

void HistoryView::showHeaderMenu(const QPoint& position)
{
    const QList<QAction*> actions = m_columnActions->actions();
    for (auto& action : actions) {
        Q_ASSERT(static_cast<QMetaType::Type>(action->data().type()) == QMetaType::Int);
        if (static_cast<QMetaType::Type>(action->data().type()) != QMetaType::Int) {
            continue;
        }
        int columnIndex = action->data().toInt();
        action->setChecked(!isColumnHidden(columnIndex));
    }

    m_headerMenu->popup(mapToGlobal(position));
}

void HistoryView::toggleColumnVisibility(QAction* action)
{
    // Verify action carries a column index as data. Since QVariant.toInt()
    // below will accept anything that's interpretable as int, perform a type
    // check here to make sure data actually IS int
    Q_ASSERT(static_cast<QMetaType::Type>(action->data().type()) == QMetaType::Int);
    if (static_cast<QMetaType::Type>(action->data().type()) != QMetaType::Int) {
        return;
    }

    // Toggle column visibility. Visible columns will only be hidden if at
    // least one visible column remains, as the table header will disappear
    // entirely when all columns are hidden
    int columnIndex = action->data().toInt();
    if (action->isChecked()) {
        header()->showSection(columnIndex);
        if (header()->sectionSize(columnIndex) == 0) {
            header()->resizeSection(columnIndex, header()->defaultSectionSize());
        }
        return;
    }
    if ((header()->count() - header()->hiddenSectionCount()) > 1) {
        header()->hideSection(columnIndex);
        return;
    }
    action->setChecked(true);
}

void HistoryView::fitColumnsToWindow()
{
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(TransactionHistoryModel::Description, QHeaderView::Stretch);
    header()->setStretchLastSection(false);
    QCoreApplication::processEvents();
}

void HistoryView::fitColumnsToContents()
{
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    QCoreApplication::processEvents();
    header()->setSectionResizeMode(QHeaderView::Interactive);
}

void HistoryView::resetViewToDefaults()
{
    if (m_inSearchMode) {
        header()->showSection(TransactionHistoryModel::TxID);
    } else {
        header()->hideSection(TransactionHistoryModel::TxID);
    }
    header()->showSection(TransactionHistoryModel::Date);
    header()->showSection(TransactionHistoryModel::Description);
    header()->showSection(TransactionHistoryModel::Amount);
    header()->showSection(TransactionHistoryModel::FiatAmount);

    // Reset column order to logical indices
    for (int i = 0; i < header()->count(); ++i) {
        header()->moveSection(header()->visualIndex(i), i);
    }

    m_model->sort(TransactionHistoryModel::Date, Qt::DescendingOrder);
    sortByColumn(TransactionHistoryModel::Date, Qt::DescendingOrder);

    // The following call only relayouts reliably if the widget has been shown
    // already, so only do it if the widget is visible and let showEvent() handle
    // the initial default layout.
    if (isVisible()) {
        fitColumnsToWindow();
    }
}

void HistoryView::keyPressEvent(QKeyEvent *event) {
    TransactionInfo* tx = this->currentEntry();

    if (event->matches(QKeySequence::Copy) && tx) {
        Utils::copyToClipboard(tx->hash());
    }
    else {
        QTreeView::keyPressEvent(event);
    }
}