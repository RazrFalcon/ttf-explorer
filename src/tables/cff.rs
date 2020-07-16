use std::num::NonZeroU16;

use tinyvec::ArrayVec;

use crate::parser::*;
use crate::{TitleKind, ValueType, Error, Result};

mod dict_operator {
    pub const VERSION: u8 = 0;
    pub const NOTICE: u8 = 1;
    pub const FULL_NAME: u8 = 2;
    pub const FAMILY_NAME: u8 = 3;
    pub const WEIGHT: u8 = 4;
    pub const FONT_BBOX: u8 = 5;
    pub const BLUE_VALUES: u8 = 6;
    pub const OTHER_BLUES: u8 = 7;
    pub const FAMILY_BLUES: u8 = 8;
    pub const FAMILY_OTHER_BLUES: u8 = 9;
    pub const STD_HW: u8 = 10;
    pub const STD_VW: u8 = 11;
    pub const UNIQUE_ID: u8 = 13;
    pub const XUID: u8 = 14;
    pub const CHARSET: u8 = 15;
    pub const ENCODING: u8 = 16;
    pub const CHAR_STRINGS: u8 = 17;
    pub const PRIVATE: u8 = 18;
    pub const SUBRS: u8 = 19;
    pub const DEFAULT_WIDTH_X: u8 = 20;
    pub const NOMINAL_WIDTH_X: u8 = 21;

    pub const COPYRIGHT: u16 = 1200;
    pub const IS_FIXED_PITCH: u16 = 1201;
    pub const ITALIC_ANGLE: u16 = 1202;
    pub const UNDERLINE_POSITION: u16 = 1203;
    pub const UNDERLINE_THICKNESS: u16 = 1204;
    pub const PAINT_TYPE: u16 = 1205;
    pub const CHAR_STRING_TYPE: u16 = 1206;
    pub const FONT_MATRIX: u16 = 1207;
    pub const STROKE_WIDTH: u16 = 1208;
    pub const BLUE_SCALE: u16 = 1209;
    pub const BLUE_SHIFT: u16 = 1210;
    pub const BLUE_FUZZ: u16 = 1211;
    pub const STEM_SNAP_H: u16 = 1212;
    pub const STEM_SNAP_V: u16 = 1213;
    pub const FORCE_BOLD: u16 = 1214;
    pub const LANGUAGE_GROUP: u16 = 1217;
    pub const EXPANSION_FACTOR: u16 = 1218;
    pub const INITIAL_RANDOM_SEED: u16 = 1219;
    pub const SYNTHETIC_BASE: u16 = 1220;
    pub const POST_SCRIPT: u16 = 1221;
    pub const BASE_FONT_NAME: u16 = 1222;
    pub const BASE_FONT_BLEND: u16 = 1223;
    pub const ROS: u16 = 1230;
    pub const CID_FONT_VERSION: u16 = 1231;
    pub const CID_FONT_REVISION: u16 = 1232;
    pub const CID_FONT_TYPE: u16 = 1233;
    pub const CID_COUNT: u16 = 1234;
    pub const UID_BASE: u16 = 1235;
    pub const FD_ARRAY: u16 = 1236;
    pub const FD_SELECT: u16 = 1237;
    pub const FONT_NAME: u16 = 1238;
}

#[derive(Clone, Copy, Debug)]
pub enum OffsetSize {
    One,
    Two,
    Three,
    Four,
}

impl FromData for OffsetSize {
    const SIZE: usize = 1;
    const NAME: ValueType = ValueType::OffsetSize;

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        match data[0] {
            1 => Ok(OffsetSize::One),
            2 => Ok(OffsetSize::Two),
            3 => Ok(OffsetSize::Three),
            4 => Ok(OffsetSize::Four),
            n => Err(Error::Custom(format!("{} is not a valid OffsetSize", n))),
        }
    }
}

impl std::fmt::Display for OffsetSize {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let n = match self {
            OffsetSize::One => 1,
            OffsetSize::Two => 2,
            OffsetSize::Three => 3,
            OffsetSize::Four => 4,
        };

        write!(f, "{}", n)
    }
}


