#include "tables.h"

void parseHmtx(const quint16 numberOfMetrics, const quint16 numberOfGlyphs, Parser &parser)
{
    parser.readArray("Metrics", numberOfMetrics, [&](const auto index){
        parser.beginGroup(index);
        parser.read<UInt16>("Advance width");
        parser.read<Int16>("Left side bearing");
        parser.endGroup();
    });

    if (numberOfGlyphs <= numberOfMetrics) {
        return;
    }

    parser.readArray("Additional Metrics", numberOfGlyphs - numberOfMetrics, [&](const auto index){
        parser.beginGroup(numberOfMetrics + index);
        parser.read<Int16>("Left side bearing");
        parser.endGroup();
    });
}
