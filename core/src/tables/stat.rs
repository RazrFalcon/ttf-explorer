use crate::parser::*;
use crate::Result;

pub fn parse(parser: &mut Parser) -> Result<()> {
    parser.read::<u16>("Major version")?;
    let minor_version = parser.read::<u16>("Minor version")?;
    parser.read::<u16>("Axis record size")?;
    let design_axis_count = parser.read::<u16>("Number of records")?;
    parser.read::<Offset32>("Offset to the axes array")?;
    let axis_value_count = parser.read::<u16>("Number of axis value tables")?;
    parser.read::<Offset32>("Offset to the axes value offsets array")?;

    if minor_version > 0 {
        parser.read::<u16>("Fallback name ID")?;
    }

    parser.begin_group_with_value("Design axes", design_axis_count.to_string());
    for _ in 0..design_axis_count {
        parser.begin_group("Record");
        parser.read::<Tag>("Tag")?;
        parser.read::<u16>("Name ID")?;
        parser.read::<u16>("Axis ordering")?;
        parser.end_group();
    }
    parser.end_group();

    // TODO: parse axis
    parser.read_array::<u16>("Axis value tables offsets", "Offset", axis_value_count as usize)?;

    Ok(())
}
