#include "parser.h"

const QString Int8::Type = "Int8";
const QString UInt8::Type = "UInt8";
const QString Int16::Type = "Int16";
const QString UInt16::Type = "UInt16";
const QString UInt24::Type = "UInt24";
const QString UInt32::Type = "UInt32";
const QString Tag::Type = "Tag";
const QString Fixed::Type = "Fixed";
const QString F2DOT14::Type = "F2DOT14";
const QString Offset16::Type = "Offset16";
const QString Offset32::Type = "Offset32";
const QString GlyphId::Type = "GlyphId";
const QString LongDateTime::Type = "LongDateTime";
const QString CFFCommon::OffsetSize::Type = "OffsetSize";

static const quint8 END_OF_FLOAT_FLAG = 0xf;
static const quint8 FLOAT_STACK_LEN = 64;

static int parseFloatNibble(quint8 nibble, int idx, quint8 *stack)
{
    if (idx == FLOAT_STACK_LEN) {
        throw "invalid float";
    }

    if (nibble <= 9) {
        stack[idx] = '0' + nibble;
    } else if (nibble == 10) {
        stack[idx] = '.';
    } else if (nibble == 11) {
        stack[idx] = 'E';
    } else if (nibble == 12) {
        if (idx + 1 == FLOAT_STACK_LEN) {
            throw "invalid float";
        }

        stack[idx] = 'E';
        idx++;
        stack[idx] = '-';
    } else if (nibble == 13) {
        throw "invalid float";
    } else if (nibble == 14) {
        stack[idx] = '-';
    } else {
        throw "invalid float";
    }

    return idx + 1;
}

float CFFCommon::parseFloat(ShadowParser &parser)
{
    int idx = 0;
    std::array<quint8, FLOAT_STACK_LEN> stack = {0};
    while (!parser.atEnd()) {
        const auto b1 = parser.read<UInt8>();
        const quint8 nibble1 = b1 >> 4;
        const quint8 nibble2 = b1 & 15;

        if (nibble1 == END_OF_FLOAT_FLAG) {
            break;
        }

        idx = parseFloatNibble(nibble1, idx, stack.data());

        if (nibble2 == END_OF_FLOAT_FLAG) {
            break;
        }

        idx = parseFloatNibble(nibble2, idx, stack.data());
    }

    bool ok = false;
    const auto n = QString::fromUtf8((const char *)stack.data(), idx).toFloat(&ok);
    if (!ok) {
        throw "invalid float";
    }

    return n;
}
