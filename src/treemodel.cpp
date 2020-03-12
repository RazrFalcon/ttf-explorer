#include "treemodel.h"

TreeItem::TreeItem(const TreeItemData &data, TreeItem *parent)
    : m_parentItem(parent)
    , m_d(data)
{
}

TreeItem::~TreeItem()
{
    qDeleteAll(m_childItems);
}

bool TreeItem::appendChild(TreeItem *item)
{
    m_childItems.append(item);
    return true;
}

void TreeItem::removeChildren()
{
    qDeleteAll(m_childItems);
    m_childItems.clear();
}

int TreeItem::row() const
{
    if (m_parentItem != nullptr) {
        return m_parentItem->m_childItems.indexOf(const_cast<TreeItem*>(this));
    }

    return 0;
}


TreeModel::TreeModel(QObject *parent)
    : QAbstractItemModel(parent)
    ,  m_rootItem(new TreeItem(TreeItemData()))
{
}

TreeModel::~TreeModel()
{
    delete m_rootItem;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    const TreeItemData &d = item->data();

    if (role == Qt::ToolTipRole) {
        if (index.column() == Column::Title) {
            return d.title;
        } else if (index.column() == Column::Value) {
            return d.value;
        }
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (index.column()) {
        case Column::Title : return d.title;
        case Column::Value : return d.value;
        case Column::Type : return d.type;
        default: break;
    }

    return QVariant();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    return item->flags();
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case Column::Title : return QLatin1String("Title");
            case Column::Value : return QLatin1String("Value");
            case Column::Type  : return QLatin1String("Type");
            default: break;
        }
    }

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    TreeItem *parentItem;

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = static_cast<TreeItem*>(parent.internalPointer());
    }

    TreeItem *childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    } else {
        return QModelIndex();
    }
}

QModelIndex TreeModel::index(TreeItem *item) const
{
    return createIndex(item->row(), 0, item);
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parent();

    if (parentItem == m_rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = static_cast<TreeItem*>(parent.internalPointer());
    }

    return parentItem->childrenCount();
}

int TreeModel::columnCount(const QModelIndex &) const
{
    return Column::LastColumn;
}

TreeItem *TreeModel::itemByIndex(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return nullptr;
    }
    return static_cast<TreeItem*>(index.internalPointer());
}

static TreeItem *itemByByteImpl(const uint index, TreeItem *parent)
{
    for (const auto item : parent->childrenList()) {
        if (item->contains(index)) {
            if (item->hasChildren()) {
                return itemByByteImpl(index, item);
            }

            return item;
        }
    }

    return nullptr;
}

TreeItem *TreeModel::itemByByte(const uint index) const
{
    return itemByByteImpl(index, m_rootItem);
}

TreeItem *TreeModel::rootItem() const
{
    return m_rootItem;
}

TreeItem* TreeModel::appendChild(const TreeItemData &data, TreeItem *parent)
{
    if (!parent) {
        parent = rootItem();
    }

    beginInsertRows(index(rowCount(), 0, QModelIndex()), rowCount(), rowCount());

    auto item = new TreeItem(data, parent);
    parent->appendChild(item);

    endInsertRows();

    return item;
}

bool TreeModel::isEmpty() const
{
    return !rootItem()->hasChildren();
}

void TreeModel::itemEditFinished(TreeItem *item)
{
    emit dataChanged(index(item->row(), 0, QModelIndex()),
                     index(item->row(), columnCount(), QModelIndex()));
}

void TreeModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, rowCount());
    m_rootItem->removeChildren();
    endRemoveRows();
}
