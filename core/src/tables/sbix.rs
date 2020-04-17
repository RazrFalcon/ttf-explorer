use crate::parser::*;
use crate::{Error, Result};

#[derive(Clone, Copy, Debug)]
struct SbixFlags(u16);

impl FromData for SbixFlags {
    const NAME: &'static str = "BitFlags";

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        u16::parse(data).map(SbixFlags)
    }
}

impl std::fmt::Display for SbixFlags {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        use std::fmt::Write;

        let mut s = String::new();

        let bits = U16Bits(self.0);
        writeln!(&mut s, "{}", bits)?;
        if bits[1] { s.push_str("Bit 1: Draw outlines\n"); }

        s.pop(); // pop last new line

        f.write_str(&s)
    }
}

pub fn parse(number_of_glyphs: u16, parser: &mut Parser) -> Result<()> {
    let start = parser.offset();

    let version = parser.read::<u16>("Version")?;
    if version != 1 {
        return Err(Error::InvalidTableVersion);
    }

    parser.read::<SbixFlags>("Flags")?;
    let num_strikes = parser.read::<u32>("Number of bitmap strikes")?;

    let mut offsets = Vec::new();
    parser.begin_group_with_value("Offsets", num_strikes.to_string());
    for i in 0..num_strikes {
        let offset = parser.read2::<Offset32>(format!("Offset {}", i))?.to_usize();
        offsets.push(offset);
    }
    parser.end_group();

    offsets.sort();
    offsets.dedup();

    let mut glyph_offsets = Vec::new();
    for offset in offsets {
        glyph_offsets.clear();

        parser.jump_to(start + offset)?;
        parser.begin_group("Strike");

        parser.read::<u16>("PPEM")?;
        parser.read::<u16>("PPI")?;

        parser.begin_group_with_value("Offsets", number_of_glyphs.to_string());
        for i in 0..=number_of_glyphs {
            let offset = parser.read2::<Offset32>(format!("Offset {}", i))?.to_usize();
            glyph_offsets.push(offset);
        }
        parser.end_group();

        glyph_offsets.sort();
        glyph_offsets.dedup();

        // The last offset is the end byte of the last glyph.
        for i in 0..glyph_offsets.len()-1 {
            let data_size = glyph_offsets[i + 1] - glyph_offsets[i];

            parser.begin_group("Glyph data");
            parser.jump_to(start + offset + glyph_offsets[i])?;
            parser.read::<i16>("Horizontal offset")?;
            parser.read::<i16>("Vertical offset")?;
            parser.read::<Tag>("Type")?;
            parser.read_bytes(data_size - 8, "Data")?;
            parser.end_group();
        }

        parser.end_group();
    }

    Ok(())
}
