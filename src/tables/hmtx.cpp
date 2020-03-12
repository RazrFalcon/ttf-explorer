#include "src/parser.h"
#include "tables.h"

void parseHmtx(const quint16 numberOfMetrics, const quint16 numberOfGlyphs, Parser &parser)
{
    for (int i = 0; i < numberOfMetrics; ++i) {
        parser.beginGroup(QString("Glyph %1").arg(i));
        parser.read<UInt16>("Advance width");
        parser.read<Int16>("Left side bearing");
        parser.endGroup();
    }

    for (int i = 0; i < (numberOfGlyphs - numberOfMetrics); ++i) {
        parser.beginGroup(QString("Glyph %1").arg(numberOfMetrics + i));
        parser.read<Int16>("Left side bearing");
        parser.endGroup();
    }
}
