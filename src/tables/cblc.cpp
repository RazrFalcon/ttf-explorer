#include <bitset>

#include "src/algo.h"
#include "src/parser.h"
#include "tables.h"

struct EblcBitmapFlags
{
    static const int Size = 1;
    static const QString Type;

    static EblcBitmapFlags parse(const quint8 *data)
    {
        return { data[0] };
    }

    static QString toString(const EblcBitmapFlags &value)
    {
        std::bitset<8> bits(value.d);
        auto flagsStr = QString::fromUtf8(bits.to_string().c_str()) + '\n';

        if (bits[0]) flagsStr += "Bit 0: Horizontal\n";
        if (bits[1]) flagsStr += "Bit 1: Vertical\n";

        flagsStr.chop(1); // trim trailing newline
        return flagsStr;
    }

    quint8 d;
};

const QString EblcBitmapFlags::Type = "BitFlags";

static void parseSbitLineMetrics(Parser &parser)
{
    parser.read<Int8>("Ascender");
    parser.read<Int8>("Descender");
    parser.read<UInt8>("Max width");
    parser.read<Int8>("Caret slope numerator");
    parser.read<Int8>("Caret slope denominator");
    parser.read<Int8>("Caret offset");
    parser.read<Int8>("Min origin SB");
    parser.read<Int8>("Min advance SB");
    parser.read<Int8>("Max before BL");
    parser.read<Int8>("Min after BL");
    parser.readBytes(2, "Padding");
}

void parseSbitSmallGlyphMetrics(Parser &parser)
{
    parser.read<UInt8>("Height");
    parser.read<UInt8>("Width");
    parser.read<Int8>("X-axis bearing");
    parser.read<Int8>("Y-axis bearing");
    parser.read<UInt8>("Advance");
}

void parseSbitBigGlyphMetrics(Parser &parser)
{
    parser.read<UInt8>("Height");
    parser.read<UInt8>("Width");
    parser.read<Int8>("Horizontal X-axis bearing");
    parser.read<Int8>("Horizontal Y-axis bearing");
    parser.read<UInt8>("Horizontal advance");
    parser.read<Int8>("Vertical X-axis bearing");
    parser.read<Int8>("Vertical Y-axis bearing");
    parser.read<UInt8>("Vertical advance");
}

void parseCblc(Parser &parser)
{
    const auto start = parser.offset();

    const auto majorVersion = parser.read<UInt16>("Major version");
    const auto minorVersion = parser.read<UInt16>("Minor version");
    // Some old Noto Emoji fonts have a 2.0 version.
    if (!((majorVersion == 2 || majorVersion == 3) && minorVersion == 0)) {
        throw "invalid table version";
    }

    const auto numSizes = parser.read<UInt32>("Number of tables");

    struct SubtableArray
    {
        quint32 offset;
        quint32 numOfSubtables;
    };

    QVector<SubtableArray> subtableArrays;
    for (uint i = 0; i < numSizes; ++i) {
        parser.beginGroup("Table");

        const auto offset = parser.read<Offset32>("Offset to index subtable");
        parser.read<UInt32>("Index tables size");
        const auto numOfSubtables = parser.read<UInt32>("Number of index subtables");
        parser.read<UInt32>("Reserved");

        parser.beginGroup("Line metrics for horizontal text");
        parseSbitLineMetrics(parser);
        parser.endGroup();

        parser.beginGroup("Line metrics for vertical text");
        parseSbitLineMetrics(parser);
        parser.endGroup();

        parser.read<GlyphId>("Lowest glyph index");
        parser.read<GlyphId>("Highest glyph index");
        parser.read<UInt8>("Horizontal pixels per em");
        parser.read<UInt8>("Vertical pixels per em");
        parser.read<UInt8>("Bit depth");
        parser.read<EblcBitmapFlags>("Flags");

        parser.endGroup();

        subtableArrays.append(SubtableArray { offset, numOfSubtables });
    }

    algo::sort_all_by_key(subtableArrays, &SubtableArray::offset);
    algo::dedup_vector_by_key(subtableArrays, &SubtableArray::offset);

    struct SubtableInfo
    {
        quint16 firstGlyph;
        quint16 lastGlyph;
        quint32 offset;
    };

    QVector<SubtableInfo> subtables;
    for (const auto array : subtableArrays) {
        parser.jumpTo(start + array.offset);

        for (uint i = 0; i < array.numOfSubtables; ++i) {
            parser.beginGroup("Index subtable array");
            const auto firstGlyph = parser.read<GlyphId>("First glyph ID");
            const auto lastGlyph = parser.read<GlyphId>("Last glyph ID");
            const auto offset2 = parser.read<Offset32>("Additional offset to index subtable");
            parser.endGroup();

            subtables.append(SubtableInfo { firstGlyph, lastGlyph, start + array.offset + offset2 });
        }
    }

    algo::sort_all_by_key(subtables, &SubtableInfo::offset);
    algo::dedup_vector_by_key(subtables, &SubtableInfo::offset);

    for (const auto info : subtables) {
        parser.jumpTo(info.offset);
        parser.beginGroup("Index subtable");
        const auto indexFormat = parser.read<UInt16>("Index format");
        parser.read<UInt16>("Image format");
        parser.read<Offset32>("Offset to image data");

        if (indexFormat == 1) {
            // TODO: check
            const auto count = info.lastGlyph - info.firstGlyph + 2;
            parser.readArray<Offset32>("Offsets", "Offset", quint32(count));
        } else if (indexFormat == 2) {
            parser.read<UInt32>("Image size");
            parseSbitBigGlyphMetrics(parser);
        } else if (indexFormat == 3) {
            // TODO: check
            const auto count = info.lastGlyph - info.firstGlyph + 2;
            parser.readArray<Offset16>("Offsets", "Offset", quint32(count));
        } else if (indexFormat == 4) {
            const auto numGlyphs = parser.read<UInt32>("Number of glyphs");
            for (uint i = 0; i <= numGlyphs; ++i) {
                parser.read<GlyphId>("Glyph ID");
                parser.read<Offset16>("Offset");
            }
        } else if (indexFormat == 5) {
            parser.read<UInt32>("Image size");
            parseSbitBigGlyphMetrics(parser);
            const auto numGlyphs = parser.read<UInt32>("Number of glyphs");
            parser.readArray<GlyphId>("Glyphs", "Glyph ID", numGlyphs);
        } else {
            throw "unsupported index format";
        }

        parser.endGroup();
    }
}

