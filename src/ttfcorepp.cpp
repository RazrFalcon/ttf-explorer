#include <ttfcore.h>

#include "ttfcorepp.h"

std::pair<TTFCore::Tree, QString> TTFCore::Tree::parse(const QByteArray &data)
{
    ttfcore_tree *coreTree = nullptr;
    const bool ok = ttfcore_parse_data(data.data(), data.size(), &coreTree);
    if (!ok) {
        throw QString(QLatin1String("Failed to parse a font file."));
    }

    QString warnings;
    {
        uintptr_t len = 0;
        const auto ptr = ttfcore_tree_warnings(coreTree, &len);
        warnings = QString::fromUtf8(ptr, (int)len);
    }

    return std::make_pair(Tree(coreTree), warnings);
}

QString TTFCore::Tree::itemTitle(const TTFCore::TreeItemId id) const
{
    uintptr_t len = 0;
    const auto ptr = ttfcore_tree_item_title(d.get(), id, &len);
    return QString::fromUtf8(ptr, (int)len);
}

QString TTFCore::Tree::itemValue(const TTFCore::TreeItemId id) const
{
    uintptr_t len = 0;
    const auto ptr = ttfcore_tree_item_value(d.get(), id, &len);
    return QString::fromUtf8(ptr, (int)len);
}

QString TTFCore::Tree::itemValueType(const TTFCore::TreeItemId id) const
{
    uintptr_t len = 0;
    const auto ptr = ttfcore_tree_item_value_type(d.get(), id, &len);
    return QString::fromUtf8(ptr, (int)len);
}

Range TTFCore::Tree::itemRange(const TTFCore::TreeItemId id) const
{
    uintptr_t start = 0;
    uintptr_t end = 0;
    ttfcore_tree_item_range(d.get(), id, &start, &end);
    return { (uint)start, (uint)end };
}

std::optional<TTFCore::TreeItemId> TTFCore::Tree::childAt(const TTFCore::TreeItemId parentId, const int row) const
{
    const auto id = ttfcore_tree_item_child_at(d.get(), parentId, (uintptr_t)row);
    if (id != 0) {
        return id;
    } else {
        return std::nullopt;
    }
}

int TTFCore::Tree::childIndex(const TTFCore::TreeItemId childId) const
{
    return (int)ttfcore_tree_item_child_index(d.get(), childId);
}

int TTFCore::Tree::childrenCount(const TTFCore::TreeItemId id) const
{
    return (int)ttfcore_tree_item_children_count(d.get(), id);
}

bool TTFCore::Tree::hasChildren(const TTFCore::TreeItemId id) const
{
    return ttfcore_tree_item_has_children(d.get(), id);
}

std::optional<TTFCore::TreeItemId> TTFCore::Tree::parentItem(const TTFCore::TreeItemId id) const
{
    const auto parentId = ttfcore_tree_item_parent(d.get(), id);
    if (parentId != 0) {
        return parentId;
    } else {
        return std::nullopt;
    }
}

void TTFCore::Tree::freeTree(TTFCore::ttfcore_tree *tree)
{
    ttfcore_free_tree(tree);
}
