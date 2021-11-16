#include <bitset>

#include "tables.h"

struct HeadFlags
{
    static const int Size = 2;
    static const QString Type;

    static HeadFlags parse(const quint8 *data)
    { return { qFromBigEndian<quint16>(data) }; }

    static QString toString(const HeadFlags &value)
    {
        std::bitset<16> bits(value.d);
        auto flagsStr = QString::fromUtf8(bits.to_string().c_str()) + '\n';

        if (bits[0])  flagsStr += "Bit 0: Baseline for font at y=0\n";
        if (bits[1])  flagsStr += "Bit 1: Left sidebearing point at x=0\n";
        if (bits[2])  flagsStr += "Bit 2: Instructions may depend on point size\n";
        if (bits[3])  flagsStr += "Bit 3: Force ppem to integer values\n";
        if (bits[4])  flagsStr += "Bit 4: Instructions may alter advance width\n";
        if (bits[5])  flagsStr += "Bit 5: (AAT only) Vertical layout\n";
        // 6 - reserved
        if (bits[7])  flagsStr += "Bit 7: (AAT only) Requires linguistic rendering\n";
        if (bits[8])  flagsStr += "Bit 8: (AAT only) Has metamorphosis effects\n";
        if (bits[9])  flagsStr += "Bit 9: (AAT only) Font contains strong right-to-left glyphs\n";
        if (bits[10]) flagsStr += "Bit 10: (AAT only) Font contains Indic-style rearrangement effects\n";
        if (bits[11]) flagsStr += "Bit 11: Font data is “lossless”\n";
        if (bits[12]) flagsStr += "Bit 12: Font converted\n";
        if (bits[13]) flagsStr += "Bit 13: Font optimized for ClearType\n";
        if (bits[14]) flagsStr += "Bit 14: Last Resort font\n";
        // 15 - reserved

        flagsStr.chop(1); // trim trailing newline

        return flagsStr;
    }

    quint16 d;
};

const QString HeadFlags::Type = Parser::BitflagsType;


struct MacStyleFlags
{
    static const int Size = 2;
    static const QString Type;

    static MacStyleFlags parse(const quint8 *data)
    { return { qFromBigEndian<quint16>(data) }; }

    static QString toString(const MacStyleFlags &value)
    {
        std::bitset<16> bits(value.d);
        auto flagsStr = QString::fromUtf8(bits.to_string().c_str()) + '\n';

        if (bits[0])  flagsStr += "Bit 0: Bold\n";
        if (bits[1])  flagsStr += "Bit 1: Italic\n";
        if (bits[2])  flagsStr += "Bit 2: Underline\n";
        if (bits[3])  flagsStr += "Bit 3: Outline\n";
        if (bits[4])  flagsStr += "Bit 4: Shadow\n";
        if (bits[5])  flagsStr += "Bit 5: Condensed\n";
        if (bits[6])  flagsStr += "Bit 6: Extended\n";
        // 7-15 - reserved

        flagsStr.chop(1); // trim trailing newline

        return flagsStr;
    }

    quint16 d;
};

const QString MacStyleFlags::Type = Parser::BitflagsType;


void parseHead(Parser &parser)
{
    const auto majorVersion = parser.read<UInt16>("Major version");
    const auto minorVersion = parser.read<UInt16>("Minor version");
    if (!(majorVersion == 1 && minorVersion == 0)) {
        throw QString("invalid table version");
    }

    parser.read<F16DOT16>("Font revision");
    parser.read<UInt32>("Checksum adjustment");
    parser.read<UInt32>("Magic number");
    parser.read<HeadFlags>("Flags");
    parser.read<UInt16>("Units per EM");
    parser.read<LongDateTime>("Created");
    parser.read<LongDateTime>("Modified");
    parser.read<Int16>("X min for all glyph bounding boxes");
    parser.read<Int16>("Y min for all glyph bounding boxes");
    parser.read<Int16>("X max for all glyph bounding boxes");
    parser.read<Int16>("Y max for all glyph bounding boxes");
    parser.read<MacStyleFlags>("Mac style");
    parser.read<UInt16>("Smallest readable size in pixels");
    parser.read<Int16>("Font direction hint");
    parser.read<Int16>("Index to location format");
    parser.read<Int16>("Glyph data format");
}
