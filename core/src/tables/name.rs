use crate::parser::*;
use crate::{Error, Result};

#[derive(Clone, Copy, PartialEq, Debug)]
pub enum PlatformId {
    Unicode,
    Macintosh,
    Iso,
    Windows,
    Custom,
}

impl FromData for PlatformId {
    const SIZE: usize = 2;
    const NAME: &'static str = "PlatformId";

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        match u16::parse(data)? {
            0 => Ok(PlatformId::Unicode),
            1 => Ok(PlatformId::Macintosh),
            2 => Ok(PlatformId::Iso),
            3 => Ok(PlatformId::Windows),
            4 => Ok(PlatformId::Custom),
            _ => Err(Error::InvalidValue),
        }
    }
}

impl std::fmt::Display for PlatformId {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{:?}", self)
    }
}

pub fn encoding_name(platform: PlatformId, encoding: u16) -> String {
    match platform {
        PlatformId::Unicode => unicode_encoding_name(encoding).to_string(),
        PlatformId::Macintosh => mac_encoding_name(encoding).to_string(),
        PlatformId::Iso => iso_encoding_name(encoding).to_string(),
        PlatformId::Windows => win_encoding_name(encoding).to_string(),
        PlatformId::Custom => ToString::to_string(&encoding),
    }
}

fn unicode_encoding_name(encoding: u16) -> &'static str {
    match encoding {
        0 => "Unicode 1.0",
        1 => "Unicode 1.1",
        2 => "ISO/IEC 10646",
        3 => "Unicode 2.0 BMP",
        4 => "Unicode 2.0 full repertoire",
        5 => "Unicode Variation Sequences",
        6 => "Unicode full repertoire",
        _ => "Unknown",
    }
}

fn win_encoding_name(encoding: u16) -> &'static str {
    match encoding {
        0  => "Symbol",
        1  => "Unicode BMP",
        2  => "ShiftJIS",
        3  => "PRC",
        4  => "Big5",
        5  => "Wansung",
        6  => "Johab",
        7  => "Reserved",
        8  => "Reserved",
        9  => "Reserved",
        10 => "Unicode full repertoire",
        _  => "Unknown",
    }
}

const MAC_ENCODING_NAMES: &[&str] = &[
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
];

fn mac_encoding_name(encoding: u16) -> &'static str {
    MAC_ENCODING_NAMES.get(encoding as usize).unwrap_or(&"Unknown")
}

fn iso_encoding_name(encoding: u16) -> &'static str {
    match encoding {
        0 => "7-bit ASCII",
        1 => "ISO 10646",
        2 => "ISO 8859-1",
        _ => "Unknown",
    }
}

pub fn language_name(platform: PlatformId, lang: u16) -> String {
    match platform {
        PlatformId::Macintosh => mac_language_name(lang).to_string(),
        PlatformId::Windows => win_language_name(lang).to_string(),
        _ => ToString::to_string(&lang),
    }
}

