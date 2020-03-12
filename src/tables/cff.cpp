#include <QDebug>

#include "src/algo.h"
#include "src/parser.h"
#include "tables.h"

namespace DictOperator {
    static const quint8 VERSION = 0;
    static const quint8 NOTICE = 1;
    static const quint8 FULL_NAME = 2;
    static const quint8 FAMILY_NAME = 3;
    static const quint8 WEIGHT = 4;
    static const quint8 FONT_BBOX = 5;
    static const quint8 BLUE_VALUES = 6;
    static const quint8 OTHER_BLUES = 7;
    static const quint8 FAMILY_BLUES = 8;
    static const quint8 FAMILY_OTHER_BLUES = 9;
    static const quint8 STD_HW = 10;
    static const quint8 STD_VW = 11;
    static const quint8 UNIQUE_ID = 13;
    static const quint8 XUID = 14;
    static const quint8 CHARSER = 15;
    static const quint8 ENCODING = 16;
    static const quint8 CHAR_STRINGS = 17;
    static const quint8 PRIVATE = 18;
    static const quint8 SUBRS = 19;
    static const quint8 DEFAULT_WIDTH_X = 20;
    static const quint8 NOMINAL_WIDTH_X = 21;

    static const quint16 COPYRIGHT = 1200;
    static const quint16 IS_FIXED_PITCH = 1201;
    static const quint16 ITALIC_ANGLE = 1202;
    static const quint16 UNDERLINE_POSITION = 1203;
    static const quint16 UNDERLINE_THICKNESS = 1204;
    static const quint16 PAINT_TYPE = 1205;
    static const quint16 CHAR_STRING_TYPE = 1206;
    static const quint16 FONT_MATRIX = 1207;
    static const quint16 STROKE_WIDTH = 1208;
    static const quint16 BLUE_SCALE = 1209;
    static const quint16 BLUE_SHIFT = 1210;
    static const quint16 BLUE_FUZZ = 1211;
    static const quint16 STEM_SNAP_H = 1212;
    static const quint16 STEM_SNAP_V = 1213;
    static const quint16 FORCE_BOLD = 1214;
    static const quint16 LANGUAGE_GROUP = 1217;
    static const quint16 EXPANSION_FACTOR = 1218;
    static const quint16 INITIAL_RANDOM_SEED = 1219;
    static const quint16 SYNTHETIC_BASE = 1220;
    static const quint16 POST_SCRIPT = 1221;
    static const quint16 BASE_FONT_NAME = 1222;
    static const quint16 BASE_FONT_BLEND = 1223;
    static const quint16 ROS = 1230;
    static const quint16 CID_FONT_VERSION = 1231;
    static const quint16 CID_FONT_REVISION = 1232;
    static const quint16 CID_FONT_TYPE = 1233;
    static const quint16 CID_COUNT = 1234;
    static const quint16 UID_BASE = 1235;
    static const quint16 FD_ARRAY = 1236;
    static const quint16 FD_SELECT = 1237;
    static const quint16 FONT_NAME = 1238;
}

using namespace CFFCommon;

template<typename Predicate>
static void parseIndex(const QString &name, Parser &parser, Predicate p)
{
    parser.beginGroup(name);

    const auto count = parser.read<UInt16>("Count");
    if (count == std::numeric_limits<quint16>::max()) {
        throw "index items count overflow";
    }

    if (count == 0) {
        parser.endGroup();
        return;
    }

    const auto offsetSize = parser.read<OffsetSize>("Offset size");

    parser.beginGroup("Indexes", QString::number(count + 1));
    QVector<quint32> offsets;
    // INDEX has one more index at the end to indicate data length, so we have to add 1 to count.
    for (auto i = 0; i < count + 1; ++i) {
        const auto title = QString("Index %1").arg(i);
        quint32 offset = 0;
        switch (offsetSize.to_bytes()) {
            case OffsetSizeBytes::One :   offset = parser.read<UInt8>(title); break;
            case OffsetSizeBytes::Two :   offset = parser.read<UInt16>(title); break;
            case OffsetSizeBytes::Three : offset = parser.read<UInt24>(title); break;
            case OffsetSizeBytes::Four :  offset = parser.read<UInt32>(title); break;
        }
        offsets << offset;
    }

    parser.endGroup();

    for (auto i = 1; i < offsets.size(); ++i) {
        // All offsets start from 1 and not 0, so we have to shift them.
        const auto start = offsets[i - 1] - 1;
        const auto end = offsets[i] - 1;
        p(start, end, i - 1, parser);
    }

    parser.endGroup();
}

