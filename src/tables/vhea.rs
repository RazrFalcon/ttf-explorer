use crate::parser::*;
use crate::{Error, Result};

pub fn parse(parser: &mut Parser) -> Result<()> {
    let version: Fixed = parser.read("Version")?;

    // 1.0625 is actully means 1.1
    if version.0 != 1.0 && version.0 != 1.0625 {
        return Err(Error::InvalidTableVersion);
    }

    // The difference between 1.0 and 1.1 only in field names,
    // and we are using 1.1 definitions.

    parser.read::<i16>("Vertical typographic ascender")?;
    parser.read::<i16>("Vertical typographic descender")?;
    parser.read::<i16>("Vertical typographic line gap")?;
    parser.read::<u16>("Maximum advance width")?;
    parser.read::<i16>("Minimum top sidebearing")?;
    parser.read::<i16>("Minimum bottom sidebearing")?;
    parser.read::<i16>("Maximum Y extent")?;
    parser.read::<i16>("Caret slope rise")?;
    parser.read::<i16>("Caret slope run")?;
    parser.read::<i16>("Caret offset")?;
    parser.read::<i16>("Reserved")?;
    parser.read::<i16>("Reserved")?;
    parser.read::<i16>("Reserved")?;
    parser.read::<i16>("Reserved")?;
    parser.read::<i16>("Metric data format")?;
    parser.read::<u16>("Number of vertical metrics")?;

    Ok(())
}

pub fn parse_number_of_metrics(data: &[u8]) -> Result<u16> {
    let mut parser = SimpleParser::new(data);
    parser.advance(34);
    parser.read()
}