#[derive(Default)]
pub struct DictRecord {
    pub op: u16,
    pub operands: Vec<f32>,
}

#[derive(Default)]
pub struct Dict {
    pub records: Vec<DictRecord>,
}

impl Dict {
    pub fn get(&self, op: u16) -> Option<&[f32]> {
        self.records.iter().find(|r| r.op == op).map(|rec| rec.operands.as_slice())
    }
}


pub fn parse(parser: &mut Parser) -> Result<()> {
    let table_start = parser.offset();

    parser.begin_group("Header");
    parser.read::<u8>("Major version")?;
    parser.read::<u8>("Minor version")?;
    let header_size = parser.read::<u8>("Header size")? as usize;
    parser.read::<u8>("Absolute offset")?;
    parser.end_group();

    if header_size > 4 {
        parser.read_bytes(header_size - 4, "Padding")?;
    }

    parser.begin_group("Name INDEX");
    parse_index(parser, |start, end, index, parser| {
        parser.read_string(end - start, TitleKind::Name, Some(index as u32))?;
        Ok(())
    })?;
    parser.end_group();

    let mut top_dict = Dict::default();
    parser.begin_group("Top DICT INDEX");
    parse_index(parser, |start, end, _, parser| {
        // TODO: this
        // if index != 0 {
        //     throw "Top DICT INDEX should have only one dictionary";
        // }

        top_dict = parse_dict(end - start, parser)?;
        Ok(())
    })?;
    parser.end_group();

    parser.begin_group("String INDEX");
    parse_index(parser, |start, end, index, parser| {
        parser.read_string(end - start, TitleKind::String, Some(index as u32))?;
        Ok(())
    })?;
    parser.end_group();

    parser.begin_group("Global Subr INDEX");
    parse_index(parser, parse_subr)?;
    parser.end_group();

    // TODO: Encodings

    // 'The number of glyphs is the value of the count field in the CharStrings INDEX.'
    let mut number_of_glyphs = 0;
    if let Some(cs_operands) = top_dict.get(dict_operator::CHAR_STRINGS as u16) {
        if cs_operands.len() != 1 || cs_operands[0] < 0.0 {
            return Err(Error::Custom("invalid charstrings offset".to_string()));
        }

        let offset = cs_operands[0] as usize;
        parser.jump_to(table_start + offset)?;
        number_of_glyphs = parser.peek()?;
    }

    if let Some(operands) = top_dict.get(dict_operator::CHARSET as u16) {
        if operands.len() != 1 || operands[0] < 0.0 {
            return Err(Error::Custom("invalid charset offset".to_string()));
        }

        // The are no charsets when number of glyphs is zero.
        if let Some(number_of_glyphs) = NonZeroU16::new(number_of_glyphs) {
            let offset = operands[0] as usize;
            parser.jump_to(table_start + offset)?;
            parser.begin_group("Charsets");
            parse_charset(number_of_glyphs, parser)?;
            parser.end_group();
        }
    }

    if let Some(operands) = top_dict.get(dict_operator::FD_SELECT) {
        if operands.len() != 1 || operands[0] < 0.0 {
            return Err(Error::Custom("invalid FDSelect offset".to_string()));
        }

        if let Some(number_of_glyphs) = NonZeroU16::new(number_of_glyphs) {
            let offset = operands[0] as usize;
            parser.jump_to(table_start + offset)?;
            parser.begin_group("FDSelect");
            parse_fd_select(number_of_glyphs, parser)?;
            parser.end_group();
        }
    }

    if let Some(operands) = top_dict.get(dict_operator::CHAR_STRINGS as u16) {
        if operands.len() != 1 || operands[0] < 0.0 {
            return Err(Error::Custom("invalid charstrings offset".to_string()));
        }

        let offset = operands[0] as usize;
        parser.jump_to(table_start + offset)?;
        parser.begin_group("CharStrings INDEX");
        parse_index(parser, parse_subr)?;
        parser.end_group();
    }

    let mut local_subrs_offset = None;
    if let Some(operands) = top_dict.get(dict_operator::PRIVATE as u16) {
        if operands.len() != 2 || operands[0] < 0.0 || operands[1] < 0.0 {
            return Err(Error::Custom("invalid private dict operands".to_string()));
        }

        let len = operands[0] as usize;
        let private_dict_offset = table_start + operands[1] as usize;

        parser.jump_to(private_dict_offset)?;
        parser.begin_group("Private DICT");
        let private_dict = parse_dict(len, parser)?;
        parser.end_group();

        if let Some(subrs_operands) = private_dict.get(dict_operator::SUBRS as u16) {
            if subrs_operands.len() != 1 || subrs_operands[0] < 0.0 {
                return Err(Error::Custom("invalid local subrs offset".to_string()));
            }

            let offset = subrs_operands[0] as usize;
            // 'The local subroutines offset is relative to the beginning
            // of the Private DICT data.'
            local_subrs_offset = Some(private_dict_offset + offset);
        }
    }

    if let Some(offset) = local_subrs_offset {
        parser.jump_to(offset)?;
        parser.begin_group("Local Subr INDEX");
        parse_index(parser, parse_subr)?;
        parser.end_group();
    }

    if let Some(operands) = top_dict.get(dict_operator::FD_ARRAY) {
        if operands.len() != 1 || operands[0] < 0.0 {
            return Err(Error::Custom("invalid FDArray offset".to_string()));
        }

        let mut private_dict_ranges = Vec::new();
        let offset = operands[0] as usize;
        parser.jump_to(table_start + offset)?;
        parser.begin_group("FDArray");
        parse_index(parser, |start, end, index, parser| {
            parser.begin_group_with_index("Font DICT".into(), index as u32);
            let font_dict = parse_dict(end - start, parser)?;
            parser.end_group();

            if let Some(operands) = font_dict.get(dict_operator::PRIVATE as u16) {
                if operands.len() != 2 || operands[0] < 0.0 || operands[1] < 0.0 {
                    return Err(Error::Custom("invalid private dict operands".to_string()));
                }

                let len = operands[0] as usize;
                let private_dict_offset = table_start + operands[1] as usize;

                private_dict_ranges.push(private_dict_offset..private_dict_offset+len);
            }

            Ok(())
        })?;
        parser.end_group();

        for (i, range) in private_dict_ranges.iter().enumerate() {
            parser.jump_to(range.start)?;
            parser.begin_group_with_index("Private DICT".into(), i as u32);
            let private_dict = parse_dict(range.len(), parser)?;
            parser.end_group();

            if let Some(subrs_operands) = private_dict.get(dict_operator::SUBRS as u16) {
                if subrs_operands.len() != 1 || subrs_operands[0] < 0.0 {
                    return Err(Error::Custom("invalid local subrs offset".to_string()));
                }

                let offset = subrs_operands[0] as usize;
                // 'The local subroutines offset is relative to the beginning
                // of the Private DICT data.'
                parser.jump_to(range.start + offset)?;
                parser.begin_group_with_index("Local Subr INDEX".into(), i as u32);
                parse_index(parser, parse_subr)?;
                parser.end_group();
            }
        }
    }

    Ok(())
}

