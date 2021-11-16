#include "src/algo.h"
#include "src/parser.h"
#include "tables.h"

void parseMvar(Parser &parser)
{
    const auto majorVersion = parser.read<UInt16>("Major version");
    const auto minorVersion = parser.read<UInt16>("Minor version");
    if (!(majorVersion == 1 && minorVersion == 0)) {
        throw QString("invalid table version");
    }

    parser.read<UInt16>("Reserved");
    parser.read<UInt16>("Value record size");
    const auto valuesCount = parser.read<UInt16>("Number of Value Records");
    parser.read<Offset16>("Offset to the Item Variation Store");

    if (valuesCount == 0) {
        // Nothing else to do.
        return;
    }

    parser.readArray("Records", valuesCount, [&](const auto index){
        parser.beginGroup(index);
        const auto tag = parser.read<Tag>("Tag");
        parser.read<UInt16>("A delta-set outer index");
        parser.read<UInt16>("A delta-set inner index");
        parser.endGroup(QString(), tag.toString());
    });

    parser.beginGroup("Item Variation Store");
    parseItemVariationStore(parser);
    parser.endGroup();
}

void parseVariationRegionList(Parser &parser)
{
    const auto axisCount = parser.read<UInt16>("Axis count");
    const auto regionCount = parser.read<UInt16>("Region count");

    parser.readArray("Regions", regionCount, [&](const auto index){
        parser.beginGroup(index);
        parser.readArray("Axes", axisCount, [&](const auto index2){
            parser.beginGroup(index2);
            parser.read<F2DOT14>("Start coordinate");
            parser.read<F2DOT14>("Peak coordinate");
            parser.read<F2DOT14>("End coordinate");
            parser.endGroup();
        });
        parser.endGroup();
    });
}

void parseItemVariationData(Parser &parser)
{
    const auto itemCount = parser.read<UInt16>("Number of delta sets");
    const auto shortDeltaCount = parser.read<UInt16>("Number of short deltas");
    const auto regionIndexCount = parser.read<UInt16>("Number of variation regions");

    parser.readBasicArray<UInt16>("Region Indices", regionIndexCount);

    parser.readArray("Delta-set Rows", itemCount, [&](const auto index){
        parser.beginGroup(index);
        parser.readBasicArray<Int16>("Deltas", shortDeltaCount);
        parser.readBasicArray<Int8>("Short Deltas", regionIndexCount - shortDeltaCount);
        parser.endGroup();
    });
}

void parseItemVariationStore(Parser &parser)
{
    const auto start = parser.offset();

    parser.read<UInt16>("Format");
    const auto varListOffset = parser.read<Offset32>("Offset to the variation region list");
    const auto dataCount = parser.read<UInt16>("Number of item variation subtables");

    QVector<quint32> offsets;
    parser.readArray("Offsets", dataCount, [&](const auto index){
        offsets << parser.read<Offset32>(index);
    });

    if (varListOffset != 0) {
        parser.advanceTo(start + varListOffset);
        parser.beginGroup("Region List");
        parseVariationRegionList(parser);
        parser.endGroup();
    }

    algo::sort_all(offsets);
    parser.readArray("Item Variation Subtables", offsets.size(), [&](const auto index){
        parser.advanceTo(start + offsets[index]);
        parser.beginGroup(index);
        parseItemVariationData(parser);
        parser.endGroup();
    });
}
