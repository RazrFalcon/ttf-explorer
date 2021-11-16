#include "tables.h"

void parseStat(const NamesHash &names, Parser &parser)
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

    parser.readArray("Design Axes", designAxisCount, [&](const auto index){
        parser.beginGroup(index);
        const auto tag = parser.read<Tag>("Tag");
        const auto name = parser.readNameId("Name ID", names);
        parser.read<UInt16>("Axis ordering");
        parser.endGroup(QString(), QString("%1 (%2)").arg(name).arg(tag.toString()));
    });

    // TODO: parse axis
    parser.readArray("Axis Value Tables Offsets", axisValueCount, [&](const auto index){
        parser.read<UInt16>(index);
    });
}