fn parse_index<P>(parser: &mut Parser, mut p: P) -> Result<()>
    where P: FnMut(usize, usize, usize, &mut Parser) -> Result<()>
{
    let count = parser.read::<u16>("Count")? as u32;
    if count == 0 {
        return Ok(());
    }

    let offset_size = parser.read::<OffsetSize>("Offset size")?;

    parser.begin_group_with_value("Indexes", (count + 1).to_string());
    let mut offsets = Vec::new();
    // INDEX has one more index at the end to indicate data length, so we have to add 1 to count.
    for i in 0..u32::from(count+1) {
        let offset = match offset_size {
            OffsetSize::One => parser.read_index::<u8>(TitleKind::Index, i)? as usize,
            OffsetSize::Two => parser.read_index::<u16>(TitleKind::Index, i)? as usize,
            OffsetSize::Three => parser.read_index::<U24>(TitleKind::Index, i)?.0 as usize,
            OffsetSize::Four => parser.read_index::<u32>(TitleKind::Index, i)? as usize,
        };
        offsets.push(offset);
    }

    parser.end_group();

    for i in 1..offsets.len() {
        // All offsets start from 1 and not 0, so we have to shift them.
        let start = offsets[i - 1] - 1;
        let end = offsets[i] - 1;
        if start == end {
            continue;
        }

        let parser_start = parser.offset();
        p(start, end, i - 1, parser)?;

        let diff = (parser.offset() - parser_start) as isize - (end - start) as isize;
        if diff < 0 {
            parser.read_bytes(diff.abs() as usize, "Padding")?;
        } else if diff > 0 {
            // TODO: better error
            return Err(Error::ReadOutOfBounds);
        }
    }

    Ok(())
}

