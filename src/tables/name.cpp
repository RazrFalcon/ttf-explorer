#include "src/algo.h"
#include "name.h"
#include "tables.h"

const QString Name::PlatformID::Type = UInt16::Type;

static QString unicodeEncodingName(const quint16 id)
{
    switch (id) {
        case 0  : return QLatin1String("Unicode 1.0");
        case 1  : return QLatin1String("Unicode 1.1");
        case 2  : return QLatin1String("ISO/IEC 10646");
        case 3  : return QLatin1String("Unicode 2.0 BMP");
        case 4  : return QLatin1String("Unicode 2.0 full repertoire");
        case 5  : return QLatin1String("Unicode Variation Sequences");
        case 6  : return QLatin1String("Unicode full repertoire");
        default : return QLatin1String("Unknown");
    }
}

static QString winEncodingName(const quint16 id)
{
    switch (id) {
        case 0  : return QLatin1String("Symbol");
        case 1  : return QLatin1String("Unicode BMP");
        case 2  : return QLatin1String("ShiftJIS");
        case 3  : return QLatin1String("PRC");
        case 4  : return QLatin1String("Big5");
        case 5  : return QLatin1String("Wansung");
        case 6  : return QLatin1String("Johab");
        case 7  : return QLatin1String("Reserved");
        case 8  : return QLatin1String("Reserved");
        case 9  : return QLatin1String("Reserved");
        case 10 : return QLatin1String("Unicode full repertoire");
        default : return QLatin1String("Unknown");
    }
}

static const std::array<const char *, 33> MAC_ENCODING_NAMES {
    "Roman",
    "Japanese",
    "Chinese (Traditional)",
    "Korean",
    "Arabic",
    "Hebrew",
    "Greek",
    "Russian",
    "RSymbol",
    "Devanagari",
    "Gurmukhi",
    "Gujarati",
    "Oriya",
    "Bengali",
    "Tamil",
    "Telugu",
    "Kannada",
    "Malayalam",
    "Sinhalese",
    "Burmese",
    "Khmer",
    "Thai",
    "Laotian",
    "Georgian",
    "Armenian",
    "Chinese (Simplified)",
    "Tibetan",
    "Mongolian",
    "Geez",
    "Slavic",
    "Vietnamese",
    "Sindhi",
    "Uninterpreted",
};

static QString macEncodingName(const quint16 id)
{
    if (id < MAC_ENCODING_NAMES.size())  {
        return QString::fromLatin1(MAC_ENCODING_NAMES.at(id));
    } else {
        return QLatin1String("Unknown");
    }
}

static QString isoEncodingName(const quint16 id)
{
    switch (id) {
        case 0  : return QLatin1String("7-bit ASCII");
        case 1  : return QLatin1String("ISO 10646");
        case 2  : return QLatin1String("ISO 8859-1");
        default : return QLatin1String("Unknown");
    }
}

QString Name::encodingName(const PlatformID platform, const quint16 id)
{
    switch (platform) {
        case PlatformID::Unicode : return unicodeEncodingName(id);
        case PlatformID::Macintosh : return macEncodingName(id);
        case PlatformID::Iso : return isoEncodingName(id);
        case PlatformID::Windows : return winEncodingName(id);
        case PlatformID::Custom : return QString::number(id);
    }
}

