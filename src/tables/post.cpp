#include "src/parser.h"
#include "tables.h"

void parsePost(const quint32 end, Parser &parser)
{
    const auto version = parser.read<Fixed>("Version");
    parser.read<Fixed>("Italic angle");
    parser.read<Int16>("Underline position");
    parser.read<Int16>("Underline thickness");
    parser.read<UInt32>("Is fixed pitch");
    parser.read<UInt32>("Min memory when font is downloaded");
    parser.read<UInt32>("Max memory when font is downloaded");
    parser.read<UInt32>("Min memory when font is downloaded as a Type 1");
    parser.read<UInt32>("Max memory when font is downloaded as a Type 1");

    if (version != 2.0f) {
        return;
    }

    const auto numGlyphs = parser.read<UInt16>("Number of glyphs");
    if (numGlyphs != 0) {
        parser.beginGroup("Glyph name indexes");

        for (auto i = 0; i < numGlyphs; ++i) {
            parser.read<UInt16>("Index");
        }

        parser.endGroup();
    }

    while (parser.offset() < end) {
        parser.beginGroup("");
        const auto len = parser.read<UInt8>("Length");
        if (len != 0) {
            const auto name = parser.readString(len, "Data");
            parser.endGroup(name);
        } else {
            parser.endGroup();
        }
    }
}
