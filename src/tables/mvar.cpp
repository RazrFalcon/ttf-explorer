#include "src/algo.h"
#include "src/parser.h"
#include "tables.h"

void parseMvar(Parser &parser)
{
    const auto majorVersion = parser.read<UInt16>("Major version");
    const auto minorVersion = parser.read<UInt16>("Minor version");
    if (!(majorVersion == 1 && minorVersion == 0)) {
        throw "invalid table version";
    }

    parser.read<UInt16>("Reserved");
    parser.read<UInt16>("Value record size");
    const auto valuesCount = parser.read<UInt16>("Number of Value Records");
    parser.read<Offset16>("Offset to the Item Variation Store");

    if (valuesCount == 0) {
        // Nothing else to do.
        return;
    }

    parser.beginGroup("Records");
    for (int i = 0; i < valuesCount; ++i) {
        parser.beginGroup(QString("Record %1").arg(i));
        parser.read<Tag>("Tag");
        parser.read<UInt16>("A delta-set outer index");
        parser.read<UInt16>("A delta-set inner index");
        parser.endGroup();
    }
    parser.endGroup();

    parser.beginGroup("Item variation store");
    parseItemVariationStore(parser);
    parser.endGroup();
}

void parseVariationRegionList(Parser &parser)
{
    const auto axisCount = parser.read<UInt16>("Axis count");
    const auto regionCount = parser.read<UInt16>("Region count");

    for (int i = 0; i < regionCount; ++i) {
        parser.beginGroup("Region");

        for (int a = 0; a < axisCount; ++a) {
            parser.beginGroup("Region axis");

            parser.read<F2DOT14>("Start coordinate");
            parser.read<F2DOT14>("Peak coordinate");
            parser.read<F2DOT14>("End coordinate");

            parser.endGroup();
        }

        parser.endGroup();
    }
}

void parseItemVariationData(Parser &parser)
{
    const auto itemCount = parser.read<UInt16>("Number of delta sets");
    const auto shortDeltaCount = parser.read<UInt16>("Number of short deltas");
    const auto regionIndexCount = parser.read<UInt16>("Number of variation regions");

    if (regionIndexCount != 0) {
        parser.beginGroup("Region indices");
        for (int i = 0; i < regionIndexCount; ++i) {
            parser.read<UInt16>(QString("Index %1").arg(i));
        }
        parser.endGroup();
    }

    if (itemCount != 0) {
        parser.beginGroup("Delta-set rows");
        for (int i = 0; i < itemCount; ++i) {
            parser.beginGroup(QString("Delta-set %1").arg(i));

            for (int j = 0; j < shortDeltaCount; ++j) {
                parser.read<Int16>("Delta");
            }

            for (int j = 0; j < regionIndexCount - shortDeltaCount; ++j) {
                parser.read<Int8>("Delta");
            }

            parser.endGroup();
        }
        parser.endGroup();
    }
}

void parseItemVariationStore(Parser &parser)
{
    const auto start = parser.offset();

    parser.read<UInt16>("Format");
    const auto varListOffset = parser.read<Offset32>("Offset to the variation region list");
    const auto dataCount = parser.read<UInt16>("Number of item variation subtables");

    QVector<quint32> offsets;
    if (dataCount != 0) {
        parser.beginGroup("Offsets");
        for (int i = 0; i < dataCount; ++i) {
            offsets << parser.read<Offset32>(QString("Offset %1").arg(i));
        }
        parser.endGroup();
    }

    if (varListOffset != 0) {
        parser.jumpTo(start + varListOffset);
        parser.beginGroup("Region list");
        parseVariationRegionList(parser);
        parser.endGroup();
    }

    algo::sort_all(offsets);
    for (const auto offset : offsets) {
        parser.jumpTo(start + offset);
        parser.beginGroup("Item variation subtable");
        parseItemVariationData(parser);
        parser.endGroup();
    }
}
