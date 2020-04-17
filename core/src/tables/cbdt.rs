use crate::parser::*;
use crate::{Error, Result};
use super::cblc;

pub fn parse(indices: &[super::cblc::CblcIndex], parser: &mut Parser) -> Result<()> {
    let start = parser.offset();

    let major_version = parser.read::<u16>("Major version")?;
    let minor_version = parser.read::<u16>("Minor version")?;
    if !((major_version == 2 || major_version == 3) && minor_version == 0) {
        return Err(Error::InvalidTableVersion);
    }

    for loca in indices {
        parser.jump_to(start + loca.range.start)?;
        parser.begin_group(format!("Bitmap Format {}", loca.image_format));

        match loca.image_format {
            1 => {
                cblc::parse_small_glyph_metrics(parser)?;
                parser.read_bytes(loca.range.len() - 5, "Byte-aligned bitmap data")?;
            }
            2 => {
                cblc::parse_small_glyph_metrics(parser)?;
                parser.read_bytes(loca.range.len() - 5, "Bit-aligned bitmap data")?;
            }
            5 => {
                parser.read_bytes(loca.range.len(), "Bit-aligned bitmap data")?;
            }
            6 => {
                cblc::parse_big_glyph_metrics(parser)?;
                parser.read_bytes(loca.range.len() - 5, "Byte-aligned bitmap data")?;
            }
            7 => {
                cblc::parse_big_glyph_metrics(parser)?;
                parser.read_bytes(loca.range.len() - 5, "Bit-aligned bitmap data")?;
            }
            8 => {
                cblc::parse_small_glyph_metrics(parser)?;
                parser.read::<u8>("Pad")?;
                let count = parser.read::<u16>("Number of components")?;
                for _ in 0..count {
                    parser.begin_group("Ebdt component");
                    parser.read::<GlyphId>("Glyph ID")?;
                    parser.read::<i8>("X-axis offset")?;
                    parser.read::<i8>("Y-axis offset")?;
                    parser.end_group();
                }
            }
            9 => {
                cblc::parse_big_glyph_metrics(parser)?;
                let count = parser.read::<u16>("Number of components")?;
                for _ in 0..count {
                    parser.begin_group("Ebdt component");
                    parser.read::<GlyphId>("Glyph ID")?;
                    parser.read::<i8>("X-axis offset")?;
                    parser.read::<i8>("Y-axis offset")?;
                    parser.end_group();
                }
            }
            17 => {
                cblc::parse_small_glyph_metrics(parser)?;
                let len = parser.read::<u32>("Length of data")? as usize;
                parser.read_bytes(len, "Raw PNG data")?;
            }
            18 => {
                cblc::parse_big_glyph_metrics(parser)?;
                let len = parser.read::<u32>("Length of data")? as usize;
                parser.read_bytes(len, "Raw PNG data")?;
            }
            19 => {
                let len = parser.read::<u32>("Length of data")? as usize;
                parser.read_bytes(len, "Raw PNG data")?;
            }
            _ => {}
        }

        parser.end_group();
    }

    Ok(())
}
