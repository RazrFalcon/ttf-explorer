use crate::parser::*;
use crate::{TitleKind, Error, Result};
use super::cff::{Dict, DictRecord, OffsetSize, parse_float};

mod dict_operator {
    pub const BLUE_VALUES: u8 = 6;
    pub const OTHER_BLUES: u8 = 7;
    pub const FAMILY_BLUES: u8 = 8;
    pub const FAMILY_OTHER_BLUES: u8 = 9;
    pub const STD_HW: u8 = 10;
    pub const STD_VW: u8 = 11;
    pub const CHAR_STRINGS: u8 = 17;
    pub const PRIVATE: u8 = 18;
    pub const SUBRS: u8 = 19;
    pub const VS_INDEX: u8 = 22;
    pub const BLEND: u8 = 23;
    pub const VSTORE: u8 = 24;

    pub const FONT_MATRIX: u16 = 1207;
    pub const BLUE_SCALE: u16 = 1209;
    pub const BLUE_SHIFT: u16 = 1210;
    pub const BLUE_FUZZ: u16 = 1211;
    pub const STEM_SNAP_H: u16 = 1212;
    pub const STEM_SNAP_V: u16 = 1213;
    pub const LANGUAGE_GROUP: u16 = 1217;
    pub const EXPANSION_FACTOR: u16 = 1218;
    pub const FD_ARRAY: u16 = 1236;
    pub const FD_SELECT: u16 = 1237;
}


pub fn parse(parser: &mut Parser) -> Result<()> {
    let table_start = parser.offset();

    parser.begin_group("Header");
    parser.read::<u8>("Major version")?;
    parser.read::<u8>("Minor version")?;
    let header_size = parser.read::<u8>("Header size")? as usize;
    let top_dict_size = parser.read::<u16>("Length of Top DICT")? as usize;
    parser.end_group();

    if header_size > 5 {
        parser.read_bytes(header_size - 5, "Padding")?;
    }

    parser.begin_group("Top DICT");
    let top_dict = parse_dict(top_dict_size, parser)?;
    parser.end_group();

    parse_index("Global Subr INDEX", parser, parse_subr)?;

    if let Some(record) = top_dict.records.iter().find(|r| r.op == dict_operator::VSTORE as u16) {
        if record.operands.len() != 1 || record.operands[0] < 0.0 {
            return Err(Error::Custom("invalid vstore offset".to_string()));
        }

        let offset = record.operands[0] as usize;
        parser.jump_to(table_start + offset)?;
        parser.begin_group("Variation Store");
        parser.read::<u16>("Variation Store size")?;
        super::mvar::parse_item_variation_store(parser)?;
        parser.end_group();
    }

    if let Some(record) = top_dict.records.iter().find(|r| r.op == dict_operator::CHAR_STRINGS as u16) {
        if record.operands.len() != 1 || record.operands[0] < 0.0 {
            return Err(Error::Custom("invalid charstrings offset".to_string()));
        }

        let offset = record.operands[0] as usize;
        parser.jump_to(table_start + offset)?;
        parse_index("CharStrings INDEX", parser, parse_subr)?;
    }


    let mut private_dict_ranges = Vec::new();
    if let Some(record) = top_dict.records.iter().find(|r| r.op == dict_operator::FD_ARRAY as u16) {
        if record.operands.len() != 1 || record.operands[0] < 0.0 {
            return Err(Error::Custom("invalid FD array offset".to_string()));
        }

        let offset = record.operands[0] as usize;
        parser.jump_to(table_start + offset)?;
        parse_index("Font DICT INDEX", parser, |start, end, index, parser| {
            parser.begin_group(format!("DICT {}", index));
            let dict = parse_dict(end - start, parser)?;
            parser.end_group();

            if let Some(record) = dict.records.iter().find(|r| r.op == dict_operator::PRIVATE as u16) {
                if record.operands.len() != 2 || record.operands[0] < 0.0 || record.operands[1] < 0.0 {
                    return Err(Error::Custom("invalid private dict operands".to_string()));
                }

                let len = record.operands[0] as usize;
                let offset = record.operands[1] as usize;
                private_dict_ranges.push(offset..offset+len);
            }

            Ok(())
        })?;
    }

    private_dict_ranges.sort_by_key(|r| r.start);

    let mut subrs_offsets = Vec::new();
    for range in private_dict_ranges {
        parser.jump_to(table_start + range.start)?;
        parser.begin_group("Private DICT");
        let private_dict = parse_dict(range.len(), parser)?;
        parser.end_group();

        if let Some(record) = private_dict.records.iter().find(|r| r.op == dict_operator::SUBRS as u16) {
            if record.operands.len() != 1 || record.operands[0] < 0.0 {
                return Err(Error::Custom("invalid subrs offset".to_string()));
            }

            let offset = record.operands[0] as usize;
            // 'The local subroutines offset is relative to the beginning
            // of the Private DICT data.'
            subrs_offsets.push(table_start + range.start + offset);
        }
    }

    for offset in subrs_offsets {
        parser.jump_to(offset)?;
        parse_index("Local Subr INDEX", parser, parse_subr)?;
    }

    Ok(())
}

