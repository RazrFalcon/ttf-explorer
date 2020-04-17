use std::collections::HashSet;

use crate::parser::*;
use crate::Result;

// TODO: ligCaretListOffset

pub fn parse(parser: &mut Parser) -> Result<()> {
    let start = parser.offset();

    let major_version = parser.read::<u16>("Major version")?;
    let minor_version = parser.read::<u16>("Minor version")?;
    let glyph_class_def_offset = parser.read::<OptionalOffset16>("Offset to class definition table")?.to_usize();
    let attachment_point_list_offset = parser.read::<OptionalOffset16>("Offset to attachment point list table")?.to_usize();
    parser.read::<OptionalOffset16>("Offset to ligature caret list table")?.to_usize();
    let mark_attach_class_def_offset = parser.read::<OptionalOffset16>("Offset to class definition table for mark attachment type")?.to_usize();

    let mut mark_glyph_sets_def_offset = None;
    let mut var_store_offset = None;
    if major_version == 1 && minor_version == 2 {
        mark_glyph_sets_def_offset = parser.read::<OptionalOffset16>("Offset to the table of mark glyph set definitions")?.to_usize();
    } else if major_version == 1 && minor_version == 3 {
        mark_glyph_sets_def_offset = parser.read::<OptionalOffset16>("Offset to the table of mark glyph set definitions")?.to_usize();
        var_store_offset = parser.read::<OptionalOffset32>("Offset to the Item Variation Store table")?.to_usize();
    }

    // All subtable offsets are from a beginning of the GDEF header.

    // TODO: sort offsets

    if let Some(offset) = glyph_class_def_offset {
        parser.jump_to(start + offset)?;
        parser.begin_group("Class Definition Table");
        parse_class_definition_table(parser)?;
        parser.end_group();
    }

    if let Some(offset) = attachment_point_list_offset {
        parser.jump_to(start + offset)?;
        parser.begin_group("Attachment Point List Table");
        let coverage_offset = parser.read::<Offset16>("Offset to Coverage table")?.to_usize();
        let count = parser.read::<u16>("Number of glyphs with attachment points")?;

        let mut offsets = Vec::new();
        if count > 0 {
            parser.begin_group("Offsets to Attach Point tables");
            for i in 0..count {
                let offset = parser.read2::<Offset16>(format!("Offset {}", i))?.to_usize();
                offsets.push(offset);
            }
            parser.end_group();
        }

        parser.jump_to(start + offset + coverage_offset)?;
        parser.begin_group("Coverage table");
        parse_coverage_table(parser)?;
        parser.end_group();

        if !offsets.is_empty() {
            offsets.sort();
            offsets.dedup();

            parser.begin_group("Attach Point tables");
            for (i, offset) in offsets.iter().enumerate() {
                parser.jump_to(start + attachment_point_list_offset.unwrap() + *offset)?;
                parser.begin_group(format!("Attach Point {}" ,i));
                let count = parser.read::<u16>("Number of attachment points")?;
                for _ in 0..count {
                    parser.read::<u16>("Contour point index")?;
                }
                parser.end_group();
            }
            parser.end_group();
        }

        parser.end_group();
    }

    if let Some(offset) = mark_attach_class_def_offset {
        parser.jump_to(start + offset)?;
        parser.begin_group("Mark Attachment Class Definition Table");
        parse_class_definition_table(parser)?;
        parser.end_group();
    }

    if let Some(offset) = mark_glyph_sets_def_offset {
        parser.jump_to(start + offset)?;
        parser.begin_group("Mark Glyph Sets Table");

        let substart = parser.offset();
        parser.read::<u16>("Format")?; // TODO: format must be 1
        let mark_glyph_set_count = parser.read::<u16>("Number of mark glyph sets")?;

        if mark_glyph_set_count != 0 {
            let mut offsets_set = HashSet::new(); // Table can have duplicated offsets.
            parser.begin_group("Offsets to mark glyph set coverage tables");
            for _ in 0..mark_glyph_set_count {
                offsets_set.insert(parser.read::<Offset32>("Offset")?.to_usize());
            }
            parser.end_group();

            let mut offsets_list: Vec<_> = offsets_set.iter().collect();
            offsets_list.sort();

            for offset in offsets_list {
                parser.jump_to(substart + offset)?;
                parser.begin_group("Coverage table");
                parse_coverage_table(parser)?;
                parser.end_group();
            }
        }

        parser.end_group();
    }

    if let Some(offset) = var_store_offset {
        parser.jump_to(start + offset)?;
        parser.begin_group("Item Variation Store Table");
        super::mvar::parse_item_variation_store(parser)?;
        parser.end_group();
    }

    Ok(())
}

fn parse_class_definition_table(parser: &mut Parser) -> Result<()> {
    let class_format = parser.read::<u16>("Format")?;

    if class_format == 1 {
        parser.read::<u16>("First glyph ID")?;
        let glyph_count = parser.read::<u16>("Number of classes")?;
        for _ in 0..glyph_count {
            parser.read::<u16>("Class")?;
        }
    } else if class_format == 2 {
        let class_range_count = parser.read::<u16>("Number of records")?;
        for _ in 0..class_range_count {
            parser.begin_group("");
            let first = parser.read::<u16>("First glyph ID")?;
            let last = parser.read::<u16>("Last glyph ID")?;
            let class = parser.read::<u16>("Class")?;
            parser.end_group_with_title_and_value(
                "Class Range Record",
                format!("{}..{} {}", first, last, class),
            );
        }
    } else {
        // TODO: what to do?
    }

    Ok(())
}

fn parse_coverage_table(parser: &mut Parser) -> Result<()> {
    let format = parser.read::<u16>("Format")?;

    if format == 1 {
        let glyph_count = parser.read::<u16>("Number of glyphs")?;
        for _ in 0..glyph_count {
            parser.read::<GlyphId>("Glyph")?;
        }
    } else if format == 2 {
        let range_count = parser.read::<u16>("Number of records")?;
        for _ in 0..range_count {
            parser.begin_group("");
            let first = parser.read::<u16>("First glyph ID")?;
            let last = parser.read::<u16>("Last glyph ID")?;
            let index = parser.read::<u16>("Coverage Index of first glyph ID")?;
            parser.end_group_with_title_and_value(
                "Range Record",
                format!("{}..{} {}", first, last, index),
            );
        }
    } else {
        // TODO: what to do?
    }

    Ok(())
}
