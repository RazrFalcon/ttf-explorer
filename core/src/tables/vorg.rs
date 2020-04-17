use crate::parser::*;
use crate::{Error, Result};

pub fn parse(parser: &mut Parser) -> Result<()> {
    let major_version: u16 = parser.read("Major version")?;
    let minor_version: u16 = parser.read("Minor version")?;
    if !(major_version == 1 && minor_version == 0) {
        return Err(Error::InvalidTableVersion);
    }

    parser.read::<i16>("Default vertical origin")?;
    let count = parser.read::<u16>("Number of metrics")?;
    for i in 0..count {
        parser.begin_group(format!("Metric {}", i));
        parser.read::<GlyphId>("Glyph index")?;
        parser.read::<i16>("Coordinate")?;
        parser.end_group();
    }

    Ok(())
}
