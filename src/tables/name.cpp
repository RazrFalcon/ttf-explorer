#include <QDebug>

#include "src/algo.h"
#include "src/parser.h"
#include "tables.h"

struct Range
{
    quint32 start;
    quint32 end;

    quint32 size() const { return end - start; }
};

static const int WINDOWS_UNICODE_BMP_ENCODING_ID = 1;


static QString winEncodingName(const quint16 id)
{
    switch (id) {
        case 0: return "Symbol";
        case 1: return "Unicode BMP";
        case 2: return "ShiftJIS";
        case 3: return "PRC";
        case 4: return "Big5";
        case 5: return "Wansung";
        case 6: return "Johab";
        case 7: return "Reserved";
        case 8: return "Reserved";
        case 9: return "Reserved";
        case 10: return "Unicode full repertoire";
        default: return "Unknown";
    }
}

// https://docs.microsoft.com/en-us/typography/opentype/spec/name#windows-language-ids
static QString winLanguageName(const quint16 id)
{
    switch (id) {
        case 0x0436 : return "Afrikaans, South Africa";
        case 0x041C : return "Albanian, Albania";
        case 0x0484 : return "Alsatian, France";
        case 0x045E : return "Amharic, Ethiopia";
        case 0x1401 : return "Arabic, Algeria";
        case 0x3C01 : return "Arabic, Bahrain";
        case 0x0C01 : return "Arabic, Egypt";
        case 0x0801 : return "Arabic, Iraq";
        case 0x2C01 : return "Arabic, Jordan";
        case 0x3401 : return "Arabic, Kuwait";
        case 0x3001 : return "Arabic, Lebanon";
        case 0x1001 : return "Arabic, Libya";
        case 0x1801 : return "Arabic, Morocco";
        case 0x2001 : return "Arabic, Oman";
        case 0x4001 : return "Arabic, Qatar";
        case 0x0401 : return "Arabic, Saudi Arabia";
        case 0x2801 : return "Arabic, Syria";
        case 0x1C01 : return "Arabic, Tunisia";
        case 0x3801 : return "Arabic, U.A.E.";
        case 0x2401 : return "Arabic, Yemen";
        case 0x042B : return "Armenian, Armenia";
        case 0x044D : return "Assamese, India";
        case 0x082C : return "Azeri (Cyrillic), Azerbaijan";
        case 0x042C : return "Azeri (Latin), Azerbaijan";
        case 0x046D : return "Bashkir, Russia";
        case 0x042D : return "Basque, Basque";
        case 0x0423 : return "Belarusian, Belarus";
        case 0x0845 : return "Bengali, Bangladesh";
        case 0x0445 : return "Bengali, India";
        case 0x201A : return "Bosnian (Cyrillic), Bosnia and Herzegovina";
        case 0x141A : return "Bosnian (Latin), Bosnia and Herzegovina";
        case 0x047E : return "Breton, France";
        case 0x0402 : return "Bulgarian, Bulgaria";
        case 0x0403 : return "Catalan, Catalan";
        case 0x0C04 : return "Chinese, Hong Kong S.A.R.";
        case 0x1404 : return "Chinese, Macao S.A.R.";
        case 0x0804 : return "Chinese, People’s Republic of China";
        case 0x1004 : return "Chinese, Singapore";
        case 0x0404 : return "Chinese, Taiwan";
        case 0x0483 : return "Corsican, France";
        case 0x041A : return "Croatian, Croatia";
        case 0x101A : return "Croatian (Latin), Bosnia and Herzegovina";
        case 0x0405 : return "Czech, Czech Republic";
        case 0x0406 : return "Danish, Denmark";
        case 0x048C : return "Dari, Afghanistan";
        case 0x0465 : return "Divehi, Maldives";
        case 0x0813 : return "Dutch, Belgium";
        case 0x0413 : return "Dutch, Netherlands";
        case 0x0C09 : return "English, Australia";
        case 0x2809 : return "English, Belize";
        case 0x1009 : return "English, Canada";
        case 0x2409 : return "English, Caribbean";
        case 0x4009 : return "English, India";
        case 0x1809 : return "English, Ireland";
        case 0x2009 : return "English, Jamaica";
        case 0x4409 : return "English, Malaysia";
        case 0x1409 : return "English, New Zealand";
        case 0x3409 : return "English, Republic of the Philippines";
        case 0x4809 : return "English, Singapore";
        case 0x1C09 : return "English, South Africa";
        case 0x2C09 : return "English, Trinidad and Tobago";
        case 0x0809 : return "English, United Kingdom";
        case 0x0409 : return "English, United States";
        case 0x3009 : return "English, Zimbabwe";
        case 0x0425 : return "Estonian, Estonia";
        case 0x0438 : return "Faroese, Faroe Islands";
        case 0x0464 : return "Filipino, Philippines";
        case 0x040B : return "Finnish, Finland";
        case 0x080C : return "French, Belgium";
        case 0x0C0C : return "French, Canada";
        case 0x040C : return "French, France";
        case 0x140c : return "French, Luxembourg";
        case 0x180C : return "French, Principality of Monaco";
        case 0x100C : return "French, Switzerland";
        case 0x0462 : return "Frisian, Netherlands";
        case 0x0456 : return "Galician, Galician";
        case 0x0437 : return "Georgian, Georgia";
        case 0x0C07 : return "German, Austria";
        case 0x0407 : return "German, Germany";
        case 0x1407 : return "German, Liechtenstein";
        case 0x1007 : return "German, Luxembourg";
        case 0x0807 : return "German, Switzerland";
        case 0x0408 : return "Greek, Greece";
        case 0x046F : return "Greenlandic, Greenland";
        case 0x0447 : return "Gujarati, India";
        case 0x0468 : return "Hausa (Latin), Nigeria";
        case 0x040D : return "Hebrew, Israel";
        case 0x0439 : return "Hindi, India";
        case 0x040E : return "Hungarian, Hungary";
        case 0x040F : return "Icelandic, Iceland";
        case 0x0470 : return "Igbo, Nigeria";
        case 0x0421 : return "Indonesian, Indonesia";
        case 0x045D : return "Inuktitut, Canada";
        case 0x085D : return "Inuktitut (Latin), Canada";
        case 0x083C : return "Irish, Ireland";
        case 0x0434 : return "isiXhosa, South Africa";
        case 0x0435 : return "isiZulu, South Africa";
        case 0x0410 : return "Italian, Italy";
        case 0x0810 : return "Italian, Switzerland";
        case 0x0411 : return "Japanese, Japan";
        case 0x044B : return "Kannada, India";
        case 0x043F : return "Kazakh, Kazakhstan";
        case 0x0453 : return "Khmer, Cambodia";
        case 0x0486 : return "K’iche, Guatemala";
        case 0x0487 : return "Kinyarwanda, Rwanda";
        case 0x0441 : return "Kiswahili, Kenya";
        case 0x0457 : return "Konkani, India";
        case 0x0412 : return "Korean, Korea";
        case 0x0440 : return "Kyrgyz, Kyrgyzstan";
        case 0x0454 : return "Lao, Lao P.D.R.";
        case 0x0426 : return "Latvian, Latvia";
        case 0x0427 : return "Lithuanian, Lithuania";
        case 0x082E : return "Lower, Sorbian Germany";
        case 0x046E : return "Luxembourgish, Luxembourg";
        case 0x042F : return "Macedonian (FYROM), Former Yugoslav Republic of Macedonia";
        case 0x083E : return "Malay, Brunei Darussalam";
        case 0x043E : return "Malay, Malaysia";
        case 0x044C : return "Malayalam, India";
        case 0x043A : return "Maltese, Malta";
        case 0x0481 : return "Maori, New Zealand";
        case 0x047A : return "Mapudungun, Chile";
        case 0x044E : return "Marathi, India";
        case 0x047C : return "Mohawk, Mohawk";
        case 0x0450 : return "Mongolian (Cyrillic), Mongolia";
        case 0x0850 : return "Mongolian (Traditional), People’s Republic of China";
        case 0x0461 : return "Nepali, Nepal";
        case 0x0414 : return "Norwegian (Bokmal), Norway";
        case 0x0814 : return "Norwegian (Nynorsk), Norway";
        case 0x0482 : return "Occitan, France";
        case 0x0448 : return "Odia (formerly Oriya), India";
        case 0x0463 : return "Pashto, Afghanistan";
        case 0x0415 : return "Polish, Poland";
        case 0x0416 : return "Portuguese, Brazil";
        case 0x0816 : return "Portuguese, Portugal";
        case 0x0446 : return "Punjabi, India";
        case 0x046B : return "Quechua, Bolivia";
        case 0x086B : return "Quechua, Ecuador";
        case 0x0C6B : return "Quechua, Peru";
        case 0x0418 : return "Romanian, Romania";
        case 0x0417 : return "Romansh, Switzerland";
        case 0x0419 : return "Russian, Russia";
        case 0x243B : return "Sami (Inari), Finland";
        case 0x103B : return "Sami (Lule), Norway";
        case 0x143B : return "Sami (Lule), Sweden";
        case 0x0C3B : return "Sami (Northern), Finland";
        case 0x043B : return "Sami (Northern), Norway";
        case 0x083B : return "Sami (Northern), Sweden";
        case 0x203B : return "Sami (Skolt), Finland";
        case 0x183B : return "Sami (Southern), Norway";
        case 0x1C3B : return "Sami (Southern), Sweden";
        case 0x044F : return "Sanskrit, India";
        case 0x1C1A : return "Serbian (Cyrillic), Bosnia and Herzegovina";
        case 0x0C1A : return "Serbian (Cyrillic), Serbia";
        case 0x181A : return "Serbian (Latin), Bosnia and Herzegovina";
        case 0x081A : return "Serbian (Latin), Serbia";
        case 0x046C : return "Sesotho sa Leboa, South Africa";
        case 0x0432 : return "Setswana, South Africa";
        case 0x045B : return "Sinhala, Sri Lanka";
        case 0x041B : return "Slovak, Slovakia";
        case 0x0424 : return "Slovenian, Slovenia";
        case 0x2C0A : return "Spanish, Argentina";
        case 0x400A : return "Spanish, Bolivia";
        case 0x340A : return "Spanish, Chile";
        case 0x240A : return "Spanish, Colombia";
        case 0x140A : return "Spanish, Costa Rica";
        case 0x1C0A : return "Spanish, Dominican Republic";
        case 0x300A : return "Spanish, Ecuador";
        case 0x440A : return "Spanish, El Salvador";
        case 0x100A : return "Spanish, Guatemala";
        case 0x480A : return "Spanish, Honduras";
        case 0x080A : return "Spanish, Mexico";
        case 0x4C0A : return "Spanish, Nicaragua";
        case 0x180A : return "Spanish, Panama";
        case 0x3C0A : return "Spanish, Paraguay";
        case 0x280A : return "Spanish, Peru";
        case 0x500A : return "Spanish, Puerto Rico";
        case 0x0C0A : return "Spanish (Modern Sort), Spain";
        case 0x040A : return "Spanish (Traditional Sort), Spain";
        case 0x540A : return "Spanish, United States";
        case 0x380A : return "Spanish, Uruguay";
        case 0x200A : return "Spanish, Venezuela";
        case 0x081D : return "Sweden, Finland";
        case 0x041D : return "Swedish, Sweden";
        case 0x045A : return "Syriac, Syria";
        case 0x0428 : return "Tajik (Cyrillic), Tajikistan";
        case 0x085F : return "Tamazight (Latin), Algeria";
        case 0x0449 : return "Tamil, India";
        case 0x0444 : return "Tatar, Russia";
        case 0x044A : return "Telugu, India";
        case 0x041E : return "Thai, Thailand";
        case 0x0451 : return "Tibetan, PRC";
        case 0x041F : return "Turkish, Turkey";
        case 0x0442 : return "Turkmen, Turkmenistan";
        case 0x0480 : return "Uighur, PRC";
        case 0x0422 : return "Ukrainian, Ukraine";
        case 0x042E : return "Upper, Sorbian Germany";
        case 0x0420 : return "Urdu, Islamic Republic of Pakistan";
        case 0x0843 : return "Uzbek (Cyrillic), Uzbekistan";
        case 0x0443 : return "Uzbek (Latin), Uzbekistan";
        case 0x042A : return "Vietnamese, Vietnam";
        case 0x0452 : return "Welsh, United Kingdom";
        case 0x0488 : return "Wolof, Senegal";
        case 0x0485 : return "Yakut, Russia";
        case 0x0478 : return "Yi, PRC";
        case 0x046A : return "Yoruba, Nigeria";
        default: return "Unknown";
    }
}

