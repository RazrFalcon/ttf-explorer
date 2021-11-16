#include "src/algo.h"
#include "tables.h"

void parseVvar(Parser &parser)
{
    const auto start = parser.offset();

    const auto majorVersion = parser.read<UInt16>("Major version");
    const auto minorVersion = parser.read<UInt16>("Minor version");
    if (!(majorVersion == 1 && minorVersion == 0)) {
        throw QString("invalid table version");
    }

    const auto varStoreOffset = parser.read<Offset32>("Item variation store offset");
    const auto advanceHeightMappingOffset = parser.read<OptionalOffset32>("Advance height mapping offset");
    const auto tsbMappingOffset = parser.read<OptionalOffset32>("Top side bearing mapping offset");
    const auto bsbMappingOffset = parser.read<OptionalOffset32>("Bottom side bearing mapping offset");
    const auto vOrgMappingOffset = parser.read<OptionalOffset32>("Vertical origin mapping offset");

    enum class OffsetType {
        VariationStore,
        Advance,
        Tsb,
        Bsb,
        Vorg,
    };
    struct Offset {
        OffsetType type;
        quint32 offset;
    };
    std::array<Offset, 5> offsets = {{
        { OffsetType::VariationStore, (quint32)varStoreOffset },
        { OffsetType::Advance, (quint32)advanceHeightMappingOffset },
        { OffsetType::Tsb, (quint32)tsbMappingOffset },
        { OffsetType::Bsb, (quint32)bsbMappingOffset },
        { OffsetType::Vorg, (quint32)vOrgMappingOffset },
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
            parser.beginGroup("Advance Height Mapping");
            parseHvarDeltaSet(parser);
            parser.endGroup();
            break;
        }
        case OffsetType::Tsb:
            parser.beginGroup("Top Side Bearing Mapping");
            parseHvarDeltaSet(parser);
            parser.endGroup();
            break;
        case OffsetType::Bsb: {
            parser.beginGroup("Bottom Side Bearing Mapping");
            parseHvarDeltaSet(parser);
            parser.endGroup();
            break;
        }
        case OffsetType::Vorg: {
            parser.beginGroup("Vertical Origin Mapping");
            parseHvarDeltaSet(parser);
            parser.endGroup();
            break;
        }
        }
    }
}