struct DictRecord
{
    quint16 op = 0;
    QVarLengthArray<float, 8> operands;
};

struct Dict
{
    QVector<DictRecord> records;

    std::optional<QVarLengthArray<float, 8>> operands(const quint16 op) const
    {
        if (const auto rec = algo::find_if(records, [=](const auto &v){ return v.op == op; })) {
            return rec->operands;
        } else {
            return std::nullopt;
        }
    }
};

static Dict parseDict(const quint32 size, Parser &parser)
{
    Dict dict;

    parser.beginGroup(QString());

    DictRecord currRecord;

    const auto globalEnd = parser.offset() + size;
    while (parser.offset() < globalEnd) {
        const auto op1 = parser.peek<UInt8>();
        if (op1 == 12) {
            const auto op2 = parser.peek<UInt8>(2);

            QString title;
            switch (1200 + op2) {
                case DictOperator::COPYRIGHT : title = "Copyright"; break;
                case DictOperator::IS_FIXED_PITCH : title = "Is fixed pitch"; break;
                case DictOperator::ITALIC_ANGLE : title = "Italic angle"; break;
                case DictOperator::UNDERLINE_POSITION : title = "Underline position"; break;
                case DictOperator::UNDERLINE_THICKNESS : title = "Underline thickness"; break;
                case DictOperator::PAINT_TYPE : title = "Paint type"; break;
                case DictOperator::CHAR_STRING_TYPE : title = "Charstring type"; break;
                case DictOperator::FONT_MATRIX : title = "Font matrix"; break;
                case DictOperator::STROKE_WIDTH : title = "Stroke width"; break;
                case DictOperator::BLUE_SCALE : title = "Blue scale"; break;
                case DictOperator::BLUE_SHIFT : title = "Blue shift"; break;
                case DictOperator::BLUE_FUZZ : title = "Blue fuzz"; break;
                case DictOperator::STEM_SNAP_H : title = "Stem snap H"; break;
                case DictOperator::STEM_SNAP_V : title = "Stem snap V"; break;
                case DictOperator::FORCE_BOLD : title = "Force bold"; break;
                case DictOperator::LANGUAGE_GROUP : title = "Language group"; break;
                case DictOperator::EXPANSION_FACTOR : title = "Expansion factor"; break;
                case DictOperator::INITIAL_RANDOM_SEED : title = "Initial random seed"; break;
                case DictOperator::SYNTHETIC_BASE : title = "Synthetic base"; break;
                case DictOperator::POST_SCRIPT : title = "PostScript"; break;
                case DictOperator::BASE_FONT_NAME : title = "Base font name"; break;
                case DictOperator::BASE_FONT_BLEND : title = "Base font blend"; break;
                case DictOperator::ROS : title = "ROS"; break;
                case DictOperator::CID_FONT_VERSION : title = "CID font version"; break;
                case DictOperator::CID_FONT_REVISION : title = "CID font revision"; break;
                case DictOperator::CID_FONT_TYPE : title = "CID font type"; break;
                case DictOperator::CID_COUNT : title = "CID count"; break;
                case DictOperator::UID_BASE : title = "UID base"; break;
                case DictOperator::FD_ARRAY : title = "FD array"; break;
                case DictOperator::FD_SELECT : title = "FD select"; break;
                case DictOperator::FONT_NAME : title = "Font name"; break;
                default: break;
            }

            parser.read<UInt16>("Operator");

            // Keep only known operators.
            if (!title.isEmpty()) {
                currRecord.op = 1200 + op2;
                dict.records.append(currRecord);
            }

            currRecord = DictRecord();

            parser.endGroup(title);
            if (parser.offset() != globalEnd) {
                parser.beginGroup(QString());
            }
        } else if (op1 <= 21) {
            // 0..=21 bytes are operators.

            QString title;
            switch (op1) {
                case DictOperator::VERSION : title = "Version"; break;
                case DictOperator::NOTICE : title = "Notice"; break;
                case DictOperator::FULL_NAME : title = "Full name"; break;
                case DictOperator::FAMILY_NAME : title = "Family name"; break;
                case DictOperator::WEIGHT : title = "Weight"; break;
                case DictOperator::FONT_BBOX : title = "Font bbox"; break;
                case DictOperator::BLUE_VALUES : title = "Blue values"; break;
                case DictOperator::OTHER_BLUES : title = "Other blues"; break;
                case DictOperator::FAMILY_BLUES : title = "Family blues"; break;
                case DictOperator::FAMILY_OTHER_BLUES : title = "Family other blues"; break;
                case DictOperator::STD_HW : title = "Std HW"; break;
                case DictOperator::STD_VW : title = "Std VW"; break;
                case DictOperator::UNIQUE_ID : title = "Unique ID"; break;
                case DictOperator::XUID : title = "XUID"; break;
                case DictOperator::CHARSER : title = "charset"; break;
                case DictOperator::ENCODING : title = "Encoding"; break;
                case DictOperator::CHAR_STRINGS : title = "CharStrings"; break;
                case DictOperator::PRIVATE : title = "Private"; break;
                case DictOperator::SUBRS : title = "Local subroutines"; break;
                case DictOperator::DEFAULT_WIDTH_X : title = "Default width X"; break;
                case DictOperator::NOMINAL_WIDTH_X : title = "Nominal width X"; break;
                default: break;
            }

            parser.read<UInt8>("Operator");

            // Keep only known operators.
            if (!title.isEmpty()) {
                currRecord.op = op1;
                dict.records.append(currRecord);
            }

            currRecord = DictRecord();

            parser.endGroup(title);
            if (parser.offset() != globalEnd) {
                parser.beginGroup("Value");
            }
        } else if (op1 == 28) {
            auto shadow = parser.shadow();
            shadow.read<UInt8>();
            const auto n = shadow.read<UInt16>();
            parser.readValue(shadow.offset(), "Number", QString::number(n));

            currRecord.operands.append(n);
        } else if (op1 == 29) {
            auto shadow = parser.shadow();
            shadow.read<UInt8>();
            const auto n = shadow.read<UInt32>();
            parser.readValue(shadow.offset(), "Number", QString::number(n));

            currRecord.operands.append(n);
        } else if (op1 == 30) {
            auto shadow = parser.shadow();
            const auto start = shadow.offset();
            shadow.read<UInt8>();
            const auto n = CFFCommon::parseFloat(shadow);
            parser.readBytes(shadow.offset() - start, "Float");

            currRecord.operands.append(n);
        } else if (op1 >= 32 && op1 <= 246) {
            const auto n = int(op1) - 139;
            parser.readValue(1, "Number", QString::number(n));

            currRecord.operands.append(n);
        } else if (op1 >= 247 && op1 <= 250) {
            auto shadow = parser.shadow();
            const auto b0 = shadow.read<UInt8>();
            const auto b1 = shadow.read<UInt8>();
            const auto n = (int(b0) - 247) * 256 + int(b1) + 108;
            parser.readValue(shadow.offset(), "Number", QString::number(n));

            currRecord.operands.append(n);
        } else if (op1 >= 251 && op1 <= 254) {
            auto shadow = parser.shadow();
            const auto b0 = shadow.read<UInt8>();
            const auto b1 = shadow.read<UInt8>();
            const auto n = -(int(b0) - 251) * 256 - int(b1) - 108;
            parser.readValue(shadow.offset(), "Number", QString::number(n));

            currRecord.operands.append(n);
        }
    }

    return dict;
}

