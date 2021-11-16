#include <set>

#include "src/algo.h"
#include "src/parser.h"
#include "tables.h"

// TODO: ligCaretListOffset

static void parseClassDefinitionTable(Parser &parser)
{
    const auto classFormat = parser.read<UInt16>("Format");

    if (classFormat == 1) {
        parser.read<UInt16>("First glyph ID");
        const auto glyphCount = parser.read<UInt16>("Number of classes");
        for (auto i = 0; i < glyphCount; ++i) {
            parser.read<UInt16>("Class");
        }
    } else if (classFormat == 2) {
        const auto classRangeCount = parser.read<UInt16>("Number of records");
        for (auto i = 0; i < classRangeCount; ++i) {
            parser.beginGroup("Class Range Record");
            const auto first = parser.read<UInt16>("First glyph ID");
            const auto last = parser.read<UInt16>("Last glyph ID");
            const auto klass = parser.read<UInt16>("Class");
            parser.endGroup(QString(), QString("%1..%2 %3").arg(first).arg(last).arg(klass));
        }
    } else {
        throw QString("invalid class format");
    }
}

static void parseCoverageTable(Parser &parser)
{
    const auto format = parser.read<UInt16>("Format");

    if (format == 1) {
        const auto glyphCount = parser.read<UInt16>("Number of glyphs");
        for (auto i = 0; i < glyphCount; ++i) {
            parser.read<GlyphId>("Glyph");
        }
    } else if (format == 2) {
        const auto rangeCount = parser.read<UInt16>("Number of records");
        for (auto i = 0; i < rangeCount; ++i) {
            parser.beginGroup("Range Record");
            const auto first = parser.read<UInt16>("First glyph ID");
            const auto last = parser.read<UInt16>("Last glyph ID");
            const auto index = parser.read<UInt16>("Coverage Index of first glyph ID");
            parser.endGroup(QString(), QString("%1..%2 %3").arg(first).arg(last).arg(index));
        }
    } else {
        throw QString("invalid coverage format");
    }
}

void parseGdef(Parser &parser)
{
    const auto start = parser.offset();

    const auto majorVersion = parser.read<UInt16>("Major version");
    const auto minorVersion = parser.read<UInt16>("Minor version");
    const quint32 glyphClassDefOffset = parser.read<OptionalOffset16>("Offset to class definition table");
    const quint32 attachmentPointListOffset = parser.read<OptionalOffset16>("Offset to attachment point list table");
    parser.read<OptionalOffset16>("Offset to ligature caret list table");
    const quint32 markAttachClassDefOffset
        = parser.read<OptionalOffset16>("Offset to class definition table for mark attachment type");

    quint32 markGlyphSetsDefOffset = 0;
    quint32 varStoreOffset = 0;
    if (majorVersion == 1 && minorVersion == 2) {
        markGlyphSetsDefOffset = parser.read<OptionalOffset16>("Offset to the table of mark glyph set definitions");
    } else if (majorVersion == 1 && minorVersion == 3) {
        markGlyphSetsDefOffset = parser.read<OptionalOffset16>("Offset to the table of mark glyph set definitions");
        varStoreOffset = parser.read<OptionalOffset32>("Offset to the Item Variation Store table");
    } else {
        // TODO: what to do?
    }

    // All subtable offsets are from a beginning of the GDEF header.

    enum class OffsetType {
        GlyphClassDef,
        AttachmentPointList,
        MarkAttachClassDef,
        MarkGlyphSetsDef,
        VarStore,
    };
    struct Offset {
        OffsetType type;
        quint32 offset;
    };
    std::array<Offset, 5> offsets = {{
        { OffsetType::GlyphClassDef, glyphClassDefOffset },
        { OffsetType::AttachmentPointList, attachmentPointListOffset },
        { OffsetType::MarkAttachClassDef, markAttachClassDefOffset },
        { OffsetType::MarkGlyphSetsDef, markGlyphSetsDefOffset },
        { OffsetType::VarStore, varStoreOffset },
    }};

    algo::sort_all_by_key(offsets, &Offset::offset);

    for (const auto offset : offsets) {
        if (offset.offset == 0) {
            continue;
        }

        parser.advanceTo(start + offset.offset);
        switch (offset.type) {
        case OffsetType::GlyphClassDef: {
            parser.beginGroup("Class Definition Table");
            parseClassDefinitionTable(parser);
            parser.endGroup();
            break;
        }
        case OffsetType::AttachmentPointList: {
            parser.beginGroup("Attachment Point List Table");
            const auto coverageOffset = parser.read<Offset16>("Offset to Coverage table");
            const auto count = parser.read<UInt16>("Number of glyphs with attachment points");

            QVector<Offset16> offsets;
            if (count > 0) {
                parser.beginGroup("Offsets to Attach Point tables");
                for (int i = 0; i < count; ++i) {
                    offsets << parser.read<Offset16>(QString("Offset %1").arg(i));
                }
                parser.endGroup();
            }

            parser.advanceTo(start + attachmentPointListOffset + coverageOffset);
            parser.beginGroup("Coverage Table");
            parseCoverageTable(parser);
            parser.endGroup();

            if (!offsets.isEmpty()) {
                algo::sort_all(offsets);
                algo::dedup_vector(offsets);

                parser.beginGroup("Attach Point Tables");
                for (const auto [i, offset] : algo::enumerate(offsets)) {
                    parser.advanceTo(start + attachmentPointListOffset + *offset);
                    parser.beginGroup(QString("Attach Point %1").arg(i));
                    const auto count = parser.read<UInt16>("Number of attachment points");
                    for (int i = 0; i < count; ++i) {
                        parser.read<UInt16>("Contour point index");
                    }
                    parser.endGroup();
                }
                parser.endGroup();
            }

            parser.endGroup();
            break;
        }
        case OffsetType::MarkAttachClassDef:
            parser.beginGroup("Mark Attachment Class Definition Table");
            parseClassDefinitionTable(parser);
            parser.endGroup();
            break;
        case OffsetType::MarkGlyphSetsDef: {
            parser.beginGroup("Mark Glyph Sets Table");

            const auto substart = parser.offset();
            parser.read<UInt16>("Format"); // TODO: format must be 1
            const auto markGlyphSetCount = parser.read<UInt16>("Number of mark glyph sets");

            if (markGlyphSetCount != 0) {
                std::set<Offset32> offsetsSet; // Table can have duplicated offsets.
                parser.readArray("Offsets to Mark Glyph Set Coverage Tables", markGlyphSetCount,
                [&](const auto index){
                    offsetsSet.insert(parser.read<Offset32>(index));
                });

                std::vector offsetsList(offsetsSet.begin(), offsetsSet.end() );
                algo::sort_all(offsetsList);

                for (const auto &offset : offsetsList) {
                    parser.advanceTo(substart + offset);
                    parser.beginGroup("Coverage Table");
                    parseCoverageTable(parser);
                    parser.endGroup();
                }
            }

            parser.endGroup();
            break;
        }
        case OffsetType::VarStore: {
            parser.beginGroup("Item Variation Store Table");
            parseItemVariationStore(parser);
            parser.endGroup();
            break;
        }
        }
    }
}