// https://docs.microsoft.com/en-us/typography/opentype/spec/name#windows-language-ids
fn win_language_name(lang: u16) -> &'static str {
    match lang {
        0x0436 => "Afrikaans, South Africa",
        0x041C => "Albanian, Albania",
        0x0484 => "Alsatian, France",
        0x045E => "Amharic, Ethiopia",
        0x1401 => "Arabic, Algeria",
        0x3C01 => "Arabic, Bahrain",
        0x0C01 => "Arabic, Egypt",
        0x0801 => "Arabic, Iraq",
        0x2C01 => "Arabic, Jordan",
        0x3401 => "Arabic, Kuwait",
        0x3001 => "Arabic, Lebanon",
        0x1001 => "Arabic, Libya",
        0x1801 => "Arabic, Morocco",
        0x2001 => "Arabic, Oman",
        0x4001 => "Arabic, Qatar",
        0x0401 => "Arabic, Saudi Arabia",
        0x2801 => "Arabic, Syria",
        0x1C01 => "Arabic, Tunisia",
        0x3801 => "Arabic, U.A.E.",
        0x2401 => "Arabic, Yemen",
        0x042B => "Armenian, Armenia",
        0x044D => "Assamese, India",
        0x082C => "Azeri (Cyrillic), Azerbaijan",
        0x042C => "Azeri (Latin), Azerbaijan",
        0x046D => "Bashkir, Russia",
        0x042D => "Basque, Basque",
        0x0423 => "Belarusian, Belarus",
        0x0845 => "Bengali, Bangladesh",
        0x0445 => "Bengali, India",
        0x201A => "Bosnian (Cyrillic), Bosnia and Herzegovina",
        0x141A => "Bosnian (Latin), Bosnia and Herzegovina",
        0x047E => "Breton, France",
        0x0402 => "Bulgarian, Bulgaria",
        0x0403 => "Catalan, Catalan",
        0x0C04 => "Chinese, Hong Kong S.A.R.",
        0x1404 => "Chinese, Macao S.A.R.",
        0x0804 => "Chinese, People’s Republic of China",
        0x1004 => "Chinese, Singapore",
        0x0404 => "Chinese, Taiwan",
        0x0483 => "Corsican, France",
        0x041A => "Croatian, Croatia",
        0x101A => "Croatian (Latin), Bosnia and Herzegovina",
        0x0405 => "Czech, Czech Republic",
        0x0406 => "Danish, Denmark",
        0x048C => "Dari, Afghanistan",
        0x0465 => "Divehi, Maldives",
        0x0813 => "Dutch, Belgium",
        0x0413 => "Dutch, Netherlands",
        0x0C09 => "English, Australia",
        0x2809 => "English, Belize",
        0x1009 => "English, Canada",
        0x2409 => "English, Caribbean",
        0x4009 => "English, India",
        0x1809 => "English, Ireland",
        0x2009 => "English, Jamaica",
        0x4409 => "English, Malaysia",
        0x1409 => "English, New Zealand",
        0x3409 => "English, Republic of the Philippines",
        0x4809 => "English, Singapore",
        0x1C09 => "English, South Africa",
        0x2C09 => "English, Trinidad and Tobago",
        0x0809 => "English, United Kingdom",
        0x0409 => "English, United States",
        0x3009 => "English, Zimbabwe",
        0x0425 => "Estonian, Estonia",
        0x0438 => "Faroese, Faroe Islands",
        0x0464 => "Filipino, Philippines",
        0x040B => "Finnish, Finland",
        0x080C => "French, Belgium",
        0x0C0C => "French, Canada",
        0x040C => "French, France",
        0x140c => "French, Luxembourg",
        0x180C => "French, Principality of Monaco",
        0x100C => "French, Switzerland",
        0x0462 => "Frisian, Netherlands",
        0x0456 => "Galician, Galician",
        0x0437 => "Georgian, Georgia",
        0x0C07 => "German, Austria",
        0x0407 => "German, Germany",
        0x1407 => "German, Liechtenstein",
        0x1007 => "German, Luxembourg",
        0x0807 => "German, Switzerland",
        0x0408 => "Greek, Greece",
        0x046F => "Greenlandic, Greenland",
        0x0447 => "Gujarati, India",
        0x0468 => "Hausa (Latin), Nigeria",
        0x040D => "Hebrew, Israel",
        0x0439 => "Hindi, India",
        0x040E => "Hungarian, Hungary",
        0x040F => "Icelandic, Iceland",
        0x0470 => "Igbo, Nigeria",
        0x0421 => "Indonesian, Indonesia",
        0x045D => "Inuktitut, Canada",
        0x085D => "Inuktitut (Latin), Canada",
        0x083C => "Irish, Ireland",
        0x0434 => "isiXhosa, South Africa",
        0x0435 => "isiZulu, South Africa",
        0x0410 => "Italian, Italy",
        0x0810 => "Italian, Switzerland",
        0x0411 => "Japanese, Japan",
        0x044B => "Kannada, India",
        0x043F => "Kazakh, Kazakhstan",
        0x0453 => "Khmer, Cambodia",
        0x0486 => "K’iche, Guatemala",
        0x0487 => "Kinyarwanda, Rwanda",
        0x0441 => "Kiswahili, Kenya",
        0x0457 => "Konkani, India",
        0x0412 => "Korean, Korea",
        0x0440 => "Kyrgyz, Kyrgyzstan",
        0x0454 => "Lao, Lao P.D.R.",
        0x0426 => "Latvian, Latvia",
        0x0427 => "Lithuanian, Lithuania",
        0x082E => "Lower, Sorbian Germany",
        0x046E => "Luxembourgish, Luxembourg",
        0x042F => "Macedonian (FYROM), Former Yugoslav Republic of Macedoni)",
        0x083E => "Malay, Brunei Darussalam",
        0x043E => "Malay, Malaysia",
        0x044C => "Malayalam, India",
        0x043A => "Maltese, Malta",
        0x0481 => "Maori, New Zealand",
        0x047A => "Mapudungun, Chile",
        0x044E => "Marathi, India",
        0x047C => "Mohawk, Mohawk",
        0x0450 => "Mongolian (Cyrillic), Mongolia",
        0x0850 => "Mongolian (Traditional), People’s Republic of China",
        0x0461 => "Nepali, Nepal",
        0x0414 => "Norwegian (Bokmal), Norway",
        0x0814 => "Norwegian (Nynorsk), Norway",
        0x0482 => "Occitan, France",
        0x0448 => "Odia (formerly Oriya), India",
        0x0463 => "Pashto, Afghanistan",
        0x0415 => "Polish, Poland",
        0x0416 => "Portuguese, Brazil",
        0x0816 => "Portuguese, Portugal",
        0x0446 => "Punjabi, India",
        0x046B => "Quechua, Bolivia",
        0x086B => "Quechua, Ecuador",
        0x0C6B => "Quechua, Peru",
        0x0418 => "Romanian, Romania",
        0x0417 => "Romansh, Switzerland",
        0x0419 => "Russian, Russia",
        0x243B => "Sami (Inari), Finland",
        0x103B => "Sami (Lule), Norway",
        0x143B => "Sami (Lule), Sweden",
        0x0C3B => "Sami (Northern), Finland",
        0x043B => "Sami (Northern), Norway",
        0x083B => "Sami (Northern), Sweden",
        0x203B => "Sami (Skolt), Finland",
        0x183B => "Sami (Southern), Norway",
        0x1C3B => "Sami (Southern), Sweden",
        0x044F => "Sanskrit, India",
        0x1C1A => "Serbian (Cyrillic), Bosnia and Herzegovina",
        0x0C1A => "Serbian (Cyrillic), Serbia",
        0x181A => "Serbian (Latin), Bosnia and Herzegovina",
        0x081A => "Serbian (Latin), Serbia",
        0x046C => "Sesotho sa Leboa, South Africa",
        0x0432 => "Setswana, South Africa",
        0x045B => "Sinhala, Sri Lanka",
        0x041B => "Slovak, Slovakia",
        0x0424 => "Slovenian, Slovenia",
        0x2C0A => "Spanish, Argentina",
        0x400A => "Spanish, Bolivia",
        0x340A => "Spanish, Chile",
        0x240A => "Spanish, Colombia",
        0x140A => "Spanish, Costa Rica",
        0x1C0A => "Spanish, Dominican Republic",
        0x300A => "Spanish, Ecuador",
        0x440A => "Spanish, El Salvador",
        0x100A => "Spanish, Guatemala",
        0x480A => "Spanish, Honduras",
        0x080A => "Spanish, Mexico",
        0x4C0A => "Spanish, Nicaragua",
        0x180A => "Spanish, Panama",
        0x3C0A => "Spanish, Paraguay",
        0x280A => "Spanish, Peru",
        0x500A => "Spanish, Puerto Rico",
        0x0C0A => "Spanish (Modern Sort), Spain",
        0x040A => "Spanish (Traditional Sort), Spain",
        0x540A => "Spanish, United States",
        0x380A => "Spanish, Uruguay",
        0x200A => "Spanish, Venezuela",
        0x081D => "Sweden, Finland",
        0x041D => "Swedish, Sweden",
        0x045A => "Syriac, Syria",
        0x0428 => "Tajik (Cyrillic), Tajikistan",
        0x085F => "Tamazight (Latin), Algeria",
        0x0449 => "Tamil, India",
        0x0444 => "Tatar, Russia",
        0x044A => "Telugu, India",
        0x041E => "Thai, Thailand",
        0x0451 => "Tibetan, PRC",
        0x041F => "Turkish, Turkey",
        0x0442 => "Turkmen, Turkmenistan",
        0x0480 => "Uighur, PRC",
        0x0422 => "Ukrainian, Ukraine",
        0x042E => "Upper, Sorbian Germany",
        0x0420 => "Urdu, Islamic Republic of Pakistan",
        0x0843 => "Uzbek (Cyrillic), Uzbekistan",
        0x0443 => "Uzbek (Latin), Uzbekistan",
        0x042A => "Vietnamese, Vietnam",
        0x0452 => "Welsh, United Kingdom",
        0x0488 => "Wolof, Senegal",
        0x0485 => "Yakut, Russia",
        0x0478 => "Yi, PRC",
        0x046A => "Yoruba, Nigeria",
        _      => "Unknown",
    }
}

