use std::ops::Range;

use crate::parser::*;
use crate::{Error, Result};

#[derive(Clone, Copy, Debug)]
struct BitmapFlags(u8);

impl FromData for BitmapFlags {
    const NAME: &'static str = "BitFlags";

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        Ok(BitmapFlags(data[0]))
    }
}

impl std::fmt::Display for BitmapFlags {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        use std::fmt::Write;
        let mut s = String::new();

        let bits = U8Bits(self.0);
        writeln!(&mut s, "{}", bits)?;
        if bits[0] { s.push_str("Bit 0: Horizontal\n"); }
        if bits[1] { s.push_str("Bit 1: Vertical\n"); }

        s.pop(); // pop last new line

        f.write_str(&s)
    }
}

struct SubtableArray {
    offset: usize,
    num_of_subtables: u32,
}

struct SubtableInfo {
    first_glyph: GlyphId,
    last_glyph: GlyphId,
    offset: usize,
}

pub fn parse(parser: &mut Parser) -> Result<()> {
    let start = parser.offset();

    let major_version = parser.read::<u16>("Major version")?;
    let minor_version = parser.read::<u16>("Minor version")?;
    // Some old Noto Emoji fonts have a 2.0 version.
    if !((major_version == 2 || major_version == 3) && minor_version == 0) {
        return Err(Error::InvalidTableVersion);
    }

    let num_sizes = parser.read::<u32>("Number of tables")?;

    let mut subtable_arrays = Vec::new();
    for _ in 0..num_sizes {
        parser.begin_group("Table");

        let offset = parser.read::<Offset32>("Offset to index subtable")?.to_usize();
        parser.read::<u32>("Index tables size")?;
        let num_of_subtables = parser.read::<u32>("Number of index subtables")?;
        parser.read::<u32>("Reserved")?;

        parser.begin_group("Line metrics for horizontal text");
        parse_line_metrics(parser)?;
        parser.end_group();

        parser.begin_group("Line metrics for vertical text");
        parse_line_metrics(parser)?;
        parser.end_group();

        parser.read::<GlyphId>("Lowest glyph index")?;
        parser.read::<GlyphId>("Highest glyph index")?;
        parser.read::<u8>("Horizontal pixels per em")?;
        parser.read::<u8>("Vertical pixels per em")?;
        parser.read::<u8>("Bit depth")?;
        parser.read::<BitmapFlags>("Flags")?;

        parser.end_group();

        subtable_arrays.push(SubtableArray { offset, num_of_subtables });
    }

    subtable_arrays.sort_by_key(|v| v.offset);
    subtable_arrays.dedup_by_key(|v| v.offset);

    let mut subtables = Vec::new();
    for array in subtable_arrays {
        parser.jump_to(start + array.offset)?;

        for _ in 0..array.num_of_subtables {
            parser.begin_group("Index subtable array");
            let first_glyph = parser.read::<GlyphId>("First glyph ID")?;
            let last_glyph = parser.read::<GlyphId>("Last glyph ID")?;
            let offset2 = parser.read::<Offset32>("Additional offset to index subtable")?.to_usize();
            parser.end_group();

            subtables.push(SubtableInfo {
                first_glyph,
                last_glyph,
                offset: start + array.offset + offset2,
            });
        }
    }

    subtables.sort_by_key(|v| v.offset);
    subtables.dedup_by_key(|v| v.offset);

    for info in subtables {
        parser.jump_to(info.offset)?;
        parser.begin_group("Index subtable");
        let index_format = parser.read::<u16>("Index format")?;
        parser.read::<u16>("Image format")?;
        parser.read::<Offset32>("Offset to image data")?;

        match index_format {
            1 => {
                // TODO: check
                let count = info.last_glyph.0 - info.first_glyph.0 + 2;
                parser.read_array::<Offset32>("Offsets", "Offset", count as usize)?;
            }
            2 => {
                parser.read::<u32>("Image size")?;
                parse_big_glyph_metrics(parser)?;
            }
            3 => {
                // TODO: check
                let count = info.last_glyph.0 - info.first_glyph.0 + 2;
                parser.read_array::<Offset16>("Offsets", "Offset", count as usize)?;
            }
            4 => {
                let num_glyphs = parser.read::<u32>("Number of glyphs")?;
                for _ in 0..=num_glyphs {
                    parser.read::<GlyphId>("Glyph ID")?;
                    parser.read::<Offset16>("Offset")?;
                }
            }
            5 => {
                parser.read::<u32>("Image size")?;
                parse_big_glyph_metrics(parser)?;
                let num_glyphs = parser.read::<u32>("Number of glyphs")?;
                parser.read_array::<GlyphId>("Glyphs", "Glyph ID", num_glyphs as usize)?;
            }
            _ => return Err(Error::InvalidValue),
        }

        parser.end_group();
    }

    Ok(())
}

fn parse_line_metrics(parser: &mut Parser) -> Result<()> {
    parser.read::<i8>("Ascender")?;
    parser.read::<i8>("Descender")?;
    parser.read::<u8>("Max width")?;
    parser.read::<i8>("Caret slope numerator")?;
    parser.read::<i8>("Caret slope denominator")?;
    parser.read::<i8>("Caret offset")?;
    parser.read::<i8>("Min origin SB")?;
    parser.read::<i8>("Min advance SB")?;
    parser.read::<i8>("Max before BL")?;
    parser.read::<i8>("Min after BL")?;
    parser.read_bytes(2, "Padding")?;
    Ok(())
}

