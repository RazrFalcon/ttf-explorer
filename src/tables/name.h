#pragma once

#include "src/parser.h"

namespace Name
{
    static const int WINDOWS_UNICODE_BMP_ENCODING_ID = 1;

    // https://docs.microsoft.com/en-us/typography/opentype/spec/name#platform-ids
    struct PlatformID
    {
        static const int Size = 2;
        static const QString Type;

        enum ID : qint8
        {
            Unicode = 0,
            Macintosh,
            Iso,
            Windows,
            Custom,
        };

        static PlatformID parse(const quint8 *data)
        {
            const auto value = UInt16::parse(data);

            if (value < 5) {
                return { static_cast<ID>(value.d) };
            } else {
                return { ID::Custom }; // TODO: error
            }
        }

        static QString toString(const PlatformID &value)
        {
            switch (value.d) {
                case ID::Unicode : return QLatin1String("Unicode");
                case ID::Macintosh : return QLatin1String("Macintosh");
                case ID::Iso : return QLatin1String("ISO");
                case ID::Windows : return QLatin1String("Windows");
                case ID::Custom : return QLatin1String("Custom");
            }
        }

        operator ID() const { return d; }

        ID d;
    };

    QString encodingName(const PlatformID platform, const quint16 id);
    QString languageName(const PlatformID platform, const quint16 id);
}