// https://docs.microsoft.com/en-us/typography/opentype/spec/name#windows-language-ids
static QString winLanguageName(const quint16 id)
{
    switch (id) {
        case 0x0436 : return QLatin1String("Afrikaans, South Africa");
        case 0x041C : return QLatin1String("Albanian, Albania");
        case 0x0484 : return QLatin1String("Alsatian, France");
        case 0x045E : return QLatin1String("Amharic, Ethiopia");
        case 0x1401 : return QLatin1String("Arabic, Algeria");
        case 0x3C01 : return QLatin1String("Arabic, Bahrain");
        case 0x0C01 : return QLatin1String("Arabic, Egypt");
        case 0x0801 : return QLatin1String("Arabic, Iraq");
        case 0x2C01 : return QLatin1String("Arabic, Jordan");
        case 0x3401 : return QLatin1String("Arabic, Kuwait");
        case 0x3001 : return QLatin1String("Arabic, Lebanon");
        case 0x1001 : return QLatin1String("Arabic, Libya");
        case 0x1801 : return QLatin1String("Arabic, Morocco");
        case 0x2001 : return QLatin1String("Arabic, Oman");
        case 0x4001 : return QLatin1String("Arabic, Qatar");
        case 0x0401 : return QLatin1String("Arabic, Saudi Arabia");
        case 0x2801 : return QLatin1String("Arabic, Syria");
        case 0x1C01 : return QLatin1String("Arabic, Tunisia");
        case 0x3801 : return QLatin1String("Arabic, U.A.E.");
        case 0x2401 : return QLatin1String("Arabic, Yemen");
        case 0x042B : return QLatin1String("Armenian, Armenia");
        case 0x044D : return QLatin1String("Assamese, India");
        case 0x082C : return QLatin1String("Azeri (Cyrillic), Azerbaijan");
        case 0x042C : return QLatin1String("Azeri (Latin), Azerbaijan");
        case 0x046D : return QLatin1String("Bashkir, Russia");
        case 0x042D : return QLatin1String("Basque, Basque");
        case 0x0423 : return QLatin1String("Belarusian, Belarus");
        case 0x0845 : return QLatin1String("Bengali, Bangladesh");
        case 0x0445 : return QLatin1String("Bengali, India");
        case 0x201A : return QLatin1String("Bosnian (Cyrillic), Bosnia and Herzegovina");
        case 0x141A : return QLatin1String("Bosnian (Latin), Bosnia and Herzegovina");
        case 0x047E : return QLatin1String("Breton, France");
        case 0x0402 : return QLatin1String("Bulgarian, Bulgaria");
        case 0x0403 : return QLatin1String("Catalan, Catalan");
        case 0x0C04 : return QLatin1String("Chinese, Hong Kong S.A.R.");
        case 0x1404 : return QLatin1String("Chinese, Macao S.A.R.");
        case 0x0804 : return QLatin1String("Chinese, People's Republic of China");
        case 0x1004 : return QLatin1String("Chinese, Singapore");
        case 0x0404 : return QLatin1String("Chinese, Taiwan");
        case 0x0483 : return QLatin1String("Corsican, France");
        case 0x041A : return QLatin1String("Croatian, Croatia");
        case 0x101A : return QLatin1String("Croatian (Latin), Bosnia and Herzegovina");
        case 0x0405 : return QLatin1String("Czech, Czech Republic");
        case 0x0406 : return QLatin1String("Danish, Denmark");
        case 0x048C : return QLatin1String("Dari, Afghanistan");
        case 0x0465 : return QLatin1String("Divehi, Maldives");
        case 0x0813 : return QLatin1String("Dutch, Belgium");
        case 0x0413 : return QLatin1String("Dutch, Netherlands");
        case 0x0C09 : return QLatin1String("English, Australia");
        case 0x2809 : return QLatin1String("English, Belize");
        case 0x1009 : return QLatin1String("English, Canada");
        case 0x2409 : return QLatin1String("English, Caribbean");
        case 0x4009 : return QLatin1String("English, India");
        case 0x1809 : return QLatin1String("English, Ireland");
        case 0x2009 : return QLatin1String("English, Jamaica");
        case 0x4409 : return QLatin1String("English, Malaysia");
        case 0x1409 : return QLatin1String("English, New Zealand");
        case 0x3409 : return QLatin1String("English, Republic of the Philippines");
        case 0x4809 : return QLatin1String("English, Singapore");
        case 0x1C09 : return QLatin1String("English, South Africa");
        case 0x2C09 : return QLatin1String("English, Trinidad and Tobago");
        case 0x0809 : return QLatin1String("English, United Kingdom");
        case 0x0409 : return QLatin1String("English, United States");
        case 0x3009 : return QLatin1String("English, Zimbabwe");
        case 0x0425 : return QLatin1String("Estonian, Estonia");
        case 0x0438 : return QLatin1String("Faroese, Faroe Islands");
        case 0x0464 : return QLatin1String("Filipino, Philippines");
        case 0x040B : return QLatin1String("Finnish, Finland");
        case 0x080C : return QLatin1String("French, Belgium");
        case 0x0C0C : return QLatin1String("French, Canada");
        case 0x040C : return QLatin1String("French, France");
        case 0x140c : return QLatin1String("French, Luxembourg");
        case 0x180C : return QLatin1String("French, Principality of Monaco");
        case 0x100C : return QLatin1String("French, Switzerland");
        case 0x0462 : return QLatin1String("Frisian, Netherlands");
        case 0x0456 : return QLatin1String("Galician, Galician");
        case 0x0437 : return QLatin1String("Georgian, Georgia");
        case 0x0C07 : return QLatin1String("German, Austria");
        case 0x0407 : return QLatin1String("German, Germany");
        case 0x1407 : return QLatin1String("German, Liechtenstein");
        case 0x1007 : return QLatin1String("German, Luxembourg");
        case 0x0807 : return QLatin1String("German, Switzerland");
        case 0x0408 : return QLatin1String("Greek, Greece");
        case 0x046F : return QLatin1String("Greenlandic, Greenland");
        case 0x0447 : return QLatin1String("Gujarati, India");
        case 0x0468 : return QLatin1String("Hausa (Latin), Nigeria");
        case 0x040D : return QLatin1String("Hebrew, Israel");
        case 0x0439 : return QLatin1String("Hindi, India");
        case 0x040E : return QLatin1String("Hungarian, Hungary");
        case 0x040F : return QLatin1String("Icelandic, Iceland");
        case 0x0470 : return QLatin1String("Igbo, Nigeria");
        case 0x0421 : return QLatin1String("Indonesian, Indonesia");
        case 0x045D : return QLatin1String("Inuktitut, Canada");
        case 0x085D : return QLatin1String("Inuktitut (Latin), Canada");
        case 0x083C : return QLatin1String("Irish, Ireland");
        case 0x0434 : return QLatin1String("isiXhosa, South Africa");
        case 0x0435 : return QLatin1String("isiZulu, South Africa");
        case 0x0410 : return QLatin1String("Italian, Italy");
        case 0x0810 : return QLatin1String("Italian, Switzerland");
        case 0x0411 : return QLatin1String("Japanese, Japan");
        case 0x044B : return QLatin1String("Kannada, India");
        case 0x043F : return QLatin1String("Kazakh, Kazakhstan");
        case 0x0453 : return QLatin1String("Khmer, Cambodia");
        case 0x0486 : return QLatin1String("K’iche, Guatemala");
        case 0x0487 : return QLatin1String("Kinyarwanda, Rwanda");
        case 0x0441 : return QLatin1String("Kiswahili, Kenya");
        case 0x0457 : return QLatin1String("Konkani, India");
        case 0x0412 : return QLatin1String("Korean, Korea");
        case 0x0440 : return QLatin1String("Kyrgyz, Kyrgyzstan");
        case 0x0454 : return QLatin1String("Lao, Lao P.D.R.");
        case 0x0426 : return QLatin1String("Latvian, Latvia");
        case 0x0427 : return QLatin1String("Lithuanian, Lithuania");
        case 0x082E : return QLatin1String("Lower, Sorbian Germany");
        case 0x046E : return QLatin1String("Luxembourgish, Luxembourg");
        case 0x042F : return QLatin1String("Macedonian (FYROM), Former Yugoslav Republic of Macedoni)");
        case 0x083E : return QLatin1String("Malay, Brunei Darussalam");
        case 0x043E : return QLatin1String("Malay, Malaysia");
        case 0x044C : return QLatin1String("Malayalam, India");
        case 0x043A : return QLatin1String("Maltese, Malta");
        case 0x0481 : return QLatin1String("Maori, New Zealand");
        case 0x047A : return QLatin1String("Mapudungun, Chile");
        case 0x044E : return QLatin1String("Marathi, India");
        case 0x047C : return QLatin1String("Mohawk, Mohawk");
        case 0x0450 : return QLatin1String("Mongolian (Cyrillic), Mongolia");
        case 0x0850 : return QLatin1String("Mongolian (Traditional), People's Republic of China");
        case 0x0461 : return QLatin1String("Nepali, Nepal");
        case 0x0414 : return QLatin1String("Norwegian (Bokmal), Norway");
        case 0x0814 : return QLatin1String("Norwegian (Nynorsk), Norway");
        case 0x0482 : return QLatin1String("Occitan, France");
        case 0x0448 : return QLatin1String("Odia (formerly Oriya), India");
        case 0x0463 : return QLatin1String("Pashto, Afghanistan");
        case 0x0415 : return QLatin1String("Polish, Poland");
        case 0x0416 : return QLatin1String("Portuguese, Brazil");
        case 0x0816 : return QLatin1String("Portuguese, Portugal");
        case 0x0446 : return QLatin1String("Punjabi, India");
        case 0x046B : return QLatin1String("Quechua, Bolivia");
        case 0x086B : return QLatin1String("Quechua, Ecuador");
        case 0x0C6B : return QLatin1String("Quechua, Peru");
        case 0x0418 : return QLatin1String("Romanian, Romania");
        case 0x0417 : return QLatin1String("Romansh, Switzerland");
        case 0x0419 : return QLatin1String("Russian, Russia");
        case 0x243B : return QLatin1String("Sami (Inari), Finland");
        case 0x103B : return QLatin1String("Sami (Lule), Norway");
        case 0x143B : return QLatin1String("Sami (Lule), Sweden");
        case 0x0C3B : return QLatin1String("Sami (Northern), Finland");
        case 0x043B : return QLatin1String("Sami (Northern), Norway");
        case 0x083B : return QLatin1String("Sami (Northern), Sweden");
        case 0x203B : return QLatin1String("Sami (Skolt), Finland");
        case 0x183B : return QLatin1String("Sami (Southern), Norway");
        case 0x1C3B : return QLatin1String("Sami (Southern), Sweden");
        case 0x044F : return QLatin1String("Sanskrit, India");
        case 0x1C1A : return QLatin1String("Serbian (Cyrillic), Bosnia and Herzegovina");
        case 0x0C1A : return QLatin1String("Serbian (Cyrillic), Serbia");
        case 0x181A : return QLatin1String("Serbian (Latin), Bosnia and Herzegovina");
        case 0x081A : return QLatin1String("Serbian (Latin), Serbia");
        case 0x046C : return QLatin1String("Sesotho sa Leboa, South Africa");
        case 0x0432 : return QLatin1String("Setswana, South Africa");
        case 0x045B : return QLatin1String("Sinhala, Sri Lanka");
        case 0x041B : return QLatin1String("Slovak, Slovakia");
        case 0x0424 : return QLatin1String("Slovenian, Slovenia");
        case 0x2C0A : return QLatin1String("Spanish, Argentina");
        case 0x400A : return QLatin1String("Spanish, Bolivia");
        case 0x340A : return QLatin1String("Spanish, Chile");
        case 0x240A : return QLatin1String("Spanish, Colombia");
        case 0x140A : return QLatin1String("Spanish, Costa Rica");
        case 0x1C0A : return QLatin1String("Spanish, Dominican Republic");
        case 0x300A : return QLatin1String("Spanish, Ecuador");
        case 0x440A : return QLatin1String("Spanish, El Salvador");
        case 0x100A : return QLatin1String("Spanish, Guatemala");
        case 0x480A : return QLatin1String("Spanish, Honduras");
        case 0x080A : return QLatin1String("Spanish, Mexico");
        case 0x4C0A : return QLatin1String("Spanish, Nicaragua");
        case 0x180A : return QLatin1String("Spanish, Panama");
        case 0x3C0A : return QLatin1String("Spanish, Paraguay");
        case 0x280A : return QLatin1String("Spanish, Peru");
        case 0x500A : return QLatin1String("Spanish, Puerto Rico");
        case 0x0C0A : return QLatin1String("Spanish (Modern Sort), Spain");
        case 0x040A : return QLatin1String("Spanish (Traditional Sort), Spain");
        case 0x540A : return QLatin1String("Spanish, United States");
        case 0x380A : return QLatin1String("Spanish, Uruguay");
        case 0x200A : return QLatin1String("Spanish, Venezuela");
        case 0x081D : return QLatin1String("Sweden, Finland");
        case 0x041D : return QLatin1String("Swedish, Sweden");
        case 0x045A : return QLatin1String("Syriac, Syria");
        case 0x0428 : return QLatin1String("Tajik (Cyrillic), Tajikistan");
        case 0x085F : return QLatin1String("Tamazight (Latin), Algeria");
        case 0x0449 : return QLatin1String("Tamil, India");
        case 0x0444 : return QLatin1String("Tatar, Russia");
        case 0x044A : return QLatin1String("Telugu, India");
        case 0x041E : return QLatin1String("Thai, Thailand");
        case 0x0451 : return QLatin1String("Tibetan, PRC");
        case 0x041F : return QLatin1String("Turkish, Turkey");
        case 0x0442 : return QLatin1String("Turkmen, Turkmenistan");
        case 0x0480 : return QLatin1String("Uighur, PRC");
        case 0x0422 : return QLatin1String("Ukrainian, Ukraine");
        case 0x042E : return QLatin1String("Upper, Sorbian Germany");
        case 0x0420 : return QLatin1String("Urdu, Islamic Republic of Pakistan");
        case 0x0843 : return QLatin1String("Uzbek (Cyrillic), Uzbekistan");
        case 0x0443 : return QLatin1String("Uzbek (Latin), Uzbekistan");
        case 0x042A : return QLatin1String("Vietnamese, Vietnam");
        case 0x0452 : return QLatin1String("Welsh, United Kingdom");
        case 0x0488 : return QLatin1String("Wolof, Senegal");
        case 0x0485 : return QLatin1String("Yakut, Russia");
        case 0x0478 : return QLatin1String("Yi, PRC");
        case 0x046A : return QLatin1String("Yoruba, Nigeria");
        default     : return QLatin1String("Unknown");
    }
}

