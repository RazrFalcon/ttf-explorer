#include "src/parser.h"
#include "tables.h"

void parseAvar(Parser &parser)
{
    const auto majorVersion = parser.read<UInt16>("Major version");
    const auto minorVersion = parser.read<UInt16>("Minor version");
    if (!(majorVersion == 1 && minorVersion == 0)) {
        throw "invalid table version";
    }

    parser.read<UInt16>("Reserved");
    const auto axisCount = parser.read<UInt16>("Axis count");
    for (int i = 0; i < axisCount; ++i) {
        parser.beginGroup("Segment map");
        const auto pairsCount = parser.read<UInt16>("Number of map pairs");
        for (int j = 0; j < pairsCount; ++j) {
            parser.beginGroup(QString("Pair %1").arg(j));
            parser.read<F2DOT14>("From coordinate");
            parser.read<F2DOT14>("To coordinate");
            parser.endGroup();
        }

        parser.endGroup();
    }
}
