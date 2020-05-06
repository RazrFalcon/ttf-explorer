use std::ops::Range;

use crate::parser::*;
use crate::Result;

#[derive(Clone, Copy, PartialEq, Debug)]
enum GlyphType {
    Empty,
    Simple,
    Composite,
}


// A special case, where `u8` should be negative but not i8.
#[derive(Clone, Copy, Debug)]
struct NegativeU8(i16);

impl FromData for NegativeU8 {
    const SIZE: usize = 1;
    const NAME: &'static str = "UInt8";

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        Ok(NegativeU8(-(data[0] as i16)))
    }
}

impl std::fmt::Display for NegativeU8 {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.0.fmt(f)
    }
}


#[derive(Clone, Copy, Debug)]
struct SimpleGlyphFlags(u8);

impl SimpleGlyphFlags {
    #[inline] fn x_short(self) -> bool { self.0 & 0x02 != 0 }
    #[inline] fn y_short(self) -> bool { self.0 & 0x04 != 0 }
    #[inline] fn repeat_flag(self) -> bool { self.0 & 0x08 != 0 }
    #[inline] fn x_is_same_or_positive_short(self) -> bool { self.0 & 0x10 != 0 }
    #[inline] fn y_is_same_or_positive_short(self) -> bool { self.0 & 0x20 != 0 }
}

impl FromData for SimpleGlyphFlags {
    const NAME: &'static str = "BitFlags";

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        u8::parse(data).map(SimpleGlyphFlags)
    }
}

impl std::fmt::Display for SimpleGlyphFlags {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        use std::fmt::Write;

        let mut s = String::new();

        let bits = U8Bits(self.0);
        writeln!(&mut s, "{}", bits)?;
        if bits[0] { s.push_str("Bit 0: On curve point\n"); }
        if bits[1] { s.push_str("Bit 1: X-coordinate is 1 byte long\n"); }
        if bits[2] { s.push_str("Bit 2: Y-coordinate is 1 byte long\n"); }
        if bits[3] { s.push_str("Bit 3: Repeat flag\n"); }

        if bits[1] && bits[4] { s.push_str("Bit 4: X-coordinate is positive\n"); }
        if bits[1] && !bits[4] { s.push_str("Bit 4: X-coordinate is negative\n"); }
        if !bits[1] && bits[4] { s.push_str("Bit 4: Use the previous X-coordinate\n"); }
        if !bits[1] && !bits[4] { s.push_str("Bit 4: X-coordinate is 2 byte long, signed\n"); }

        if bits[2] && bits[5] { s.push_str("Bit 5: Y-coordinate is positive\n"); }
        if bits[2] && !bits[5] { s.push_str("Bit 5: Y-coordinate is negative\n"); }
        if !bits[2] && bits[5] { s.push_str("Bit 5: Use the previous Y-coordinate\n"); }
        if !bits[2] && !bits[5] { s.push_str("Bit 5: Y-coordinate is 2 byte long, signed\n"); }

        if bits[6] { s.push_str("Bit 6: Contours may overlap\n"); }
        // 7 - reserved

        s.pop(); // pop last new line

        f.write_str(&s)
    }
}


#[derive(Clone, Copy, Debug)]
struct CompositeGlyphFlags(u16);

impl CompositeGlyphFlags {
    #[inline] fn arg_1_and_2_are_words(self) -> bool { self.0 & 0x0001 != 0 }
    #[inline] fn args_are_xy_values(self) -> bool { self.0 & 0x0002 != 0 }
    #[inline] fn we_have_a_scale(self) -> bool { self.0 & 0x0008 != 0 }
    #[inline] fn more_components(self) -> bool { self.0 & 0x0020 != 0 }
    #[inline] fn we_have_an_x_and_y_scale(self) -> bool { self.0 & 0x0040 != 0 }
    #[inline] fn we_have_a_two_by_two(self) -> bool { self.0 & 0x0080 != 0 }
    #[inline] fn we_have_instructions(self) -> bool { self.0 & 0x0100 != 0 }
}

impl FromData for CompositeGlyphFlags {
    const NAME: &'static str = "BitFlags";

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        u16::parse(data).map(CompositeGlyphFlags)
    }
}

impl std::fmt::Display for CompositeGlyphFlags {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        use std::fmt::Write;
        let mut s = String::new();

        let bits = U16Bits(self.0);
        writeln!(&mut s, "{}", bits)?;
        if bits[0]  { s.push_str("Bit 0: Arguments are 16-bit\n"); }
        if bits[1]  { s.push_str("Bit 1: Arguments are signed xy values\n"); }
        if bits[2]  { s.push_str("Bit 2: Round XY to grid\n"); }
        if bits[3]  { s.push_str("Bit 3: Has a simple scale\n"); }
        // 4 - reserved
        if bits[5]  { s.push_str("Bit 5: Has more glyphs\n"); }
        if bits[6]  { s.push_str("Bit 6: Non-proportional scale\n"); }
        if bits[7]  { s.push_str("Bit 7: Has 2 by 2 transformation matrix\n"); }
        if bits[8]  { s.push_str("Bit 8: Has instructions after the last component\n"); }
        if bits[9]  { s.push_str("Bit 9: Use my metrics\n"); }
        if bits[10] { s.push_str("Bit 10: Components overlap\n"); }
        if bits[11] { s.push_str("Bit 11: Scaled component offset\n"); }
        if bits[12] { s.push_str("Bit 12: Unscaled component offset\n"); }
        // 13, 14, 15 - reserved

        s.pop(); // pop last new line

        f.write_str(&s)
    }
}


