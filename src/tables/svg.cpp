#include "src/algo.h"
#include "src/parser.h"
#include "tables.h"

void parseSvg(Parser &parser)
{
    const auto start = parser.offset();

    parser.read<UInt16>("Version");
    const auto listOffset = parser.read<Offset32>("Offset to the SVG Document List");
    parser.read<UInt32>("Reserved");

    parser.advanceTo(start + listOffset);
    parser.beginGroup("SVG Document List");
    const auto count = parser.read<UInt16>("Number of records");
    QVector<Range> ranges;
    for (int i = 0; i < count; ++i) {
        parser.beginGroup(QString("Record %1").arg(i));
        parser.read<UInt16>("First glyph ID");
        parser.read<UInt16>("Last glyph ID");
        const auto offset = parser.read<Offset32>("Offset to an SVG Document");
        const auto size = parser.read<UInt32>("SVG Document length");
        parser.endGroup();

        const auto docStart = start + listOffset + offset;
        const auto docEnd = docStart + size;
        ranges.append({ docStart, docEnd });
    }
    parser.endGroup();

    algo::sort_all_by_key(ranges, &Range::start);
    algo::dedup_vector_by_key(ranges, &Range::start);

    for (const auto &range : ranges) {
        parser.advanceTo(range.start);

        if (parser.peek<UInt16>() == 8075) {
            // Read gzip compressed SVG as is.
            parser.readBytes("SVGZ", range.size());
        } else {
            // Otherwise read as string.
            // According to the spec, it must be in UTF-8, so we are fine.
            parser.readUtf8String("SVG", range.size());
        }
    }
}
