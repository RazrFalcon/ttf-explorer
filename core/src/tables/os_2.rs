use crate::parser::*;
use crate::Result;

#[derive(Clone, Copy, Debug)]
struct WeightClass(u16);

impl FromData for WeightClass {
    const NAME: &'static str = "UInt16";

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        u16::parse(data).map(WeightClass)
    }
}

impl std::fmt::Display for WeightClass {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let name = match self.0 {
            100 => "Thin",
            200 => "Extra-light",
            300 => "Light",
            400 => "Normal",
            500 => "Medium",
            600 => "Semi-bold",
            700 => "Bold",
            800 => "Extra-bold",
            900 => "Black",
            _ => "Other",
        };

        write!(f, "{} ({})", name, self.0)
    }
}


#[derive(Clone, Copy, Debug)]
struct WidthClass(u16);

impl FromData for WidthClass {
    const NAME: &'static str = "UInt16";

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        u16::parse(data).map(WidthClass)
    }
}

impl std::fmt::Display for WidthClass {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let name = match self.0 {
            1 => "Ultra-condensed",
            2 => "Extra-condensed",
            3 => "Condensed",
            4 => "Semi-condensed",
            5 => "Normal",
            6 => "Semi-expanded",
            7 => "Expanded",
            8 => "Extra-expanded",
            9 => "Ultra-expanded",
            _ => "Invalid",
        };

        write!(f, "{} ({})", name, self.0)
    }
}


#[derive(Clone, Copy, Debug)]
struct TypeFlags(u16);

impl FromData for TypeFlags {
    const NAME: &'static str = "BitFlags";

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        u16::parse(data).map(TypeFlags)
    }
}

impl std::fmt::Display for TypeFlags {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        use std::fmt::Write;

        let bits = U16Bits(self.0);

        let mut s = String::new();
        writeln!(&mut s, "{}", bits)?;
        let permissions = match self.0 & 0x000F {
            0 => "Installable",
            2 => "Restricted License",
            4 => "Preview & Print",
            8 => "Editable",
            _ => "Invalid",
        };
        writeln!(&mut s, "Bits 0-3: Usage permissions: {}", permissions)?;

        if bits[8] { s.push_str("Bit 8: No subsetting\n"); }
        if bits[9] { s.push_str("Bit 9: Bitmap embedding only\n"); }

        s.pop(); // pop last new line

        f.write_str(&s)
    }
}


#[derive(Clone, Copy, Debug)]
struct FontSelectionFlags(u16);

impl FromData for FontSelectionFlags {
    const NAME: &'static str = "BitFlags";

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        u16::parse(data).map(FontSelectionFlags)
    }
}

impl std::fmt::Display for FontSelectionFlags {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        use std::fmt::Write;

        let mut s = String::new();

        let bits = U16Bits(self.0);
        writeln!(&mut s, "{}", bits)?;
        if bits[0] { s.push_str("Bit 0: Italic\n"); }
        if bits[1] { s.push_str("Bit 1: Underscored\n"); }
        if bits[2] { s.push_str("Bit 2: Negative\n"); }
        if bits[3] { s.push_str("Bit 3: Outlined\n"); }
        if bits[4] { s.push_str("Bit 4: Overstruck\n"); }
        if bits[5] { s.push_str("Bit 5: Bold\n"); }
        if bits[6] { s.push_str("Bit 6: Regular\n"); }
        if bits[7] { s.push_str("Bit 7: Use typographic metrics\n"); }
        if bits[8] { s.push_str("Bit 8: WWS\n"); }
        if bits[9] { s.push_str("Bit 9: Oblique\n"); }
        // 10–15 - reserved

        s.pop(); // pop last new line

        f.write_str(&s)
    }
}


pub fn parse(parser: &mut Parser) -> Result<()> {
    let version = parser.read::<u16>("Version")?;

    parser.read::<i16>("Average weighted escapement")?;
    parser.read::<WeightClass>("Weight class")?;
    parser.read::<WidthClass>("Width class")?;
    parser.read::<TypeFlags>("Type flags")?;
    parser.read::<i16>("Subscript horizontal font size")?;
    parser.read::<i16>("Subscript vertical font size")?;
    parser.read::<i16>("Subscript X offset")?;
    parser.read::<i16>("Subscript Y offset.")?;
    parser.read::<i16>("Superscript horizontal font size")?;
    parser.read::<i16>("Superscript vertical font size")?;
    parser.read::<i16>("Superscript X offset")?;
    parser.read::<i16>("Superscript Y offset")?;
    parser.read::<i16>("Strikeout size")?;
    parser.read::<i16>("Strikeout position")?;
    parser.read::<i16>("Font-family class")?;

    parser.begin_group("panose");
    parser.read::<u8>("Family type")?;
    parser.read::<u8>("Serif style")?;
    parser.read::<u8>("Weight")?;
    parser.read::<u8>("Proportion")?;
    parser.read::<u8>("Contrast")?;
    parser.read::<u8>("Stroke variation")?;
    parser.read::<u8>("Arm style")?;
    parser.read::<u8>("Letterform")?;
    parser.read::<u8>("Midline")?;
    parser.read::<u8>("x height")?;
    parser.end_group();

    parser.read::<u32>("Unicode Character Range 1")?; // TODO
    parser.read::<u32>("Unicode Character Range 2")?;
    parser.read::<u32>("Unicode Character Range 3")?;
    parser.read::<u32>("Unicode Character Range 4")?;
    parser.read::<Tag>("Font Vendor Identification")?;
    parser.read::<FontSelectionFlags>("Font selection flags")?;
    parser.read::<u16>("The minimum Unicode index")?;
    parser.read::<u16>("The maximum Unicode index")?;
    parser.read::<i16>("Typographic ascender")?;
    parser.read::<i16>("Typographic descender")?;
    parser.read::<i16>("Typographic line gap")?;
    parser.read::<u16>("Windows ascender")?;
    parser.read::<u16>("Windows descender")?;

    if version == 0 {
        return Ok(());
    }

    parser.read::<u32>("Code Page Character Range 1")?; // TODO
    parser.read::<u32>("Code Page Character Range 2")?;

    if version < 2 {
        return Ok(());
    }

    parser.read::<i16>("x height")?;
    parser.read::<i16>("Capital height")?;
    parser.read::<u16>("Default character")?;
    parser.read::<u16>("Break character")?;
    parser.read::<u16>("The maximum glyph context")?;

    if version < 5 {
        return Ok(());
    }

    parser.read::<u16>("Lower optical point size")?;
    parser.read::<u16>("Upper optical point size")?;

    Ok(())
}
