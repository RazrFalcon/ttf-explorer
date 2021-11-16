#include <bitset>

#include <QVarLengthArray>

#include "src/algo.h"

#include "tables.h"

struct NegativeUInt8
{
    static const int Size = 1;
    static const QString Type;

    static NegativeUInt8 parse(const uint8_t *data)
    { return { data[0] }; }

    static QString toString(const NegativeUInt8 &value)
    { return numberToString(-qint16(value.d)); }

    DEFAULT_DEBUG(NegativeUInt8)

    operator uint8_t() const { return d; }

    uint8_t d;
};

const QString NegativeUInt8::Type = UInt8::Type;


struct SimpleGlyphFlags
{
    static const int Size = 1;
    static const QString Type;

    enum Flag : quint8
    {
        ON_CURVE_POINT = 0x01,
        X_SHORT_VECTOR = 0x02,
        Y_SHORT_VECTOR = 0x04,
        REPEAT_FLAG = 0x08,
        X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR = 0x10,
        Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR = 0x20,
        OVERLAP_SIMPLE = 0x40,
    };

    Q_DECLARE_FLAGS(Flags, Flag)

    static SimpleGlyphFlags parse(const quint8 *data)
    {
        return { Flag(data[0]) };
    }

    static QString toString(const SimpleGlyphFlags &value)
    {
        // This method would be called a lot, therefore we have to use a cache.
        //
        // Valid flags would be in a 0..64 range, so using a QVarLengthArray
        // is more performant than QHash.
        static QVarLengthArray<QString, 64> cache;

        if (cache.isEmpty()) {
            cache.resize(64);
        }

        if (value.d < cache.size() && !cache[value.d].isEmpty()) {
            return cache[value.d];
        }

        std::bitset<8> bits(value.d);
        auto flagsStr = QString::fromUtf8(bits.to_string().c_str()) + '\n';

        if (bits[0]) flagsStr += "Bit 0: On curve point\n";
        if (bits[1]) flagsStr += "Bit 1: X-coordinate is 1 byte long\n";
        if (bits[2]) flagsStr += "Bit 2: Y-coordinate is 1 byte long\n";
        if (bits[3]) flagsStr += "Bit 3: Repeat flag\n";

        if (bits[1] && bits[4]) flagsStr += "Bit 4: X-coordinate is positive\n";
        if (bits[1] && !bits[4]) flagsStr += "Bit 4: X-coordinate is negative\n";
        if (!bits[1] && bits[4]) flagsStr += "Bit 4: Use the previous X-coordinate\n";
        if (!bits[1] && !bits[4]) flagsStr += "Bit 4: X-coordinate is 2 byte long, signed\n";

        if (bits[2] && bits[5]) flagsStr += "Bit 5: Y-coordinate is positive\n";
        if (bits[2] && !bits[5]) flagsStr += "Bit 5: Y-coordinate is negative\n";
        if (!bits[2] && bits[5]) flagsStr += "Bit 5: Use the previous Y-coordinate\n";
        if (!bits[2] && !bits[5]) flagsStr += "Bit 5: Y-coordinate is 2 byte long, signed\n";

        if (bits[6]) flagsStr += "Bit 6: Contours may overlap\n";
        // 7 - reserved

        flagsStr.chop(1); // trim trailing newline

        if (value.d < cache.size()) {
            cache[value.d] = flagsStr;
        }

        return flagsStr;
    }

    operator Flag() const { return d; }

    Flag d;
};

const QString SimpleGlyphFlags::Type = Parser::BitflagsType;


struct CompositeGlyphFlags
{
    static const int Size = 2;
    static const QString Type;

    enum Flag : quint16
    {
        ARG_1_AND_2_ARE_WORDS = 0x0001,
        ARGS_ARE_XY_VALUES = 0x0002,
        ROUND_XY_TO_GRID = 0x0004,
        WE_HAVE_A_SCALE = 0x0008,
        MORE_COMPONENTS = 0x0020,
        WE_HAVE_AN_X_AND_Y_SCALE = 0x0040,
        WE_HAVE_A_TWO_BY_TWO = 0x0080,
        WE_HAVE_INSTRUCTIONS = 0x0100,
        USE_MY_METRICS = 0x0200,
        OVERLAP_COMPOUND = 0x0400,
        SCALED_COMPONENT_OFFSET = 0x0800,
        UNSCALED_COMPONENT_OFFSET = 0x1000,
   };

    Q_DECLARE_FLAGS(Flags, Flag)

    static CompositeGlyphFlags parse(const quint8 *data)
    {
        return { Flag(quint16(UInt16::parse(data))) };
    }

