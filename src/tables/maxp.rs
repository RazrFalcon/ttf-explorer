use crate::parser::*;
use crate::{Error, Result};

pub fn parse(parser: &mut Parser) -> Result<()> {
    let version: Fixed = parser.read("Version")?;
    parser.read::<u16>("Number of glyphs")?;

    if version.0 == 0.3125 { // v0.5
        return Ok(());
    }

    if version.0 != 1.0 {
        return Err(Error::InvalidTableVersion);
    }

    parser.read::<u16>("Maximum points in a non-composite glyph")?;
    parser.read::<u16>("Maximum contours in a non-composite glyph")?;
    parser.read::<u16>("Maximum points in a composite glyph")?;
    parser.read::<u16>("Maximum contours in a composite glyph")?;
    parser.read::<u16>("Maximum zones")?;
    parser.read::<u16>("Maximum twilight points")?;
    parser.read::<u16>("Number of Storage Area locations")?;
    parser.read::<u16>("Number of FDEFs")?;
    parser.read::<u16>("Number of IDEFs")?;
    parser.read::<u16>("Maximum stack depth")?;
    parser.read::<u16>("Maximum byte count for glyph instructions")?;
    parser.read::<u16>("Maximum number of components")?;
    parser.read::<u16>("Maximum levels of recursion")?;

    Ok(())
}

pub fn parse_number_of_glyphs(data: &[u8]) -> Result<u16> {
    let mut parser = SimpleParser::new(data);
    parser.read::<Fixed>()?;
    parser.read()
}
