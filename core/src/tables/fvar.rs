use crate::parser::*;
use crate::{Error, Result};

pub fn parse(parser: &mut Parser) -> Result<()> {
    let major_version = parser.read::<u16>("Major version")?;
    let minor_version = parser.read::<u16>("Minor version")?;
    if !(major_version == 1 && minor_version == 0) {
        return Err(Error::InvalidTableVersion);
    }

    parser.read::<Offset16>("Offset to VariationAxisRecord array")?;
    parser.read::<u16>("Reserved")?;
    let axes_count = parser.read::<u16>("The number of variation axes")?;
    parser.read::<u16>("The size of VariationAxisRecord")?;
    let instances_count = parser.read::<u16>("The number of named instances")?;
    let instance_size = parser.read::<u16>("The size of InstanceRecord")?;

    parser.begin_group("Variation axis records");
    for _ in 0..axes_count {
        parser.begin_group("");
        let tag = parser.read::<Tag>("Axis tag")?;
        parser.read::<Fixed>("Minimum coordinate")?;
        parser.read::<Fixed>("Default coordinate")?;
        parser.read::<Fixed>("Maximum coordinate")?;
        parser.read::<u16>("Axis qualifiers")?;
        parser.read::<u16>("The name ID")?;
        parser.end_group_with_title(format!("Axis {}", tag));
    }
    parser.end_group();

    if instances_count > 0 {
        parser.begin_group("Instance records");
        for _ in 0..instances_count {
            parser.begin_group("Instance");

            parser.read::<u16>("Subfamily name ID")?;
            parser.read::<u16>("Reserved")?;
            for _ in 0..axes_count {
                parser.read::<Fixed>("Coordinate")?;
            }

            // TODO: explain
            if instance_size == 10 {
                parser.read::<u16>("PostScript name ID")?;
            }

            parser.end_group();
        }
        parser.end_group();
    }

    Ok(())
}