static void parseSubr(const quint32 start, const quint32 end, const int index, Parser &parser)
{
    parser.beginGroup(QString("Subroutine %1").arg(index));

    if (start > end) {
        throw "invalid Subroutine data";
    }

    const auto globalEnd = parser.offset() + (end - start);

    while (parser.offset() < globalEnd) {
        const auto b0 = parser.peek<UInt8>();
        if (b0 == 0) {
            parser.read<UInt8>("Reserved");
        } else if (b0 == 1) {
            parser.read<UInt8>("Horizontal stem (hstem)");
        } else if (b0 == 2) {
            parser.read<UInt8>("Reserved");
        } else if (b0 == 3) {
            parser.read<UInt8>("Vertical stem (vstem)");
        } else if (b0 == 4) {
            parser.read<UInt8>("Vertical move to (vmoveto)");
        } else if (b0 == 5) {
            parser.read<UInt8>("Line to (rlineto)");
        } else if (b0 == 6) {
            parser.read<UInt8>("Horizontal line to (hlineto)");
        } else if (b0 == 7) {
            parser.read<UInt8>("Vertical line to (vlineto)");
        } else if (b0 == 8) {
            parser.read<UInt8>("Curve to (rrcurveto)");
        } else if (b0 == 9) {
            parser.read<UInt8>("Reserved");
        } else if (b0 == 10) {
            parser.read<UInt8>("Call local subroutine (callsubr)");
        } else if (b0 == 11) {
            parser.read<UInt8>("Return (return)");
        } else if (b0 == 12) {
            const auto b1 = parser.peek<UInt8>(2);
            if (b1 == 3) {
                parser.read<UInt16>("(and)");
            } else if (b1 == 4) {
                parser.read<UInt16>("(or)");
            } else if (b1 == 5) {
                parser.read<UInt16>("(not)");
            } else if (b1 == 9) {
                parser.read<UInt16>("(abs)");
            } else if (b1 == 10) {
                parser.read<UInt16>("(add)");
            } else if (b1 == 11) {
                parser.read<UInt16>("(sub)");
            } else if (b1 == 12) {
                parser.read<UInt16>("(div)");
            } else if (b1 == 14) {
                parser.read<UInt16>("(neg)");
            } else if (b1 == 15) {
                parser.read<UInt16>("(eq)");
            } else if (b1 == 18) {
                parser.read<UInt16>("(drop)");
            } else if (b1 == 20) {
                parser.read<UInt16>("(put)");
            } else if (b1 == 21) {
                parser.read<UInt16>("(get)");
            } else if (b1 == 22) {
                parser.read<UInt16>("(ifelse)");
            } else if (b1 == 23) {
                parser.read<UInt16>("(random)");
            } else if (b1 == 24) {
                parser.read<UInt16>("(mul)");
            } else if (b1 == 26) {
                parser.read<UInt16>("(sqrt)");
            } else if (b1 == 27) {
                parser.read<UInt16>("(dup)");
            } else if (b1 == 28) {
                parser.read<UInt16>("(exch)");
            } else if (b1 == 29) {
                parser.read<UInt16>("(index)");
            } else if (b1 == 30) {
                parser.read<UInt16>("(roll)");
            } else if (b1 == 34) {
                parser.read<UInt16>("Horizontal flex (hflex)");
            } else if (b1 == 35) {
                parser.read<UInt16>("Flex (flex)");
            } else if (b1 == 36) {
                parser.read<UInt16>("Horizontal flex 1 (hflex1)");
            } else if (b1 == 37) {
                parser.read<UInt16>("Flex 1 (flex1)");
            } else {
                parser.read<UInt16>("Reserved");
            }
        } else if (b0 == 13) {
            parser.read<UInt8>("Reserved");
        } else if (b0 == 14) {
            parser.read<UInt8>("Endchar (endchar)");
        } else if (b0 == 15) {
            parser.read<UInt8>("Reserved");
        } else if (b0 == 16) {
            parser.read<UInt8>("Reserved");
        } else if (b0 == 17) {
            parser.read<UInt8>("Reserved");
        } else if (b0 == 18) {
            parser.read<UInt8>("Horizontal stem hint mask (hstemhm)");
        } else if (b0 == 19) {
            parser.read<UInt8>("Hint mask (hintmask)");
        } else if (b0 == 20) {
            parser.read<UInt8>("Counter mask (cntrmask)");
        } else if (b0 == 21) {
            parser.read<UInt8>("Move to (rmoveto)");
        } else if (b0 == 22) {
            parser.read<UInt8>("Horizontal move to (hmoveto)");
        } else if (b0 == 23) {
            parser.read<UInt8>("Vertical stem hint mask (vstemhm)");
        } else if (b0 == 24) {
            parser.read<UInt8>("Curve line (rcurveline)");
        } else if (b0 == 25) {
            parser.read<UInt8>("Line curve (rlinecurve)");
        } else if (b0 == 26) {
            parser.read<UInt8>("Vertical vertical curve to (vvcurveto)");
        } else if (b0 == 27) {
            parser.read<UInt8>("Horizontal horizontal curve to (hhcurveto)");
        } else if (b0 == 28) {
            if (parser.offset() + 3 > globalEnd) {
                break;
            }

            const auto b1 = parser.peek<UInt8>(2);
            const auto b2 = parser.peek<UInt8>(3);
            const auto n = (int(b1) << 24 | int(b2) << 16) >> 16;
            parser.readValue(3, "Number", QString::number(n));
        } else if (b0 == 29) {
            parser.read<UInt8>("Call global subroutine (callgsubr)");
        } else if (b0 == 30) {
            parser.read<UInt8>("Vertical horizontal curve to (vhcurveto)");
        } else if (b0 == 31) {
            parser.read<UInt8>("Horizontal vertical curve to (hvcurveto)");
        } else if (b0 >= 32 && b0 <= 246) {
            parser.readValue(1, "Number", QString::number(int(b0) - 139));
        } else if (b0 >= 247 && b0 <= 250) {
            const auto b1 = parser.peek<UInt8>(2);
            const auto n = (int(b0) - 247) * 256 + int(b1) + 108;
            parser.readValue(2, "Number", QString::number(n));
        } else if (b0 >= 251 && b0 <= 254) {
            const auto b1 = parser.peek<UInt8>(2);
            const auto n = -(int(b0) - 251) * 256 - int(b1) - 108;
            parser.readValue(2, "Number", QString::number(n));
        } else if (b0 == 255) {
            if (parser.offset() + 5 > globalEnd) {
                break;
            }

            auto shadow = parser.shadow();
            shadow.read<UInt8>(); // skip b0
            const auto n = shadow.read<UInt32>() / 65536.0;
            parser.readValue(5, "Number", QString::number(n));
        }
    }

    if (parser.offset() > globalEnd) {
        qDebug() << "diff" << index << parser.offset() << globalEnd;
        throw "invalid charstring";
    } else if (parser.offset() < globalEnd) {
        // TODO: Padding or something else?
        parser.advance(globalEnd - parser.offset());
    } else {
        parser.read<UInt8>("Unknown");
    }

    parser.endGroup();
}

