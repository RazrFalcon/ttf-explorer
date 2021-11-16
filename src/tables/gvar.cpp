#include "src/algo.h"
#include "tables.h"

static const quint16 SHARED_POINT_NUMBERS = 0x8000;
static const quint16 COUNT_MASK = 0x0FFF;

static const quint16 EMBEDDED_PEAK_TUPLE = 0x8000;
static const quint16 INTERMEDIATE_REGION = 0x4000;
static const quint16 PRIVATE_POINT_NUMBERS = 0x2000;

static const quint8 POINTS_ARE_WORDS = 0x80;
static const quint8 POINT_RUN_COUNT_MASK = 0x7F;

static const quint8 DELTAS_ARE_ZERO = 0x80;
static const quint8 DELTAS_ARE_WORDS = 0x40;
static const quint8 DELTA_RUN_COUNT_MASK = 0x3F;

static void unpackPoints(Parser &parser)
{
    const auto control = parser.read<UInt8>("Control");

    if (control == 0) {
        return;
    }

    quint16 count = control;
    if (control & POINTS_ARE_WORDS) {
        const auto b2 = parser.read<UInt8>("Control");
        count = quint16(((count & POINT_RUN_COUNT_MASK) << 8) | quint16(b2));
    }

    quint16 i = 0;
    while (i < count) {
        const auto control = parser.read<UInt8>("Control");
        const auto runCount = (control & POINT_RUN_COUNT_MASK) + 1;
        if (control & POINTS_ARE_WORDS) {
            for (int j = 0; j < runCount && i < count; ++j, ++i) {
                parser.read<UInt16>("Point");
            }
        } else {
            for (int j = 0; j < runCount && i < count; ++j, ++i) {
                parser.read<UInt8>("Point");
            }
        }
    }
}

static void unpackDeltas(Parser &parser, const quint32 size)
{
    const auto end = parser.offset() + size;
    while (parser.offset() < end) {
        const auto control = parser.read<UInt8>("Control");
        const auto runCount = (control & DELTA_RUN_COUNT_MASK) + 1;
        if (control & DELTAS_ARE_ZERO) {
            // Ignore.
        } else if (control & DELTAS_ARE_WORDS) {
            for (int i = 0; i < runCount; ++i) {
                parser.read<UInt16>("Delta");
            }
        } else {
            for (int i = 0; i < runCount; ++i) {
                parser.read<UInt8>("Delta");
            }
        }
    }
}

void parseGvar(Parser &parser)
{
    parser.read<UInt16>("Major version");
    parser.read<UInt16>("Minor version");
    const auto axisCount = parser.read<UInt16>("Axis count");
    const auto sharedTupleCount = parser.read<UInt16>("Shared tuple count");
    parser.read<Offset32>("Offset to the shared tuple records");
    const auto glyphCount = parser.read<UInt16>("Glyphs count");
    const auto flags = parser.read<UInt16>("Flags");
    parser.read<Offset32>("Offset to the array of Glyph Variation Data tables");
    const bool longFormat = (quint16(flags) & 1) == 1;

    QVector<quint32> offsets;

    // The total count is glyphCount+1.
    parser.readArray("Glyph Variation Data Offsets", glyphCount + 1, [&](const auto index){
        if (longFormat) {
            offsets << parser.read<Offset32>(index);
        } else {
            offsets << parser.read<Offset16>(index) * 2;
        }
    });

    parser.readArray("Shared Tuples", sharedTupleCount, [&](const auto index){
        parser.readBasicArray<F2DOT14>(QString("Tuple Records %1").arg(index), axisCount);
    });

    // Dedup offsets. There can be multiple records with the same offset.
    algo::dedup_vector(offsets);

    const auto start = parser.offset();

    struct TupleHeader
    {
        quint16 dataSize;
        bool hasPrivatePointNumbers;
    };
    QVector<TupleHeader> headersData;
    offsets.removeFirst();
    parser.readArray("Glyphs Variation Data", offsets.size(), [&](const auto index){
        const auto offset = offsets[index];
        headersData.clear();

        parser.beginGroup(index);

        const auto value = parser.read<UInt16>("Value");
        parser.read<Offset16>("Data offset");

        // 'The high 4 bits are flags, and the low 12 bits
        // are the number of tuple variation tables for this glyph.'
        const auto hasSharedPointNumbers = (value & SHARED_POINT_NUMBERS) != 0;
        const auto tupleVariationCount = value & COUNT_MASK;

        parser.readArray("Tuple Variation Headers", tupleVariationCount, [&](const auto index2){
            parser.beginGroup(index2);
            const auto dataSize = parser.read<UInt16>("Size of the serialized data");
            const auto tuple_index = parser.read<UInt16>("Value");

            const auto hasEmbeddedPeakTuple = (tuple_index & EMBEDDED_PEAK_TUPLE) != 0;
            const auto hasIntermediateRegion = (tuple_index & INTERMEDIATE_REGION) != 0;
            const auto hasPrivatePointNumbers = (tuple_index & PRIVATE_POINT_NUMBERS) != 0;

            headersData.append({ dataSize, hasPrivatePointNumbers });

            if (hasEmbeddedPeakTuple) {
                parser.readBasicArray<F2DOT14>("Peak Record", axisCount);
            }

            if (hasIntermediateRegion) {
                parser.readBasicArray<F2DOT14>("Peak Record", axisCount * 2);
            }

            parser.endGroup();
        });

        if (hasSharedPointNumbers) {
            parser.beginGroup("Shared Points");
            unpackPoints(parser);
            parser.endGroup();
        }

        for (const auto header : headersData) {
            const auto start = parser.offset();

            if (header.hasPrivatePointNumbers) {
                parser.beginGroup("Private Points");
                unpackPoints(parser);
                parser.endGroup();
            }

            const auto privatePointsSize = parser.offset() - start;

            parser.beginGroup("Deltas");
            unpackDeltas(parser, header.dataSize - privatePointsSize);
            parser.endGroup();
        }

        if (parser.offset() - start < offset) {
            const auto padding = offset - (parser.offset() - start);
            if (padding > 0) {
                parser.readPadding(padding);
            }
        }

        parser.endGroup();
    });
}
