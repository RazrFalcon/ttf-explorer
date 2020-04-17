use crate::parser::*;
use crate::Result;

pub fn parse(number_of_metrics: u16, number_of_glyphs: u16, parser: &mut Parser) -> Result<()> {
    for i in 0..number_of_metrics {
        parser.begin_group(format!("Glyph {}", i));
        parser.read::<u16>("Advance width")?;
        parser.read::<i16>("Left side bearing")?;
        parser.end_group();
    }

    for i in 0..(number_of_glyphs - number_of_metrics) {
        parser.begin_group(format!("Glyph {}", number_of_metrics + i));
        parser.read::<i16>("Left side bearing")?;
        parser.end_group();
    }

    Ok(())
}