pub fn parse(ranges: &[Range<usize>], parser: &mut Parser) -> Result<()> {
    let start = parser.offset();

    for (i, range) in ranges.iter().enumerate() {
        // Skip empty ranges here and not in loca::collect_ranges,
        // so we can maintain a proper glyph index.
        if range.len() == 0 {
            continue;
        }

        parser.jump_to(start + range.start)?;
        parser.begin_group("");
        let kind = parse_glyph(parser)?;
        if kind == GlyphType::Composite {
            parser.end_group_with_title(format!("Glyph {} (composite)", i));
        } else {
            parser.end_group_with_title(format!("Glyph {}", i));
        }
    }

    Ok(())
}

fn parse_glyph(parser: &mut Parser) -> Result<GlyphType> {
    let num_of_contours = parser.read::<i16>("Number of contours")?;
    parser.read::<i16>("x min")?;
    parser.read::<i16>("y min")?;
    parser.read::<i16>("x max")?;
    parser.read::<i16>("y max")?;

    if num_of_contours == 0 {
        Ok(GlyphType::Empty)
    } else if num_of_contours > 0 {
        parse_simple_glyph(num_of_contours as u16, parser)?;
        Ok(GlyphType::Simple)
    } else {
        parse_composite_glyph(parser)?;
        Ok(GlyphType::Composite)
    }
}

fn parse_simple_glyph(num_of_contours: u16, parser: &mut Parser) -> Result<()> {
    let mut last_point = 0;
    parser.begin_group("Endpoints");
    for i in 0..num_of_contours {
        last_point = parser.read2::<u16>(format!("Endpoint {}", i))?;
    }
    parser.end_group();

    let instructions_len = parser.read::<u16>("Instructions size")?;
    if instructions_len > 0 {
        parser.read_bytes(instructions_len as usize, "Instructions")?;
    }

    parser.begin_group("Flags");
    let mut all_flags = Vec::new();
    let mut has_x_coords = false;
    let mut has_y_coords = false;
    let total_points = last_point + 1;
    let mut points_left = total_points;
    while points_left > 0 {
        let flags = parser.read::<SimpleGlyphFlags>("Flag")?;
        all_flags.push(flags);

        if flags.x_short() || !flags.x_is_same_or_positive_short() {
            has_x_coords = true;
        }

        if flags.y_short() || !flags.y_is_same_or_positive_short() {
            has_y_coords = true;
        }

        let repeats = if flags.repeat_flag() {
            let repeats = parser.read::<u8>("Number of repeats")?;
            for _ in 0..repeats {
                all_flags.push(flags);
            }

            repeats as u16 + 1
        } else {
            1
        };

        points_left -= repeats;
    }
    parser.end_group();

    if has_x_coords {
        parser.begin_group("X-coordinates");
        for flags in &all_flags {
            if flags.x_short() {
                if flags.x_is_same_or_positive_short() {
                    parser.read::<u8>("Coordinate")?;
                } else {
                    parser.read::<NegativeU8>("Coordinate")?;
                }
            } else {
                if flags.x_is_same_or_positive_short() {
                    // Nothing.
                } else {
                    parser.read::<i16>("Coordinate")?;
                }
            }
        }
        parser.end_group();
    }

    if has_y_coords {
        parser.begin_group("Y-coordinates");
        for flags in &all_flags {
            if flags.y_short() {
                if flags.y_is_same_or_positive_short() {
                    parser.read::<u8>("Coordinate")?;
                } else {
                    parser.read::<NegativeU8>("Coordinate")?;
                }
            } else {
                if flags.y_is_same_or_positive_short() {
                    // Nothing.
                } else {
                    parser.read::<i16>("Coordinate")?;
                }
            }
        }
        parser.end_group();
    }

    Ok(())
}

fn parse_composite_glyph(parser: &mut Parser) -> Result<()> {
    let flags = parser.read::<CompositeGlyphFlags>("Flag")?;
    parser.read::<GlyphId>("Glyph ID")?;

    let mut matrix = [0.0; 6];

    parser.begin_group("");
    if flags.args_are_xy_values() {
        if flags.arg_1_and_2_are_words() {
            matrix[4] = parser.read::<i16>("E")? as f32;
            matrix[5] = parser.read::<i16>("F")? as f32;
        } else {
            matrix[4] = parser.read::<i8>("E")? as f32;
            matrix[5] = parser.read::<i8>("F")? as f32;
        }
    }

    if flags.we_have_a_two_by_two() {
        matrix[0] = parser.read::<F2DOT14>("A")?.to_f32();
        matrix[1] = parser.read::<F2DOT14>("B")?.to_f32();
        matrix[2] = parser.read::<F2DOT14>("C")?.to_f32();
        matrix[3] = parser.read::<F2DOT14>("D")?.to_f32();
    } else if flags.we_have_an_x_and_y_scale() {
        matrix[0] = parser.read::<F2DOT14>("A")?.to_f32();
        matrix[3] = parser.read::<F2DOT14>("D")?.to_f32();
    } else if flags.we_have_a_scale() {
        matrix[0] = parser.read::<F2DOT14>("A")?.to_f32();
        matrix[3] = matrix[0];
    }

    parser.end_group_with_title(
        format!(
            "Matrix ({} {} {} {} {} {})",
            matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5],
        )
    );

    if flags.more_components() {
        parse_composite_glyph(parser)?;
    } else if flags.we_have_instructions() {
        let number_of_instructions = parser.read::<u16>("Number of instructions")?;
        parser.read_bytes(number_of_instructions as usize, "Instructions")?;
    }

    Ok(())
}
