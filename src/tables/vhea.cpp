#include "src/parser.h"
#include "tables.h"

void parseVhea(Parser &parser)
{
    const auto version = parser.read<Fixed>("Version");

    // 1.0625 is actully means 1.1
    if (version != 1.0f && version != 1.0625f) {
        throw "invalid table version";
    }

    // The difference between 1.0 and 1.1 only in field names,
    // and we are using 1.1 definitions.

    parser.read<Int16>("Vertical typographic ascender");
    parser.read<Int16>("Vertical typographic descender");
    parser.read<Int16>("Vertical typographic line gap");
    parser.read<UInt16>("Maximum advance width");
    parser.read<Int16>("Minimum top sidebearing");
    parser.read<Int16>("Minimum bottom sidebearing");
    parser.read<Int16>("Maximum Y extent");
    parser.read<Int16>("Caret slope rise");
    parser.read<Int16>("Caret slope run");
    parser.read<Int16>("Caret offset");
    parser.read<Int16>("Reserved");
    parser.read<Int16>("Reserved");
    parser.read<Int16>("Reserved");
    parser.read<Int16>("Reserved");
    parser.read<Int16>("Metric data format");
    parser.read<UInt16>("Number of vertical metrics");
}

quint16 parseVheaNumberOfMetrics(ShadowParser parser)
{
    parser.jumpTo(34);
    return parser.read<UInt16>();
}
