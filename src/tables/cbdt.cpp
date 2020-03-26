#include "src/algo.h"
#include "src/parser.h"
#include "tables.h"

void parseCbdt(const QVector<CblcIndex> &cblcLocations, Parser &parser)
{
    const auto start = parser.offset();

    const auto majorVersion = parser.read<UInt16>("Major version");
    const auto minorVersion = parser.read<UInt16>("Minor version");
    // Some old Noto Emoji fonts have a 2.0 version.
    if (!((majorVersion == 2 || majorVersion == 3) && minorVersion == 0)) {
        throw "invalid table version";
    }

    for (const auto &loca : cblcLocations) {
        parser.jumpTo(start + loca.range.start);
        parser.beginGroup(QString("Bitmap Format %1").arg(loca.imageFormat));

        if (loca.imageFormat == 17) {
            parser.read<UInt8>("Height");
            parser.read<UInt8>("Width");
            parser.read<Int8>("X-axis bearing");
            parser.read<Int8>("Y-axis bearing");
            parser.read<UInt8>("Advance");
            const auto len = parser.read<UInt32>("Length of data");
            parser.readBytes(len, "Raw PNG data");
        } else if (loca.imageFormat == 18) {
            parser.read<UInt8>("Height");
            parser.read<UInt8>("Width");
            parser.read<Int8>("Horizontal X-axis bearing");
            parser.read<Int8>("Horizontal Y-axis bearing");
            parser.read<UInt8>("Horizontal advance");
            parser.read<Int8>("Vertical X-axis bearing");
            parser.read<Int8>("Vertical Y-axis bearing");
            parser.read<UInt8>("Vertical advance");
            const auto len = parser.read<UInt32>("Length of data");
            parser.readBytes(len, "Raw PNG data");
        } else if (loca.imageFormat == 19) {
            const auto len = parser.read<UInt32>("Length of data");
            parser.readBytes(len, "Raw PNG data");
        }

        parser.endGroup();
    }
}
