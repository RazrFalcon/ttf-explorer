use crate::parser::*;
use crate::{ValueType, Error, Result};
use super::IndexToLocFormat;

#[derive(Clone, Copy, Debug)]
struct HeadFlags(u16);

impl FromData for HeadFlags {
    const NAME: ValueType = ValueType::BitFlags;

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        u16::parse(data).map(HeadFlags)
    }
}

impl std::fmt::Display for HeadFlags {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        use std::fmt::Write;
        let mut s = String::new();

        let bits = U16Bits(self.0);
        writeln!(&mut s, "{}", bits)?;
        if bits[0]  { s.push_str("Bit 0: Baseline for font at y=0\n"); }
        if bits[1]  { s.push_str("Bit 1: Left sidebearing point at x=0\n"); }
        if bits[2]  { s.push_str("Bit 2: Instructions may depend on point size\n"); }
        if bits[3]  { s.push_str("Bit 3: Force ppem to integer values\n"); }
        if bits[4]  { s.push_str("Bit 4: Instructions may alter advance width\n"); }
        if bits[5]  { s.push_str("Bit 5: (AAT only) Vertical layout\n"); }
        // 6 - reserved
        if bits[7]  { s.push_str("Bit 7: (AAT only) Requires linguistic rendering\n"); }
        if bits[8]  { s.push_str("Bit 8: (AAT only) Has metamorphosis effects\n"); }
        if bits[9]  { s.push_str("Bit 9: (AAT only) Font contains strong right-to-left glyphs\n"); }
        if bits[10] { s.push_str("Bit 10: (AAT only) Font contains Indic-style rearrangement effects\n"); }
        if bits[11] { s.push_str("Bit 11: Font data is lossless\n"); }
        if bits[12] { s.push_str("Bit 12: Font converted\n"); }
        if bits[13] { s.push_str("Bit 13: Font optimized for ClearType\n"); }
        if bits[14] { s.push_str("Bit 14: Last Resort font\n"); }
        // 15 - reserved

        s.pop(); // pop last new line

        f.write_str(&s)
    }
}

#[derive(Clone, Copy, Debug)]
struct MacStyleFlags(u16);

impl FromData for MacStyleFlags {
    const NAME: ValueType = ValueType::BitFlags;

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        u16::parse(data).map(MacStyleFlags)
    }
}

impl std::fmt::Display for MacStyleFlags {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        use std::fmt::Write;

        let bits = U16Bits(self.0);

        let mut s = String::new();
        writeln!(&mut s, "{}", bits)?;
        if bits[0] { s.push_str("Bit 0: Bold\n"); }
        if bits[1] { s.push_str("Bit 1: Italic\n"); }
        if bits[2] { s.push_str("Bit 2: Underline\n"); }
        if bits[3] { s.push_str("Bit 3: Outline\n"); }
        if bits[4] { s.push_str("Bit 4: Shadow\n"); }
        if bits[5] { s.push_str("Bit 5: Condensed\n"); }
        if bits[6]  { s.push_str("Bit 6: Extended\n"); }
        // 7-15 - reserved

        s.pop(); // pop last new line

        f.write_str(&s)
    }
}

pub fn parse(parser: &mut Parser) -> Result<()> {
    let major_version = parser.read::<u16>("Major version")?;
    let minor_version = parser.read::<u16>("Minor version")?;
    if !(major_version == 1 && minor_version == 0) {
        return Err(Error::InvalidTableVersion);
    }

    parser.read::<Fixed>("Font revision")?;
    parser.read::<u32>("Checksum adjustment")?;
    parser.read::<u32>("Magic number")?;
    parser.read::<HeadFlags>("Flags")?;
    parser.read::<u16>("Units per EM")?;
    parser.read::<LongDateTime>("Created")?;
    parser.read::<LongDateTime>("Modified")?;
    parser.read::<i16>("X min for all glyph bounding boxes")?;
    parser.read::<i16>("Y min for all glyph bounding boxes")?;
    parser.read::<i16>("X max for all glyph bounding boxes")?;
    parser.read::<i16>("Y max for all glyph bounding boxes")?;
    parser.read::<MacStyleFlags>("Mac style")?;
    parser.read::<u16>("Smallest readable size in pixels")?;
    parser.read::<i16>("Font direction hint")?;
    parser.read::<i16>("Index to location format")?;
    parser.read::<i16>("Glyph data format")?;

    Ok(())
}

pub fn parse_index_format(data: &[u8]) -> Result<IndexToLocFormat> {
    let mut parser = SimpleParser::new(data);
    parser.advance(50);
    let idx: i16 = parser.read()?;
    match idx {
        0 => Ok(IndexToLocFormat::Short),
        1 => Ok(IndexToLocFormat::Long),
        _ => Err(Error::InvalidValue),
    }
}