// https://docs.microsoft.com/en-us/typography/opentype/spec/name#platform-ids
struct PlatformID
{
    static const int Size = 2;
    static const QString Type;

    enum ID {
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
            return { ID::Custom };
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

//    DEFAULT_DEBUG(PlatformID)

    operator ID() const { return d; }

    ID d;
};

const QString PlatformID::Type = "Platform ID";


struct Utf16BE
{
    static const QString Type;

    static Utf16BE parse(const gsl::span<const quint8> data)
    {
        ShadowParser parser(data);

        QVector<quint16> bytes;
        while (!parser.atEnd()) {
            bytes << parser.read<UInt16>();
        }

        const auto name = QString::fromUtf16(bytes.constData(), bytes.size());

        return { name };
    }

    DEFAULT_DEBUG(Utf16BE)

    operator QString() const { return d; }

    QString d;
};

const QString Utf16BE::Type = "UTF-16 BE";


void parseName(Parser &parser)
{
    const auto tableStart = parser.offset();

    const auto format = parser.read<UInt16>("Format");
    const auto count = parser.read<UInt16>("Count");
    const auto stringOffset = parser.read<Offset16>("Offset to string storage");

    QVector<Range> nameOffsets;
    parser.beginGroup("Name records", QString::number(count));
    for (int i = 0; i < count; ++i) {
        parser.beginGroup(QString());

        const auto platformID = parser.read<PlatformID>("Platform ID");

        {
            const auto id = parser.peek<UInt16>();
            if (platformID == PlatformID::Windows) {
                parser.read<UInt16>("Encoding: " + winEncodingName(id));
            } else {
                parser.read<UInt16>("Encoding ID");
            }
        }

        {
            const auto id = parser.peek<UInt16>();
            if (platformID == PlatformID::Windows) {
                parser.read<UInt16>("Language: " + winLanguageName(id));
            } else {
                parser.read<UInt16>("Language ID");
            }
        }

        const auto nameId = parser.read<UInt16>("Name ID");
        const auto len = parser.read<UInt16>("String length");
        const auto offset = parser.read<Offset16>("String offset");
        parser.endGroup(QString("Record %1").arg(nameId));

        // Parse only Unicode names.
        if (platformID == PlatformID::Unicode || (platformID == PlatformID::Windows && WINDOWS_UNICODE_BMP_ENCODING_ID)) {
            nameOffsets.append({ static_cast<quint32>(offset), static_cast<quint32>(offset) + len });
        }
    }
    parser.endGroup();

    if (format == 1) {
        const auto count = parser.read<UInt16>("Number of language-tag records");
        parser.beginGroup("Language-tag records", QString::number(count));
        for (int i = 0; i < count; ++i) {
            parser.beginGroup(QString("Record %1").arg(i));
            parser.read<UInt16>("String length");
            parser.read<Offset16>("String offset");
            parser.endGroup();
        }
        parser.endGroup();
    }

    // Sort offsets.
    algo::sort_all(nameOffsets, [](const auto &a, const auto &b) { return a.start < b.start; });

    // Dedup offsets. There can be multiple records with the same offset.
    algo::dedup_vector(nameOffsets, [](const auto &a, const auto &b) {
        return a.start == b.start;
    });

    for (const auto [i, range]: algo::enumerate(nameOffsets)) {
        parser.jumpTo(tableStart + stringOffset + range->start);
        parser.readVariable<Utf16BE>(range->size(), QString("Record %1").arg(i)); // TODO: use name ID
    }
}
