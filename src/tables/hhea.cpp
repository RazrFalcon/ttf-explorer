#include "tables.h"

void parseHhea(Parser &parser)
{
    const auto majorVersion = parser.read<UInt16>("Major version");
    const auto minorVersion = parser.read<UInt16>("Minor version");

    if (!(majorVersion == 1 && minorVersion == 0)) {
        throw QString("invalid table version");
    }

    parser.read<Int16>("Typographic ascent");
    parser.read<Int16>("Typographic descent");
    parser.read<Int16>("Typographic line gap");
    parser.read<UInt16>("Maximum advance width");
    parser.read<Int16>("Minimum left sidebearing");
    parser.read<Int16>("Minimum right sidebearing");
    parser.read<Int16>("Maximum X extent");
    parser.read<Int16>("Caret slope rise");
    parser.read<Int16>("Caret slope run");
    parser.read<Int16>("Caret offset");
    parser.read<Int16>("Reserved");
    parser.read<Int16>("Reserved");
    parser.read<Int16>("Reserved");
    parser.read<Int16>("Reserved");
    parser.read<Int16>("Metric data format");
    parser.read<UInt16>("Number of horizontal metrics");
}
