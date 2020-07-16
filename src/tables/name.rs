use crate::parser::*;
use crate::{ValueType, Error, Result};

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
    const NAME: ValueType = ValueType::PlatformId;

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        match u16::parse(data)? {
            0 => Ok(PlatformId::Unicode),
            1 => Ok(PlatformId::Macintosh),
            2 => Ok(PlatformId::Iso),
            3 => Ok(PlatformId::Windows),
            4 => Ok(PlatformId::Custom),
            n => Err(Error::Custom(format!("{} is not a valid PlatformId", n))),
        }
    }
}

impl std::fmt::Display for PlatformId {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{:?}", self)
    }
}

fn record_name(id: u16) -> &'static str {
    match id {
        0 => "Copyright notice",
        1 => "Family",
        2 => "Subfamily",
        3 => "Unique ID",
        4 => "Full name",
        5 => "Version",
        6 => "PostScript",
        7 => "Trademark",
        8 => "Manufacturer",
        9 => "Designer",
        10 => "Description",
        11 => "URL Vendor",
        12 => "URL Designer",
        13 => "License Description",
        14 => "License Info URL",
        // 15 reserved
        16 => "Typographic Family",
        17 => "Typographic Subfamily",
        18 => "Compatible Full",
        19 => "Sample text",
        20 => "PostScript CID",
        21 => "WWS Family",
        22 => "WWS Subfamily",
        23 => "Light Background Palette",
        24 => "Dark Background Palette",
        25 => "Variations PostScript Prefix",
        _ => "Unknown"
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

    struct NameRecord {
        platform_id: PlatformId,
        encoding_id: u16,
        language_id: u16,
        name_id: u16,
        range: std::ops::Range<usize>,
    }

    let mut name_records = Vec::new();
    parser.begin_group_with_value("Name records", count.to_string());
    for _ in 0..count {
        parser.begin_group("");

        let platform_id = parser.read::<PlatformId>("Platform ID")?;

        let encoding_id = {
            let id = parser.peek::<u16>()?;
            parser.read_value(2, "Encoding ID", encoding_name(platform_id, id))?;
            id
        };

        let language_id = {
            let id = parser.peek::<u16>()?;
            parser.read_value(2, "Language ID", language_name(platform_id, id))?;
            id
        };

        let name_id = parser.read::<u16>("Name ID")?;
        let len = parser.read::<u16>("String length")? as usize;
        let offset = parser.read::<Offset16>("String offset")?.to_usize();
        parser.end_group_with_title(format!("Record {}", name_id));

        if len == 0 {
            continue;
        }

        name_records.push(NameRecord {
            platform_id,
            encoding_id,
            language_id,
            name_id,
            range: offset..offset + len,
        });
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
    name_records.sort_by_key(|r| r.range.start);
    name_records.dedup_by_key(|r| r.range.start);

    // Remove overlapping ranges.
    {
        let mut rm_idx = Vec::new();
        for i in 0..name_records.len() {
            let range1 = name_records[i].range.clone();
            for j in i + 1..name_records.len() {
                let range2 = name_records[j].range.clone();
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
            name_records.remove(*i);
        }
    }

    for name in name_records {
        parser.jump_to(table_start + string_offset + name.range.start)?;

        let title = if name.name_id < 26 {
            format!(
                "{} ({}, {})",
                record_name(name.name_id),
                encoding_name(name.platform_id, name.encoding_id),
                language_name(name.platform_id, name.language_id),
            )
        } else {
            format!(
                "Record {} ({}, {})",
                name.name_id,
                encoding_name(name.platform_id, name.encoding_id),
                language_name(name.platform_id, name.language_id),
            )
        };

        // Parse only Unicode names.
        if  name.platform_id == PlatformId::Unicode ||
           (name.platform_id == PlatformId::Windows && name.encoding_id == WINDOWS_UNICODE_BMP_ENCODING_ID)
        {
            let bytes = parser.peek_bytes(name.range.len())?;
            let mut name_data: Vec<u16> = Vec::new();
            let mut sparser = SimpleParser::new(bytes);
            while !sparser.at_end() {
                name_data.push(sparser.read()?);
            }

            if let Ok(utf8_name) = String::from_utf16(&name_data) {
                parser.read_value(name.range.len(), title, utf8_name)?;
            } else {
                parser.read_bytes(name.range.len(), title)?;
            }
        } else if name.platform_id == PlatformId::Macintosh {
            let bytes = parser.peek_bytes(name.range.len())?;
            let mut raw_data = Vec::with_capacity(bytes.len());
            for b in bytes {
                raw_data.push(MAC_ROMAN[*b as usize]);
            }

            if let Ok(s) = String::from_utf16(&raw_data) {
                parser.read_value(name.range.len(), title, s)?;
            } else {
                parser.read_bytes(name.range.len(), title)?;
            }
        } else {
            parser.read_bytes(name.range.len(), title)?;
        }
    }

    Ok(())
}


/// Macintosh Roman to UTF-16 encoding table.
///
/// https://en.wikipedia.org/wiki/Mac_OS_Roman
const MAC_ROMAN: &[u16; 256] = &[
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
    0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
    0x0010, 0x2318, 0x21E7, 0x2325, 0x2303, 0x0015, 0x0016, 0x0017,
    0x0018, 0x0019, 0x001A, 0x001B, 0x001C, 0x001D, 0x001E, 0x001F,
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
    0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
    0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
    0x00C4, 0x00C5, 0x00C7, 0x00C9, 0x00D1, 0x00D6, 0x00DC, 0x00E1,
    0x00E0, 0x00E2, 0x00E4, 0x00E3, 0x00E5, 0x00E7, 0x00E9, 0x00E8,
    0x00EA, 0x00EB, 0x00ED, 0x00EC, 0x00EE, 0x00EF, 0x00F1, 0x00F3,
    0x00F2, 0x00F4, 0x00F6, 0x00F5, 0x00FA, 0x00F9, 0x00FB, 0x00FC,
    0x2020, 0x00B0, 0x00A2, 0x00A3, 0x00A7, 0x2022, 0x00B6, 0x00DF,
    0x00AE, 0x00A9, 0x2122, 0x00B4, 0x00A8, 0x2260, 0x00C6, 0x00D8,
    0x221E, 0x00B1, 0x2264, 0x2265, 0x00A5, 0x00B5, 0x2202, 0x2211,
    0x220F, 0x03C0, 0x222B, 0x00AA, 0x00BA, 0x03A9, 0x00E6, 0x00F8,
    0x00BF, 0x00A1, 0x00AC, 0x221A, 0x0192, 0x2248, 0x2206, 0x00AB,
    0x00BB, 0x2026, 0x00A0, 0x00C0, 0x00C3, 0x00D5, 0x0152, 0x0153,
    0x2013, 0x2014, 0x201C, 0x201D, 0x2018, 0x2019, 0x00F7, 0x25CA,
    0x00FF, 0x0178, 0x2044, 0x20AC, 0x2039, 0x203A, 0xFB01, 0xFB02,
    0x2021, 0x00B7, 0x201A, 0x201E, 0x2030, 0x00C2, 0x00CA, 0x00C1,
    0x00CB, 0x00C8, 0x00CD, 0x00CE, 0x00CF, 0x00CC, 0x00D3, 0x00D4,
    0xF8FF, 0x00D2, 0x00DA, 0x00DB, 0x00D9, 0x0131, 0x02C6, 0x02DC,
    0x00AF, 0x02D8, 0x02D9, 0x02DA, 0x00B8, 0x02DD, 0x02DB, 0x02C7,
];
