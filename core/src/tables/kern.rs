use std::collections::HashSet;

use crate::parser::*;
use crate::{TitleKind, ValueType, Error, Result};

pub fn parse(parser: &mut Parser) -> Result<()> {
    // The `kern` table has two variants: OpenType one and Apple one.
    // And they both have different headers.
    // There are no robust way to distinguish them, so we have to guess.
    //
    // The OpenType one has the first two bytes (UInt16) as a version set to 0.
    // While Apple one has the first four bytes (Fixed) set to 1.0
    // So the first two bytes in case of an OpenType format will be 0x0000
    // and 0x0001 in case of an Apple format.
    let version = parser.peek::<u16>()?;
    if version == 0 {
        ot::parse(parser)?;
    } else {
        aat::parse(parser)?;
    }

    Ok(())
}

// https://docs.microsoft.com/en-us/typography/opentype/spec/kern
mod ot {
    use super::*;

    #[derive(Clone, Copy, Debug)]
    struct Coverage(u8);

    impl FromData for Coverage {
        const NAME: ValueType = ValueType::BitFlags;

        #[inline]
        fn parse(data: &[u8]) -> Result<Self> {
            Ok(Coverage(data[0]))
        }
    }

    impl std::fmt::Display for Coverage {
        fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
            use std::fmt::Write;
            let mut s = String::new();

            let bits = U8Bits(self.0);
            writeln!(&mut s, "{}", bits)?;
            if bits[0] { s.push_str("Bit 0: Horizontal\n"); }
            if bits[1] { s.push_str("Bit 1: Has minimum values\n"); }
            if bits[2] { s.push_str("Bit 2: Cross-stream\n"); }
            if bits[3] { s.push_str("Bit 3: Override\n"); }
            // 4-7 - reserved

            s.pop(); // pop last new line

            f.write_str(&s)
        }
    }

    pub fn parse(parser: &mut Parser) -> Result<()> {
        parser.read::<u16>("Version")?;
        let number_of_tables = parser.read::<u16>("Number of tables")?;
        for _ in 0..number_of_tables {
            let subtable_start = parser.offset();

            parser.begin_group("");
            parser.read::<u16>("Version")?;
            parser.read::<u16>("Length")? as usize;
            let format = parser.read::<u8>("Format")?;
            parser.read::<Coverage>("Coverage")?;

            match format {
                0 => aat::parse_format0(parser)?,
                2 => aat::parse_format2(subtable_start, parser)?,
                _ => return Err(Error::Custom(format!("{} is not a valid format", format))),
            }

            parser.end_group_with_title(format!("Table Format {}", format));
        }

        Ok(())
    }
}

// https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6kern.html
mod aat {
    use super::*;

    #[derive(Clone, Copy, Debug)]
    struct Coverage(u8);

    impl FromData for Coverage {
        const NAME: ValueType = ValueType::BitFlags;

        #[inline]
        fn parse(data: &[u8]) -> Result<Self> {
            Ok(Coverage(data[0]))
        }
    }

    impl std::fmt::Display for Coverage {
        fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
            use std::fmt::Write;
            let mut s = String::new();

            let bits = U8Bits(self.0);
            writeln!(&mut s, "{}", bits)?;
            // 0-4 - reserved
            if bits[5] { s.push_str("Bit 5: Has variation\n"); }
            if bits[6] { s.push_str("Bit 6: Cross-stream\n"); }
            if bits[7] { s.push_str("Bit 7: Vertical\n"); }

            s.pop(); // pop last new line