static QString macLanguageName(const quint16 id)
{
    switch (id) {
        case 0 :   return QLatin1String("English");
        case 1 :   return QLatin1String("French");
        case 2 :   return QLatin1String("German");
        case 3 :   return QLatin1String("Italian");
        case 4 :   return QLatin1String("Dutch");
        case 5 :   return QLatin1String("Swedish");
        case 6 :   return QLatin1String("Spanish");
        case 7 :   return QLatin1String("Danish");
        case 8 :   return QLatin1String("Portuguese");
        case 9 :   return QLatin1String("Norwegian");
        case 10 :  return QLatin1String("Hebrew");
        case 11 :  return QLatin1String("Japanese");
        case 12 :  return QLatin1String("Arabic");
        case 13 :  return QLatin1String("Finnish");
        case 14 :  return QLatin1String("Greek");
        case 15 :  return QLatin1String("Icelandic");
        case 16 :  return QLatin1String("Maltese");
        case 17 :  return QLatin1String("Turkish");
        case 18 :  return QLatin1String("Croatian");
        case 19 :  return QLatin1String("Chinese (Traditional)");
        case 20 :  return QLatin1String("Urdu");
        case 21 :  return QLatin1String("Hindi");
        case 22 :  return QLatin1String("Thai");
        case 23 :  return QLatin1String("Korean");
        case 24 :  return QLatin1String("Lithuanian");
        case 25 :  return QLatin1String("Polish");
        case 26 :  return QLatin1String("Hungarian");
        case 27 :  return QLatin1String("Estonian");
        case 28 :  return QLatin1String("Latvian");
        case 29 :  return QLatin1String("Sami");
        case 30 :  return QLatin1String("Faroese");
        case 31 :  return QLatin1String("Farsi/Persian");
        case 32 :  return QLatin1String("Russian");
        case 33 :  return QLatin1String("Chinese (Simplified)");
        case 34 :  return QLatin1String("Flemish");
        case 35 :  return QLatin1String("Irish Gaelic");
        case 36 :  return QLatin1String("Albanian");
        case 37 :  return QLatin1String("Romanian");
        case 38 :  return QLatin1String("Czech");
        case 39 :  return QLatin1String("Slovak");
        case 40 :  return QLatin1String("Slovenian");
        case 41 :  return QLatin1String("Yiddish");
        case 42 :  return QLatin1String("Serbian");
        case 43 :  return QLatin1String("Macedonian");
        case 44 :  return QLatin1String("Bulgarian");
        case 45 :  return QLatin1String("Ukrainian");
        case 46 :  return QLatin1String("Byelorussian");
        case 47 :  return QLatin1String("Uzbek");
        case 48 :  return QLatin1String("Kazakh");
        case 49 :  return QLatin1String("Azerbaijani (Cyrillic script)");
        case 50 :  return QLatin1String("Azerbaijani (Arabic script)");
        case 51 :  return QLatin1String("Armenian");
        case 52 :  return QLatin1String("Georgian");
        case 53 :  return QLatin1String("Moldavian");
        case 54 :  return QLatin1String("Kirghiz");
        case 55 :  return QLatin1String("Tajiki");
        case 56 :  return QLatin1String("Turkmen");
        case 57 :  return QLatin1String("Mongolian (Mongolian script)");
        case 58 :  return QLatin1String("Mongolian (Cyrillic script)");
        case 59 :  return QLatin1String("Pashto");
        case 60 :  return QLatin1String("Kurdish");
        case 61 :  return QLatin1String("Kashmiri");
        case 62 :  return QLatin1String("Sindhi");
        case 63 :  return QLatin1String("Tibetan");
        case 64 :  return QLatin1String("Nepali");
        case 65 :  return QLatin1String("Sanskrit");
        case 66 :  return QLatin1String("Marathi");
        case 67 :  return QLatin1String("Bengali");
        case 68 :  return QLatin1String("Assamese");
        case 69 :  return QLatin1String("Gujarati");
        case 70 :  return QLatin1String("Punjabi");
        case 71 :  return QLatin1String("Oriya");
        case 72 :  return QLatin1String("Malayalam");
        case 73 :  return QLatin1String("Kannada");
        case 74 :  return QLatin1String("Tamil");
        case 75 :  return QLatin1String("Telugu");
        case 76 :  return QLatin1String("Sinhalese");
        case 77 :  return QLatin1String("Burmese");
        case 78 :  return QLatin1String("Khmer");
        case 79 :  return QLatin1String("Lao");
        case 80 :  return QLatin1String("Vietnamese");
        case 81 :  return QLatin1String("Indonesian");
        case 82 :  return QLatin1String("Tagalog");
        case 83 :  return QLatin1String("Malay (Roman script)");
        case 84 :  return QLatin1String("Malay (Arabic script)");
        case 85 :  return QLatin1String("Amharic");
        case 86 :  return QLatin1String("Tigrinya");
        case 87 :  return QLatin1String("Galla");
        case 88 :  return QLatin1String("Somali");
        case 89 :  return QLatin1String("Swahili");
        case 90 :  return QLatin1String("Kinyarwanda/Ruanda");
        case 91 :  return QLatin1String("Rundi");
        case 92 :  return QLatin1String("Nyanja/Chewa");
        case 93 :  return QLatin1String("Malagasy");
        case 94 :  return QLatin1String("Esperanto");
        // Reserved.
        case 128 : return QLatin1String("Welsh");
        case 129 : return QLatin1String("Basque");
        case 130 : return QLatin1String("Catalan");
        case 131 : return QLatin1String("Latin");
        case 132 : return QLatin1String("Quechua");
        case 133 : return QLatin1String("Guarani");
        case 134 : return QLatin1String("Aymara");
        case 135 : return QLatin1String("Tatar");
        case 136 : return QLatin1String("Uighur");
        case 137 : return QLatin1String("Dzongkha");
        case 138 : return QLatin1String("Javanese (Roman script)");
        case 139 : return QLatin1String("Sundanese (Roman script)");
        case 140 : return QLatin1String("Galician");
        case 141 : return QLatin1String("Afrikaans");
        case 142 : return QLatin1String("Breton");
        case 143 : return QLatin1String("Inuktitut");
        case 144 : return QLatin1String("Scottish Gaelic");
        case 145 : return QLatin1String("Manx Gaelic");
        case 146 : return QLatin1String("Irish Gaelic (with dot above)");
        case 147 : return QLatin1String("Tongan");
        case 148 : return QLatin1String("Greek (polytonic)");
        case 149 : return QLatin1String("Greenlandic");
        case 150 : return QLatin1String("Azerbaijani (Roman script)");
        default  : return QLatin1String("Unknown");
    }
}

