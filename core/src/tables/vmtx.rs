use crate::parser::*;
use crate::{TitleKind, Result};

pub fn parse(number_of_metrics: u16, number_of_glyphs: u16, parser: &mut Parser) -> Result<()> {
    for i in 0..number_of_metrics {
        parser.begin_group_with_index(TitleKind::Glyph, i as u32);
        parser.read::<u16>("Advance height")?;
        parser.read::<i16>("Top side bearing")?;
        parser.end_group();
    }

    for i in 0..(number_of_glyphs - number_of_metrics) {
        parser.begin_group_with_index(TitleKind::Glyph, (number_of_metrics + i) as u32);
        parser.read::<i16>("Top side bearing")?;
        parser.end_group();
    }

    Ok(())
}
