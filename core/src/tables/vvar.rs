use crate::parser::*;
use crate::{Error, Result};

pub fn parse(parser: &mut Parser) -> Result<()> {
    let start = parser.offset();

    let major_version: u16 = parser.read("Major version")?;
    let minor_version: u16 = parser.read("Minor version")?;
    if !(major_version == 1 && minor_version == 0) {
        return Err(Error::InvalidTableVersion);
    }

    let var_store_offset = parser.read::<Offset32>("Item variation store offset")?.to_usize();
    let advance_height_mapping_offset = parser.read::<OptionalOffset32>("Advance height mapping offset")?.to_usize();
    let tsb_mapping_offset = parser.read::<OptionalOffset32>("Top side bearing mapping offset")?.to_usize();
    let bsb_mapping_offset = parser.read::<OptionalOffset32>("Bottom side bearing mapping offset")?.to_usize();
    let vorg_mapping_offset = parser.read::<OptionalOffset32>("Vertical origin mapping offset")?.to_usize();

    parser.jump_to(start + var_store_offset)?;
    parser.begin_group("Item variation store");
    super::mvar::parse_item_variation_store(parser)?;
    parser.end_group();

    if let Some(offset) = advance_height_mapping_offset {
        parser.jump_to(start + offset)?;
        parser.begin_group("Advance height mapping");
        super::hvar::parse_delta_set(parser)?;
        parser.end_group();
    }

    if let Some(offset) = tsb_mapping_offset {
        parser.jump_to(start + offset)?;
        parser.begin_group("Top side bearing mapping");
        super::hvar::parse_delta_set(parser)?;
        parser.end_group();
    }

    if let Some(offset) = bsb_mapping_offset {
        parser.jump_to(start + offset)?;
        parser.begin_group("Bottom side bearing mapping");
        super::hvar::parse_delta_set(parser)?;
        parser.end_group();
    }

    if let Some(offset) = vorg_mapping_offset {
        parser.jump_to(start + offset)?;
        parser.begin_group("Vertical origin mapping");
        super::hvar::parse_delta_set(parser)?;
        parser.end_group();
    }

    Ok(())
}