QString Name::languageName(const PlatformID platform, const quint16 id)
{
    switch (platform) {
        case PlatformID::Macintosh : return macLanguageName(id);
        case PlatformID::Windows : return winLanguageName(id);
        default: return QString::number(id);
    }
}

static const std::array<const char *, 25> RECORD_NAMES {
    "Copyright notice",
    "Family",
    "Subfamily",
    "Unique ID",
    "Full name",
    "Version",
    "PostScript",
    "Trademark",
    "Manufacturer",
    "Designer",
    "Description",
    "URL Vendor",
    "URL Designer",
    "License Description",
    "License Info URL",
    "Reserved"
    "Typographic Family",
    "Typographic Subfamily",
    "Compatible Full",
    "Sample text",
    "PostScript CID",
    "WWS Family",
    "WWS Subfamily",
    "Light Background Palette",
    "Dark Background Palette",
    "Variations PostScript Prefix",
};

static QString recordName(const quint16 id)
{
    if (id < RECORD_NAMES.size())  {
        return QString::fromLatin1(RECORD_NAMES.at(id));
    } else {
        return QLatin1String("Unknown");
    }
}

struct NameRecord
{
    Name::PlatformID platformId;
    quint16 encodingId;
    quint16 languageId;
    quint16 nameId;
    quint32 offset;
    quint32 length;
};