fn mac_language_name(lang: u16) -> &'static str {
    match lang {
        0  => "English",
        1  => "French",
        2  => "German",
        3  => "Italian",
        4  => "Dutch",
        5  => "Swedish",
        6  => "Spanish",
        7  => "Danish",
        8  => "Portuguese",
        9  => "Norwegian",
        10 => "Hebrew",
        11 => "Japanese",
        12 => "Arabic",
        13 => "Finnish",
        14 => "Greek",
        15 => "Icelandic",
        16 => "Maltese",
        17 => "Turkish",
        18 => "Croatian",
        19 => "Chinese (Traditional)",
        20 => "Urdu",
        21 => "Hindi",
        22 => "Thai",
        23 => "Korean",
        24 => "Lithuanian",
        25 => "Polish",
        26 => "Hungarian",
        27 => "Estonian",
        28 => "Latvian",
        29 => "Sami",
        30 => "Faroese",
        31 => "Farsi/Persian",
        32 => "Russian",
        33 => "Chinese (Simplified)",
        34 => "Flemish",
        35 => "Irish Gaelic",
        36 => "Albanian",
        37 => "Romanian",
        38 => "Czech",
        39 => "Slovak",
        40 => "Slovenian",
        41 => "Yiddish",
        42 => "Serbian",
        43 => "Macedonian",
        44 => "Bulgarian",
        45 => "Ukrainian",
        46 => "Byelorussian",
        47 => "Uzbek",
        48 => "Kazakh",
        49 => "Azerbaijani (Cyrillic script)",
        50 => "Azerbaijani (Arabic script)",
        51 => "Armenian",
        52 => "Georgian",
        53 => "Moldavian",
        54 => "Kirghiz",
        55 => "Tajiki",
        56 => "Turkmen",
        57 => "Mongolian (Mongolian script)",
        58 => "Mongolian (Cyrillic script)",
        59 => "Pashto",
        60 => "Kurdish",
        61 => "Kashmiri",
        62 => "Sindhi",
        63 => "Tibetan",
        64 => "Nepali",
        65 => "Sanskrit",
        66 => "Marathi",
        67 => "Bengali",
        68 => "Assamese",
        69 => "Gujarati",
        70 => "Punjabi",
        71 => "Oriya",
        72 => "Malayalam",
        73 => "Kannada",
        74 => "Tamil",
        75 => "Telugu",
        76 => "Sinhalese",
        77 => "Burmese",
        78 => "Khmer",
        79 => "Lao",
        80 => "Vietnamese",
        81 => "Indonesian",
        82 => "Tagalog",
        83 => "Malay (Roman script)",
        84 => "Malay (Arabic script)",
        85 => "Amharic",
        86 => "Tigrinya",
        87 => "Galla",
        88 => "Somali",
        89 => "Swahili",
        90 => "Kinyarwanda/Ruanda",
        91 => "Rundi",
        92 => "Nyanja/Chewa",
        93 => "Malagasy",
        94 => "Esperanto",
        // Reserved.
        128 => "Welsh",
        129 => "Basque",
        130 => "Catalan",
        131 => "Latin",
        132 => "Quechua",
        133 => "Guarani",
        134 => "Aymara",
        135 => "Tatar",
        136 => "Uighur",
        137 => "Dzongkha",
        138 => "Javanese (Roman script)",
        139 => "Sundanese (Roman script)",
        140 => "Galician",
        141 => "Afrikaans",
        142 => "Breton",
        143 => "Inuktitut",
        144 => "Scottish Gaelic",
        145 => "Manx Gaelic",
        146 => "Irish Gaelic (with dot above)",
        147 => "Tongan",
        148 => "Greek (polytonic)",
        149 => "Greenlandic",
        150 => "Azerbaijani (Roman script)",
        _   => "Unknown",
    }
}