fn parse_dict(len: usize, parser: &mut Parser) -> Result<Dict> {
    let mut dict = Dict::default();

    parser.begin_group("");

    let mut curr_record = DictRecord::default();

    let global_end = parser.offset() + len;
    while parser.offset() < global_end {
        let op1 = parser.peek::<u8>()?;
        if op1 == 12 {
            let op2 = parser.peek_at::<u8>(1)? as u16;

            let title = match 1200 + op2 {
                dict_operator::COPYRIGHT => "Copyright",
                dict_operator::IS_FIXED_PITCH => "Is fixed pitch",
                dict_operator::ITALIC_ANGLE => "Italic angle",
                dict_operator::UNDERLINE_POSITION => "Underline position",
                dict_operator::UNDERLINE_THICKNESS => "Underline thickness",
                dict_operator::PAINT_TYPE => "Paint type",
                dict_operator::CHAR_STRING_TYPE => "Charstring type",
                dict_operator::FONT_MATRIX => "Font matrix",
                dict_operator::STROKE_WIDTH => "Stroke width",
                dict_operator::BLUE_SCALE => "Blue scale",
                dict_operator::BLUE_SHIFT => "Blue shift",
                dict_operator::BLUE_FUZZ => "Blue fuzz",
                dict_operator::STEM_SNAP_H => "Stem snap H",
                dict_operator::STEM_SNAP_V => "Stem snap V",
                dict_operator::FORCE_BOLD => "Force bold",
                dict_operator::LANGUAGE_GROUP => "Language group",
                dict_operator::EXPANSION_FACTOR => "Expansion factor",
                dict_operator::INITIAL_RANDOM_SEED => "Initial random seed",
                dict_operator::SYNTHETIC_BASE => "Synthetic base",
                dict_operator::POST_SCRIPT => "PostScript",
                dict_operator::BASE_FONT_NAME => "Base font name",
                dict_operator::BASE_FONT_BLEND => "Base font blend",
                dict_operator::ROS => "ROS",
                dict_operator::CID_FONT_VERSION => "CID font version",
                dict_operator::CID_FONT_REVISION => "CID font revision",
                dict_operator::CID_FONT_TYPE => "CID font type",
                dict_operator::CID_COUNT => "CID count",
                dict_operator::UID_BASE => "UID base",
                dict_operator::FD_ARRAY => "FD array",
                dict_operator::FD_SELECT => "FD select",
                dict_operator::FONT_NAME => "Font name",
                _ => "Unknown",
            };

            parser.read::<u16>("Operator")?;

            // Keep only known operators.
            if title != "Unknown" {
                curr_record.op = 1200 + op2;
                dict.records.push(curr_record);
            }

            curr_record = DictRecord::default();

            parser.end_group_with_title(title);
            if parser.offset() != global_end {
                parser.begin_group("");
            }
        } else if op1 <= 21 {
            // 0..=21 bytes are operators.

            let title = match op1 {
                dict_operator::VERSION => "Version",
                dict_operator::NOTICE => "Notice",
                dict_operator::FULL_NAME => "Full name",
                dict_operator::FAMILY_NAME => "Family name",
                dict_operator::WEIGHT => "Weight",
                dict_operator::FONT_BBOX => "Font bbox",
                dict_operator::BLUE_VALUES => "Blue values",
                dict_operator::OTHER_BLUES => "Other blues",
                dict_operator::FAMILY_BLUES => "Family blues",
                dict_operator::FAMILY_OTHER_BLUES => "Family other blues",
                dict_operator::STD_HW => "Std HW",
                dict_operator::STD_VW => "Std VW",
                dict_operator::UNIQUE_ID => "Unique ID",
                dict_operator::XUID => "XUID",
                dict_operator::CHARSET => "charset",
                dict_operator::ENCODING => "Encoding",
                dict_operator::CHAR_STRINGS => "CharStrings",
                dict_operator::PRIVATE => "Private",
                dict_operator::SUBRS => "Local subroutines",
                dict_operator::DEFAULT_WIDTH_X => "Default width X",
                dict_operator::NOMINAL_WIDTH_X => "Nominal width X",
                _ => "Unknown",
            };

            parser.read::<u8>("Operator")?;

            // Keep only known operators.
            if title != "Unknown" {
                curr_record.op = op1 as u16;
                dict.records.push(curr_record);
            }

            curr_record = DictRecord::default();

            parser.end_group_with_title(title);
            if parser.offset() != global_end {
                parser.begin_group("Value");
            }
        } else if op1 == 28 {
            let mut shadow = parser.to_simple();
            shadow.read::<u8>()?;
            let n = shadow.read::<i16>()?;
            parser.read_value(3, TitleKind::Number, n.to_string())?;

            curr_record.operands.push(n as f32);
        } else if op1 == 29 {
            let mut shadow = parser.to_simple();
            shadow.read::<u8>()?;
            let n = shadow.read::<i32>()?;
            parser.read_value(5, TitleKind::Number, n.to_string())?;

            curr_record.operands.push(n as f32);
        } else if op1 == 30 {
            let mut shadow = parser.to_simple();
            shadow.read::<u8>()?;
            let n = parse_float(&mut shadow)?;
            let num_len = shadow.offset() ;
            parser.read_value(num_len, TitleKind::Number, n.to_string())?;

            curr_record.operands.push(n);
        } else if op1 >= 32 && op1 <= 246 {
            let n = op1 as i32 - 139;
            parser.read_value(1, TitleKind::Number, n.to_string())?;

            curr_record.operands.push(n as f32);
        } else if op1 >= 247 && op1 <= 250 {
            let mut shadow = parser.to_simple();
            let b0 = shadow.read::<u8>()?;
            let b1 = shadow.read::<u8>()?;
            let n = ((b0 as i32) - 247) * 256 + (b1 as i32) + 108;
            parser.read_value(2, TitleKind::Number, n.to_string())?;

            curr_record.operands.push(n as f32);
        } else if op1 >= 251 && op1 <= 254 {
            let mut shadow = parser.to_simple();
            let b0 = shadow.read::<u8>()?;
            let b1 = shadow.read::<u8>()?;
            let n = -((b0 as i32) - 251) * 256 - (b1 as i32) - 108;
            parser.read_value(2, TitleKind::Number, n.to_string())?;

            curr_record.operands.push(n as f32);
        }
    }

    Ok(dict)
}

