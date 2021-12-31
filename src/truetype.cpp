#include <bitset>

#include "src/algo.h"
#include "src/tables/tables.h"

#include "truetype.h"

struct FontTable
{
    quint32 faceIndex;
    Tag tag;
    uint32_t offset;
    uint32_t length;
};

static QString tableName(const Tag tag) {
    switch (tag.d) {
    case FOURCC("acnt"): return "Accent Attachment Table";
    case FOURCC("ankr"): return "Anchor Point Table";
    case FOURCC("avar"): return "Axis Variations Table";
    case FOURCC("BASE"): return "Baseline Table";
    case FOURCC("bdat"): return "Bitmap Data Table";
    case FOURCC("bhed"): return "Bitmap Font Header Table";
    case FOURCC("bloc"): return "Bitmap Location Table";
    case FOURCC("bsln"): return "Baseline Table";
    case FOURCC("CBDT"): return "Color Bitmap Data Table";
    case FOURCC("CBLC"): return "Color Bitmap Location Table";
    case FOURCC("CFF "): return "Compact Font Format Table";
    case FOURCC("CFF2"): return "Compact Font Format 2 Table";
    case FOURCC("cmap"): return "Character to Glyph Index Mapping Table";
    case FOURCC("COLR"): return "Color Table";
    case FOURCC("CPAL"): return "Color Palette Table";
    case FOURCC("cvar"): return "CVT Variations Table";
    case FOURCC("cvt "): return "Control Value Table";
    case FOURCC("DSIG"): return "Digital Signature Table";
    case FOURCC("EBDT"): return "Embedded Bitmap Data Table";
    case FOURCC("EBLC"): return "Embedded Bitmap Location Table";
    case FOURCC("EBSC"): return "Embedded Bitmap Scaling Table";
    case FOURCC("fdsc"): return "Font Descriptors Table";
    case FOURCC("feat"): return "Feature Name Table";
    case FOURCC("fmtx"): return "Font Metrics Table";
    case FOURCC("fpgm"): return "Font Program Table";
    case FOURCC("fvar"): return "Font Variations Table";
    case FOURCC("gasp"): return "Grid-fitting and Scan-conversion Procedure Table";
    case FOURCC("gcid"): return "Character to CID Table";
    case FOURCC("GDEF"): return "Glyph Definition Table";
    case FOURCC("glyf"): return "Glyph Data Table";
    case FOURCC("GPOS"): return "Glyph Positioning Table";
    case FOURCC("GSUB"): return "Glyph Substitution Table";
    case FOURCC("gvar"): return "Glyph Variations Table";
    case FOURCC("hdmx"): return "Horizontal Device Metrics";
    case FOURCC("head"): return "Font Header Table";
    case FOURCC("hhea"): return "Horizontal Header Table";
    case FOURCC("hmtx"): return "Horizontal Metrics Table";
    case FOURCC("HVAR"): return "Horizontal Metrics Variations Table";
    case FOURCC("JSTF"): return "Justification Table";
    case FOURCC("just"): return "Justification Table";
    case FOURCC("kern"): return "Kerning Table";
    case FOURCC("kerx"): return "Extended Kerning Table";
    case FOURCC("lcar"): return "Ligature Caret Table";
    case FOURCC("loca"): return "Index to Location Table";
    case FOURCC("ltag"): return "IETF Language Tags Table";
    case FOURCC("LTSH"): return "Linear Threshold Table";
    case FOURCC("MATH"): return "The Mathematical Typesetting Table";
    case FOURCC("maxp"): return "Maximum Profile Table";
    case FOURCC("MERG"): return "Merge Table";
    case FOURCC("meta"): return "Metadata Table";
    case FOURCC("mort"): return "Glyph Metamorphosis Table";
    case FOURCC("morx"): return "Extended Glyph Metamorphosis Table";
    case FOURCC("MVAR"): return "Metrics Variations Table";
    case FOURCC("name"): return "Naming Table";
    case FOURCC("opbd"): return "Optical Bounds Table";
    case FOURCC("OS/2"): return "OS/2 and Windows Metrics Table";
    case FOURCC("PCLT"): return "PCL 5 Table";
    case FOURCC("post"): return "PostScript Table";
    case FOURCC("prep"): return "Control Value Program";
    case FOURCC("prop"): return "Glyph Properties Table";
    case FOURCC("sbix"): return "Standard Bitmap Graphics Table";
    case FOURCC("STAT"): return "Style Attributes Table";
    case FOURCC("SVG "): return "Scalable Vector Graphics Table";
    case FOURCC("trak"): return "Tracking Table";
    case FOURCC("VDMX"): return "Vertical Device Metrics";
    case FOURCC("vhea"): return "Vertical Header Table";
    case FOURCC("vmtx"): return "Vertical Metrics Table";
    case FOURCC("VORG"): return "Vertical Origin Table";
    case FOURCC("VVAR"): return "Vertical Metrics Variations Table";
    case FOURCC("Zapf"): return "Glyph Information Table";
    default: return "Unknown Table";
    }
}

