use crate::parser::*;
use crate::Result;

const SHARED_POINT_NUMBERS: u16 = 0x8000;
const COUNT_MASK: u16 = 0x0FFF;

const EMBEDDED_PEAK_TUPLE: u16 = 0x8000;
const INTERMEDIATE_REGION: u16 = 0x4000;
const PRIVATE_POINT_NUMBERS: u16 = 0x2000;

const POINTS_ARE_WORDS: u8 = 0x80;
const POINT_RUN_COUNT_MASK: u8 = 0x7F;

const DELTAS_ARE_ZERO: u8 = 0x80;
const DELTAS_ARE_WORDS: u8 = 0x40;
const DELTA_RUN_COUNT_MASK: u8 = 0x3F;

pub fn parse(parser: &mut Parser) -> Result<()> {
    parser.read::<u16>("Major version")?;
    parser.read::<u16>("Minor version")?;
    let axis_count = parser.read::<u16>("Axis count")?;
    let shared_tuple_count = parser.read::<u16>("Shared tuple count")?;
    parser.read::<Offset32>("Offset to the shared tuple records")?;
    let glyph_count = parser.read::<u16>("Glyphs count")?;
    let flags = parser.read::<u16>("Flags")?;
    parser.read::<Offset32>("Offset to the array of Glyph Variation Data tables")?;
    let long_format = flags & 1 == 1;

    let mut offsets = Vec::new();

    // The total count is glyphCount+1.
    parser.begin_group_with_value("GlyphVariationData offsets", (glyph_count + 1).to_string());
    for _ in 0..=glyph_count {
        if long_format {
            offsets.push(parser.read::<Offset32>("Offset")?.to_usize());
        } else {
            offsets.push(parser.read::<Offset16>("Offset")?.to_usize() * 2);
        }
    }
    parser.end_group();

    parser.begin_group_with_value("Shared tuples", shared_tuple_count.to_string());
    for _ in 0..shared_tuple_count {
        parser.begin_group("Tuple record");
        for _ in 0..axis_count {
            parser.read::<F2DOT14>("Coordinate")?;
        }
        parser.end_group();
    }
    parser.end_group();

    // Dedup offsets. There can be multiple records with the same offset.
    offsets.dedup();

    let start = parser.offset();

    struct TupleHeader {
        data_size: u16,
        has_private_point_numbers: bool,
    }
    let mut headers_data = Vec::new();

    parser.begin_group_with_value("Tables", (offsets.len() - 1).to_string());
    offsets.remove(0);
    for (i, offset) in offsets.iter().enumerate() {
        headers_data.clear();

        parser.begin_group(format!("Glyph Variation Data {}", i));

        let value = parser.read::<u16>("Value")?;
        parser.read::<Offset16>("Data offset")?;

        // 'The high 4 bits are flags, and the low 12 bits
        // are the number of tuple variation tables for this glyph.'
        let has_shared_point_numbers = value & SHARED_POINT_NUMBERS != 0;
        let tuple_variation_count = value & COUNT_MASK;

        for _ in 0..tuple_variation_count {
            parser.begin_group("Tuple Variation Header");
            let data_size = parser.read::<u16>("Size of the serialized data")?;
            let tuple_index = parser.read::<u16>("Value")?;

            let has_embedded_peak_tuple = (tuple_index & EMBEDDED_PEAK_TUPLE) != 0;
            let has_intermediate_region = (tuple_index & INTERMEDIATE_REGION) != 0;
            let has_private_point_numbers = (tuple_index & PRIVATE_POINT_NUMBERS) != 0;

            headers_data.push(TupleHeader { data_size, has_private_point_numbers });

            if has_embedded_peak_tuple {
                parser.begin_group("Peak record");
                for _ in 0..axis_count {
                    parser.read::<F2DOT14>("Coordinate")?;
                }
                parser.end_group();
            }

            if has_intermediate_region {
                parser.begin_group("Peak record");
                for _ in 0..axis_count {
                    parser.read::<F2DOT14>("Coordinate")?;
                }
                for _ in 0..axis_count {
                    parser.read::<F2DOT14>("Coordinate")?;
                }
                parser.end_group();
            }

            parser.end_group();
        }

        if has_shared_point_numbers {
            parser.begin_group("Shared points");
            unpack_points(parser)?;
            parser.end_group();
        }

        for header in &headers_data {
            let start = parser.offset();

            if header.has_private_point_numbers {
                parser.begin_group("Private points");
                unpack_points(parser)?;
                parser.end_group();
            }

            let private_points_size = parser.offset() - start;

            parser.begin_group("Deltas");
            unpack_deltas(header.data_size as usize - private_points_size, parser)?;
            parser.end_group();
        }

        if parser.offset() - start < *offset {
            let padding = offset - (parser.offset() - start);
            if padding > 0 {
                parser.read_bytes(padding, "Padding")?;
            }
        }

        parser.end_group();
    }

    parser.end_group();

    Ok(())
}

fn unpack_points(parser: &mut Parser) -> Result<()> {
    let control = parser.read::<u8>("Control")?;

    if control == 0 {
        return Ok(());
    }

    let mut count = control as u16;
    if control & POINTS_ARE_WORDS != 0 {
        let b2 = parser.read::<u8>("Control")?;
        count = ((control & POINT_RUN_COUNT_MASK) as u16) << 8 | b2 as u16;
    }

    let mut i = 0;
    while i < count {
        let control = parser.read::<u8>("Control")?;
        let run_count = (control & POINT_RUN_COUNT_MASK) + 1;
        let mut j = 0;
        if control & POINTS_ARE_WORDS != 0 {
            while j < run_count && i < count {
                parser.read::<u16>("Point")?;
                i += 1;
                j += 1;
            }
        } else {
            while j < run_count && i < count {
                parser.read::<u8>("Point")?;
                i += 1;
                j += 1;
            }
        }
    }

    Ok(())
}

fn unpack_deltas(len: usize, parser: &mut Parser) -> Result<()> {
    let end = parser.offset() + len as usize;
    while parser.offset() < end {
        let control = parser.read::<u8>("Control")?;
        let run_count = (control & DELTA_RUN_COUNT_MASK) + 1;
        if control & DELTAS_ARE_ZERO != 0 {
            // Ignore.
        } else if control & DELTAS_ARE_WORDS != 0 {
            for _ in 0..run_count {
                parser.read::<u16>("Delta")?;
            }
        } else {
            for _ in 0..run_count {
                parser.read::<u8>("Delta")?;
            }
        }
    }

    Ok(())
}
