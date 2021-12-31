#include <bitset>

#include "tables.h"

struct Flags
{
    static const int Size = 1;
    static const QString Type;

    static Flags parse(const uint8_t *data)
    { return { data[0] }; }

    static QString toString(const Flags &value)
    {
        std::bitset<8> bits(value.d);
        auto flagsStr = QString::fromUtf8(bits.to_string().c_str()) + '\n';

        if (bits[6]) flagsStr += "Bit 6: Next byte is the default setting index\n";
        if (bits[7]) flagsStr += "Bit 7: Exclusive settings\n";

        flagsStr.chop(1); // trim trailing newline

        return flagsStr;
    }

    DEFAULT_DEBUG(Flags)

    quint8 d;
};

const QString Flags::Type = Parser::BitflagsType;


void parseFeat(const NamesHash &names, Parser &parser)
{
    parser.read<F16DOT16>("Version");
    const auto numberOfFeatures = parser.read<UInt16>("Number of features");
    parser.read<UInt16>("Reserved");
    parser.read<UInt32>("Reserved");

    uint numberOfSettings = 0;
    parser.readArray("Feature Name Array", numberOfFeatures, [&](const auto index){
        parser.beginGroup(index);
        parser.read<UInt16>("Type");
        numberOfSettings += parser.read<UInt16>("Number of settings");
        parser.read<Offset32>("Offset to setting name array");
        parser.read<Flags>("Flags");
        parser.read<UInt8>("Default setting index");
        const auto name = parser.readNameId("Name ID", names);
        parser.endGroup(QString(), name);
    });

    parser.readArray("Setting Name Array", numberOfSettings, [&](const auto index){
        parser.beginGroup(index);
        parser.read<UInt16>("Setting");
        const auto name = parser.readNameId("Name ID", names);
        parser.endGroup(QString(), name);
    });
}
