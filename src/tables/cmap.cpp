#include "src/algo.h"
#include "src/parser.h"
#include "tables.h"

using namespace NameCommon;

static void parseLanguage16(const PlatformID platformID, Parser &parser)
{
    const auto id = parser.peek<UInt16>();
    parser.readValue(2, "Language ID", languageName(platformID, id));
}

static void parseLanguage32(const PlatformID platformID, Parser &parser)
{
    const auto id = parser.peek<UInt32>();
    parser.readValue(4, "Language ID", languageName(platformID, quint16(id)));
}

static void parseFormat0(const PlatformID platformID, Parser &parser)
{
    parser.read<UInt16>("Subtable size");
    parseLanguage16(platformID, parser);
    parser.readArray<UInt8>("Glyphs", "Glyph", 256);
}

static void parseFormat2(const PlatformID platformID, Parser &parser)
{
    const auto tableStart = parser.offset() - 2;
    const auto tableSize = parser.read<UInt16>("Subtable size");
    parseLanguage16(platformID, parser);

    quint16 subHeadersCount = 0;
    parser.beginGroup("SubHeader heys", QString::number(256));
    for (quint32 i = 0; i < 256; ++i) {
        const quint16 key = parser.read<UInt16>("Key " + QString::number(i));
        subHeadersCount = qMax(subHeadersCount, quint16(key / 8));
    }
    parser.endGroup();

    subHeadersCount += 1;
    parser.beginGroup("SubHeader records", QString::number(subHeadersCount));
    for (quint32 i = 0; i < subHeadersCount; ++i) {
        parser.beginGroup(QString("SubHeader %1").arg(i));
        parser.read<UInt16>("First valid low byte");
        parser.read<UInt16>("Number of valid low bytes");
        parser.read<Int16>("ID delta");
        parser.read<UInt16>("ID range offset");
        parser.endGroup();
    }
    parser.endGroup();

    // TODO: technically, we should split the tail into subarrays,
    //       but looks like ranges can overlap and ttf-explorer doesn't support this

    const auto tailSize = tableSize - (parser.offset() - tableStart);
    parser.readArray<GlyphId>("Glyph index array", "Glyph", tailSize / 2);
}

static void parseFormat4(const PlatformID platformID, Parser &parser)
{
    const auto tableStart = parser.offset() - 2;
    const auto tableSize = parser.read<UInt16>("Subtable size");
    parseLanguage16(platformID, parser);
    const auto segCount2 = parser.read<UInt16>("2 × segCount");
    const auto segCount = quint16(segCount2 / 2);
    parser.read<UInt16>("Search range");
    parser.read<UInt16>("Entry selector");
    parser.read<UInt16>("Range shift");
    parser.readArray<UInt16>("End character codes", "Code", segCount);
    parser.read<UInt16>("Reserved");
    parser.readArray<UInt16>("Start character codes", "Code", segCount);
    parser.readArray<Int16>("Deltas", "Delta", segCount);
    parser.readArray<UInt16>("Offsets into Glyph index array", "Offset", segCount);

    const auto tailSize = tableSize - (parser.offset() - tableStart);
    parser.readArray<GlyphId>("Glyph index array", "Glyph", tailSize / 2);
}

static void parseFormat6(const PlatformID platformID, Parser &parser)
{
    parser.read<UInt16>("Subtable size");
    parseLanguage16(platformID, parser);
    parser.read<UInt16>("First code");
    const auto count = parser.read<UInt16>("Number of codes");
    parser.readArray<GlyphId>("Glyph index array", "Glyph", count);
}

static void parseFormat8(const PlatformID platformID, Parser &parser)
{
    parser.read<UInt16>("Reserved");
    parser.read<UInt32>("Subtable size");
    parseLanguage32(platformID, parser);
    parser.readBytes(8192, "Packed data");
    const auto count = parser.read<UInt32>("Number of groups");

    parser.beginGroup("SequentialMapGroup records", QString::number(count));
    for (quint32 i = 0; i < count; ++i) {
        parser.beginGroup(QString("Record %1").arg(i));
        parser.read<UInt32>("First character code");
        parser.read<UInt32>("Last character code");
        parser.read<UInt32>("Starting glyph index");
        parser.endGroup();
    }
    parser.endGroup();
}

static void parseFormat10(const PlatformID platformID, Parser &parser)
{
    parser.read<UInt16>("Reserved");
    parser.read<UInt32>("Subtable size");
    parseLanguage32(platformID, parser);
    parser.read<UInt32>("First code");
    const auto count = parser.read<UInt32>("Number of codes");
    parser.readArray<GlyphId>("Glyph index array", "Glyph", count);
}

static void parseFormat12(const PlatformID platformID, Parser &parser)
{
    parser.read<UInt16>("Reserved");
    parser.read<UInt32>("Subtable size");
    parseLanguage32(platformID, parser);
    const auto count = parser.read<UInt32>("Number of groups");

    parser.beginGroup("SequentialMapGroup records", QString::number(count));
    for (quint32 i = 0; i < count; ++i) {
        parser.beginGroup(QString("Record %1").arg(i));
        parser.read<UInt32>("First character code");
        parser.read<UInt32>("Last character code");
        parser.read<UInt32>("Starting glyph index");
        parser.endGroup();
    }
    parser.endGroup();
}

