#include "tables.h"

void parseMaxp(Parser &parser)
{
    const auto version = parser.read<F16DOT16>("Version");
    parser.read<UInt16>("Number of glyphs");

    if (version == 0.3125f) { // v0.5
        return;
    }

    if (version != 1.0f) {
        throw QString("invalid table version");
    }

    parser.read<UInt16>("Maximum points in a non-composite glyph");
    parser.read<UInt16>("Maximum contours in a non-composite glyph");
    parser.read<UInt16>("Maximum points in a composite glyph");
    parser.read<UInt16>("Maximum contours in a composite glyph");
    parser.read<UInt16>("Maximum zones");
    parser.read<UInt16>("Maximum twilight points");
    parser.read<UInt16>("Number of Storage Area locations");
    parser.read<UInt16>("Number of FDEFs");
    parser.read<UInt16>("Number of IDEFs");
    parser.read<UInt16>("Maximum stack depth");
    parser.read<UInt16>("Maximum byte count for glyph instructions");
    parser.read<UInt16>("Maximum number of components");
    parser.read<UInt16>("Maximum levels of recursion");
}
