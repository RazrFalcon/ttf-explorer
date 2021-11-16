#include <bitset>

#include "src/algo.h"
#include "src/parser.h"
#include "tables.h"

struct SbixFlags
{
    static const int Size = 2;
    static const QString Type;

    static SbixFlags parse(const quint8 *data)
    {
        return { qFromBigEndian<quint16>(data) };
    }

    static QString toString(const SbixFlags &value)
    {
        std::bitset<16> bits(value.d);
        auto flagsStr = QString::fromUtf8(bits.to_string().c_str()) + '\n';

        if (bits[1]) flagsStr += "Bit 1: Draw outlines\n";

        flagsStr.chop(1); // trim trailing newline
        return flagsStr;
    }

    quint16 d;
};

const QString SbixFlags::Type = "BitFlags";

void parseSbix(const quint16 numberOfGlyphs, Parser &parser)
{
    const auto start = parser.offset();

    const auto version = parser.read<UInt16>("Version");
    if (version != 1) {
        throw "invalid table version";
    }

    parser.read<SbixFlags>("Flags");
    const auto numStrikes = parser.read<UInt32>("Number of bitmap strikes");

    QVector<quint32> offsets;
    parser.beginGroup("Offsets", QString::number(numStrikes));
    for (quint32 i = 0; i < numStrikes; ++i) {
        offsets << parser.read<Offset32>("Offset " + QString::number(i));
    }
    parser.endGroup();

    algo::sort_all(offsets);
    algo::dedup_vector(offsets);

    QVector<quint32> glyphOffsets;

    for (const auto offset : offsets) {
        glyphOffsets.clear();

        parser.jumpTo(start + offset);
        parser.beginGroup("Strike");

        parser.read<UInt16>("PPEM");
        parser.read<UInt16>("PPI");

        parser.beginGroup("Offsets", QString::number(numberOfGlyphs));
        for (quint32 i = 0; i <= numberOfGlyphs; ++i) {
            glyphOffsets << parser.read<Offset32>("Offset " + QString::number(i));
        }
        parser.endGroup();

        algo::sort_all(glyphOffsets);
        algo::dedup_vector(glyphOffsets);

        // The last offset is the end byte of the last glyph.
        for (int i = 0; i < glyphOffsets.size() - 1; ++i) {
            const auto dataSize = glyphOffsets.at(i + 1) - glyphOffsets.at(i);

            parser.beginGroup("Glyph data");
            parser.jumpTo(start + offset + glyphOffsets.at(i));
            parser.read<Int16>("Horizontal offset");
            parser.read<Int16>("Vertical offset");
            parser.read<Tag>("Type");
            parser.readBytes(dataSize - 8, "Data");
            parser.endGroup();
        }

        parser.endGroup();
    }
}
