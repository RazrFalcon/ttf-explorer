#pragma once

#include <QAbstractItemModel>
#include <QStyledItemDelegate>

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

struct TreeItemData
{
    QString title;
    QString value;
    QString type;
    quint32 start = 0;
    quint32 end = 0;
};

class TreeItem
{
public:
    TreeItem(const TreeItemData &data, TreeItem *parent = nullptr);
    ~TreeItem();

    TreeItem *parent()
    { return m_parentItem; }

    int row() const;

    Qt::ItemFlags flags() const
    { return Qt::ItemIsSelectable | Qt::ItemIsEnabled; }

    TreeItem *child(int row)
    { return m_childItems.value(row); }

    QVector<TreeItem *> childrenList() const
    { return m_childItems; }

    int childrenCount() const
    { return m_childItems.count(); }

    bool hasChildren() const
    { return !m_childItems.isEmpty(); }

    bool appendChild(TreeItem *child);
    void removeChildren();

    const TreeItemData& data() const
    { return m_d; }

    TreeItemData& data()
    { return m_d; }

    bool contains(const uint index) const
    { return index >= m_d.start && index < m_d.end; }

private:
    TreeItem * const m_parentItem;
    TreeItemData m_d;
    QVector<TreeItem*> m_childItems; // TODO: optimize
};

class TreeModel : public QAbstractItemModel
{
public:
    explicit TreeModel(QObject *parent = nullptr);
    ~TreeModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(TreeItem *item) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    void itemEditFinished(TreeItem *item);

    TreeItem *itemByIndex(const QModelIndex &index) const;
    TreeItem *itemByByte(const uint index) const;
    TreeItem *rootItem() const;

    TreeItem *appendChild(const TreeItemData &data, TreeItem *parent = nullptr);

    bool isEmpty() const;
    void clear();

private:
    TreeItem * const m_rootItem;
};