    static QString toString(const CompositeGlyphFlags &value)
    {
        std::bitset<16> bits(value.d);
        auto flagsStr = QString::fromUtf8(bits.to_string().c_str()) + '\n';

        if (bits[0]) flagsStr += "Bit 0: Arguments are 16-bit\n";
        if (bits[1]) flagsStr += "Bit 1: Arguments are signed xy values\n";
        if (bits[2]) flagsStr += "Bit 2: Round XY to grid\n";
        if (bits[3]) flagsStr += "Bit 3: Has a simple scale\n";
        // 4 - reserved
        if (bits[5]) flagsStr += "Bit 5: Has more glyphs\n";
        if (bits[6]) flagsStr += "Bit 6: Non-propotional scale\n";
        if (bits[7]) flagsStr += "Bit 7: Has 2 by 2 transformation matrix\n";
        if (bits[8]) flagsStr += "Bit 8: Has instructions after the last component\n";
        if (bits[9]) flagsStr += "Bit 9: Use my metrics\n";
        if (bits[10]) flagsStr += "Bit 10: Components overlap\n";
        if (bits[11]) flagsStr += "Bit 11: Scaled component offset\n";
        if (bits[12]) flagsStr += "Bit 12: Unscaled component offset\n";
        // 13, 14, 15 - reserved

        flagsStr.chop(1); // trim trailing newline

        return flagsStr;
    }

    operator Flag() const { return d; }

    Flag d;
};

const QString CompositeGlyphFlags::Type = Parser::BitflagsType;


static void parseSimpleGlyph(const quint16 numOfContours, Parser &parser)
{
    quint16 lastPoint = 0;
    parser.readArray("Endpoints", numOfContours, [&](const auto index){
        lastPoint = parser.read<UInt16>(index);
    });

    const auto instructionLength = parser.read<UInt16>("Instructions size");
    parser.readBytes("Instructions", instructionLength);

    parser.beginGroup("Flags");
    QVarLengthArray<SimpleGlyphFlags> allFlags;
    const auto totalPoints = lastPoint + 1;
    auto pointsLeft = totalPoints;
    while (pointsLeft > 0) {
        const auto flags = parser.read<SimpleGlyphFlags>("Flag");
        allFlags << flags;

        auto repeats = 1;
        if (flags & SimpleGlyphFlags::REPEAT_FLAG) {
            repeats = parser.read<UInt8>("Number of repeats");
            for (int i = 0; i < repeats; ++i) {
                allFlags << flags;
            }

            repeats += 1;
        }

        pointsLeft -= repeats;
    }
    parser.endGroup();

    {
        // The number of coordinates can be lower than the number of flags.
        int xCoords = 0;
        for (const auto flags : allFlags) {
            if (flags & SimpleGlyphFlags::X_SHORT_VECTOR) {
                if (flags & SimpleGlyphFlags::X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR) {
                    xCoords += 1;
                } else {
                    xCoords += 1;
                }
            } else {
                if (flags & SimpleGlyphFlags::X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR) {
                    // Nothing.
                } else {
                    xCoords += 1;
                }
            }
        }

        parser.beginArray("X-coordinates", xCoords);
        for (auto [index, flags] : algo::enumerate(allFlags)) {
            if (*flags & SimpleGlyphFlags::X_SHORT_VECTOR) {
                if (*flags & SimpleGlyphFlags::X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR) {
                    parser.read<UInt8>(index);
                } else {
                    parser.read<NegativeUInt8>(index);
                }
            } else {
                if (*flags & SimpleGlyphFlags::X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR) {
                    // Nothing.
                } else {
                    parser.read<Int16>(index);
                }
            }
        }
        parser.endArray();
    }

    {
        // The number of coordinates can be lower than the number of flags.
        int yCoords = 0;
        for (const auto flags : allFlags) {
            if (flags & SimpleGlyphFlags::Y_SHORT_VECTOR) {
                if (flags & SimpleGlyphFlags::Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR) {
                    yCoords += 1;
                } else {
                    yCoords += 1;
                }
            } else {
                if (flags & SimpleGlyphFlags::Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR) {
                    // Nothing.
                } else {
                    yCoords += 1;
                }
            }
        }

        parser.beginArray("Y-coordinates", yCoords);
        for (auto [index, flags] : algo::enumerate(allFlags)) {
            if (*flags & SimpleGlyphFlags::Y_SHORT_VECTOR) {
                if (*flags & SimpleGlyphFlags::Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR) {
                    parser.read<UInt8>(index);
                } else {
                    parser.read<NegativeUInt8>(index);
                }
            } else {
                if (*flags & SimpleGlyphFlags::Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR) {
                    // Nothing.
                } else {
                    parser.read<Int16>(index);
                }
            }
        }
        parser.endArray();
    }
}

