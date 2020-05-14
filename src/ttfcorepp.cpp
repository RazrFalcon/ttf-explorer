#include <ttfcore.h>

#include "ttfcorepp.h"

namespace Title {
    static const QString Action = QLatin1String("Action");
    static const QString Class = QLatin1String("Class");
    static const QString Code = QLatin1String("Code");
    static const QString Delta = QLatin1String("Delta");
    static const QString Endpoint = QLatin1String("Endpoint");
    static const QString Glyph = QLatin1String("Glyph");
    static const QString Index = QLatin1String("Index");
    static const QString Name = QLatin1String("Name");
    static const QString Number = QLatin1String("Number");
    static const QString Offset = QLatin1String("Offset");
    static const QString String = QLatin1String("String");
    static const QString StringID = QLatin1String("StringID");
    static const QString Subroutine = QLatin1String("Subroutine");
    static const QString Value = QLatin1String("Value");
}

namespace TypeName {
    static const QString Magic = QLatin1String("Magic");
    static const QString Int8 = QLatin1String("Int8");
    static const QString Int16 = QLatin1String("Int16");
    static const QString Int32 = QLatin1String("Int32");
    static const QString UInt8 = QLatin1String("UInt8");
    static const QString UInt16 = QLatin1String("UInt16");
    static const QString UInt24 = QLatin1String("UInt24");
    static const QString UInt32 = QLatin1String("UInt32");
    static const QString F2DOT14 = QLatin1String("F2DOT14");
    static const QString Fixed = QLatin1String("Fixed");
    static const QString Offset16 = QLatin1String("Offset16");
    static const QString Offset32 = QLatin1String("Offset32");
    static const QString GlyphId = QLatin1String("GlyphId");
    static const QString Tag = QLatin1String("Tag");
    static const QString BitFlags = QLatin1String("BitFlags");
    static const QString Masks = QLatin1String("Masks");
    static const QString PlatformId = QLatin1String("PlatformId");
    static const QString OffsetSize = QLatin1String("OffsetSize");
    static const QString LongDateTime = QLatin1String("LongDateTime");
    static const QString String = QLatin1String("String");
    static const QString Bytes = QLatin1String("Bytes");
}

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
    switch (ttfcore_tree_item_title_type(d.get(), id)) {
        case  0 : {
            uintptr_t len = 0;
            const auto ptr = ttfcore_tree_item_title(d.get(), id, &len);
            return QString::fromUtf8(ptr, (int)len);
        }
        case  1 : return Title::Action;
        case  2 : return Title::Class;
        case  3 : return Title::Code;
        case  4 : return Title::Delta;
        case  5 : return Title::Endpoint;
        case  6 : return Title::Glyph;
        case  7 : return Title::Index;
        case  8 : return Title::Name;
        case  9 : return Title::Number;
        case 10 : return Title::Offset;
        case 11 : return Title::String;
        case 12 : return Title::StringID;
        case 13 : return Title::Subroutine;
        case 14 : return Title::Value;
        default: Q_UNREACHABLE();
    }
}

std::optional<quint32> TTFCore::Tree::itemIndex(const TTFCore::TreeItemId id) const
{
    const int idx = ttfcore_tree_item_index(d.get(), id);
    if (idx >= 0) {
        return quint32(idx);
    } else {
        return std::nullopt;
    }
}

QString TTFCore::Tree::itemValue(const TTFCore::TreeItemId id) const
{
    uintptr_t len = 0;
    const auto ptr = ttfcore_tree_item_value(d.get(), id, &len);
    return QString::fromUtf8(ptr, (int)len);
}

QString TTFCore::Tree::itemValueType(const TTFCore::TreeItemId id) const
{
    const auto typeId = ttfcore_tree_item_value_type(d.get(), id);
    switch (typeId) {
        case  0 : return QString();
        case  1 : return TypeName::Magic;
        case  2 : return TypeName::Int8;
        case  3 : return TypeName::Int16;
        case  4 : return TypeName::Int32;
        case  5 : return TypeName::UInt8;
        case  6 : return TypeName::UInt16;
        case  7 : return TypeName::UInt24;
        case  8 : return TypeName::UInt32;
        case  9 : return TypeName::F2DOT14;
        case 10 : return TypeName::Fixed;
        case 11 : return TypeName::Offset16;
        case 12 : return TypeName::Offset32;
        case 13 : return TypeName::GlyphId;
        case 14 : return TypeName::Tag;
        case 15 : return TypeName::BitFlags;
        case 16 : return TypeName::Masks;
        case 17 : return TypeName::PlatformId;
        case 18 : return TypeName::OffsetSize;
        case 19 : return TypeName::LongDateTime;
        case 20 : return TypeName::String;
        case 21 : return TypeName::Bytes;
        default: Q_UNREACHABLE();
    }
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