void parseCff(Parser &parser)
{
    const auto tableStart = parser.offset();

    parser.beginGroup("Header");
    parser.read<UInt8>("Major version");
    parser.read<UInt8>("Minor version");
    const auto headerSize = parser.read<UInt8>("Header size");
    parser.read<UInt8>("Absolute offset");
    parser.endGroup();

    if (headerSize > 4) {
        parser.advance(headerSize - 4); // Padding
    } else if (headerSize < 4) {
        throw "header size is too small";
    }

    parseIndex("Name INDEX", parser, [](const auto start, const auto end, const auto index, auto &parser){
        parser.readString(end - start, QString("Name %1").arg(index));
    });

    Dict topDict;
    parseIndex("Top DICT INDEX", parser, [&](const auto start, const auto end, const auto index, auto &parser){
        if (index != 0) {
            throw "Top DICT INDEX should have only one dictionary";
        }

        topDict = parseDict(end - start, parser);
    });

    parseIndex("String INDEX", parser, [](const auto start, const auto end, const auto index, auto &parser){
        parser.readString(end - start, QString("String %1").arg(index));
    });

    parseIndex("Global Subr INDEX", parser, parseSubr);

    // TODO: Encodings
    // TODO: Charsets
    // TODO: FDSelect
    // TODO: Font DICT INDEX

    if (const auto operands = topDict.operands(DictOperator::CHAR_STRINGS)) {
        if (operands->size() != 1 || operands->at(0) < 0) {
            throw "invalid CharStrings operands";
        }

        const auto offset = quint32(operands->at(0));

        parser.jumpTo(tableStart + offset);
        parseIndex("CharStrings INDEX", parser, parseSubr);
    }

    quint32 local_subrs_offset = 0;
    if (const auto operands = topDict.operands(DictOperator::PRIVATE)) {
        if (operands->size() != 2 || operands->at(0) < 0 || operands->at(1) < 0) {
            throw "invalid Private DICT operands";
        }

        const auto size = quint32(operands->at(0));
        const auto dictOffset = tableStart + quint32(operands->at(1));

        parser.jumpTo(dictOffset);
        parser.beginGroup("Private DICT");
        const auto privateDict = parseDict(size, parser);
        parser.endGroup();

        if (const auto operands = privateDict.operands(DictOperator::SUBRS)) {
            if (operands->size() != 1 || operands->at(0) < 0) {
                throw "invalid Subrs operands";
            }

            const auto offset = quint32(operands->at(0));
            // 'The local subroutines offset is relative to the beginning
            // of the Private DICT data.'
            local_subrs_offset = dictOffset + offset;
        }
    }

    if (local_subrs_offset != 0) {
        parser.jumpTo(local_subrs_offset);
        parseIndex("Local Subr INDEX", parser, parseSubr);
    }
}
