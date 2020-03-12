#include "src/algo.h"
#include "src/parser.h"
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
    parser.beginGroup("GlyphVariationData offsets", QString::number(glyphCount + 1));
    for (int i = 0; i <= glyphCount; ++i) {
        if (longFormat) {
            offsets << parser.read<Offset32>("Offset");
        } else {
            offsets << parser.read<Offset16>("Offset") * 2;
        }
    }
    parser.endGroup();

    parser.beginGroup("Shared tuples", QString::number(sharedTupleCount));
    for (int i = 0; i < sharedTupleCount; ++i) {
        parser.beginGroup("Tuple record");
        for (int a = 0; a < axisCount; ++a) {
            parser.read<F2DOT14>("Coordinate");
        }
        parser.endGroup();
    }
    parser.endGroup();

    // Dedup offsets. There can be multiple records with the same offset.
    algo::dedup_vector(offsets);

    const auto start = parser.offset();

    struct TupleHeader
    {
        quint16 dataSize;
        bool hasPrivatePointNumbers;
    };
    QVector<TupleHeader> headersData;

    parser.beginGroup("Tables", QString::number(offsets.size() - 1));
    offsets.removeFirst();
    qint32 i = 0;
    for (const auto &offset : offsets) {
        headersData.clear();

        parser.beginGroup(QString("Glyph Variation Data %1").arg(i));
        i += 1;

        const auto value = parser.read<UInt16>("Value");
        parser.read<Offset16>("Data offset");

        // 'The high 4 bits are flags, and the low 12 bits
        // are the number of tuple variation tables for this glyph.'
        const auto hasSharedPointNumbers = (value & SHARED_POINT_NUMBERS) != 0;
        const auto tupleVariationCount = value & COUNT_MASK;

        for (int h = 0; h < tupleVariationCount; ++h) {
            parser.beginGroup("Tuple Variation Header");
            const auto dataSize = parser.read<UInt16>("Size of the serialized data");
            const auto tuple_index = parser.read<UInt16>("Value");

            const auto hasEmbeddedPeakTuple = (tuple_index & EMBEDDED_PEAK_TUPLE) != 0;
            const auto hasIntermediateRegion = (tuple_index & INTERMEDIATE_REGION) != 0;
            const auto hasPrivatePointNumbers = (tuple_index & PRIVATE_POINT_NUMBERS) != 0;

            headersData.append({ dataSize, hasPrivatePointNumbers });

            if (hasEmbeddedPeakTuple) {
                parser.beginGroup("Peak record");
                for (int a = 0; a < axisCount; ++a) {
                    parser.read<F2DOT14>("Coordinate");
                }
                parser.endGroup();
            }

            if (hasIntermediateRegion) {
                parser.beginGroup("Peak record");
                for (int a = 0; a < axisCount; ++a) {
                    parser.read<F2DOT14>("Coordinate");
                }
                for (int a = 0; a < axisCount; ++a) {
                    parser.read<F2DOT14>("Coordinate");
                }
                parser.endGroup();
            }

            parser.endGroup();
        }

        if (hasSharedPointNumbers) {
            parser.beginGroup("Shared points");
            unpackPoints(parser);
            parser.endGroup();
        }

        for (const auto header : headersData) {
            const auto start = parser.offset();

            if (header.hasPrivatePointNumbers) {
                parser.beginGroup("Private points");
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
                parser.advance(padding); // Padding
            }
        }

        parser.endGroup();
    }

    parser.endGroup();
}