static void parseFontHeader(const quint32 fontIndex, QVector<FontTable> &tables, Parser &parser)
{
    parser.beginGroup("Header");
    const auto magic = parser.read<UInt32>("Magic");
    if (magic != 0x00010000 && magic != 0x4F54544F) {
        throw QString("not a TrueType font");
    }
    const auto numberOfTables = parser.read<UInt16>("Number of tables");
    parser.read<UInt16>("Search range");
    parser.read<UInt16>("Entry selector");
    parser.read<UInt16>("Range shift");
    parser.endGroup();

    parser.readArray("Table Records", numberOfTables, [&](const auto /*index*/) {
        parser.beginGroup("");
        const auto tag = parser.read<Tag>("Tag");
        parser.read<UInt32>("Checksum");
        const auto offset = parser.read<Offset32>("Offset");
        const auto length = parser.read<UInt32>("Length");
        tables.push_back({ fontIndex, tag, offset, length });
        parser.endGroup(tableName(tag), tag.toString());
    });
}

struct CommonFaceData
{
    quint16 numberOfGlyphs = 0;
    quint16 indexToLocationFormat = 0;
    quint16 numberOfHMetrics = 0;
    quint16 numberOfVMetrics = 0;
    NamesHash names;
    QVector<quint32> locaOffsets;
    QVector<CblcIndex> blocLocations;
    QVector<CblcIndex> eblcLocations;
    QVector<CblcIndex> cblcLocations;
};

static std::optional<FontTable> findTable(const QVector<FontTable> &tables, const quint32 faceIndex, const char* tag)
{
    return algo::find_if(tables, [=](const auto table){ return table.faceIndex == faceIndex && table.tag == tag; });
}

static CommonFaceData parseCommonFaceData(const QVector<FontTable> &tables, const quint32 faceIndex, ShadowParser shadow)
{
    CommonFaceData faceData;

    if (const auto table = findTable(tables, faceIndex, "maxp")) {
        auto s = shadow;
        s.advanceTo(table->offset + 4);
        faceData.numberOfGlyphs = s.read<UInt16>();
    }

    if (const auto table = findTable(tables, faceIndex, "head")) {
        auto s = shadow;
        s.advanceTo(table->offset + 50);
        faceData.indexToLocationFormat = s.read<UInt16>();
    }

    if (const auto table = findTable(tables, faceIndex, "hhea")) {
        auto s = shadow;
        s.advanceTo(table->offset + 34);
        faceData.numberOfHMetrics = s.read<UInt16>();
    }

    if (const auto table = findTable(tables, faceIndex, "vhea")) {
        auto s = shadow;
        s.advanceTo(table->offset + 34);
        faceData.numberOfVMetrics = s.read<UInt16>();
    }

    if (const auto table = findTable(tables, faceIndex, "loca")) {
        auto s = shadow;
        s.advanceTo(table->offset);
        faceData.locaOffsets = collectLocaOffsets(faceData.numberOfGlyphs, faceData.indexToLocationFormat, s);
    }

    if (const auto table = findTable(tables, faceIndex, "name")) {
        auto s = shadow;
        s.advanceTo(table->offset);
        faceData.names = collectNameNames(s);
    }

    if (const auto table = findTable(tables, faceIndex, "bloc")) {
        auto s = shadow;
        s.advanceTo(table->offset);
        faceData.blocLocations = parseCblcLocations(s);
    }

    return faceData;
}

