#include "src/parser.h"
#include "tables.h"

void parseVorg(Parser &parser)
{
    const auto majorVersion = parser.read<UInt16>("Major version");
    const auto minorVersion = parser.read<UInt16>("Minor version");

    if (!(majorVersion == 1 && minorVersion == 0)) {
        throw "invalid table version";
    }

    parser.read<Int16>("Default vertical origin");
    const auto count = parser.read<UInt16>("Number of metrics");
    for (int i = 0; i < count; ++i) {
        parser.beginGroup(QString("Metric %1").arg(i));
        parser.read<GlyphId>("Glyph index");
        parser.read<Int16>("Coordinate");
        parser.endGroup();
    }
}
