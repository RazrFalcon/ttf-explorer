#include "src/algo.h"
#include "tables.h"

struct HvarMasks
{
    static const int Size = 2;
    static const QString Type;

    static HvarMasks parse(const quint8 *data)
    { return { UInt16::parse(data) }; }

    quint16 innerIndexBits() const { return d & 0x000F; }
    quint16 entrySize() const { return ((d & 0x0030) >> 4) + 1; }

    static QString toString(const HvarMasks &value)
    {
        return QString("Inner index bit count: %1\n"
                       "Map entry size: %2")
            .arg(value.innerIndexBits()).arg(value.entrySize());
    }

    quint16 d;
};

const QString HvarMasks::Type = "Masks";


void parseHvarDeltaSet(Parser &parser)
{
    const auto format = parser.read<HvarMasks>("Entry format");
    const auto count = parser.read<UInt16>("Number of entries");

    const auto innerIndexBits = format.innerIndexBits();
    const auto entrySize = format.entrySize();
    parser.readArray("Entries", count, [&](const auto index){
        if (entrySize == 1) {
            static QHash<quint8, QString> cache;
            const auto entry = parser.peek<UInt8>();
            if (!cache.contains(entry)) {
                const auto outerIndex = entry >> (innerIndexBits + 1);
                const auto innerIndex = entry & ((1 << (innerIndexBits + 1)) - 1);
                const auto value = QString("Outer index: %1\nInner index: %2")
                    .arg(outerIndex).arg(innerIndex);
                cache.insert(entry, value);
            }
            parser.readValue<UInt8>(numberToString(index), cache.value(entry));
        } else if (entrySize == 2) {
            static QHash<quint8, QString> cache;
            const auto entry = parser.peek<UInt16>();
            if (!cache.contains(entry)) {
                const auto outerIndex = entry >> (innerIndexBits + 1);
                const auto innerIndex = entry & ((1 << (innerIndexBits + 1)) - 1);
                const auto value = QString("Outer index: %1\nInner index: %2")
                    .arg(outerIndex).arg(innerIndex);
                cache.insert(entry, value);
            }
            parser.readValue<UInt16>(numberToString(index), cache.value(entry));
        } else {
            throw QString("unsupported entry size");
        }
    });
}

void parseHvar(Parser &parser)
{
    const auto start = parser.offset();

    const auto majorVersion = parser.read<UInt16>("Major version");
    const auto minorVersion = parser.read<UInt16>("Minor version");
    if (!(majorVersion == 1 && minorVersion == 0)) {
        throw QString("invalid table version");
    }

    const auto varStoreOffset = parser.read<Offset32>("Item Variation Store offset");
    const auto advanceWidthMappingOffset = parser.read<OptionalOffset32>("Advance width mapping offset");
    const auto lsbMappingOffset = parser.read<OptionalOffset32>("Left side bearing mapping offset");
    const auto rsbMappingOffset = parser.read<OptionalOffset32>("Right side bearing mapping offset");

    enum class OffsetType {
        VariationStore,
        Advance,
        Lsb,
        Rsb,
    };
    struct Offset {
        OffsetType type;
        quint32 offset;
    };
    std::array<Offset, 4> offsets = {{
        { OffsetType::VariationStore, (quint32)varStoreOffset },
        { OffsetType::Advance, (quint32)advanceWidthMappingOffset },
        { OffsetType::Lsb, (quint32)lsbMappingOffset },
        { OffsetType::Rsb, (quint32)rsbMappingOffset },
    }};

    algo::sort_all_by_key(offsets, &Offset::offset);

    for (const auto offset : offsets) {
        if (offset.offset == 0) {
            continue;
        }

        parser.advanceTo(start + offset.offset);
        switch (offset.type) {
        case OffsetType::VariationStore: {
            parser.beginGroup("Item Variation Store");
            parseItemVariationStore(parser);
            parser.endGroup();
            break;
        }
        case OffsetType::Advance: {
            parser.beginGroup("Advance Width Mapping");
            parseHvarDeltaSet(parser);
            parser.endGroup();
            break;
        }
        case OffsetType::Lsb: {
            parser.beginGroup("Left Side Bearing Mapping");
            parseHvarDeltaSet(parser);
            parser.endGroup();
            break;
        }
        case OffsetType::Rsb: {
            parser.beginGroup("Right Side Bearing Mapping");
            parseHvarDeltaSet(parser);
            parser.endGroup();
            break;
        }
        }
    }
}
