#include "tables.h"

void parseAvar(Parser &parser)
{
    const auto majorVersion = parser.read<UInt16>("Major version");
    const auto minorVersion = parser.read<UInt16>("Minor version");
    if (!(majorVersion == 1 && minorVersion == 0)) {
        throw QString("invalid table version");
    }

    parser.read<UInt16>("Reserved");
    const auto axisCount = parser.read<UInt16>("Axis count");
    parser.readArray("Axes", axisCount, [&](const auto index){
        parser.beginGroup(QString("Segment Map %1").arg(index));
        const auto pairsCount = parser.read<UInt16>("Number of map pairs");
        parser.readArray("Pairs", pairsCount, [&](const auto index2){
            parser.beginGroup(index2);
            parser.read<F2DOT14>("From coordinate");
            parser.read<F2DOT14>("To coordinate");
            parser.endGroup();
        });

        parser.endGroup();
    });
}
