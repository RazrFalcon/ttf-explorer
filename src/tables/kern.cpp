#include <bitset>

#include "src/algo.h"
#include "tables.h"

struct OpenTypeCoverage
{
    static const int Size = 1;
    static const QString Type;

    static OpenTypeCoverage parse(const quint8 *data)
    { return { data[0] }; }

    static QString toString(const OpenTypeCoverage &value)
    {
        std::bitset<8> bits(value.d);
        auto flagsStr = QString::fromUtf8(bits.to_string().c_str()) + '\n';

        if (bits[0]) { flagsStr += "Bit 0: Horizontal\n"; }
        if (bits[1]) { flagsStr += "Bit 1: Has minimum values\n"; }
        if (bits[2]) { flagsStr += "Bit 2: Cross-stream\n"; }
        if (bits[3]) { flagsStr += "Bit 3: Override\n"; }
        // 4-7 - reserved

        flagsStr.chop(1); // trim trailing newline

        return flagsStr;
    }

    quint8 d;
};

const QString OpenTypeCoverage::Type = Parser::BitflagsType;


struct AppleCoverage
{
    static const int Size = 1;
    static const QString Type;

    static AppleCoverage parse(const quint8 *data)
    { return { data[0] }; }

    static QString toString(const AppleCoverage &value)
    {
        std::bitset<8> bits(value.d);
        auto flagsStr = QString::fromUtf8(bits.to_string().c_str()) + '\n';

        // 0-4 - reserved
        if (bits[5]) { flagsStr += "Bit 5: Has variation\n"; }
        if (bits[6]) { flagsStr += "Bit 6: Cross-stream\n"; }
        if (bits[7]) { flagsStr += "Bit 7: Vertical\n"; }

        flagsStr.chop(1); // trim trailing newline

        return flagsStr;
    }

    quint8 d;
};

const QString AppleCoverage::Type = Parser::BitflagsType;


static void parseFormat0(Parser &parser)
{
    const auto count = parser.read<UInt16>("Number of kerning pairs");
    parser.read<UInt16>("Search range");
    parser.read<UInt16>("Entry selector");
    parser.read<UInt16>("Range shift");

    parser.readArray("Values", count, [&](const auto index){
        parser.beginGroup(index);
        parser.read<GlyphId>("Left");
        parser.read<GlyphId>("Right");
        parser.read<Int16>("Value");
        parser.endGroup();
    });
}

namespace Format1 {
    struct EntryFlags
    {
        static const int Size = 2;
        static const QString Type;

        static EntryFlags parse(const quint8 *data)
        { return { UInt16::parse(data) }; }

        static QString toString(const EntryFlags &value)
        {
            std::bitset<16> bits(value.d);
            auto flagsStr = "Offset " + numberToString(value.d & 0x3FFF) + '\n';
            flagsStr += QString::fromUtf8(bits.to_string().c_str()) + '\n';

            if (bits[15]) { flagsStr += "Bit 15: Push onto the kerning stack\n"; }

            flagsStr.chop(1); // trim trailing newline

            return flagsStr;
        }

        quint16 d;
    };

    const QString EntryFlags::Type = Parser::BitflagsType;


    struct Action
    {
        static const int Size = 2;
        static const QString Type;

        static Action parse(const quint8 *data)
        { return { UInt16::parse(data) }; }

        static QString toString(const Action &value)
        {
            static const QString value1 = QLatin1String("Kerning 0. End of List.");
            static const QString value2 = QLatin1String("Reset cross-stream. End of List.");

            if (value.d == 0x0001) {
                return value1;
            } else if (value.d == 0x8001) {
                return value1;
            } else {
                return QString("Kerning %1").arg(qint16(value.d));
            }
        }

        quint16 d;
    };

    const QString Action::Type = QLatin1String("Action");


    struct Entry {
        quint16 newState;
        quint16 flags;
    };


