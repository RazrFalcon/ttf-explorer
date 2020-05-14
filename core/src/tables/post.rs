use crate::parser::*;
use crate::{TitleKind, Result};

pub fn parse(table_end_offset: usize, parser: &mut Parser) -> Result<()> {
    let version = parser.read::<Fixed>("Version")?;
    parser.read::<Fixed>("Italic angle")?;
    parser.read::<i16>("Underline position")?;
    parser.read::<i16>("Underline thickness")?;
    parser.read::<u32>("Is fixed pitch")?;
    parser.read::<u32>("Min memory when font is downloaded")?;
    parser.read::<u32>("Max memory when font is downloaded")?;
    parser.read::<u32>("Min memory when font is downloaded as a Type 1")?;
    parser.read::<u32>("Max memory when font is downloaded as a Type 1")?;

    if version.0 != 2.0 {
        return Ok(());
    }

    let num_glyphs = parser.read::<u16>("Number of glyphs")?;
    if num_glyphs != 0 {
        parser.read_array::<u16>("Glyph name indexes", TitleKind::Index, num_glyphs as usize)?;
    }

    while parser.offset() < table_end_offset {
        parser.begin_group("Name");
        let len = parser.read::<u8>("Length")?;
        if len != 0 {
            parser.read_string(len as usize, TitleKind::String, None)?;
        }

        parser.end_group();
    }

    Ok(())
}
