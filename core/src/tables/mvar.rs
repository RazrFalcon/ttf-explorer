use crate::parser::*;
use crate::{Error, Result};

pub fn parse(parser: &mut Parser) -> Result<()> {
    let major_version: u16 = parser.read("Major version")?;
    let minor_version: u16 = parser.read("Minor version")?;
    if !(major_version == 1 && minor_version == 0) {
        return Err(Error::InvalidTableVersion);
    }

    parser.read::<u16>("Reserved")?;
    parser.read::<u16>("Value record size")?;
    let values_count = parser.read::<u16>("Number of Value Records")?;
    parser.read::<Offset16>("Offset to the Item Variation Store")?;

    if values_count == 0 {
        // Nothing else to do.
        return Ok(());
    }

    parser.begin_group("Records");
    for i in 0..values_count {
        parser.begin_group(format!("Record {}", i));
        parser.read::<Tag>("Tag")?;
        parser.read::<u16>("A delta-set outer index")?;
        parser.read::<u16>("A delta-set inner index")?;
        parser.end_group();
    }
    parser.end_group();

    parser.begin_group("Item variation store");
    parse_item_variation_store(parser)?;
    parser.end_group();

    Ok(())
}

pub fn parse_item_variation_store(parser: &mut Parser) -> Result<()> {
    let start = parser.offset();

    parser.read::<u16>("Format")?;
    let var_list_offset = parser.read::<Offset32>("Offset to the variation region list")?.to_usize();
    let data_count = parser.read::<u16>("Number of item variation subtables")?;

    let mut offsets = Vec::new();
    if data_count != 0 {
        parser.begin_group("Offsets");
        for i in 0..data_count {
            let offset = parser.read2::<Offset32>(format!("Offset {}", i))?.to_usize();
            offsets.push(offset);
        }
        parser.end_group();
    }

    if var_list_offset != 0 {
        parser.jump_to(start + var_list_offset)?;
        parser.begin_group("Region list");
        parse_variation_region_list(parser)?;
        parser.end_group();
    }

    offsets.sort();
    for offset in offsets {
        parser.jump_to(start + offset)?;
        parser.begin_group("Item variation subtable");
        parse_item_variation_data(parser)?;
        parser.end_group();
    }

    Ok(())
}

fn parse_variation_region_list(parser: &mut Parser) -> Result<()> {
    let axis_count = parser.read::<u16>("Axis count")?;
    let region_count = parser.read::<u16>("Region count")?;

    for _ in 0..region_count {
        parser.begin_group("Region");

        for _ in 0..axis_count {
            parser.begin_group("Region axis");
            parser.read::<F2DOT14>("Start coordinate")?;
            parser.read::<F2DOT14>("Peak coordinate")?;
            parser.read::<F2DOT14>("End coordinate")?;
            parser.end_group();
        }

        parser.end_group();
    }

    Ok(())
}

fn parse_item_variation_data(parser: &mut Parser) -> Result<()> {
    let item_count = parser.read::<u16>("Number of delta sets")?;
    let short_delta_count = parser.read::<u16>("Number of short deltas")?;
    let region_index_count = parser.read::<u16>("Number of variation regions")?;

    if region_index_count != 0 {
        parser.begin_group("Region indices");
        for i in 0..region_index_count {
            parser.read2::<u16>(format!("Index {}", i))?;
        }
        parser.end_group();
    }

    if item_count != 0 {
        parser.begin_group("Delta-set rows");
        for i in 0..item_count {
            parser.begin_group(format!("Delta-set {}", i));

            for _ in 0..short_delta_count {
                parser.read::<i16>("Delta")?;
            }

            for _ in 0..(region_index_count - short_delta_count) {
                parser.read::<i8>("Delta")?;
            }

            parser.end_group();
        }
        parser.end_group();
    }

    Ok(())
}