    // Based on: https://github.com/harfbuzz/harfbuzz/blob/5b91c52083aee1653c0cf1e778923de00c08fa5d/src/hb-aat-layout-common.hh#L524
    static quint32 detectNumberOfEntries(
        const qint32 numberOfClasses,
        const quint16 stateArrayOffset,
        const QByteArray states,
        const QVector<Entry> &entries
    ) {
        qint32 minState = 0;
        qint32 maxState = 0;
        quint32 numEntries = 0;

        qint32 statePos = 0;
        qint32 stateNeg = 0;
        quint32 entry = 0;
        qint32 maxOps = 0x3FFFFFFF;
        while (minState < stateNeg || statePos <= maxState) {
            if (minState < stateNeg) {
                // Negative states.

                maxOps -= stateNeg - minState;
                if (maxOps <= 0) {
                    throw QString("invalid state machine");
                }

                // Sweep new states.
                const qint32 end = minState * numberOfClasses;
                if (end > 0) {
                    for (qint32 i = end-1; i >= 0; i--) {
                        if (i - 1 < states.size()) {
                            numEntries = qMax(numEntries, quint32(states[i - 1]) + 1);
                        } else {
                            throw QString("invalid index");
                        }
                    }
                }

                stateNeg = minState;
            }

            if (statePos <= maxState) {
                // Positive states.

                maxOps -= maxState - statePos + 1;
                if (maxOps <= 0) {
                    throw QString("invalid state machine");
                }

                // Sweep new states.
                const auto start = statePos * numberOfClasses;
                const auto end = (maxState + 1) * numberOfClasses;
                for (int i = start; i < end; i++) {
                    if (i < states.size()) {
                        numEntries = qMax(numEntries, quint32(states[i]) + 1);
                    } else {
                        throw QString("invalid index");
                    }
                }

                statePos = maxState + 1;
            }

            maxOps -= int(numEntries - entry);
            if (maxOps <= 0) {
                throw QString("invalid state machine");
            }

            // Sweep new entries.
            for (quint32 i = entry; i < numEntries; i++) {
                if (i >= quint32(entries.size())) {
                    throw QString("invalid index");
                }

                const auto newState = (int(entries[i].newState) - int(stateArrayOffset))
                    / int(numberOfClasses);

                minState = qMin(minState, newState);
                maxState = qMax(maxState, newState);
            }

            entry = numEntries;
        }

        return numEntries;
    }