static QStringList parseTables(const int numberOfFaces, const QVector<FontTable> &tables, ShadowParser shadow, Parser &parser)
{
    // We cannot use algo::dedup_vector otherwise findTable will break down.
    QVector<quint32> processedOffsets;

    QVector<CommonFaceData> facesData;
    try {
        for (int i = 0; i < numberOfFaces; i++) {
            facesData << parseCommonFaceData(tables, i, shadow);
        }
    } catch (const QString &msg) {
        throw QString("common face data parsing failed because %1").arg(msg);
    }

    QStringList warnings;

    for (const auto table : tables) {
        if (table.offset < parser.offset()) {
            continue;
        }

        if (processedOffsets.contains(table.offset)) {
            continue;
        }
        processedOffsets << table.offset;

        QString currTableName = tableName(table.tag);
        if (numberOfFaces > 1) {
            currTableName += QString(" (Face %1)").arg(table.faceIndex);
        }
        parser.beginGroup(currTableName, table.tag.toString());

        try {
            parser.advanceTo(table.offset);

            const CommonFaceData &fd = facesData[table.faceIndex];

            switch (table.tag.d) {
            case FOURCC("avar"): parseAvar(parser); break;
            case FOURCC("bdat"): parseCbdt(fd.blocLocations, parser); break;
            case FOURCC("bloc"): parseCblc(parser); break;
            case FOURCC("CBDT"): parseCbdt(fd.cblcLocations, parser); break;
            case FOURCC("CBLC"): parseCblc(parser); break;
            case FOURCC("CFF "): parseCff(parser); break;
            case FOURCC("CFF2"): parseCff2(parser); break;
            case FOURCC("cmap"): parseCmap(parser); break;
            case FOURCC("EBDT"): parseCbdt(fd.eblcLocations, parser); break;
            case FOURCC("EBLC"): parseCblc(parser); break;
            case FOURCC("feat"): parseFeat(fd.names, parser); break;
            case FOURCC("fvar"): parseFvar(fd.names, parser); break;
            case FOURCC("GDEF"): parseGdef(parser); break;
            case FOURCC("glyf"): parseGlyf(fd.numberOfGlyphs, fd.locaOffsets, parser); break;
            case FOURCC("gvar"): parseGvar(parser); break;
            case FOURCC("head"): parseHead(parser); break;
            case FOURCC("hhea"): parseHhea(parser); break;
            case FOURCC("hmtx"): parseHmtx(fd.numberOfHMetrics, fd.numberOfGlyphs, parser); break;
            case FOURCC("HVAR"): parseHvar(parser); break;
            case FOURCC("kern"): parseKern(parser); break;
            case FOURCC("loca"): parseLoca(fd.numberOfGlyphs, fd.indexToLocationFormat, parser); break;
            case FOURCC("maxp"): parseMaxp(parser); break;
            case FOURCC("MVAR"): parseMvar(parser); break;
            case FOURCC("name"): parseName(parser); break;
            case FOURCC("OS/2"): parseOS2(parser); break;
            case FOURCC("post"): parsePost(parser); break;
            case FOURCC("sbix"): parseSbix(fd.numberOfGlyphs, parser); break;
            case FOURCC("STAT"): parseStat(fd.names, parser); break;
            case FOURCC("SVG "): parseSvg(parser); break;
            case FOURCC("trak"): parseTrak(fd.names, parser); break;
            case FOURCC("vhea"): parseVhea(parser); break;
            case FOURCC("vmtx"): parseVmtx(fd.numberOfVMetrics, fd.numberOfGlyphs, parser); break;
            case FOURCC("VVAR"): parseVvar(parser); break;
            case FOURCC("VORG"): parseVorg(parser); break;
            default: parser.readUnsupported(table.length); break;
            }
        } catch (const QString &msg) {
            warnings << QString("'%1' table parsing failed because %2")
                .arg(table.tag.toString()).arg(msg);
        }

        if (parser.offset() != table.offset + table.length) {
            parser.advanceTo(table.offset + table.length);
        }

        const auto pad = ((table.length + 3) & ~3) - table.length;
        parser.readPadding(pad);

        parser.endGroup();
    }

    parser.finish();

    return warnings;
}

QStringList TrueType::parse(Parser &parser)
{
    QVector<FontTable> tables;

    auto shadow = parser.shadow();

    const auto magic = parser.peek<UInt32>();
    if (magic != 0x00010000 && magic != 0x4F54544F && magic != 0x74746366) {
        throw QString("not a TrueType font");
    }

    int numberOfFaces = 1;
    if (magic != 0x74746366) {
        parseFontHeader(0, tables, parser);
    } else {
        // A font collection.

        parser.beginGroup("Header");
        parser.read<UInt32>("Magic");
        const auto majorVersion = parser.read<UInt16>("Major version");
        parser.read<UInt16>("Minor version");
        numberOfFaces = parser.read<UInt32>("Number of fonts");

        QVector<quint32> offsets;
        parser.readArray("Offsets", numberOfFaces, [&](const auto index) {
            offsets << parser.read<Offset32>(index);
        });
        algo::sort_all(offsets);
        algo::dedup_vector(offsets);

        if (majorVersion == 2) {
            parser.read<Tag>("DSIG tag");
            parser.read<UInt32>("DSIG table length");
            parser.read<OptionalOffset32>("DSIG table offset");
        }

        parser.readArray("Faces", offsets.size(), [&](const auto index) {
            parser.advanceTo(offsets[index]);
            parser.beginGroup(index);
            parseFontHeader(quint32(index), tables, parser);
            parser.endGroup();
        });

        parser.endGroup();
    }

    algo::sort_all_by_key(tables, &FontTable::offset);
    return parseTables(numberOfFaces, tables, shadow, parser);
}
