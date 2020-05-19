use crate::parser::*;
use crate::{TitleKind, Error, Result};
use super::name::{self, PlatformId};

struct Record {
    offset: usize,
    platform_id: PlatformId,
}

pub fn parse(parser: &mut Parser) -> Result<()> {
    let table_start = parser.offset();

    let version = parser.read::<u16>("Version")?;
    if version != 0 {
        return Err(Error::InvalidTableVersion);
    }

    let num_tables = parser.read::<u16>("Number of tables")?;
    if num_tables == 0 {
        return Ok(());
    }

    let mut records = Vec::new();
    parser.begin_group("Encoding records");
    for i in 0..num_tables {
        parser.begin_group(format!("Record {}", i));
        let platform_id = parser.read::<PlatformId>("Platform ID")?;
        {
            let id = parser.peek::<u16>()?;
            parser.read_value(2, "Encoding ID", name::encoding_name(platform_id, id))?;
        }
        let offset = parser.read::<Offset32>("Offset")?.to_usize();
        parser.end_group();

        records.push(Record { offset, platform_id });
    }
    parser.end_group();

    records.sort_by_key(|r| r.offset);
    records.dedup_by_key(|r| r.offset);

    for record in records {
        parser.jump_to(table_start + record.offset)?;
        parser.begin_group("Table");
        let format = parser.read::<u16>("Format")?;
        let title = match format {
            0 => {
                parse_format0(record.platform_id, parser)?;
                "Byte encoding table"
            }
            2 => {
                parse_format2(record.platform_id, parser)?;
                "High-byte mapping through table"
            }
            4 => {
                parse_format4(record.platform_id, parser)?;
                "Segment mapping to delta values"
            }
            6 => {
                parse_format6(record.platform_id, parser)?;
                "Trimmed table mapping"
            }
            8 => {
                parse_format8(record.platform_id, parser)?;
                "Mixed 16-bit and 32-bit coverage"
            }
            10 => {
                parse_format10(record.platform_id, parser)?;
                "Trimmed array"
            }
            12 => {
                parse_format12(record.platform_id, parser)?;
                "Segmented coverage"
            }
            13 => {
                parse_format13(record.platform_id, parser)?;
                "Many-to-one range mappings"
            }
            14 => {
                parse_format14(parser)?;
                "Unicode Variation Sequences"
            }
            _ => {
                return Err(Error::Custom(format!("{} is not a valid subtable format", format)));
            }
        };

        parser.end_group_with_title(format!("Subtable {}: {}", format, title));
    }

    Ok(())
}

fn parse_language16(platform_id: PlatformId, parser: &mut Parser) -> Result<()> {
    let id = parser.peek::<u16>()?;
    parser.read_value(2, "Language ID", name::language_name(platform_id, id))
}

fn parse_language32(platform_id: PlatformId, parser: &mut Parser) -> Result<()> {
    let id = parser.peek::<u32>()?;
    parser.read_value(4, "Language ID", name::language_name(platform_id, id as u16)) // TODO: check cast
}

fn parse_format0(platform_id: PlatformId, parser: &mut Parser) -> Result<()> {
    parser.read::<u16>("Subtable size")?;
    parse_language16(platform_id, parser)?;
    parser.read_array::<u8>("Glyphs", TitleKind::Glyph, 256)
}

fn parse_format2(platform_id: PlatformId, parser: &mut Parser) -> Result<()> {
    let table_start = parser.offset() - 2;
    let table_size = parser.read::<u16>("Subtable size")? as usize;
    parse_language16(platform_id, parser)?;

    let mut sub_headers_count = 0;
    parser.begin_group_with_value("SubHeader keys", "256");
    for i in 0..256 {
        let key = parser.read2::<u16>(format!("Key {}", i))?; // TODO: this
        sub_headers_count = std::cmp::max(sub_headers_count, (key / 8) as u16);
    }
    parser.end_group();

    sub_headers_count += 1;
    parser.begin_group_with_value("SubHeader records", sub_headers_count.to_string());
    for i in 0..sub_headers_count {
        parser.begin_group(format!("SubHeader {}", i));
        parser.read::<u16>("First valid low byte")?;
        parser.read::<u16>("Number of valid low bytes")?;
        parser.read::<i16>("ID delta")?;
        parser.read::<u16>("ID range offset")?;
        parser.end_group();
    }
    parser.end_group();

    // TODO: technically, we should split the tail into subarrays,
    //       but looks like ranges can overlap and ttf-explorer doesn't support this

    let tail_size = table_size - (parser.offset() - table_start);
    parser.read_array::<GlyphId>("Glyph index array", TitleKind::Glyph, tail_size / 2)?;

    Ok(())
}

fn parse_format4(platform_id: PlatformId, parser: &mut Parser) -> Result<()> {
    let table_start = parser.offset() - 2;
    let table_size = parser.read::<u16>("Subtable size")? as usize;
    parse_language16(platform_id, parser)?;
    let seg_count2 = parser.read::<u16>("2 × segCount")?;
    let seg_count = (seg_count2 / 2) as usize;
    parser.read::<u16>("Search range")?;
    parser.read::<u16>("Entry selector")?;
    parser.read::<u16>("Range shift")?;
    parser.read_array::<u16>("End character codes", TitleKind::Code, seg_count)?;
    parser.read::<u16>("Reserved")?;
    parser.read_array::<u16>("Start character codes", TitleKind::Code, seg_count)?;
    parser.read_array::<i16>("Deltas", TitleKind::Delta, seg_count)?;
    parser.read_array::<u16>("Offsets into Glyph index array", TitleKind::Offset, seg_count)?;

    let tail_size = table_size - (parser.offset() - table_start);
    parser.read_array::<GlyphId>("Glyph index array", TitleKind::Glyph, tail_size / 2)?;

    Ok(())
}

