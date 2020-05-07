use crate::parser::*;
use crate::{TitleKind, Result};
use super::IndexToLocFormat;

pub fn parse(
    number_of_glyphs: u16,
    format: IndexToLocFormat,
    parser: &mut Parser,
) -> Result<()> {
    if format == IndexToLocFormat::Short {
        for i in 0..=number_of_glyphs {
            parser.read_index::<Offset16>(TitleKind::Offset, i as u32)?;
        }
    } else {
        for i in 0..=number_of_glyphs {
            parser.read_index::<Offset32>(TitleKind::Offset, i as u32)?;
        }
    }

    Ok(())
}

pub fn collect_ranges(
    number_of_glyphs: u16,
    format: IndexToLocFormat,
    data: &[u8],
) -> Result<Vec<std::ops::Range<usize>>> {
    let mut parser = SimpleParser::new(data);
    let mut offsets = Vec::new();
    if format == IndexToLocFormat::Short {
        for _ in 0..=number_of_glyphs {
            offsets.push(parser.read::<u16>()? as usize * 2);
        }
    } else {
        for _ in 0..=number_of_glyphs {
            offsets.push(parser.read::<u32>()? as usize);
        }
    }

    let mut start = 0; // The first offset is always 0, I hope.
    let mut ranges = Vec::new();
    for offset in offsets.iter().skip(1).copied() {
        ranges.push(start..offset);
        start = offset;
    }

    Ok(ranges)
}
