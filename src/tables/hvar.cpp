#include "src/parser.h"
#include "tables.h"

struct HvarMasks
{
    static const int Size = 2;
    static const QString Type;

    static HvarMasks parse(const quint8 *data)
    {
        return { qFromBigEndian<quint16>(data) };
    }

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
    parser.beginGroup("Entries");
    for (int i = 0; i < count; ++i) {
        if (entrySize == 1) {
            const auto entry = parser.peek<UInt8>();
            const auto outerIndex = entry >> (innerIndexBits + 1);
            const auto innerIndex = entry & ((1 << (innerIndexBits + 1)) - 1);
            parser.readValue(1, "Entry",
                QString("Outer index: %1\nInner index: %2").arg(outerIndex).arg(innerIndex));
        } else if (entrySize == 2) {
            const auto entry = parser.peek<UInt16>();
            const auto outerIndex = entry >> (innerIndexBits + 1);
            const auto innerIndex = entry & ((1 << (innerIndexBits + 1)) - 1);
            parser.readValue(2, "Entry",
                QString("Outer index: %1\nInner index: %2").arg(outerIndex).arg(innerIndex));
        } else {
            throw "unsupported entry size";
        }
    }
    parser.endGroup();
}

void parseHvar(Parser &parser)
{
    const auto start = parser.offset();

    const auto majorVersion = parser.read<UInt16>("Major version");
    const auto minorVersion = parser.read<UInt16>("Minor version");
    if (!(majorVersion == 1 && minorVersion == 0)) {
        throw "invalid table version";
    }

    const auto varStoreOffset = parser.read<Offset32>("Item variation store offset");
    const auto advanceWidthMappingOffset = parser.read<std::optional<Offset32>>("Advance width mapping offset");
    const auto lsbMappingOffset = parser.read<std::optional<Offset32>>("Left side bearing mapping offset");
    const auto rsbMappingOffset = parser.read<std::optional<Offset32>>("Right side bearing mapping offset");

    parser.jumpTo(start + varStoreOffset);
    parser.beginGroup("Item variation store");
    parseItemVariationStore(parser);
    parser.endGroup();

    if (advanceWidthMappingOffset.has_value()) {
        parser.jumpTo(start + advanceWidthMappingOffset.value());
        parser.beginGroup("Advance width mapping");
        parseHvarDeltaSet(parser);
        parser.endGroup();
    }

    if (lsbMappingOffset.has_value()) {
        parser.jumpTo(start + lsbMappingOffset.value());
        parser.beginGroup("Left side bearing mapping");
        parseHvarDeltaSet(parser);
        parser.endGroup();
    }

    if (rsbMappingOffset.has_value()) {
        parser.jumpTo(start + rsbMappingOffset.value());
        parser.beginGroup("Right side bearing mapping");
        parseHvarDeltaSet(parser);
        parser.endGroup();
    }
}
