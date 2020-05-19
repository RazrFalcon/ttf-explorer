#pragma once

#include <QString>
#include <QScopedPointer>

#include <optional>
#include <memory>

#include "range.h"

namespace TTFCore {

typedef struct ttfcore_tree ttfcore_tree;

using TreeItemId = uintptr_t;

class Tree {
public:
    static std::pair<Tree, QString> parse(const QByteArray &data);

    QString itemTitle(const TreeItemId id) const;
    std::optional<quint32> itemIndex(const TreeItemId id) const;
    QString itemValue(const TreeItemId id) const;
    QString itemValueType(const TreeItemId id) const;
    Range itemRange(const TreeItemId id) const;

    std::optional<TreeItemId> childAt(const TreeItemId parentId, const int row) const;
    int childIndex(const TreeItemId childId) const;
    int childrenCount(const TreeItemId id) const;
    bool hasChildren(const TreeItemId id) const;
    std::optional<TreeItemId> parentItem(const TreeItemId id) const;

private:
    Tree(ttfcore_tree *tree) : d(tree, Tree::freeTree) {}

    static void freeTree(ttfcore_tree *tree);

private:
    std::unique_ptr<ttfcore_tree, void(*)(ttfcore_tree*)> d;
};

}
