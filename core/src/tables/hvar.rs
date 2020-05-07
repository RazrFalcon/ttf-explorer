use crate::parser::*;
use crate::{ValueType, Error, Result};

#[derive(Clone, Copy, Debug)]
struct Masks(u16);

impl Masks {
    #[inline] fn inner_index_bits(self) -> u16 { self.0 & 0x000F }
    #[inline] fn entry_size(self) -> u16 { ((self.0 & 0x0030) >> 4) + 1 }
}

impl FromData for Masks {
    const NAME: ValueType = ValueType::Masks;

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        u16::parse(data).map(Masks)
    }
}

impl std::fmt::Display for Masks {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Inner index bit count: {}\nMap entry size: {}",
               self.inner_index_bits(), self.entry_size())
    }
}

pub fn parse(parser: &mut Parser) -> Result<()> {
    let start = parser.offset();

    let major_version: u16 = parser.read("Major version")?;
    let minor_version: u16 = parser.read("Minor version")?;
    if !(major_version == 1 && minor_version == 0) {
        return Err(Error::InvalidTableVersion);
    }

    let var_store_offset = parser.read::<Offset32>("Item variation store offset")?.to_usize();
    let advance_width_mapping_offset = parser.read::<OptionalOffset32>("Advance width mapping offset")?.to_usize();
    let lsb_mapping_offset = parser.read::<OptionalOffset32>("Left side bearing mapping offset")?.to_usize();
    let rsb_mapping_offset = parser.read::<OptionalOffset32>("Right side bearing mapping offset")?.to_usize();

    parser.jump_to(start + var_store_offset)?;
    parser.begin_group("Item variation store");
    super::mvar::parse_item_variation_store(parser)?;
    parser.end_group();

    if let Some(offset) = advance_width_mapping_offset {
        parser.jump_to(start + offset)?;
        parser.begin_group("Advance width mapping");
        parse_delta_set(parser)?;
        parser.end_group();
    }

    if let Some(offset) = lsb_mapping_offset {
        parser.jump_to(start + offset)?;
        parser.begin_group("Left side bearing mapping");
        parse_delta_set(parser)?;
        parser.end_group();
    }

    if let Some(offset) = rsb_mapping_offset {
        parser.jump_to(start + offset)?;
        parser.begin_group("Right side bearing mapping");
        parse_delta_set(parser)?;
        parser.end_group();
    }

    Ok(())
}

pub fn parse_delta_set(parser: &mut Parser) -> Result<()> {
    let format = parser.read::<Masks>("Entry format")?;
    let count = parser.read::<u16>("Number of entries")?;

    let inner_index_bits = format.inner_index_bits();
    let entry_size = format.entry_size();
    parser.begin_group("Entries");
    for _ in 0..count {
        if entry_size == 1 {
            let entry = parser.peek::<u8>()?;
            let outer_index = entry >> (inner_index_bits + 1);
            let inner_index = entry & ((1 << (inner_index_bits + 1)) - 1);
            parser.read_value(1, "Entry",
                format!("Outer index: {}\nInner index: {}", outer_index, inner_index))?;
        } else if entry_size == 2 {
            let entry = parser.peek::<u16>()?;
            let outer_index = entry >> (inner_index_bits + 1);
            let inner_index = entry & ((1 << (inner_index_bits + 1)) - 1);
            parser.read_value(2, "Entry",
                 format!("Outer index: {}\nInner index: {}", outer_index, inner_index))?;
        } else {
            return Err(Error::InvalidValue);
        }
    }
    parser.end_group();

    Ok(())
}
