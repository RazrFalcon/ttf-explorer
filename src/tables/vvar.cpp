#include "src/parser.h"
#include "tables.h"

void parseVvar(Parser &parser)
{
    const auto start = parser.offset();

    const auto majorVersion = parser.read<UInt16>("Major version");
    const auto minorVersion = parser.read<UInt16>("Minor version");
    if (!(majorVersion == 1 && minorVersion == 0)) {
        throw "invalid table version";
    }

    const auto varStoreOffset = parser.read<Offset32>("Item variation store offset");
    const auto advanceHeightMappingOffset = parser.read<std::optional<Offset32>>("Advance height mapping offset");
    const auto tsbMappingOffset = parser.read<std::optional<Offset32>>("Top side bearing mapping offset");
    const auto bsbMappingOffset = parser.read<std::optional<Offset32>>("Bottom side bearing mapping offset");
    const auto vOrgMappingOffset = parser.read<std::optional<Offset32>>("Vertical origin mapping offset");

    parser.jumpTo(start + varStoreOffset);
    parser.beginGroup("Item variation store");
    parseItemVariationStore(parser);
    parser.endGroup();

    if (advanceHeightMappingOffset.has_value()) {
        parser.jumpTo(start + advanceHeightMappingOffset.value());
        parser.beginGroup("Advance height mapping");
        parseHvarDeltaSet(parser);
        parser.endGroup();
    }

    if (tsbMappingOffset.has_value()) {
        parser.jumpTo(start + tsbMappingOffset.value());
        parser.beginGroup("Top side bearing mapping");
        parseHvarDeltaSet(parser);
        parser.endGroup();
    }

    if (bsbMappingOffset.has_value()) {
        parser.jumpTo(start + bsbMappingOffset.value());
        parser.beginGroup("Bottom side bearing mapping");
        parseHvarDeltaSet(parser);
        parser.endGroup();
    }

    if (vOrgMappingOffset.has_value()) {
        parser.jumpTo(start + vOrgMappingOffset.value());
        parser.beginGroup("Vertical origin mapping");
        parseHvarDeltaSet(parser);
        parser.endGroup();
    }
}