            f.write_str(&s)
        }
    }


    #[derive(Clone, Copy, Debug)]
    struct EntryFlags(u16);

    impl FromData for EntryFlags {
        const NAME: ValueType = ValueType::BitFlags;

        #[inline]
        fn parse(data: &[u8]) -> Result<Self> {
            u16::parse(data).map(EntryFlags)
        }
    }

    impl std::fmt::Display for EntryFlags {
        fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
            use std::fmt::Write;
            let mut s = String::new();

            let bits = U16Bits(self.0);
            writeln!(&mut s, "{}", self.0 & 0x3FFF)?;
            writeln!(&mut s, "Offset {}", bits)?;
            if bits[15] { s.push_str("Bit 15: Push onto the kerning stack\n"); }

            s.pop(); // pop last new line

            f.write_str(&s)
        }
    }


    #[derive(Clone, Copy, Debug)]
    struct Action(u16);

    impl FromData for Action {
        const NAME: ValueType = ValueType::BitFlags;

        #[inline]
        fn parse(data: &[u8]) -> Result<Self> {
            u16::parse(data).map(Action)
        }
    }

    impl std::fmt::Display for Action {
        fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
            match self.0 {
                0x0001 => f.write_str("Kerning 0. End of List."),
                0x8001 => f.write_str("Reset cross-stream. End of List."),
                _ => write!(f, "Kerning {}", self.0 as i16),
            }
        }
    }


    struct Entry {
        new_state: u16,
        #[allow(dead_code)]
        flags: u16,
    }


    pub fn parse(parser: &mut Parser) -> Result<()> {
        parser.read::<Fixed>("Version")?;
        let number_of_tables = parser.read::<u32>("Number of tables")?;
        for _ in 0..number_of_tables {
            let subtable_start = parser.offset();

            parser.begin_group("");
            let length = parser.read::<u32>("Length")? as usize;
            // Yes, the coverage and format order is inverted in AAT.
            parser.read::<Coverage>("Coverage")?;
            let format = parser.read::<u8>("Format")?;
            parser.read::<u16>("Tuple index")?;

            match format {
                0 => parse_format0(parser)?,
                1 => parse_format1(length, parser)?,
                2 => parse_format2(subtable_start, parser)?,
                3 => parse_format3(subtable_start, length, parser)?,
                _ => return Err(Error::Custom(format!("{} is not a valid format", format))),
            }

            parser.end_group_with_title(format!("Table Format {}", format));
        }

        Ok(())
    }

    pub fn parse_format0(parser: &mut Parser) -> Result<()> {
        let count = parser.read::<u16>("Number of kerning pairs")?;
        parser.read::<u16>("Search range")?;
        parser.read::<u16>("Entry selector")?;
        parser.read::<u16>("Range shift")?;

        parser.begin_group("Values");
        for _ in 0..count {
            parser.begin_group("Pair");
            parser.read::<GlyphId>("Left")?;
            parser.read::<GlyphId>("Right")?;
            parser.read::<i16>("Value")?;
            parser.end_group();
        }
        parser.end_group();

        Ok(())
    }

    fn parse_format1(subtable_length: usize, parser: &mut Parser) -> Result<()> {
        // Sadly, the *State Table for Contextual Kerning* format
        // isn't really documented.
        // The AAT kern documentation has only a rough outline and a single example,
        // which is clearly not enough to understand how it actually stored.
        // And the only FOSS application I know that supports it is HarfBuzz,
        // so we are borrowing its implementation.
        // No idea how long it took them to figure it out.
        //
        // Also, there is an obsolete tool called `ftxanalyzer`,
        // from the *Apple Font Tool Suite* that can dump an AAT kern table as an XML.
        // But the result is still pretty abstract and it's hard to understand
        // how it was produced.
        // It's not trivial to install it on a modern Mac, so here is a useful
        // instruction (not mine):
        // https://gist.github.com/thetrevorharmon/9afdeb41a74f8f32b9561eeb83b10eff
        //
        // And here are some Apple fonts that actually use this format
        // (at least in macOS 10.14):
        // - Diwan Thuluth.ttf
        // - Farisi.ttf
        // - GeezaPro.ttc
        // - Mishafi.ttf
        // - Waseem.ttc

        let start = parser.offset();

        let num_of_classes = parser.read::<u16>("Number of classes")?;
        // Note that offsets are not from the subtable start,
        // but from subtable start + header.
        let class_table_offset = parser.read::<Offset16>("Offset to class subtable")?.to_usize();
        let state_array_offset = parser.read::<Offset16>("Offset to state array")?.to_usize();
        let entry_table_offset = parser.read::<Offset16>("Offset to entry table")?.to_usize();
        let values_offset = parser.read::<Offset16>("Offset to values")?.to_usize();

        // Check that offsets are properly ordered.
        // We do not support random order.
        assert!(class_table_offset < state_array_offset);
        assert!(state_array_offset < entry_table_offset);
        assert!(entry_table_offset < values_offset);

        let number_of_entries = {
            // Collect states.
            parser.jump_to(start + state_array_offset)?;
            let mut simple = parser.to_simple();
            // We don't know the length of the states yet,
            // so use all the data from offset to the end of the subtable.
            let states = simple.read_bytes(subtable_length - state_array_offset)?.to_vec();

            // Collect entries.
            parser.jump_to(start + entry_table_offset)?;
            let mut simple = parser.to_simple();
            // We don't know the length of the states yet,
            // so use all the data from offset to the end of the subtable.
            let entries_count = (subtable_length - entry_table_offset) / 4;
            let mut entries = Vec::new();
            for _ in 0..entries_count {
                entries.push(Entry {
                    new_state: simple.read()?,
                    flags: simple.read()?,
                });
            }

            detect_number_of_entries(
                num_of_classes, state_array_offset as u16, &states, &entries,
            )?
        };

        parser.jump_to(start + class_table_offset)?;
        parser.begin_group("Class subtable");
        parser.read::<GlyphId>("First glyph")?;
        let num_glyphs = parser.read::<u16>("Number of glyphs")?;
        parser.read_array::<u8>("Classes", TitleKind::Class, num_glyphs as usize)?;
        parser.end_group();

        parser.jump_to(start + state_array_offset)?;
        parser.begin_group("State array");
        // We only assume that an *entry table* is right after *state array*.
        for _ in 0..((entry_table_offset - state_array_offset) / num_of_classes as usize) {
            parser.read_bytes(num_of_classes as usize, "State")?;
        }
        parser.end_group();

        parser.jump_to(start + entry_table_offset)?;
        parser.begin_group_with_value("Entry table", number_of_entries.to_string());
        for i in 0..number_of_entries {
            parser.begin_group(format!("Entry {}", i));
            parser.read::<Offset16>("State offset")?;
            parser.read::<EntryFlags>("Flags")?;
            parser.end_group();
        }
        parser.end_group();

        parser.jump_to(start + values_offset)?;
        let num_of_actions = ((subtable_length - 8) - (parser.offset() - start)) / 2;
        parser.read_array::<Action>("Actions", TitleKind::Action, num_of_actions)?;

        Ok(())
    }

    // Based on: https://github.com/harfbuzz/harfbuzz/blob/5b91c52083aee1653c0cf1e778923de00c08fa5d/src/hb-aat-layout-common.hh#L524
    fn detect_number_of_entries(
        num_of_classes: u16,
        state_array_offset: u16,
        states: &[u8],
        entries: &[Entry],
    ) -> Result<u32> {
        let num_of_classes = num_of_classes as i32;

        let mut min_state: i32 = 0;
        let mut max_state: i32 = 0;
        let mut num_entries: u32 = 0;

        let mut state_pos: i32 = 0;
        let mut state_neg: i32 = 0;
        let mut entry: u32 = 0;
        let mut max_ops: i32 = 0x3FFFFFFF;
        while min_state < state_neg || state_pos <= max_state {
            if min_state < state_neg {
                // Negative states.

                max_ops -= state_neg - min_state;
                if max_ops <= 0 {
                    return Err(Error::Custom("invalid state machine".to_string()));
                }

                // Sweep new states.
                let end = min_state * num_of_classes;
                for i in (0..end).rev() {
                    num_entries = std::cmp::max(num_entries, states[(i-1) as usize] as u32 + 1);
                }

                state_neg = min_state;
            }

            if state_pos <= max_state {
                // Positive states.

                max_ops -= max_state - state_pos + 1;
                if max_ops <= 0 {
                    return Err(Error::Custom("invalid state machine".to_string()));
                }

                // Sweep new states.
                let start = state_pos * num_of_classes;
                let end = (max_state + 1) * num_of_classes;
                for i in start..end {
                    num_entries = std::cmp::max(num_entries, states[i as usize] as u32 + 1);
                }

                state_pos = max_state + 1;
            }

            max_ops -= (num_entries - entry) as i32;
            if max_ops <= 0 {
                return Err(Error::Custom("invalid state machine".to_string()));
            }

            // Sweep new entries.
            for i in entry..num_entries {
                let new_state = (entries[i as usize].new_state as i32 - state_array_offset as i32)
                    / num_of_classes as i32;

                min_state = std::cmp::min(min_state, new_state);
                max_state = std::cmp::max(max_state, new_state);
            }

            entry = num_entries;
        }

        Ok(num_entries)
    }

    pub fn parse_format2(subtable_start: usize, parser: &mut Parser) -> Result<()> {
        // Apple fonts that are using this table: Apple Chancery.ttf

        parser.read::<u16>("Row width in bytes")?;

        enum OffsetKind {
            LeftHandTable,
            RightHandTable,
            Array,
        }

        let left_hand_table_offset = parser.read::<Offset16>("Offset to left-hand class table")?.to_usize();
        let right_hand_table_offset = parser.read::<Offset16>("Offset to right-hand class table")?.to_usize();
        let array_offset = parser.read::<Offset16>("Offset to kerning array")?.to_usize();

        let rows = detect_number_of_classes(subtable_start + left_hand_table_offset, parser)?;
        let columns = detect_number_of_classes(subtable_start + right_hand_table_offset, parser)?;

        // We have to sort offsets, because they can be in any order.
        let mut offsets = [
            (OffsetKind::LeftHandTable, left_hand_table_offset),
            (OffsetKind::RightHandTable, right_hand_table_offset),
            (OffsetKind::Array, array_offset),
        ];
        offsets.sort_by_key(|v| v.1);

        for (kind, offset) in offsets.iter() {
            match kind {
                OffsetKind::LeftHandTable => {
                    parser.jump_to(subtable_start + offset)?;
                    parser.begin_group("Left-hand class table");
                    parser.read::<GlyphId>("First glyph")?;
                    let count = parser.read::<u16>("Number of glyphs")?;
                    parser.read_array::<u16>("Classes", TitleKind::Class, count as usize)?;
                    parser.end_group();
                }
                OffsetKind::RightHandTable => {
                    parser.jump_to(subtable_start + offset)?;
                    parser.begin_group("Right-hand class table");
                    parser.read::<GlyphId>("First glyph")?;
                    let count = parser.read::<u16>("Number of glyphs")?;
                    parser.read_array::<u16>("Classes", TitleKind::Class, count as usize)?;
                    parser.end_group();
                }
                OffsetKind::Array => {
                    parser.jump_to(subtable_start + offset)?;
                    parser.read_array::<i16>("Kerning values", TitleKind::Value, rows * columns)?;
                }
            }
        }

        Ok(())
    }

    fn detect_number_of_classes(offset: usize, parser: &mut Parser) -> Result<usize> {
        parser.jump_to(offset)?;
        let mut parser = parser.to_simple();
        parser.read::<GlyphId>()?;
        let count = parser.read::<u16>()?;
        let mut classes = HashSet::new();
        for _ in 0..count {
            classes.insert(parser.read::<u16>()?);
        }

        Ok(classes.len())
    }

    fn parse_format3(subtable_start: usize, subtable_len: usize, parser: &mut Parser) -> Result<()> {
        // Apple fonts that are using this table: Skia.ttf

        let glyph_count = parser.read::<u16>("Number of glyphs")?;
        let kern_values = parser.read::<u8>("Number of kerning values")?;
        let left_hand_classes = parser.read::<u8>("Number of left-hand classes")?;
        let right_hand_classes = parser.read::<u8>("Number of right-hand classes")?;
        parser.read::<u8>("Reserved")?;

        parser.read_array::<i16>("Kerning values", TitleKind::Value, kern_values as usize)?;
        parser.read_array::<u8>("Left-hand classes", TitleKind::Class, glyph_count as usize)?;
        parser.read_array::<u8>("Right-hand classes", TitleKind::Class, glyph_count as usize)?;
        parser.read_array::<u8>("Indices", TitleKind::Index,
                                left_hand_classes as usize * right_hand_classes as usize)?;

        let left = subtable_len as isize - (parser.offset() - subtable_start) as isize;
        if left > 0 {
            parser.read_bytes(left as usize, "Padding")?;
        }

        Ok(())
    }
}
