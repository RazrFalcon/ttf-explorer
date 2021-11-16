#include <zlib.h>

#include "src/algo.h"
#include "src/parser.h"
#include "tables.h"

// https://stackoverflow.com/a/7351507
static QByteArray gUncompress(const QByteArray &data)
{
    if (data.size() <= 4) {
        qWarning("gUncompress: Input data is truncated");
        return QByteArray();
    }

    QByteArray result;

    int ret;
    z_stream strm;
    static const uint CHUNK_SIZE = 1024;
    char out[CHUNK_SIZE];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = quint32(data.size());
    strm.next_in = (Bytef*)data.data();

    ret = inflateInit2(&strm, 15 +  32); // gzip decoding
    if (ret != Z_OK) {
        return QByteArray();
    }

    do {
        strm.avail_out = CHUNK_SIZE;
        strm.next_out = (Bytef*)(out);

        ret = inflate(&strm, Z_NO_FLUSH);
        Q_ASSERT(ret != Z_STREAM_ERROR);  // state not clobbered

        switch (ret) {
        case Z_NEED_DICT:
            ret = Z_DATA_ERROR; // and fall through
        [[fallthrough]];
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            (void)inflateEnd(&strm);
            return QByteArray();
        }

        result.append(out, int(CHUNK_SIZE - strm.avail_out));
    } while (strm.avail_out == 0);

    // clean up and return
    inflateEnd(&strm);
    return result;
}

void parseSvg(Parser &parser)
{
    const auto start = parser.offset();

    parser.read<UInt16>("Version");
    const auto listOffset = parser.read<Offset32>("Offset to the SVG Document List");
    parser.read<UInt32>("Reserved");

    parser.advanceTo(start + listOffset);
    parser.beginGroup("SVG Document List");
    const auto count = parser.read<UInt16>("Number of records");
    QVector<Range> ranges;
    for (int i = 0; i < count; ++i) {
        parser.beginGroup(QString("Record %1").arg(i));
        parser.read<UInt16>("First glyph ID");
        parser.read<UInt16>("Last glyph ID");
        const auto offset = parser.read<Offset32>("Offset to an SVG Document");
        const auto size = parser.read<UInt32>("SVG Document length");
        parser.endGroup();

        const auto docStart = start + listOffset + offset;
        const auto docEnd = docStart + size;
        ranges.append({ docStart, docEnd });
    }
    parser.endGroup();

    algo::sort_all_by_key(ranges, &Range::start);
    algo::dedup_vector_by_key(ranges, &Range::start);

    for (const auto &range : ranges) {
        parser.advanceTo(range.start);

        if (parser.peek<UInt16>() == 8075) {
            // Read gzip compressed SVG as is.
            auto shadow = parser.shadow();
            const auto gzipData = shadow.readBytes(range.size());
            const auto value = QString::fromUtf8(gUncompress(gzipData));
            parser.readValue("SVGZ", value, Parser::BytesType, range.size());
        } else {
            // Otherwise read as string.
            // According to the spec, it must be in UTF-8, so we are fine.
            parser.readUtf8String("SVG", range.size());
        }
    }
}
