#include <bitset>

#include "src/algo.h"
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

const QString SbixFlags::Type = Parser::BitflagsType;


void parseSbix(const quint16 numberOfGlyphs, Parser &parser)
{
    const auto start = parser.offset();

    const auto version = parser.read<UInt16>("Version");
    if (version != 1) {
        throw QString("invalid table version");
    }

    parser.read<SbixFlags>("Flags");
    const auto numStrikes = parser.read<UInt32>("Number of bitmap strikes");

    QVector<quint32> offsets;
    parser.readArray("Offsets", numStrikes, [&](const auto index){
        offsets << parser.read<Offset32>(index);
    });

    algo::sort_all(offsets);
    algo::dedup_vector(offsets);

    QVector<quint32> glyphOffsets;
    parser.readArray("Strikes", offsets.size(), [&](const auto index){
        glyphOffsets.clear();

        const auto offset = offsets[index];
        parser.advanceTo(start + offset);
        parser.beginGroup(index);

        parser.read<UInt16>("PPEM");
        parser.read<UInt16>("PPI");

        QVector<quint32> offsets;
        parser.readArray("Offsets", numberOfGlyphs + 1, [&](const auto index){
            glyphOffsets << parser.read<Offset32>(index);
        });

        algo::sort_all(glyphOffsets);
        algo::dedup_vector(glyphOffsets);

        // The last offset is the end byte of the last glyph.
        parser.readArray("Glyphs", glyphOffsets.size() - 1, [&](const auto index){
            const auto dataSize = glyphOffsets.at(index + 1) - glyphOffsets.at(index);

            parser.beginGroup(index);
            parser.advanceTo(start + offset + glyphOffsets.at(index));
            parser.read<Int16>("Horizontal offset");
            parser.read<Int16>("Vertical offset");
            parser.read<Tag>("Type");
            parser.readBytes("Data", dataSize - 8);
            parser.endGroup();
        });

        parser.endGroup();
    });
}
