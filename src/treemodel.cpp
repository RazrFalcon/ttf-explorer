#include "treemodel.h"

TreeModel::TreeModel(TTFCore::Tree &&tree)
    : QAbstractItemModel(nullptr)
    , m_tree(std::move(tree))
{
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (index.column()) {
        case Column::Title : return m_tree.itemTitle(index.internalId());
        case Column::Value : return m_tree.itemValue(index.internalId());
        case Column::Type : return m_tree.itemValueType(index.internalId());
        default: break;
    }

    return QVariant();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case Column::Title : return QLatin1String("Title");
            case Column::Value : return QLatin1String("Value");
            case Column::Type  : return QLatin1String("Type");
            default : break;
        }
    }

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    TreeItemId parentId = m_rootId;
    if (parent.isValid()) {
        parentId = parent.internalId();
    }

    if (const auto childId = m_tree.childAt(parentId, row)) {
        return createIndex(row, column, childId.value());
    } else {
        return QModelIndex();
    }
}

QModelIndex TreeModel::index(const TreeItemId id) const
{
    return createIndex(m_tree.childIndex(id), 0, id);
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    const auto parentIdOpt = parentItem(index.internalId());
    if (!parentIdOpt) {
        return QModelIndex();
    }
    const auto parentId = parentIdOpt.value();

    if (parentId == m_rootId) {
        return QModelIndex();
    }

    return createIndex(0, 0, parentId);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return m_tree.childrenCount(m_rootId);
    } else {
        return m_tree.childrenCount(parent.internalId());
    }
}

int TreeModel::columnCount(const QModelIndex &) const
{
    return Column::LastColumn;
}

QVector<Range> TreeModel::collectRanges() const
{
    QVector<Range> ranges;
    collectRangesImpl(m_rootId, ranges);

    // Make sure to sort them.
    std::sort(std::begin(ranges), std::end(ranges), [](const auto &a, const auto &b) {
        return a.start < b.start;
    });

    return ranges;
}

std::optional<TreeItemId> TreeModel::parentItem(const TreeItemId id) const
{
    return m_tree.parentItem(id);
}

QString TreeModel::itemTitle(const TreeItemId id) const
{
    return m_tree.itemTitle(id);
}

Range TreeModel::itemRange(const TreeItemId id) const
{
    return m_tree.itemRange(id);
}

void TreeModel::collectRangesImpl(TreeItemId parentId, QVector<Range> &ranges) const
{
    const auto childrenCount = m_tree.childrenCount(parentId);
    for (int i = 0; i < childrenCount; ++i) {
        const auto childId = m_tree.childAt(parentId, i).value();

        if (m_tree.hasChildren(childId)) {
            collectRangesImpl(childId, ranges);
        } else {
            ranges.append(m_tree.itemRange(childId));
        }
    }
}

std::optional<TreeItemId> TreeModel::itemByByte(const uint index) const
{
    return itemByByteImpl(index, m_rootId);
}

std::optional<TreeItemId> TreeModel::itemByByteImpl(const uint index, const TreeItemId parentId) const
{
    const auto childrenCount = m_tree.childrenCount(parentId);
    for (int i = 0; i < childrenCount; ++i) {
        const auto childId = m_tree.childAt(parentId, i).value();
        const auto range = itemRange(childId);

        if (range.contains(index)) {
            if (m_tree.hasChildren(childId)) {
                return itemByByteImpl(index, childId);
            }

            return childId;
        }
    }

    return std::nullopt;
}