    static void parse(const quint32 subtableSize, Parser &parser)
    {
        // Sadly, the *State Table for Contextual Kerning* format isn't really documented.
        // The AAT kern documentation has only a rough outline and a single example,
        // which is clearly not enough to understand how it actually stored.
        // And the only FOSS application I know that supports it is HarfBuzz,
        // so we are borrowing its implementation.
        // No idea how long it took them to figure it out.
        //
        // Also, there is an obsolete tool called `ftxanalyzer`,
        // from the *Apple Font Tool Suite* that can dump an AAT kern table as an XML.
        // But the result is still pretty abstract and it's hard to understand
        // how it was produced.
        // It's not trivial to install it on a modern Mac, so here is a useful
        // instruction (not mine):
        // https://gist.github.com/thetrevorharmon/9afdeb41a74f8f32b9561eeb83b10eff
        //
        // And here are some Apple fonts that actually use this format
        // (at least in macOS 10.14):
        // - Diwan Thuluth.ttf
        // - Farisi.ttf
        // - GeezaPro.ttc
        // - Mishafi.ttf
        // - Waseem.ttc

        const auto start = parser.offset();
        const auto shadow = parser.shadow();

        const quint16 numberOfClasses = parser.read<UInt16>("Number of classes");
        // Note that offsets are not from the subtable start,
        // but from subtable start + header.
        const auto classTableOffset = parser.read<Offset16>("Offset to class subtable");
        const auto stateArrayOffset = parser.read<Offset16>("Offset to state array");
        const auto entryTableOffset = parser.read<Offset16>("Offset to entry table");
        const auto valuesOffset = parser.read<Offset16>("Offset to values");

        // Check that offsets are properly ordered.
        // We do not support random order.
        Q_ASSERT(classTableOffset < stateArrayOffset);
        Q_ASSERT(stateArrayOffset < entryTableOffset);
        Q_ASSERT(entryTableOffset < valuesOffset);

        auto numberOfEntries = 0;
        {
            // Collect states.
            auto s1 = shadow;
            s1.advanceTo(stateArrayOffset);
            // We don't know the length of the states yet,
            // so use all the data from offset to the end of the subtable.
            const auto states = s1.readBytes(subtableSize - stateArrayOffset);

            // Collect entries.
            auto s2 = shadow;
            s2.advanceTo(entryTableOffset);
            // We don't know the length of the states yet,
            // so use all the data from offset to the end of the subtable.
            const auto entriesCount = (subtableSize - entryTableOffset) / 4;
            QVector<Entry> entries;
            for (quint32 i = 0; i < entriesCount; i++) {
                entries.append({
                    s2.read<UInt16>(),
                    s2.read<UInt16>(),
                });
            }

            numberOfEntries = detectNumberOfEntries(numberOfClasses, stateArrayOffset, states, entries);
        }

        parser.padTo(start + classTableOffset);
        parser.beginGroup("Class Subtable");
        parser.read<GlyphId>("First glyph");
        const auto numberOfGlyphs = parser.read<UInt16>("Number of glyphs");
        parser.readBasicArray<UInt8>("Classes", numberOfGlyphs);
        parser.endGroup();

        parser.padTo(start + stateArrayOffset);
        // We only assume that an *entry table* is right after *state array*.
        const auto arraysCount = (entryTableOffset - stateArrayOffset) / numberOfClasses;
        parser.readArray("State Array", arraysCount, [&](const auto /*index*/){
            parser.readBytes("Data", numberOfClasses);
        });

        parser.padTo(start + entryTableOffset);
        parser.readArray("Entries", numberOfEntries, [&](const auto index){
            parser.beginGroup(index);
            parser.read<Offset16>("State offset");
            parser.read<EntryFlags>("Flags");
            parser.endGroup();
        });

        parser.padTo(start + valuesOffset);
        const auto numberOfActions = ((subtableSize - 8) - (parser.offset() - start)) / 2;
        parser.readBasicArray<Action>("Actions", numberOfActions);
    }
}

static quint32 format2DetectNumberOfClasses(const quint32 offset, ShadowParser shadow) {
    shadow.advanceTo(offset);
    shadow.read<GlyphId>();
    const auto count = shadow.read<UInt16>();
    QSet<quint16> classes;
    for (quint32 i = 0; i < count; i++) {
        classes.insert(shadow.read<UInt16>());
    }

    return classes.size();
}

static void parseFormat2(const quint32 subtableStart, Parser &parser)
{
    // Apple fonts that are using this table: Apple Chancery.ttf

    const auto shadow = parser.shadow();
    const auto headerSize = parser.offset() - subtableStart;

    parser.read<UInt16>("Row width in bytes");

    const auto leftHandTableOffset = parser.read<Offset16>("Offset to left-hand class table");
    const auto rightHandTableOffset = parser.read<Offset16>("Offset to right-hand class table");
    const auto arrayOffset = parser.read<Offset16>("Offset to kerning array");

    const auto rows = format2DetectNumberOfClasses(leftHandTableOffset - headerSize, shadow);
    const auto columns = format2DetectNumberOfClasses(rightHandTableOffset - headerSize, shadow);

    enum class OffsetType {
        LeftHandTable,
        RightHandTable,
        Array,
    };
    struct Offset {
        OffsetType type;
        quint32 offset;
    };
    std::array<Offset, 3> offsets = {{
        { OffsetType::LeftHandTable, (quint32)leftHandTableOffset },
        { OffsetType::RightHandTable, (quint32)rightHandTableOffset },
        { OffsetType::Array, (quint32)arrayOffset },
    }};

    algo::sort_all_by_key(offsets, &Offset::offset);

    for (const auto offset : offsets) {
        if (offset.offset == 0) {
            continue;
        }

        parser.advanceTo(subtableStart + offset.offset);
        switch (offset.type) {
        case OffsetType::LeftHandTable: {
            parser.beginGroup("Left-hand Class Table");
            parser.read<GlyphId>("First glyph");
            const quint16 count = parser.read<UInt16>("Number of glyphs");
            parser.readBasicArray<UInt16>("Classes", count);
            parser.endGroup();
            break;
        }
        case OffsetType::RightHandTable: {
            parser.beginGroup("Right-hand Class Table");
            parser.read<GlyphId>("First glyph");
            const quint16 count = parser.read<UInt16>("Number of glyphs");
            parser.readBasicArray<UInt16>("Classes", count);
            parser.endGroup();
            break;
        }
        case OffsetType::Array: {
            parser.readBasicArray<Int16>("Kerning Values", rows * columns);
            break;
        }
        }
    }
}

