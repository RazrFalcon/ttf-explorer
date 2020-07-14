#pragma once

#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include <QStaticText>

#include "range.h"
#include "ttfcorepp.h"

struct ttfcore_tree;

namespace Column
{
    enum Column
    {
        Title,
        Value,
        Type,
        LastColumn,
    };
}

using TreeItemId = uintptr_t;

class TitleItemDelegate : public QStyledItemDelegate
{
public:
    TitleItemDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

private:
    // Keep indices string representation since `QString::number` is expensive.
    mutable QVector<QString> m_indexCache;
};

class TreeModel : public QAbstractItemModel
{
public:
    explicit TreeModel(TTFCore::Tree &&tree);

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(const TreeItemId id) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVector<Range> collectRanges() const;

    std::optional<TreeItemId> parentItem(const TreeItemId id) const;
    QString itemTitle(const TreeItemId id) const;
    std::optional<quint32> itemIndex(const TreeItemId id) const;
    Range itemRange(const TreeItemId id) const;
    std::optional<TreeItemId> itemByByte(const uint index) const;
    TreeItemId rootId() const { return m_rootId; }

private:
    std::optional<TreeItemId> itemByByteImpl(const uint index, const TreeItemId parentId) const;

private:
    const TreeItemId m_rootId = 1;
    TTFCore::Tree m_tree;
};