pub fn parse_small_glyph_metrics(parser: &mut Parser) -> Result<()> {
    parser.read::<u8>("Height")?;
    parser.read::<u8>("Width")?;
    parser.read::<i8>("X-axis bearing")?;
    parser.read::<i8>("Y-axis bearing")?;
    parser.read::<u8>("Advance")?;
    Ok(())
}

pub fn parse_big_glyph_metrics(parser: &mut Parser) -> Result<()> {
    parser.read::<u8>("Height")?;
    parser.read::<u8>("Width")?;
    parser.read::<i8>("Horizontal X-axis bearing")?;
    parser.read::<i8>("Horizontal Y-axis bearing")?;
    parser.read::<u8>("Horizontal advance")?;
    parser.read::<i8>("Vertical X-axis bearing")?;
    parser.read::<i8>("Vertical Y-axis bearing")?;
    parser.read::<u8>("Vertical advance")?;
    Ok(())
}


pub struct CblcIndex {
    pub image_format: u16,
    pub range: Range<usize>,
}

pub fn collect_indices(data: &[u8]) -> Result<Vec<CblcIndex>> {
    let mut parser = SimpleParser::new(data);
    let mut locations = Vec::new();

    let start = parser.offset();

    parser.skip::<u16>(); // Major version
    parser.skip::<u16>(); // Minor version

    let num_sizes = parser.read::<u32>()?;

    let mut subtable_arrays = Vec::new();
    for _ in 0..num_sizes {
        let offset = parser.read::<Offset32>()?.to_usize();
        parser.skip::<u32>(); // Index tables size
        let num_of_subtables = parser.read::<u32>()?;
        parser.advance(36);

        subtable_arrays.push(SubtableArray { offset, num_of_subtables });
    }

    subtable_arrays.sort_by_key(|v| v.offset);
    subtable_arrays.dedup_by_key(|v| v.offset);

    let mut subtables = Vec::new();
    for array in subtable_arrays {
        parser.jump_to(start + array.offset)?;

        for _ in 0..array.num_of_subtables {
            let first_glyph = parser.read::<GlyphId>()?;
            let last_glyph = parser.read::<GlyphId>()?;
            let offset2 = parser.read::<Offset32>()?.to_usize();

            subtables.push(SubtableInfo {
                first_glyph,
                last_glyph,
                offset: start + array.offset + offset2
            });
        }
    }

    subtables.sort_by_key(|v| v.offset);
    subtables.dedup_by_key(|v| v.offset);

    for info in subtables {
        parser.jump_to(info.offset)?;
        let index_format = parser.read::<u16>()?;
        let image_format = parser.read::<u16>()?;
        let image_data_offset = parser.read::<Offset32>()?.to_usize();

        match index_format {
            1 => {
                let count = info.last_glyph.0 - info.first_glyph.0 + 2;
                let mut offsets = Vec::new();
                for _ in 0..count {
                    let offset = parser.read::<Offset32>()?.to_usize();
                    offsets.push(image_data_offset + offset);
                }

                offsets.sort();
                offsets.dedup();

                for i in 0..offsets.len()-1 {
                    let start = offsets[i];
                    let end = offsets[i + 1];
                    locations.push(CblcIndex { image_format, range: start..end });
                }
            }
            2 => {
                let image_size = parser.read::<u32>()? as usize;

                let count = info.last_glyph.0 - info.first_glyph.0 + 1;
                let mut offset = image_data_offset;
                for _ in 0..count {
                    let start = offset;
                    offset += image_size;
                    let end = offset;
                    locations.push(CblcIndex { image_format, range: start..end });
                }
            }
            3 => {
                let count = info.last_glyph.0 - info.first_glyph.0 + 2;
                let mut offsets = Vec::new();
                for _ in 0..count {
                    let offset = parser.read::<Offset16>()?.to_usize();
                    offsets.push(image_data_offset + offset);
                }

                offsets.sort();
                offsets.dedup();

                for i in 0..offsets.len() - 1 {
                    let start = offsets[i];
                    let end = offsets[i + 1];
                    locations.push(CblcIndex { image_format, range: start..end });
                }
            }
            4 => {
                let num_glyphs = parser.read::<u32>()?;
                let mut offsets = Vec::new();
                for _ in 0..=num_glyphs {
                    parser.skip::<GlyphId>();
                    let offset = parser.read::<Offset16>()?.to_usize();
                    offsets.push(image_data_offset + offset);
                }

                offsets.sort();
                offsets.dedup();

                for i in 0..offsets.len() - 1 {
                    let start = offsets[i];
                    let end = offsets[i + 1];
                    locations.push(CblcIndex { image_format, range: start..end });
                }
            }
            5 => {
                let image_size = parser.read::<u32>()? as usize;
                parser.advance(8); // big metrics
                let num_glyphs = parser.read::<u32>()?;

                let mut offset = image_data_offset;
                let mut offsets = Vec::new();
                for _ in 0..=num_glyphs {
                    offsets.push(offset);
                    offset += image_size;
                }

                offsets.sort();
                offsets.dedup();

                for i in 0..offsets.len() - 1 {
                    let start = offsets[i];
                    let end = offsets[i + 1];
                    locations.push(CblcIndex { image_format, range: start..end });
                }
            }
            _ => return Err(Error::InvalidValue),
        }
    }

    locations.sort_by_key(|v| v.range.start);

    Ok(locations)
}
