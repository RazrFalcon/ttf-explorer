#include "tables.h"

void parsePost(Parser &parser)
{
    const auto version = parser.read<F16DOT16>("Version");
    parser.read<F16DOT16>("Italic angle");
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

    const auto numberOfGlyphs = parser.read<UInt16>("Number of glyphs");
    int numberOfNames2 = 0;
    parser.readArray("Glyph Name Indexes", numberOfGlyphs, [&](const auto index){
        const auto n = parser.read<UInt16>(index);
        if (n > 257) {
            numberOfNames2 += 1;
        }
    });

    parser.readArray("Names", numberOfNames2, [&](const auto index){
        parser.readPascalString(numberToString(index));
    });
}
