use crate::parser::*;
use crate::{Error, Result};

pub fn parse(parser: &mut Parser) -> Result<()> {
    let major_version: u16 = parser.read("Major version")?;
    let minor_version: u16 = parser.read("Minor version")?;
    if !(major_version == 1 && minor_version == 0) {
        return Err(Error::InvalidTableVersion);
    }

    parser.read::<i16>("Typographic ascent")?;
    parser.read::<i16>("Typographic descent")?;
    parser.read::<i16>("Typographic line gap")?;
    parser.read::<u16>("Maximum advance width")?;
    parser.read::<i16>("Minimum left sidebearing")?;
    parser.read::<i16>("Minimum right sidebearing")?;
    parser.read::<i16>("Maximum X extent")?;
    parser.read::<i16>("Caret slope rise")?;
    parser.read::<i16>("Caret slope run")?;
    parser.read::<i16>("Caret offset")?;
    parser.read::<i16>("Reserved")?;
    parser.read::<i16>("Reserved")?;
    parser.read::<i16>("Reserved")?;
    parser.read::<i16>("Reserved")?;
    parser.read::<i16>("Metric data format")?;
    parser.read::<u16>("Number of horizontal metrics")?;

    Ok(())
}

pub fn parse_number_of_metrics(data: &[u8]) -> Result<u16> {
    let mut parser = SimpleParser::new(data);
    parser.advance(34);
    parser.read()
}
