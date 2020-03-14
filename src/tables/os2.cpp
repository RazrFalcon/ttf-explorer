#include <bitset>

#include "src/parser.h"
#include "tables.h"

struct WeightClass
{
    static const int Size = 2;
    static const QString Type;

    static WeightClass parse(const quint8 *data)
    { return { qFromBigEndian<quint16>(data) }; }

    static QString toString(const WeightClass &value)
    {
        QString name;
        switch (value.d) {
            case 100 : name = QLatin1String("Thin"); break;
            case 200 : name = QLatin1String("Extra-light"); break;
            case 300 : name = QLatin1String("Light"); break;
            case 400 : name = QLatin1String("Normal"); break;
            case 500 : name = QLatin1String("Medium"); break;
            case 600 : name = QLatin1String("Semi-bold"); break;
            case 700 : name = QLatin1String("Bold"); break;
            case 800 : name = QLatin1String("Extra-bold"); break;
            case 900 : name = QLatin1String("Black"); break;
            default  : name = QLatin1String("Other"); break;
        }

        name += QString(" (%1)").arg(value.d);

        return name;
    }

    quint16 d;
};

const QString WeightClass::Type = "UInt16";


struct WidthClass
{
    static const int Size = 2;
    static const QString Type;

    static WidthClass parse(const quint8 *data)
    { return { qFromBigEndian<quint16>(data) }; }

    static QString toString(const WidthClass &value)
    {
        QString name;
        switch (value.d) {
            case 1  : name = QLatin1String("Ultra-condensed"); break;
            case 2  : name = QLatin1String("Extra-condensed"); break;
            case 3  : name = QLatin1String("Condensed"); break;
            case 4  : name = QLatin1String("Semi-condensed"); break;
            case 5  : name = QLatin1String("Normal"); break;
            case 6  : name = QLatin1String("Semi-expanded"); break;
            case 7  : name = QLatin1String("Expanded"); break;
            case 8  : name = QLatin1String("Extra-expanded"); break;
            case 9  : name = QLatin1String("Ultra-expanded"); break;
            default : name = QLatin1String("Invalid"); break;
        }

        name += QString(" (%1)").arg(value.d);

        return name;
    }

    quint16 d;
};

const QString WidthClass::Type = "UInt16";


struct TypeFlags
{
    static const int Size = 2;
    static const QString Type;

    static TypeFlags parse(const quint8 *data)
    {
        return { qFromBigEndian<quint16>(data) };
    }

    static QString toString(const TypeFlags &value)
    {
        std::bitset<16> bits(value.d);
        auto flagsStr = QString::fromUtf8(bits.to_string().c_str()) + '\n';

        flagsStr += "Bits 0-3: Usage permissions: ";
        const auto permissions = value.d & 0x000F;
        switch (permissions) {
            case 0  : flagsStr += "Installable"; break;
            case 2  : flagsStr += "Restricted License"; break;
            case 4  : flagsStr += "Preview & Print"; break;
            case 8  : flagsStr += "Editable"; break;
            default : flagsStr += "Invalid"; break;
        }
        flagsStr += '\n';

        // 4–7 - reserved
        if (bits[8]) flagsStr += "Bit 8: No subsetting\n";
        if (bits[9]) flagsStr += "Bit 9: Bitmap embedding only\n";
        // 10–15 - reserved

        flagsStr.chop(1); // trim trailing newline

        return flagsStr;
    }

    quint16 d;
};

const QString TypeFlags::Type = "BitFlags";


struct FontSelectionFlags
{
    static const int Size = 2;
    static const QString Type;

    static FontSelectionFlags parse(const quint8 *data)
    {
        return { qFromBigEndian<quint16>(data) };
    }

    static QString toString(const FontSelectionFlags &value)
    {
        std::bitset<16> bits(value.d);
        auto flagsStr = QString::fromUtf8(bits.to_string().c_str()) + '\n';

        if (bits[0])  flagsStr += "Bit 0: Italic\n";
        if (bits[1])  flagsStr += "Bit 1: Underscored\n";
        if (bits[2])  flagsStr += "Bit 2: Negative\n";
        if (bits[3])  flagsStr += "Bit 3: Outlined\n";
        if (bits[4])  flagsStr += "Bit 4: Overstruck\n";
        if (bits[5])  flagsStr += "Bit 5: Bold\n";
        if (bits[6])  flagsStr += "Bit 6: Regular\n";
        if (bits[7])  flagsStr += "Bit 7: Use typographic metrics\n";
        if (bits[8])  flagsStr += "Bit 8: WWS\n";
        if (bits[9])  flagsStr += "Bit 9: Oblique\n";
        // 10–15 - reserved

        flagsStr.chop(1); // trim trailing newline

        return flagsStr;
    }

    quint16 d;
};

const QString FontSelectionFlags::Type = "BitFlags";


void parseOS2(Parser &parser)
{
    const auto version = parser.read<UInt16>("Version");

    parser.read<Int16>("Average weighted escapement");
    parser.read<WeightClass>("Weight class");
    parser.read<WidthClass>("Width class");
    parser.read<TypeFlags>("Type flags");
    parser.read<Int16>("Subscript horizontal font size");
    parser.read<Int16>("Subscript vertical font size");
    parser.read<Int16>("Subscript X offset");
    parser.read<Int16>("Subscript Y offset.");
    parser.read<Int16>("Superscript horizontal font size");
    parser.read<Int16>("Superscript vertical font size");
    parser.read<Int16>("Superscript X offset");
    parser.read<Int16>("Superscript Y offset");
    parser.read<Int16>("Strikeout size");
    parser.read<Int16>("Strikeout position");
    parser.read<Int16>("Font-family class");

    parser.beginGroup("panose");
    parser.read<UInt8>("Family type");
    parser.read<UInt8>("Serif style");
    parser.read<UInt8>("Weight");
    parser.read<UInt8>("Proportion");
    parser.read<UInt8>("Contrast");
    parser.read<UInt8>("Stroke variation");
    parser.read<UInt8>("Arm style");
    parser.read<UInt8>("Letterform");
    parser.read<UInt8>("Midline");
    parser.read<UInt8>("x height");
    parser.endGroup();

    parser.read<UInt32>("Unicode Character Range 1"); // TODO
    parser.read<UInt32>("Unicode Character Range 2");
    parser.read<UInt32>("Unicode Character Range 3");
    parser.read<UInt32>("Unicode Character Range 4");
    parser.read<Tag>("Font Vendor Identification");
    parser.read<FontSelectionFlags>("Font selection flags");
    parser.read<UInt16>("The minimum Unicode index");
    parser.read<UInt16>("The maximum Unicode index");
    parser.read<Int16>("Typographic ascender");
    parser.read<Int16>("Typographic descender");
    parser.read<Int16>("Typographic line gap");
    parser.read<UInt16>("Windows ascender");
    parser.read<UInt16>("Windows descender");

    if (version == 0) {
        return;
    }

    parser.read<UInt32>("Code Page Character Range 1"); // TODO
    parser.read<UInt32>("Code Page Character Range 2");

    if (version < 2) {
        return;
    }

    parser.read<Int16>("x height");
    parser.read<Int16>("Capital height");
    parser.read<UInt16>("Default character");
    parser.read<UInt16>("Break character");
    parser.read<UInt16>("The maximum glyph context");

    if (version < 5) {
        return;
    }

    parser.read<UInt16>("Lower optical point size");
    parser.read<UInt16>("Upper optical point size");
}
