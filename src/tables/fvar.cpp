#include "src/parser.h"
#include "tables.h"

void parseFvar(Parser &parser)
{
    const auto majorVersion = parser.read<UInt16>("Major version");
    const auto minorVersion = parser.read<UInt16>("Minor version");
    if (!(majorVersion == 1 && minorVersion == 0)) {
        throw "invalid table version";
    }

    parser.read<Offset16>("Offset to VariationAxisRecord array");
    parser.read<UInt16>("Reserved");
    const auto axesCount = parser.read<UInt16>("The number of variation axes");
    parser.read<UInt16>("The size of VariationAxisRecord");
    const auto instancesCount = parser.read<UInt16>("The number of named instances");
    parser.read<UInt16>("The size of InstanceRecord");

    parser.beginGroup("Variation axis records");
    for (int i = 0; i < axesCount; ++i) {
        parser.beginGroup(QString());
        const auto tag = parser.read<Tag>("Axis tag");
        parser.read<Fixed>("Minimum coordinate");
        parser.read<Fixed>("Default coordinate");
        parser.read<Fixed>("Maximum coordinate");
        parser.read<UInt16>("Axis qualifiers");
        parser.read<UInt16>("The name ID");
        parser.endGroup("Axis " + tag.toString());
    }
    parser.endGroup();

    if (instancesCount > 0) {
        parser.beginGroup("Instance records");
        for (int i = 0; i < instancesCount; ++i) {
            parser.beginGroup("Instance");

            parser.read<UInt16>("Subfamily name ID");
            parser.read<UInt16>("Reserved");
            for (int a = 0; a < axesCount; ++a) {
                parser.read<Fixed>("Coordinate");
            }
            parser.read<UInt16>("PostScript name ID"); // TODO: technically optional

            parser.endGroup();
        }
        parser.endGroup();
    }
}