void parseName(Parser &parser)
{
    using namespace Name;

    const auto tableStart = parser.offset();

    const auto format = parser.read<UInt16>("Format");
    const auto count = parser.read<UInt16>("Number of records");
    const auto stringOffset = parser.read<Offset16>("Offset to string storage");

    QVector<NameRecord> nameRecords;
    parser.readArray("Name Records", count, [&](const auto index){
        parser.beginGroup();

        const auto platformId = parser.read<PlatformID>("Platform ID");

        const auto encodingId = parser.peek<UInt16>();
        parser.readValue<UInt16>("Encoding ID", encodingName(platformId, encodingId));

        const auto languageId = parser.peek<UInt16>();
        parser.readValue<UInt16>("Language ID", languageName(platformId, languageId));

        const auto nameId = parser.read<UInt16>("Name ID");
        const auto len = parser.read<UInt16>("String length");
        const auto offset = parser.read<Offset16>("String offset");
        parser.endGroup(numberToString(index));

        if (len == 0) {
            return;
        }

        nameRecords.append({
            platformId,
            encodingId,
            languageId,
            nameId,
            quint32(offset.d),
            quint32(len),
        });
    });

    if (format == 1) {
        const auto count = parser.read<UInt16>("Number of language-tag records");
        parser.readArray("Language-tag Records", count, [&](const auto index){
            parser.beginGroup(index);
            parser.read<UInt16>("String length");
            parser.read<Offset16>("String offset");
            parser.endGroup();
        });
    }

    // Sort offsets.
    algo::sort_all_by_key(nameRecords, &NameRecord::offset);

    // Dedup offsets. There can be multiple records with the same offset.
    algo::dedup_vector_by_key(nameRecords, &NameRecord::offset);

    // Remove overlapping ranges.
    {
        QVector<int> rmIdx;
        for (int i = 0; i < nameRecords.size(); i++) {
            const auto start1 = nameRecords[i].offset;
            const auto end1 = start1 + nameRecords[i].length;
            for (int j = i + 1; j < nameRecords.size(); j++) {
                const auto start2 = nameRecords[j].offset;
                const auto end2 = start2 + nameRecords[j].length;
                if ((start2 >= start1 && start2 < end1) ||
                    (end2 >= start1 && end2 < end1))
                {
                    rmIdx.append(j);
                }
            }
        }

        algo::sort_all(rmIdx);
        algo::dedup_vector(rmIdx);
        for (int i = rmIdx.size() - 1; i >= 0; i--) {
            nameRecords.remove(rmIdx[i]);
        }
    }

    parser.readArray("Names", nameRecords.size(), [&](const auto index){
        const auto record = nameRecords[index];
        parser.advanceTo(tableStart + stringOffset + record.offset);

        QString title;
        if (record.nameId < 26) {
            title = QString("%1 (%2, %3)")
                .arg(recordName(record.nameId))
                .arg(encodingName(record.platformId, record.encodingId))
                .arg(languageName(record.platformId, record.languageId));
        } else {
            title = QString("Record %1 (%2, %3)")
                .arg(record.nameId)
                .arg(encodingName(record.platformId, record.encodingId))
                .arg(languageName(record.platformId, record.languageId));
        }

        if (record.platformId == PlatformID::Unicode ||
           (record.platformId == PlatformID::Windows && record.encodingId == WINDOWS_UNICODE_BMP_ENCODING_ID))
        {
            parser.readUtf16String(title, record.length);
        } else if (record.platformId == PlatformID::Macintosh) {
            parser.readMacRomanString(title, record.length);
        } else {
            parser.readUnsupported(record.length);
        }
    });
}