static void parseCompositeGlyph(Parser &parser)
{
    const auto flags = parser.read<CompositeGlyphFlags>("Flag");
    parser.read<GlyphId>("Glyph ID");

    std::array<double, 6> matrix = {0};
    bool hasTs = false;

    if (flags & CompositeGlyphFlags::ARGS_ARE_XY_VALUES) {
        parser.beginGroup();
        hasTs = true;
        if (flags & CompositeGlyphFlags::ARG_1_AND_2_ARE_WORDS) {
            matrix[4] = parser.read<Int16>("E");
            matrix[5] = parser.read<Int16>("F");
        } else {
            matrix[4] = parser.read<Int8>("E");
            matrix[5] = parser.read<Int8>("F");
        }
    } else {
        if (flags & CompositeGlyphFlags::ARG_1_AND_2_ARE_WORDS) {
            parser.read<UInt16>("Point 1");
            parser.read<UInt16>("Point 2");
        } else {
            parser.read<UInt8>("Point 1");
            parser.read<UInt8>("Point 2");
        }
    }

    if (flags & CompositeGlyphFlags::WE_HAVE_A_TWO_BY_TWO) {
        if (!hasTs) {
            parser.beginGroup();
        }
        hasTs = true;

        matrix[0] = parser.read<F2DOT14>("A");
        matrix[1] = parser.read<F2DOT14>("B");
        matrix[2] = parser.read<F2DOT14>("C");
        matrix[3] = parser.read<F2DOT14>("D");
    } else if (flags & CompositeGlyphFlags::WE_HAVE_AN_X_AND_Y_SCALE) {
        if (!hasTs) {
            parser.beginGroup();
        }
        hasTs = true;

        matrix[0] = parser.read<F2DOT14>("A");
        matrix[3] = parser.read<F2DOT14>("D");
    } else if (flags & CompositeGlyphFlags::WE_HAVE_A_SCALE) {
        if (!hasTs) {
            parser.beginGroup();
        }
        hasTs = true;

        matrix[0] = parser.read<F2DOT14>("A");
        matrix[3] = matrix[0];
    }

    if (hasTs) {
        const auto matrixStr = QString("%1 %2 %3 %4 %5 %6")
            .arg(floatToString(matrix.at(0)))
            .arg(floatToString(matrix.at(1)))
            .arg(floatToString(matrix.at(2)))
            .arg(floatToString(matrix.at(3)))
            .arg(floatToString(matrix.at(4)))
            .arg(floatToString(matrix.at(5)));
        parser.endGroup("Matrix", matrixStr);
    }

    if (flags & CompositeGlyphFlags::MORE_COMPONENTS) {
        parseCompositeGlyph(parser);
    } else if (flags & CompositeGlyphFlags::WE_HAVE_INSTRUCTIONS) {
         const auto size = parser.read<UInt16>("Number of instructions");
         parser.readBytes("Instructions", size);
    }
}

void parseGlyf(const quint16 numberOfGlyphs, const QVector<quint32> &glyphOffsets, Parser &parser)
{
    Q_ASSERT(int(numberOfGlyphs) + 1 == glyphOffsets.size());

    const auto tableStart = parser.offset();

    // Glyphs can be empty, therefore the real number of glyphs can be lower than numberOfGlyphs.
    int glyphsCount = 0;
    for (quint16 i = 0; i < numberOfGlyphs; i++) {
        if (glyphOffsets[i] != glyphOffsets[i + 1]) {
            glyphsCount += 1;
        }
    }

    parser.beginArray("Glyphs", glyphsCount);
    for (quint16 index = 0; index < numberOfGlyphs; index++) {
        const auto start = tableStart + glyphOffsets[index];
        const auto end = tableStart + glyphOffsets[index + 1];
        if (start == end) {
            continue;
        }

        parser.beginGroup();

        const auto numberOfContours = parser.read<Int16>("Number of contours");
        parser.read<Int16>("x min");
        parser.read<Int16>("y min");
        parser.read<Int16>("x max");
        parser.read<Int16>("y max");

        if (numberOfContours == 0) {
            // Empty.
        } else if (numberOfContours > 0) {
            parseSimpleGlyph(quint16(numberOfContours), parser);
        } else {
            parseCompositeGlyph(parser);
        }

        if (parser.offset() < end) {
            const auto diff = end - parser.offset();
            if (diff < 4) {
                parser.readPadding(diff);
            } else {
                parser.readUnsupported(diff);
            }
        }

        if (numberOfContours == 0) {
            parser.endGroup(QString("Glyph %1 (empty)").arg(index));
        } else if (numberOfContours > 0) {
            parser.endGroup(QString("Glyph %1").arg(index));
        } else {
            parser.endGroup(QString("Glyph %1 (composite)").arg(index));
        }
    }
    parser.endArray();
}
