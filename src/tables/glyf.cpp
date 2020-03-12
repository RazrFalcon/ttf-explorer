#include <bitset>

#include <QFlag>

#include "src/parser.h"
#include "tables.h"

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

        return flagsStr;
    }

    operator Flag() const { return d; }

    Flag d;
};

const QString SimpleGlyphFlags::Type = "BitFlags";


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

const QString CompositeGlyphFlags::Type = "BitFlags";


static QVector<quint32> collectGlyphSizes(const quint16 numberOfGlyphs,
                                          const IndexToLocFormat format,
                                          const gsl::span<const quint8> locaData)
{
    quint32 lastOffset = 0;
    QVector<quint32> glyphSizes;
    ShadowParser locaParser(locaData);
    for (int i = 0; i <= numberOfGlyphs; ++i) {
        quint32 offset = 0;
        if (format == IndexToLocFormat::Short) {
            offset = locaParser.read<Offset16>() * 2;
        } else {
            offset = locaParser.read<Offset32>();
        }

        if (offset < lastOffset) {
            throw "invalid offset";
        } else {
            // Can be zero.
            glyphSizes << offset - lastOffset;
        }

        lastOffset = offset;
    }

    glyphSizes.remove(0);

    return glyphSizes;
}

static void parseSimpleGlyph(const quint16 numOfContours, Parser &parser)
{
    quint16 lastPoint = 0;
    parser.beginGroup("Endpoints");
    for (uint i = 0; i < numOfContours; ++i) {
        lastPoint = parser.read<UInt16>(QString("Endpoint %1").arg(i));
    }
    parser.endGroup();

    const auto instructionLength = parser.read<UInt16>("Instructions size");
    if (instructionLength > 0) {
        parser.readBytes(instructionLength, "Instructions");
    }

    parser.beginGroup("Flags");
    QVector<SimpleGlyphFlags> allFlags;
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

    parser.beginGroup("X-coordinates");
    for (const auto flags : allFlags) {
        if (flags & SimpleGlyphFlags::X_SHORT_VECTOR) {
            if (flags & SimpleGlyphFlags::X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR) {
                parser.read<UInt8>("Coordinate");
            } else {
                parser.read<UInt8>("Coordinate"); // TODO: negative
            }
        } else {
            if (flags & SimpleGlyphFlags::X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR) {
                // Nothing.
            } else {
                parser.read<Int16>("Coordinate");
            }
        }
    }
    parser.endGroup();

    parser.beginGroup("Y-coordinates");
    for (const auto flags : allFlags) {
        if (flags & SimpleGlyphFlags::Y_SHORT_VECTOR) {
            if (flags & SimpleGlyphFlags::Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR) {
                parser.read<UInt8>("Coordinate");
            } else {
                parser.read<UInt8>("Coordinate"); // TODO: negative
            }
        } else {
            if (flags & SimpleGlyphFlags::Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR) {
                // Nothing.
            } else {
                parser.read<Int16>("Coordinate");
            }
        }
    }
    parser.endGroup();
}

static void parseCompositeGlyph(Parser &parser)
{
    const auto flags = parser.read<CompositeGlyphFlags>("Flag");
    parser.read<GlyphId>("Glyph ID");


    std::array<double, 6> matrix = {0};

    parser.beginGroup(QString());
    if (flags & CompositeGlyphFlags::ARGS_ARE_XY_VALUES) {
        if (flags & CompositeGlyphFlags::ARG_1_AND_2_ARE_WORDS) {
            matrix[4] = parser.read<Int16>("E");
            matrix[5] = parser.read<Int16>("F");
        } else {
            matrix[4] = parser.read<Int8>("E");
            matrix[5] = parser.read<Int8>("F");
        }
    }

    if (flags & CompositeGlyphFlags::WE_HAVE_A_TWO_BY_TWO) {
        matrix[0] = parser.read<F2DOT14>("A");
        matrix[1] = parser.read<F2DOT14>("B");
        matrix[2] = parser.read<F2DOT14>("C");
        matrix[3] = parser.read<F2DOT14>("D");
    } else if (flags & CompositeGlyphFlags::WE_HAVE_AN_X_AND_Y_SCALE) {
        matrix[0] = parser.read<F2DOT14>("A");
        matrix[3] = parser.read<F2DOT14>("D");
    } else if (flags & CompositeGlyphFlags::WE_HAVE_A_SCALE) {
        matrix[0] = parser.read<F2DOT14>("A");
        matrix[3] = matrix[0];
    }

    const auto matrixStr = QString("Matrix (%1 %2 %3 %4 %5 %6)")
        .arg(QString::number(matrix.at(0)))
        .arg(QString::number(matrix.at(1)))
        .arg(QString::number(matrix.at(2)))
        .arg(QString::number(matrix.at(3)))
        .arg(QString::number(matrix.at(4)))
        .arg(QString::number(matrix.at(5)));
    parser.endGroup(matrixStr);

    if (flags & CompositeGlyphFlags::MORE_COMPONENTS) {
        parseCompositeGlyph(parser);
    }
}

enum class GlyphType
{
    Empty,
    Simple,
    Composite,
};

static GlyphType parseGlyph(Parser &parser)
{
    const auto numOfContours = parser.read<Int16>("Number of contours");
    parser.read<Int16>("x min");
    parser.read<Int16>("y min");
    parser.read<Int16>("x max");
    parser.read<Int16>("y max");

    if (numOfContours == 0) {
        return GlyphType::Empty;
    } else if (numOfContours > 0) {
        parseSimpleGlyph(quint16(numOfContours), parser);
        return GlyphType::Simple;
    } else {
        parseCompositeGlyph(parser);
        return GlyphType::Composite;
    }
}

void parseGlyf(const quint16 numberOfGlyphs, const IndexToLocFormat format,
               const gsl::span<const quint8> locaData, Parser &parser)
{
    const auto glyphSizes = collectGlyphSizes(numberOfGlyphs, format, locaData);

    for (int gid = 0; gid < numberOfGlyphs; ++gid) {
        const auto glyphSize = glyphSizes.at(gid);
        if (glyphSize == 0) {
            continue;
        }

        const auto start = parser.offset();

        parser.beginGroup(QString("Glyph %1").arg(gid));
        const auto type = parseGlyph(parser);

        if (type == GlyphType::Composite) {
            parser.endGroup(QString("Glyph %1 (composite)").arg(gid));
        } else {
            parser.endGroup();
        }

        const auto diff = parser.offset() - start;
        if (glyphSize > diff) {
            parser.readBytes(glyphSize - diff, "Padding");
        } else if (glyphSize < diff) {
            throw "malformed glyph";
        }
    }
}
