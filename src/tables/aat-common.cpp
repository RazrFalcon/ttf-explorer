#include "aat-common.h"
#include "src/parser.h"
#include "src/algo.h"

template <typename F>
static void parseAatBinarySearchTable(const quint16 format, Parser &parser, F f)
{
    parser.beginGroup("Binary Search Table");
    parser.read<UInt16>("Segment size");
    quint16 numberOfSegments = parser.read<UInt16>("Number of segments");
    parser.read<UInt16>("Search range");
    parser.read<UInt16>("Entry selector");
    parser.read<UInt16>("Range shift");

    if (numberOfSegments < 2) {
        parser.endGroup();
        return;
    }

    // Not specified in the spec, but present in all Apple fonts.
    if (format == 6) {
        numberOfSegments += 1;
    }

    parser.readArray("Segments", numberOfSegments, [&](const auto index){
        f(index, parser);
    });

    parser.endGroup();
}

QVector<quint32> parseAatLookup(const quint16 numberOfGlyphs, Parser &parser)
{
    const auto start = parser.offset();

    QVector<quint32> offsets;

    parser.beginGroup("Lookup Table");
    const auto format = parser.read<UInt16>("Format");
    // TODO: format 10
    switch (format) {
    case 0: {
        parser.readArray("Offsets", numberOfGlyphs, [&](const auto index){
            offsets << (quint32)parser.read<Offset16>(index);
        });
        break;
    }
    case 2: {
        parseAatBinarySearchTable(format, parser, [&offsets](const quint32 index, Parser &p) {
            p.beginGroup(index);
            const quint32 last = p.read<UInt16>("Last glyph");
            p.read<UInt16>("First glyph");
            const quint32 offset = p.read<Offset16>("Offset");
            p.endGroup();

            if (last == 0xFFFF) {
                return;
            }

            offsets << offset;
        });
        break;
    }
    case 4: {
        struct Data {
            quint32 offset;
            quint32 count;
        };
        QVector<Data> localOffsets;
        parseAatBinarySearchTable(format, parser, [&localOffsets](const quint32 index, Parser &p) {
            p.beginGroup(index);
            const quint32 last = p.read<UInt16>("Last glyph");
            const quint32 first = p.read<UInt16>("First glyph");
            const quint32 offset = p.read<Offset16>("Offset");
            p.endGroup();

            if (last == 0xFFFF) {
                return;
            }

            if (last < first) {
                throw QString("invalid values count");
            }

            const auto count = last - first + 1;
            localOffsets.append({ offset, count });
        });
        algo::sort_all_by_key(localOffsets, &Data::offset);
        for (const auto offset : localOffsets) {
            parser.advanceTo(start + offset.offset);
            parser.readArray("Offsets", offset.count, [&](const auto index){
                offsets << (quint32)parser.read<Offset16>(index);
            });
        }
        break;
    }
    case 6: {
        parseAatBinarySearchTable(format, parser, [&offsets](const quint32 index, Parser &p) {
            p.beginGroup(index);
            p.read<UInt16>("Glyph");
            const quint32 offset = p.read<Offset16>("Offset");
            p.endGroup();

            if (offset == 0xFFFF) {
                return;
            }

            offsets << offset;
        });
        break;
    }
    case 8: {
        parser.read<UInt16>("First glyph");
        const auto count = parser.read<UInt16>("Glyph count");
        parser.readArray("Offsets", count, [&](const auto index){
            offsets << (quint32)parser.read<Offset16>(index);
        });
        break;
    }
    default:
        throw QString("unsupported lookup table format");
    }
    parser.endGroup();

    algo::sort_all(offsets);
    return offsets;
}