fn parse_charset(number_of_glyphs: NonZeroU16, parser: &mut Parser) -> Result<()> {
    // -1, since `.notdef` is omitted.

    let format: u8 = parser.read("Format")?;
    match format {
        0 => {
            parser.read_array::<u16>("Glyph name array", TitleKind::StringId,
                                     number_of_glyphs.get() as usize - 1)?;
            Ok(())
        }
        1 => {
            // The number of ranges is not defined, so we have to
            // read until no glyphs are left.
            let mut left = number_of_glyphs.get() - 1;
            while left > 0 {
                parser.begin_group("Range");
                parser.read::<u16>("First glyph")?;
                left -= parser.read::<u8>("Glyphs left")? as u16 + 1;
                parser.end_group();
            }

            Ok(())
        }
        2 => {
            // The same as format1, but uses u16 instead.
            let mut left = number_of_glyphs.get() - 1;
            while left > 0 {
                parser.begin_group("Range");
                parser.read::<u16>("First glyph")?;
                left -= parser.read::<u16>("Glyphs left")? + 1;
                parser.end_group();
            }

            Ok(())
        }
        _ => {
            Err(Error::Custom(format!("{} is not a valid charset format", format)))
        }
    }
}

fn parse_fd_select(number_of_glyphs: NonZeroU16, parser: &mut Parser) -> Result<()> {
    let format: u8 = parser.read("Format")?;
    match format {
        0 => {
            parser.read_array::<u8>("FD selector array", TitleKind::Index,
                                    number_of_glyphs.get() as usize)?;
            Ok(())
        }
        3 => {
            let num_of_ranges = parser.read::<u16>("Number of ranges")?;
            for i in 0..num_of_ranges {
                parser.begin_group_with_index("Range".into(), i as u32);
                parser.read::<u16>("First glyph")?;
                parser.read::<u8>("FD index")?;
                parser.end_group();
            }

            parser.read::<u16>("Sentinel GID")?;

            Ok(())
        }
        _ => {
            Err(Error::Custom(format!("{} is not a valid FDSelect format", format)))
        }
    }
}