QVector<CblcIndex> parseCblcLocations(ShadowParser parser)
{
    QVector<CblcIndex> locations;

    const auto start = parser.offset();

    parser.skip<UInt16>(); // Major version
    parser.skip<UInt16>(); // Minor version

    const auto numSizes = parser.read<UInt32>();

    struct SubtableArray
    {
        quint32 offset;
        quint32 numOfSubtables;
    };

    QVector<SubtableArray> subtableArrays;
    for (uint i = 0; i < numSizes; ++i) {
        const auto offset = parser.read<Offset32>();
        parser.skip<UInt32>(); // Index tables size
        const auto numOfSubtables = parser.read<UInt32>();
        parser.advance(36);

        subtableArrays.append(SubtableArray { offset, numOfSubtables });
    }

    algo::sort_all_by_key(subtableArrays, &SubtableArray::offset);
    algo::dedup_vector_by_key(subtableArrays, &SubtableArray::offset);

    struct SubtableInfo
    {
        quint16 firstGlyph;
        quint16 lastGlyph;
        quint32 offset;
    };

    QVector<SubtableInfo> subtables;
    for (const auto array : subtableArrays) {
        parser.jumpTo(start + array.offset);

        for (uint i = 0; i < array.numOfSubtables; ++i) {
            const auto firstGlyph = parser.read<GlyphId>();
            const auto lastGlyph = parser.read<GlyphId>();
            const auto offset2 = parser.read<Offset32>();

            subtables.append(SubtableInfo { firstGlyph, lastGlyph, start + array.offset + offset2 });
        }
    }

    algo::sort_all_by_key(subtables, &SubtableInfo::offset);
    algo::dedup_vector_by_key(subtables, &SubtableInfo::offset);

    for (const auto info : subtables) {
        parser.jumpTo(info.offset);
        const auto indexFormat = parser.read<UInt16>();
        const auto imageFormat = parser.read<UInt16>();
        const auto imageDataOffset = parser.read<Offset32>();

        if (indexFormat == 1) {
            const auto count = quint32(info.lastGlyph - info.firstGlyph + 2);
            QVector<quint32> offsets;
            for (quint32 i = 0; i < count; ++i) {
                offsets.append(imageDataOffset + parser.read<Offset32>());
            }

            algo::sort_all(offsets);
            algo::dedup_vector(offsets);

            for (int i = 0; i < offsets.size() - 1; ++i) {
                const auto start = offsets.at(i);
                const auto end = offsets.at(i + 1);
                locations.append(CblcIndex { imageFormat, Range { start, end } });
            }
        } else if (indexFormat == 2) {
            const auto imageSize = parser.read<UInt32>();

            const auto count = quint32(info.lastGlyph - info.firstGlyph + 1);
            quint32 offset = imageDataOffset;
            for (quint32 i = 0; i < count; ++i) {
                const auto start = offset;
                offset += imageSize;
                const auto end = offset;
                locations.append(CblcIndex { imageFormat, Range { start, end } });
            }
        } else if (indexFormat == 3) {
            const auto count = quint32(info.lastGlyph - info.firstGlyph + 2);
            QVector<quint32> offsets;
            for (quint32 i = 0; i < count; ++i) {
                offsets.append(imageDataOffset + parser.read<Offset16>());
            }

            algo::sort_all(offsets);
            algo::dedup_vector(offsets);

            for (int i = 0; i < offsets.size() - 1; ++i) {
                const auto start = offsets.at(i);
                const auto end = offsets.at(i + 1);
                locations.append(CblcIndex { imageFormat, Range { start, end } });
            }
        } else if (indexFormat == 4) {
            const auto numGlyphs = parser.read<UInt32>();
            QVector<quint32> offsets;
            for (quint32 i = 0; i <= numGlyphs; ++i) {
                parser.skip<GlyphId>();
                offsets.append(imageDataOffset + parser.read<Offset16>());
            }

            algo::sort_all(offsets);
            algo::dedup_vector(offsets);

            for (int i = 0; i < offsets.size() - 1; ++i) {
                const auto start = offsets.at(i);
                const auto end = offsets.at(i + 1);
                locations.append(CblcIndex { imageFormat, Range { start, end } });
            }
        } else if (indexFormat == 5) {
            const auto imageSize = parser.read<UInt32>();
            parser.advance(8); // big metrics
            const auto numGlyphs = parser.read<UInt32>();

            quint32 offset = imageDataOffset;
            QVector<quint32> offsets;
            for (quint32 i = 0; i <= numGlyphs; ++i) {
                offsets.append(offset);
                offset += imageSize;
            }

            algo::sort_all(offsets);
            algo::dedup_vector(offsets);

            for (int i = 0; i < offsets.size() - 1; ++i) {
                const auto start = offsets.at(i);
                const auto end = offsets.at(i + 1);
                locations.append(CblcIndex { imageFormat, Range { start, end } });
            }
        }
    }

    algo::sort_all(locations, [](const auto &a, const auto &b) {
        return a.range.start < b.range.start;
    });

    return locations;
}
