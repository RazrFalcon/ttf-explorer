#include "tables.h"

void parseVmtx(const quint16 numberOfMetrics, const quint16 numberOfGlyphs, Parser &parser)
{
    parser.readArray("Metrics", numberOfMetrics, [&](const auto index){
        parser.beginGroup(index);
        parser.read<UInt16>("Advance height");
        parser.read<Int16>("Top side bearing");
        parser.endGroup();
    });

    if (numberOfGlyphs <= numberOfMetrics) {
        return;
    }

    parser.readArray("Additional Metrics", numberOfGlyphs - numberOfMetrics, [&](const auto index){
        parser.beginGroup(numberOfMetrics + index);
        parser.read<Int16>("Top side bearing");
        parser.endGroup();
    });
}
