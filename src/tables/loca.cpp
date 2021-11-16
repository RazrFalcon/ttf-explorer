#include "tables.h"

void parseLoca(const quint16 numberOfGlyphs, const quint16 indexToLocationFormat, Parser &parser)
{
    if (indexToLocationFormat == 0) {
        parser.readArray("Offsets", quint32(numberOfGlyphs) + 1, [&](const auto index){
            parser.read<UInt16>(index);
        });
    } else if (indexToLocationFormat == 1) {
        parser.readArray("Offsets", quint32(numberOfGlyphs) + 1, [&](const auto index){
            parser.read<UInt32>(index);
        });
    } else {
        throw QString("invalid index to location format");
    }
}

QVector<quint32> collectLocaOffsets(const quint16 numberOfGlyphs,
                                    const quint16 indexToLocationFormat,
                                    ShadowParser &parser)
{
    QVector<quint32> offsets;
    offsets.reserve(numberOfGlyphs + 1);
    if (indexToLocationFormat == 0) {
        for (quint32 i = 0; i < quint32(numberOfGlyphs) + 1; i++) {
            offsets.push_back(parser.read<UInt16>() * 2); // Has to be multiplied by 2.
        }
    } else if (indexToLocationFormat == 1) {
        for (quint32 i = 0; i < quint32(numberOfGlyphs) + 1; i++) {
            offsets.push_back(parser.read<UInt32>());
        }
    } else {
        throw QString("invalid index to location format");
    }

    return offsets;
}