static void parseFormat3(const quint32 subtableStart, const quint32 subtableSize, Parser &parser)
{
    // Apple fonts that are using this table: Skia.ttf

    const auto glyphCount = parser.read<UInt16>("Number of glyphs");
    const auto kernValues = parser.read<UInt8>("Number of kerning values");
    const auto leftHandClasses = parser.read<UInt8>("Number of left-hand classes");
    const auto rightHandClasses = parser.read<UInt8>("Number of right-hand classes");
    parser.read<UInt8>("Reserved");

    parser.readBasicArray<Int16>("Kerning Values", kernValues);
    parser.readBasicArray<UInt8>("Left-hand Classes", glyphCount);
    parser.readBasicArray<UInt8>("Right-hand Classes", glyphCount);
    parser.readBasicArray<UInt8>("Indices", int(leftHandClasses) * int(rightHandClasses));

    const auto left = int(subtableSize) - int(parser.offset() - subtableStart);
    parser.readPadding(left);
}

// https://docs.microsoft.com/en-us/typography/opentype/spec/kern
static void parseKernOpenType(Parser &parser)
{
    parser.read<UInt16>("Version");
    const auto numberOfTables = parser.read<UInt16>("Number of tables");
    parser.readArray("Subtables", numberOfTables, [&](const auto index){
        const auto subtableStart = parser.offset();

        parser.beginGroup(index);
        parser.read<UInt16>("Version");
        parser.read<UInt16>("Length");
        const auto format = parser.read<UInt8>("Format");
        parser.read<OpenTypeCoverage>("Coverage");

        switch (format) {
        case 0: parseFormat0(parser); break;
        case 2: parseFormat2(subtableStart, parser); break;
        default: throw QString("%1 is not a valid format").arg(format);
        }

        parser.endGroup("", QString("Format %1").arg(format));
    });
}

// https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6kern.html
static void parseKernApple(Parser &parser)
{
    parser.read<F16DOT16>("Version");
    const auto numberOfTables = parser.read<UInt32>("Number of tables");
    parser.readArray("Subtables", numberOfTables, [&](const auto index){
        const auto subtableStart = parser.offset();

        parser.beginGroup(index);
        const auto length = parser.read<UInt32>("Length");
        // Yes, the coverage and format order is inverted in AAT.
        parser.read<AppleCoverage>("Coverage");
        const auto format = parser.read<UInt8>("Format");
        parser.read<UInt16>("Tuple index");

        switch (format) {
        case 0: parseFormat0(parser); break;
        case 1: Format1::parse(length, parser); break;
        case 2: parseFormat2(subtableStart, parser); break;
        case 3: parseFormat3(subtableStart, length, parser); break;
        default: throw QString("%1 is not a valid format").arg(format);
        }

        parser.endGroup("", QString("Format %1").arg(format));
    });
}

void parseKern(Parser &parser)
{
    // The `kern` table has two variants: OpenType one and Apple one.
    // And they both have different headers.
    // There are no robust way to distinguish them, so we have to guess.
    //
    // The OpenType one has the first two bytes (UInt16) as a version set to 0.
    // While Apple one has the first four bytes (Fixed) set to 1.0
    // So the first two bytes in case of an OpenType format will be 0x0000
    // and 0x0001 in case of an Apple format.
    const auto version = parser.peek<UInt16>();
    if (version == 0) {
        parseKernOpenType(parser);
    } else {
        parseKernApple(parser);
    }
}