fn parse_format6(platform_id: PlatformId, parser: &mut Parser) -> Result<()> {
    parser.read::<u16>("Subtable size")?;
    parse_language16(platform_id, parser)?;
    parser.read::<u16>("First code")?;
    let count = parser.read::<u16>("Number of codes")? as usize;
    parser.read_array::<GlyphId>("Glyph index array", TitleKind::Glyph, count)?;

    Ok(())
}

fn parse_format8(platform_id: PlatformId, parser: &mut Parser) -> Result<()> {
    parser.read::<u16>("Reserved")?;
    parser.read::<u32>("Subtable size")?;
    parse_language32(platform_id, parser)?;
    parser.read_bytes(8192, "Packed data")?;
    let count = parser.read::<u32>("Number of groups")?;

    parser.begin_group_with_value("SequentialMapGroup records", count.to_string());
    for i in 0..count {
        parser.begin_group(format!("Record {}", i));
        parser.read::<u32>("First character code")?;
        parser.read::<u32>("Last character code")?;
        parser.read::<u32>("Starting glyph index")?;
        parser.end_group();
    }
    parser.end_group();

    Ok(())
}

fn parse_format10(platform_id: PlatformId, parser: &mut Parser) -> Result<()> {
    parser.read::<u16>("Reserved")?;
    parser.read::<u32>("Subtable size")?;
    parse_language32(platform_id, parser)?;
    parser.read::<u32>("First code")?;
    let count = parser.read::<u32>("Number of codes")? as usize;
    parser.read_array::<GlyphId>("Glyph index array", TitleKind::Glyph, count)?;

    Ok(())
}

fn parse_format12(platform_id: PlatformId, parser: &mut Parser) -> Result<()> {
    parser.read::<u16>("Reserved")?;
    parser.read::<u32>("Subtable size")?;
    parse_language32(platform_id, parser)?;
    let count = parser.read::<u32>("Number of groups")?;

    parser.begin_group_with_value("SequentialMapGroup records", count.to_string());
    for i in 0..count {
        parser.begin_group(format!("Record {}", i));
        parser.read::<u32>("First character code")?;
        parser.read::<u32>("Last character code")?;
        parser.read::<u32>("Starting glyph index")?;
        parser.end_group();
    }
    parser.end_group();

    Ok(())
}

fn parse_format13(platform_id: PlatformId, parser: &mut Parser) -> Result<()> {
    parser.read::<u16>("Reserved")?;
    parser.read::<u32>("Subtable size")?;
    parse_language32(platform_id, parser)?;
    let count = parser.read::<u32>("Number of groups")?;

    parser.begin_group_with_value("ConstantMapGroup records", count.to_string());
    for i in 0..count {
        parser.begin_group(format!("Record {}", i));
        parser.read::<u32>("First character code")?;
        parser.read::<u32>("Last character code")?;
        parser.read::<u32>("Glyph index")?;
        parser.end_group();
    }
    parser.end_group();

    Ok(())
}

fn parse_format14(parser: &mut Parser) -> Result<()> {
    let table_start = parser.offset() - 2;

    parser.read::<u32>("Subtable size")?;
    let count = parser.read::<u32>("Number of records")?;

    struct Record {
        default: bool,
        offset: usize,
    };

    let mut records = Vec::new();
    parser.begin_group_with_value("VariationSelector records", count.to_string());
    for i in 0..count {
        parser.begin_group(format!("Record {}", i));
        parser.read::<U24>("Variation selector")?;
        let def_offset = parser.read::<Offset32>("Offset to Default UVS Table")?.to_usize();
        let non_def_offset = parser.read::<Offset32>("Offset to Non-Default UVS Table")?.to_usize();
        parser.end_group();

        if def_offset != 0 {
            records.push(Record { default: true, offset: table_start + def_offset });
        }

        if non_def_offset != 0 {
            records.push(Record { default: false, offset: table_start + non_def_offset });
        }
    }
    parser.end_group();

    records.sort_by_key(|r| r.offset);
    records.dedup_by_key(|r| r.offset);

    for record in records {
        parser.jump_to(record.offset)?;
        if record.default {
            parser.begin_group("Default UVS table");
            let count = parser.read::<u32>("Number of Unicode character ranges")?;
            for _ in 0..count {
                parser.begin_group("Unicode range");
                parser.read::<U24>("First value in this range")?;
                parser.read::<u8>("Number of additional values")?;
                parser.end_group();
            }
            parser.end_group();
        } else {
            parser.begin_group("Non-Default UVS table");
            let count = parser.read::<u32>("Number of UVS Mappings")?;
            for _ in 0..count {
                parser.begin_group("UVS mapping");
                parser.read::<U24>("Base Unicode value")?;
                parser.read::<GlyphId>("Glyph ID")?;
                parser.end_group();
            }
            parser.end_group();
        }
    }

    Ok(())
}