NamesHash collectNameNames(ShadowParser &parser)
{
    using namespace Name;

    const auto tableStart = parser.offset();

    parser.read<UInt16>();
    const auto count = parser.read<UInt16>();
    const auto stringOffset = parser.read<Offset16>();

    QVector<NameRecord> nameRecords;
    for (quint16 i = 0; i < count; i++) {
        const auto platformId = parser.read<PlatformID>();
        const auto encodingId = parser.read<UInt16>();
        const auto languageId = parser.read<UInt16>();
        const auto nameId = parser.read<UInt16>();
        const auto len = parser.read<UInt16>();
        const auto offset = parser.read<Offset16>();

        if (len == 0) {
            continue;
        }

        nameRecords.append({
            platformId,
            encodingId,
            languageId,
            nameId,
            quint32(offset.d),
            quint32(len),
        });
    }

    NamesHash names;
    for (const auto record : nameRecords) {
        parser.jumpTo(tableStart + stringOffset + record.offset);

        if (record.platformId == PlatformID::Unicode ||
           (record.platformId == PlatformID::Windows && record.encodingId == WINDOWS_UNICODE_BMP_ENCODING_ID))
        {
            const auto name = parser.readUtf16String(record.length);
            names.insert(record.nameId, name);
        } else if (record.platformId == PlatformID::Macintosh && record.languageId == 0) {
            // Read only English names.
            const auto name = parser.readMacRomanString(record.length);
            names.insert(record.nameId, name);
        }
    }

    return names;
}
