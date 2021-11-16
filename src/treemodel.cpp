#include <QFont>

#include "utils.h"

#include "treemodel.h"

TreeItem::TreeItem(TreeItem *parent)
    : m_parent(parent)
{
}

TreeItem::~TreeItem()
{
    qDeleteAll(m_children);
}

TreeItem* TreeItem::child(int number)
{
    return m_children.value(number);
}

int TreeItem::childCount() const
{
    return m_children.count();
}

int TreeItem::childIndex() const
{
    if (m_parent) {
        return m_parent->m_children.indexOf(const_cast<TreeItem*>(this));
    }

    return 0;
}

void TreeItem::reserveChildren(const qsizetype n)
{
    m_children.reserve(n);
}

QVariant TreeItem::data(int column) const
{
    switch (column) {
        case Column::Title: return title;
        case Column::Value: return value;
        case Column::Type: return type;
        case Column::Size: return size;
        default: return QVariant();
    }
}

void TreeItem::addChild(TreeItem *item)
{
    m_children.append(item);
}

TreeItem* TreeItem::parent()
{
    return m_parent;
}


TreeModel::TreeModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_rootItem(new TreeItem(nullptr))
{
}

TreeModel::~TreeModel()
{
    delete m_rootItem;
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0) {
        return QModelIndex();
    }

    const auto parentItem = itemByIndex(parent);
    const auto childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    } else {
        return QModelIndex();
    }
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    const auto childItem = itemByIndex(index);
    const auto parentItem = childItem->parent();

    if (parentItem == m_rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->childIndex(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0) {
        return 0;
    }

    TreeItem *parentItem = nullptr;
    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = itemByIndex(parent);
    }

    return parentItem->childCount();
}

int TreeModel::columnCount(const QModelIndex &/* parent */) const
{
    return (int)Column::LastColumn;
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &/*index*/) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const auto item = itemByIndex(index);

    if (role == Qt::FontRole && index.column() >= Column::Value) {
        return QVariant(QFont(Utils::monospacedFont()));
    }

    if (role == Qt::ToolTipRole && index.column() == Column::Value) {
        return item->data(index.column());
    }

    if (role == Qt::TextAlignmentRole && index.column() == Column::Size) {
        return Qt::AlignRight;
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    return item->data(index.column());
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case Column::Title: return QLatin1String("Title");
            case Column::Value: return QLatin1String("Value");
            case Column::Type: return QLatin1String("Type");
            case Column::Size: return QLatin1String("Size");
        }
    }

    return QVariant();
}

TreeItem* TreeModel::itemByIndex(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item) {
            return item;
        }
    }
    return m_rootItem;
}