fn parse_index<P>(name: &'static str, parser: &mut Parser, mut p: P) -> Result<()>
    where P: FnMut(usize, usize, usize, &mut Parser) -> Result<()>
{
    parser.begin_group(name);

    let count = parser.read::<u32>("Count")?;
    if count == std::u32::MAX {
        return Err(Error::Custom("index items count overflow".to_string()));
    }

    if count == 0 {
        parser.end_group();
        return Ok(());
    }

    let offset_size = parser.read::<OffsetSize>("Offset size")?;

    parser.begin_group_with_value("Indexes", (count + 1).to_string());
    let mut offsets = Vec::new();
    // INDEX has one more index at the end to indicate data length, so we have to add 1 to count.
    for i in 0..count+1 {
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

    parser.end_group();

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
                dict_operator::FONT_MATRIX => "Font matrix",
                dict_operator::BLUE_SCALE => "Blue scale",
                dict_operator::BLUE_SHIFT => "Blue shift",
                dict_operator::BLUE_FUZZ => "Blue fuzz",
                dict_operator::STEM_SNAP_H => "Stem snap H",
                dict_operator::STEM_SNAP_V => "Stem snap V",
                dict_operator::LANGUAGE_GROUP => "Language group",
                dict_operator::EXPANSION_FACTOR => "Expansion factor",
                dict_operator::FD_ARRAY => "Font DICT INDEX",
                dict_operator::FD_SELECT => "FD select",
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
        } else if op1 <= 27 {
            // 0..=27 bytes are operators.

            let title = match op1 {
                dict_operator::BLUE_VALUES => "Blue values",
                dict_operator::OTHER_BLUES => "Other blues",
                dict_operator::FAMILY_BLUES => "Family blues",
                dict_operator::FAMILY_OTHER_BLUES => "Family other blues",
                dict_operator::STD_HW => "Std HW",
                dict_operator::STD_VW => "Std VW",
                dict_operator::CHAR_STRINGS => "CharStrings",
                dict_operator::PRIVATE => "Private",
                dict_operator::SUBRS => "Local subroutines",
                dict_operator::VS_INDEX => "Variation Store index",
                dict_operator::BLEND => "Blend",
                dict_operator::VSTORE => "Variation Store offset",
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
            11 => { parser.read::<u8>("Reserved")?; }
            12 => {
                let b1 = parser.peek_at::<u8>(1)?;
                match b1 {
                    34 => parser.read::<u16>("Horizontal flex (hflex)")?,
                    35 => parser.read::<u16>("Flex (flex)")?,
                    36 => parser.read::<u16>("Horizontal flex 1 (hflex1)")?,
                    37 => parser.read::<u16>("Flex 1 (flex1)")?,
                    _ => parser.read::<u16>("Reserved")?,
                };
            }
            13 | 14 => { parser.read::<u8>("Reserved")?; }
            15 => { parser.read::<u8>("Variation Store index (vsindex)")?; }
            16 => { parser.read::<u8>("Blend (blend)")?; }
            17 => { parser.read::<u8>("Reserved")?; }
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
