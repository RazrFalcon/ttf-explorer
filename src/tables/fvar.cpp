#include "tables.h"

void parseFvar(const NamesHash &names, Parser &parser)
{
    const auto majorVersion = parser.read<UInt16>("Major version");
    const auto minorVersion = parser.read<UInt16>("Minor version");
    if (!(majorVersion == 1 && minorVersion == 0)) {
        throw QString("invalid table version");
    }

    parser.read<Offset16>("Offset to Variation Axis Records array");
    parser.read<UInt16>("Reserved");
    const auto axesCount = parser.read<UInt16>("Number of variation axes");
    parser.read<UInt16>("The size of Variation Axis Record");
    const auto instancesCount = parser.read<UInt16>("Number of named instances");
    const auto instanceSize = parser.read<UInt16>("The size of Instance Record");

    parser.readArray("Variation Axis Records", axesCount, [&](const auto index){
        parser.beginGroup(index);
        const auto tag = parser.read<Tag>("Axis tag");
        parser.read<F16DOT16>("Minimum coordinate");
        parser.read<F16DOT16>("Default coordinate");
        parser.read<F16DOT16>("Maximum coordinate");
        parser.read<UInt16>("Axis qualifiers");
        parser.readNameId("The name ID", names);
        parser.endGroup(QString(), tag.toString());
    });

    const bool hasPostScriptName = instanceSize == axesCount * 4 + 6;

    parser.readArray("Instance Records", instancesCount, [&](const auto index){
        parser.beginGroup(index);

        const auto name = parser.readNameId("Subfamily name ID", names);
        parser.read<UInt16>("Reserved");
        parser.readArray("Coordinates", axesCount, [&](const auto index2){
            parser.read<F16DOT16>(index2);
        });

        if (hasPostScriptName) {
            parser.readNameId("PostScript name ID", names);
        }

        parser.endGroup(QString(), name);
    });
}