fn parse_subr(start: usize, end: usize, index: usize, parser: &mut Parser) -> Result<()> {
    if start > end {
        return Err(Error::Custom("invalid subroutine data".to_string()));
    }

    // TODO: does 1 byte subroutines are malformed?
    if end - start < 2 {
        // Skip empty.
        return Ok(());
    }

    parser.begin_group_with_index(TitleKind::Subroutine, index as u32);

    let global_end = parser.offset() + (end - start);

    while parser.offset() < global_end {
        let b0 = parser.peek::<u8>()?;
        match b0 {
            0  => { parser.read::<u8>("Reserved")?; }
            1  => { parser.read::<u8>("Horizontal stem (hstem)")?; }
            2  => { parser.read::<u8>("Reserved")?; }
            3  => { parser.read::<u8>("Vertical stem (vstem)")?; }
            4  => { parser.read::<u8>("Vertical move to (vmoveto)")?; }
            5  => { parser.read::<u8>("Line to (rlineto)")?; }
            6  => { parser.read::<u8>("Horizontal line to (hlineto)")?; }
            7  => { parser.read::<u8>("Vertical line to (vlineto)")?; }
            8  => { parser.read::<u8>("Curve to (rrcurveto)")?; }
            9  => { parser.read::<u8>("Reserved")?; }
            10 => { parser.read::<u8>("Call local subroutine (callsubr)")?; }
            11 => {
                parser.read::<u8>("Return (return)")?;
                break;
            }
            12 => {
                let b1 = parser.peek_at::<u8>(1)?;
                match b1 {
                    3 => parser.read::<u16>("(and)")?,
                    4 => parser.read::<u16>("(or)")?,
                    5 => parser.read::<u16>("(not)")?,
                    9 => parser.read::<u16>("(abs)")?,
                    10 => parser.read::<u16>("(add)")?,
                    11 => parser.read::<u16>("(sub)")?,
                    12 => parser.read::<u16>("(div)")?,
                    14 => parser.read::<u16>("(neg)")?,
                    15 => parser.read::<u16>("(eq)")?,
                    18 => parser.read::<u16>("(drop)")?,
                    20 => parser.read::<u16>("(put)")?,
                    21 => parser.read::<u16>("(get)")?,
                    22 => parser.read::<u16>("(ifelse)")?,
                    23 => parser.read::<u16>("(random)")?,
                    24 => parser.read::<u16>("(mul)")?,
                    26 => parser.read::<u16>("(sqrt)")?,
                    27 => parser.read::<u16>("(dup)")?,
                    28 => parser.read::<u16>("(exch)")?,
                    29 => parser.read::<u16>("(index)")?,
                    30 => parser.read::<u16>("(roll)")?,
                    34 => parser.read::<u16>("Horizontal flex (hflex)")?,
                    35 => parser.read::<u16>("Flex (flex)")?,
                    36 => parser.read::<u16>("Horizontal flex 1 (hflex1)")?,
                    37 => parser.read::<u16>("Flex 1 (flex1)")?,
                    _ => parser.read::<u16>("Reserved")?,
                };
            }
            13 => { parser.read::<u8>("Reserved")?; }
            14 => {
                parser.read::<u8>("Endchar (endchar)")?;
                break;
            }
            15 | 16 | 17 => { parser.read::<u8>("Reserved")?; }
            18 => { parser.read::<u8>("Horizontal stem hint mask (hstemhm)")?; }
            19 => { parser.read::<u8>("Hint mask (hintmask)")?; }
            20 => { parser.read::<u8>("Counter mask (cntrmask)")?; }
            21 => { parser.read::<u8>("Move to (rmoveto)")?; }
            22 => { parser.read::<u8>("Horizontal move to (hmoveto)")?; }
            23 => { parser.read::<u8>("Vertical stem hint mask (vstemhm)")?; }
            24 => { parser.read::<u8>("Curve line (rcurveline)")?; }
            25 => { parser.read::<u8>("Line curve (rlinecurve)")?; }
            26 => { parser.read::<u8>("Vertical vertical curve to (vvcurveto)")?; }
            27 => { parser.read::<u8>("Horizontal horizontal curve to (hhcurveto)")?; }
            28 => {
                if parser.offset() + 3 > global_end {
                    break;
                }

                let b1 = parser.peek_at::<u8>(1)?;
                let b2 = parser.peek_at::<u8>(2)?;
                let n = ((b1 as i32) << 24 | (b2 as i32) << 16) >> 16;
                parser.read_value(3, TitleKind::Number, n.to_string())?;
            }
            29 => { parser.read::<u8>("Call global subroutine (callgsubr)")?; }
            30 => { parser.read::<u8>("Vertical horizontal curve to (vhcurveto)")?; }
            31 => { parser.read::<u8>("Horizontal vertical curve to (hvcurveto)")?; }
            32..=246 => parser.read_value(1, TitleKind::Number, ((b0 as i32) - 139).to_string())?,
            247..=250 => {
                if parser.offset() + 2 > global_end {
                    break;
                }

                let b1 = parser.peek_at::<u8>(1)?;
                let n = ((b0 as i32) - 247) * 256 + (b1 as i32) + 108;
                parser.read_value(2, TitleKind::Number, n.to_string())?;
            }
            251..=254 => {
                if parser.offset() + 2 > global_end {
                    break;
                }

                let b1 = parser.peek_at::<u8>(1)?;
                let n = -((b0 as i32) - 251) * 256 - (b1 as i32) - 108;
                parser.read_value(2, TitleKind::Number, n.to_string())?;
            }
            255 => {
                if parser.offset() + 5 > global_end {
                    break;
                }

                let n = parser.peek_at::<u32>(1)? as f32 / 65536.0;
                parser.read_value(5, TitleKind::Number, n.to_string())?;
            }
        };
    }

    parser.end_group();

    Ok(())
}

