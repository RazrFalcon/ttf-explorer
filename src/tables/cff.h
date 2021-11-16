#pragma once

#include "src/parser.h"

namespace CFF
{
    enum class OffsetSizeBytes
    {
        One,
        Two,
        Three,
        Four,
    };

    struct OffsetSize
    {
        constexpr static int Size = 1;
        static const QString Type;

        static OffsetSize parse(const quint8 *data)
        {
            const auto n = data[0];
            if (n >= 1 && n <= 4) {
                return { n };
            }

            throw QString("invalid OffsetSize");
        }

        static QString toString(const OffsetSize &value)
        {
            return QString::number(value.d);
        }

        DEFAULT_DEBUG(OffsetSize)

        OffsetSizeBytes to_bytes() const
        {
            switch (d) {
                case 1: return OffsetSizeBytes::One;
                case 2: return OffsetSizeBytes::Two;
                case 3: return OffsetSizeBytes::Three;
                case 4: return OffsetSizeBytes::Four;
                default: Q_UNREACHABLE();
            }
        }

        operator quint8() const { return d; }

        quint8 d;
    };

    float parseFloat(ShadowParser &parser);
}
