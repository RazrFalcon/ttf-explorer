#include "src/algo.h"
#include "src/tables/tables.h"

static void parseSbitSmallGlyphMetrics(Parser &parser)
{
    parser.read<UInt8>("Height");
    parser.read<UInt8>("Width");
    parser.read<Int8>("X-axis bearing");
    parser.read<Int8>("Y-axis bearing");
    parser.read<UInt8>("Advance");
}

static void parseSbitBigGlyphMetrics(Parser &parser)
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

void parseCbdt(const QVector<CblcIndex> &cblcLocations, Parser &parser)
{
    const auto start = parser.offset();

    const auto majorVersion = parser.read<UInt16>("Major version");
    const auto minorVersion = parser.read<UInt16>("Minor version");

    if (!((majorVersion == 2 || majorVersion == 3) && minorVersion == 0)) {
        throw QString("invalid table version");
    }

    for (const auto &loca : cblcLocations) {
        parser.advanceTo(start + loca.range.start);
        parser.beginGroup(QString("Bitmap Format %1").arg(loca.imageFormat));

        if (loca.imageFormat == 1) {
            parseSbitSmallGlyphMetrics(parser);
            parser.readBytes("Byte-aligned bitmap data", loca.range.size() - 5);
        } else if (loca.imageFormat == 2) {
            parseSbitSmallGlyphMetrics(parser);
            parser.readBytes("Bit-aligned bitmap data", loca.range.size() - 5);
        } else if (loca.imageFormat == 5) {
            parser.readBytes("Bit-aligned bitmap data", loca.range.size() - 5);
        } else if (loca.imageFormat == 6) {
            parseSbitBigGlyphMetrics(parser);
            parser.readBytes("Byte-aligned bitmap data", loca.range.size() - 8);
        } else if (loca.imageFormat == 7) {
            parseSbitBigGlyphMetrics(parser);
            parser.readBytes("Bit-aligned bitmap data", loca.range.size() - 8);
        } else if (loca.imageFormat == 8) {
            parseSbitSmallGlyphMetrics(parser);
            parser.read<UInt8>("Pad");
            const auto count = parser.read<UInt16>("Number of components");
            for (int i = 0; i < count; ++i) {
                parser.beginGroup("Ebdt component");
                parser.read<GlyphId>("Glyph ID");
                parser.read<Int8>("X-axis offset");
                parser.read<Int8>("Y-axis offset");
                parser.endGroup();
            }
        } else if (loca.imageFormat == 9) {
            parseSbitBigGlyphMetrics(parser);
            const auto count = parser.read<UInt16>("Number of components");
            for (int i = 0; i < count; ++i) {
                parser.beginGroup("Ebdt component");
                parser.read<GlyphId>("Glyph ID");
                parser.read<Int8>("X-axis offset");
                parser.read<Int8>("Y-axis offset");
                parser.endGroup();
            }
        } else if (loca.imageFormat == 17) {
            parseSbitSmallGlyphMetrics(parser);
            const auto len = parser.read<UInt32>("Length of data");
            parser.readBytes("Raw PNG data", len);
        } else if (loca.imageFormat == 18) {
            parseSbitBigGlyphMetrics(parser);
            const auto len = parser.read<UInt32>("Length of data");
            parser.readBytes("Raw PNG data", len);
        } else if (loca.imageFormat == 19) {
            const auto len = parser.read<UInt32>("Length of data");
            parser.readBytes("Raw PNG data", len);
        }

        parser.endGroup();
    }
}