const END_OF_FLOAT_FLAG: u8 = 0xf;
const FLOAT_STACK_LEN: u8 = 64;

fn parse_float_nibble(nibble: u8, stack: &mut ArrayVec<[u8; 64]>) -> Result<()> {
    if stack.len() == FLOAT_STACK_LEN as usize {
        return Err(Error::Custom("invalid float".to_string()));
    }

    match nibble {
        0..=9 => stack.push(b'0' + nibble),
        10 => stack.push(b'.'),
        11 => stack.push(b'E'),
        12 => {
            if stack.len() + 1 == FLOAT_STACK_LEN as usize {
                return Err(Error::Custom("invalid float".to_string()));
            }

            stack.push(b'E');
            stack.push(b'-');
        }
        14 => stack.push(b'-'),
        _ => return Err(Error::Custom("invalid float".to_string())),
    }

    Ok(())
}

pub fn parse_float(parser: &mut SimpleParser) -> Result<f32> {
    let mut stack = ArrayVec::from([0; FLOAT_STACK_LEN as usize]);
    stack.clear();
    while !parser.at_end() {
        let b1 = parser.read::<u8>()?;
        let nibble1 = b1 >> 4;
        let nibble2 = b1 & 15;

        if nibble1 == END_OF_FLOAT_FLAG {
            break;
        }

        parse_float_nibble(nibble1, &mut stack)?;

        if nibble2 == END_OF_FLOAT_FLAG {
            break;
        }

        parse_float_nibble(nibble2, &mut stack)?;
    }

    let float_str = std::str::from_utf8(&stack)
        .map_err(|_| Error::Custom("invalid float".to_string()))?;
    float_str.parse().map_err(|_| Error::Custom("invalid float".to_string()))
}
