#include <set>

#include "src/algo.h"
#include "src/parser.h"
#include "tables.h"

// TODO: ligCaretListOffset
// TODO: itemVarStoreOffset

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
        // TODO: what to do?
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
        // TODO: what to do?
    }
}

void parseGdef(Parser &parser)
{
    const auto start = parser.offset();

    const auto majorVersion = parser.read<UInt16>("Major version");
    const auto minorVersion = parser.read<UInt16>("Minor version");
    const auto glyphClassDefOffset = parser.read<std::optional<Offset16>>("Offset to class definition table");
    const auto attachmentPointListOffset = parser.read<std::optional<Offset16>>("Offset to attachment point list table");
    parser.read<std::optional<Offset16>>("Offset to ligature caret list table");
    const auto markAttachClassDefOffset = parser.read<std::optional<Offset16>>("Offset to class definition table for mark attachment type");

    std::optional<Offset16> markGlyphSetsDefOffset = std::nullopt;
    if (majorVersion == 1 && minorVersion == 2) {
        markGlyphSetsDefOffset = parser.read<std::optional<Offset16>>("Offset to the table of mark glyph set definitions");
    } else if (majorVersion == 1 && minorVersion == 3) {
        markGlyphSetsDefOffset = parser.read<std::optional<Offset16>>("Offset to the table of mark glyph set definitions");
        parser.read<std::optional<Offset32>>("Offset to the Item Variation Store table");
    } else {
        // TODO: what to do?
    }

    // All subtable offsets are from a beginning of the GDEF header.

    // TODO: sort offsets

    if (glyphClassDefOffset) {
        parser.jumpTo(start + glyphClassDefOffset.value());
        parser.beginGroup("Class Definition Table");
        parseClassDefinitionTable(parser);
        parser.endGroup();
    }

    if (attachmentPointListOffset) {
        parser.jumpTo(start + attachmentPointListOffset.value());
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

        parser.jumpTo(start + attachmentPointListOffset.value() + coverageOffset);
        parser.beginGroup("Coverage table");
        parseCoverageTable(parser);
        parser.endGroup();

        if (!offsets.isEmpty()) {
            algo::sort_all(offsets);
            algo::dedup_vector(offsets);

            parser.beginGroup("Attach Point tables");
            for (const auto [i, offset] : algo::enumerate(offsets)) {
                parser.jumpTo(start + attachmentPointListOffset.value() + *offset);
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
    }

    if (markAttachClassDefOffset) {
        parser.jumpTo(start + markAttachClassDefOffset.value());
        parser.beginGroup("Mark Attachment Class Definition Table");
        parseClassDefinitionTable(parser);
        parser.endGroup();
    }

    if (markGlyphSetsDefOffset) {
        parser.jumpTo(start + markGlyphSetsDefOffset.value());
        parser.beginGroup("Mark Glyph Sets Table");

        const auto substart = parser.offset();
        parser.read<UInt16>("Format"); // TODO: format must be 1
        const auto markGlyphSetCount = parser.read<UInt16>("Number of mark glyph sets");

        if (markGlyphSetCount != 0) {
            std::set<Offset32> offsetsSet; // Table can have duplicated offsets.
            parser.beginGroup("Offsets to mark glyph set coverage tables");
            for (auto i = 0; i < markGlyphSetCount; ++i) {
                offsetsSet.insert(parser.read<Offset32>("Offset"));
            }
            parser.endGroup();

            std::vector offsetsList(offsetsSet.begin(), offsetsSet.end() );
            algo::sort_all(offsetsList, [](const auto a, const auto b){ return a < b; });

            for (const auto &offset : offsetsList) {
                parser.jumpTo(substart + offset);
                parser.beginGroup("Coverage table");
                parseCoverageTable(parser);
                parser.endGroup();
            }
        }

        parser.endGroup();
    }
}
