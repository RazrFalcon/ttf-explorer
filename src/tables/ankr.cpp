#include "aat-common.h"
#include "tables.h"

void parseAnkr(const quint16 numberOfGlyphs, Parser &parser)
{
    const auto tableStart = parser.offset();

    parser.read<UInt16>("Version");
    parser.read<UInt16>("Unused");
    const quint32 lookupTableOffset = parser.read<OptionalOffset32>("Offset to lookup table");
    const quint32 glyphDataTableOffset = parser.read<OptionalOffset32>("Offset to glyph data table");

    if (lookupTableOffset == 0) {
        throw QString("invalid lookup table offset");
    }

    parser.advanceTo(tableStart + lookupTableOffset);
    const auto offsets = parseAatLookup(numberOfGlyphs, parser);

    if (glyphDataTableOffset == 0) {
        return;
    }

    parser.readArray("Glyphs Data", offsets.size(), [&](const auto index){
        parser.advanceTo(tableStart + glyphDataTableOffset + offsets[index]);
        parser.beginGroup(index);
        const auto numberOfPoints = parser.read<UInt32>("Number of points");
        parser.readArray("Points", numberOfPoints, [&](const auto index){
            parser.beginGroup(index);
            parser.read<Int16>("X");
            parser.read<Int16>("Y");
            parser.endGroup();
        });
        parser.endGroup();
    });
}
