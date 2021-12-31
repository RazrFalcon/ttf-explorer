#include "src/algo.h"
#include "tables.h"

static void parseTrackData(const NamesHash &names, const quint32 tableStart, Parser &parser)
{
    QVector<quint32> offsets;

    const auto numberOfTracks = parser.read<UInt16>("Number of tracks");
    const auto numberOfSizes = parser.read<UInt16>("Number of point sizes");
    parser.read<Offset32>("Offset to size subtable");
    parser.readArray("Tracks", numberOfTracks, [&](const auto index){
        parser.beginGroup(index);
        parser.read<F16DOT16>("Value");
        const auto name = parser.readNameId("Name ID", names);
        offsets << (quint32)parser.read<Offset16>("Offset to per-size tracking values");
        parser.endGroup(QString(), name);
    });
    parser.readBasicArray<F16DOT16>("Point Sizes", numberOfSizes);

    algo::sort_all(offsets);
    algo::dedup_vector(offsets);

    parser.readArray("Tracks Values", offsets.size(), [&](const auto index){
        parser.advanceTo(tableStart + offsets[index]);
        parser.readBasicArray<Int16>(QString("Track %1").arg(index), numberOfSizes);
    });
}

void parseTrak(const NamesHash &names, Parser &parser)
{
    const auto tableStart = parser.offset();

    parser.read<F16DOT16>("Version");
    parser.read<UInt16>("Format");
    const quint16 horOffset = parser.read<OptionalOffset16>("Offset to horizontal Track Data");
    const quint16 verOffset = parser.read<OptionalOffset16>("Offset to vertical Track Data");
    parser.read<UInt16>("Reserved");

    if (horOffset) {
        parser.beginGroup("Horizontal Track Data");
        parseTrackData(names, tableStart, parser);
        parser.endGroup();
    }

    if (verOffset) {
        parser.beginGroup("Vertical Track Data");
        parseTrackData(names, tableStart, parser);
        parser.endGroup();
    }
}
