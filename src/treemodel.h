#pragma once

#include <QAbstractItemModel>

#include "range.h"

namespace Column
{
    enum Column
    {
        Title,
        Value,
        Type,
        Size,
        LastColumn,
    };
}

class TreeItem
{
public:
    explicit TreeItem(TreeItem *parent);
    virtual ~TreeItem();

    TreeItem *child(int number);
    bool hasChildren() const { return !m_children.isEmpty(); };
    int childCount() const;
    virtual QVariant data(int column) const;
    void addChild(TreeItem *item);
    TreeItem *parent();
    int childIndex() const;
    void reserveChildren(const qsizetype n);

public:
    QString title;
    QString value;
    QString type;
    Range range;
    QString size;

private:
    TreeItem * const m_parent;
    QVector<TreeItem*> m_children;
};

class TreeModel : public QAbstractItemModel
{
public:
    TreeModel(QObject *parent = nullptr);
    ~TreeModel();

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    TreeItem* rootItem() const { return m_rootItem; }
    TreeItem* itemByIndex(const QModelIndex &index) const;

private:
    TreeItem * const m_rootItem;
};