const WINDOWS_UNICODE_BMP_ENCODING_ID: u16 = 1;

pub fn parse(parser: &mut Parser) -> Result<()> {
    let table_start = parser.offset();

    let format = parser.read::<u16>("Format")?;
    let count = parser.read::<u16>("Count")?;
    let string_offset = parser.read::<Offset16>("Offset to string storage")?.to_usize();

    let mut name_ranges = Vec::new();
    parser.begin_group_with_value("Name records", count.to_string());
    for _ in 0..count {
        parser.begin_group("");

        let platform_id = parser.read::<PlatformId>("Platform ID")?;

        let encoding_id = {
            let id = parser.peek::<u16>()?;
            parser.read_value(2, "Encoding ID", encoding_name(platform_id, id))?;
            id
        };

        {
            let id = parser.peek::<u16>()?;
            parser.read_value(2, "Language ID", language_name(platform_id, id))?;
        }

        let name_id = parser.read::<u16>("Name ID")?;
        let len = parser.read::<u16>("String length")? as usize;
        let offset = parser.read::<Offset16>("String offset")?.to_usize();
        parser.end_group_with_title(format!("Record {}", name_id));

        // Parse only Unicode names.
        if  platform_id == PlatformId::Unicode ||
           (platform_id == PlatformId::Windows && encoding_id == WINDOWS_UNICODE_BMP_ENCODING_ID)
        {
            name_ranges.push(offset..offset + len);
        }
    }
    parser.end_group();

    if format == 1 {
        let count = parser.read::<u16>("Number of language-tag records")?;
        parser.begin_group_with_value("Language-tag records", count.to_string());
        for i in 0..count {
            parser.begin_group(format!("Record {}", i));
            parser.read::<u16>("String length")?;
            parser.read::<Offset16>("String offset")?;
            parser.end_group();
        }
        parser.end_group();
    }

    // Dedup offsets. There can be multiple records with the same offset.
    name_ranges.sort_by_key(|r| r.start);
    name_ranges.dedup_by_key(|r| r.start);

    // Remove overlapping ranges.
    {
        let mut rm_idx = Vec::new();
        for i in 0..name_ranges.len() {
            let range1 = name_ranges[i].clone();
            for j in i + 1..name_ranges.len() {
                let range2 = name_ranges[j].clone();
                if (range2.start >= range1.start && range2.start < range1.end) ||
                    (range2.end >= range1.start && range2.end < range1.end)
                {
                    rm_idx.push(j);
                }
            }
        }

        rm_idx.sort();
        rm_idx.dedup();
        for i in rm_idx.iter().rev() {
            name_ranges.remove(*i);
        }
    }

    for range in name_ranges {
        parser.jump_to(table_start + string_offset + range.start)?;
        let bytes = parser.peek_bytes(range.len())?;

        let mut name: Vec<u16> = Vec::new();
        let mut sparser = SimpleParser::new(bytes);
        while !sparser.at_end() {
            name.push(sparser.read()?);
        }

        let name = String::from_utf16(&name).map_err(|_| Error::InvalidValue)?;
        parser.read_value(range.len(), "Record", name)?;
    }

    Ok(())
}