static void parseFormat13(const PlatformID platformID, Parser &parser)
{
    parser.read<UInt16>("Reserved");
    parser.read<UInt32>("Subtable size");
    parseLanguage32(platformID, parser);
    const auto count = parser.read<UInt32>("Number of groups");

    parser.beginGroup("ConstantMapGroup records", QString::number(count));
    for (quint32 i = 0; i < count; ++i) {
        parser.beginGroup(QString("Record %1").arg(i));
        parser.read<UInt32>("First character code");
        parser.read<UInt32>("Last character code");
        parser.read<UInt32>("Glyph index");
        parser.endGroup();
    }
    parser.endGroup();
}

static void parseFormat14(Parser &parser)
{
    const auto tableStart = parser.offset() - 2;

    parser.read<UInt32>("Subtable size");
    const auto count = parser.read<UInt32>("Number of records");

    struct Record
    {
        bool isDefault;
        quint32 offset;
    };

    QVector<Record> records;
    parser.beginGroup("VariationSelector records", QString::number(count));
    for (quint32 i = 0; i < count; ++i) {
        parser.beginGroup(QString("Record %1").arg(i));
        parser.read<UInt24>("Variation selector");
        const auto defOffset = parser.read<Offset32>("Offset to Default UVS Table");
        const auto nonDefOffset = parser.read<Offset32>("Offset to Non-Default UVS Table");
        parser.endGroup();

        if (defOffset != 0) {
            records.append({ true, tableStart + defOffset });
        }

        if (nonDefOffset != 0) {
            records.append({ false, tableStart + nonDefOffset });
        }
    }
    parser.endGroup();

    algo::sort_all_by_key(records, &Record::offset);
    algo::dedup_vector_by_key(records, &Record::offset);

    for (const auto record : records) {
        parser.jumpTo(record.offset);
        if (record.isDefault) {
            parser.beginGroup("Default UVS table");
            const auto count = parser.read<UInt32>("Number of Unicode character ranges");
            for (uint i = 0; i < count; ++i) {
                parser.beginGroup("Unicode range");
                parser.read<UInt24>("First value in this range");
                parser.read<UInt8>("Number of additional values");
                parser.endGroup();
            }
            parser.endGroup();
        } else {
            parser.beginGroup("Non-Default UVS table");
            const auto count = parser.read<UInt32>("Number of UVS Mappings");
            for (uint i = 0; i < count; ++i) {
                parser.beginGroup("UVS mapping");
                parser.read<UInt24>("Base Unicode value");
                parser.read<GlyphId>("Glyph ID");
                parser.endGroup();
            }
            parser.endGroup();
        }
    }
}

void parseCmap(Parser &parser)
{
    const auto tableStart = parser.offset();

    const auto version = parser.read<UInt16>("Version");
    if (version != 0) {
        throw "invalid table version";
    }

    struct Record
    {
        quint32 offset;
        PlatformID platformID;
    };

    const auto numTables = parser.read<UInt16>("Number of tables");
    QVector<Record> records;
    parser.beginGroup("Encoding records");
    for (int i = 0; i < numTables; ++i) {
        parser.beginGroup(QString("Record %1").arg(i));
        const auto platformID = parser.read<PlatformID>("Platform ID");
        {
            const auto id = parser.peek<UInt16>();
            parser.readValue(2, "Encoding ID", encodingName(platformID, id));
        }
        const auto offset = parser.read<Offset32>("Offset");
        parser.endGroup();

        records.append({ offset, platformID });
    }
    parser.endGroup();

    algo::sort_all_by_key(records, &Record::offset);
    algo::dedup_vector_by_key(records, &Record::offset);

    for (const auto record : records) {
        parser.jumpTo(tableStart + record.offset);
        parser.beginGroup("Table");
        QString title;
        const auto format = parser.read<UInt16>("Format");
        switch (format) {
            case 0 : {
                parseFormat0(record.platformID, parser);
                title = "Byte encoding table";
                break;
            }
            case 2 : {
                parseFormat2(record.platformID, parser);
                title = "High-byte mapping through table";
                break;
            }
            case 4 : {
                parseFormat4(record.platformID, parser);
                title = "Segment mapping to delta values";
                break;
            }
            case 6 : {
                parseFormat6(record.platformID, parser);
                title = "Trimmed table mapping";
                break;
            }
            case 8 : {
                parseFormat8(record.platformID, parser);
                title = "Mixed 16-bit and 32-bit coverage";
                break;
            }
            case 10 : {
                parseFormat10(record.platformID, parser);
                title = "Trimmed array";
                break;
            }
            case 12 : {
                parseFormat12(record.platformID, parser);
                title = "Segmented coverage";
                break;
            }
            case 13 : {
                parseFormat13(record.platformID, parser);
                title = "Many-to-one range mappings";
                break;
            }
            case 14 : {
                parseFormat14(parser);
                title = "Unicode Variation Sequences";
                break;
            }
            default : break;
        }

        parser.endGroup(QString("Subtable %1: %2").arg(format).arg(title));
    }
}
