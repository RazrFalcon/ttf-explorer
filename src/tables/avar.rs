use crate::parser::*;
use crate::{Error, Result};

pub fn parse(parser: &mut Parser) -> Result<()> {
    let major_version = parser.read::<u16>("Major version")?;
    let minor_version = parser.read::<u16>("Minor version")?;
    if !(major_version == 1 && minor_version == 0) {
        return Err(Error::InvalidTableVersion);
    }

    parser.read::<u16>("Reserved")?;
    let axis_count = parser.read::<u16>("Axis count")?;
    for _ in 0..axis_count {
        parser.begin_group("Segment map");
        let pairs_count = parser.read::<u16>("Number of map pairs")?;
        for j in 0..pairs_count {
            parser.begin_group(format!("Pair {}", j));
            parser.read::<F2DOT14>("From coordinate")?;
            parser.read::<F2DOT14>("To coordinate")?;
            parser.end_group();
        }

        parser.end_group();
    }

    Ok(())
}
