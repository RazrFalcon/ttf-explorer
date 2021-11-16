#include "src/parser.h"
#include "tables.h"

void parseStat(Parser &parser)
{
    parser.read<UInt16>("Major version");
    const auto minorVersion = parser.read<UInt16>("Minor version");
    parser.read<UInt16>("Axis record size");
    const auto designAxisCount = parser.read<UInt16>("Number of records");
    parser.read<Offset32>("Offset to the axes array");
    const auto axisValueCount = parser.read<UInt16>("Number of axis value tables");
    parser.read<Offset32>("Offset to the axes value offsets array");

    std::optional<UInt16> elidedFallbackNameID;
    if (minorVersion > 0) {
        elidedFallbackNameID = parser.read<UInt16>("Fallback name ID");
    }

    parser.beginGroup("Design axes", QString::number(designAxisCount));
    for (int i = 0; i < designAxisCount; ++i) {
        parser.beginGroup("Record");
        parser.read<Tag>("Tag");
        parser.read<UInt16>("Name ID");
        parser.read<UInt16>("Axis ordering");
        parser.endGroup();
    }
    parser.endGroup();

    // TODO: parse axis
    parser.beginGroup("Axis value tables offsets", QString::number(axisValueCount));
    for (int i = 0; i < axisValueCount; ++i) {
        parser.read<UInt16>("Offset");
    }
    parser.endGroup();
}
