use std::ops::Range;

use parser::*;

macro_rules! try_opt_or {
    ($value:expr, $ret:expr) => {
        match $value {
            Some(v) => v,
            None => return $ret,
        }
    };
}

mod ffi;
mod parser;
mod tables;

pub struct NodeData {
    pub title: String,
    pub value: String,
    pub value_type: String, // TODO: to enum
    pub range: std::ops::Range<usize>,
}

#[derive(Clone, Copy)]
pub enum Error {
    InvalidValue,
    InvalidTableVersion,
    MissingTable,
    ReadOutOfBounds,
}

impl std::fmt::Display for Error {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Error::InvalidValue => write!(f, "invalid value"),
            Error::InvalidTableVersion => write!(f, "invalid table version"),
            Error::MissingTable => write!(f, "missing table"),
            Error::ReadOutOfBounds => write!(f, "read out of bounds"),
        }
    }
}


type Result<T> = std::result::Result<T, Error>;

#[derive(Clone, Copy, Debug)]
enum FontMagic {
    TrueType,
    OpenType,
    FontCollection,
}

impl FromData for FontMagic {
    const SIZE: usize = 4;
    const NAME: &'static str = "Magic";

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        match u32::parse(data)? {
            0x00010000 => Ok(FontMagic::TrueType),
            0x4F54544F => Ok(FontMagic::OpenType),
            0x74746366 => Ok(FontMagic::FontCollection),
            _ => Err(Error::InvalidValue),
        }
    }
}

impl std::fmt::Display for FontMagic {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{:?}", self)
    }
}


#[derive(Clone, Debug)]
struct FontTable {
    index: u32, // Index in font collection.
    title: String,
    tag: Tag,
    range: Range<usize>,
}

fn parse(
    data: &[u8],
    warnings: &mut String,
    tree: &mut ego_tree::Tree<NodeData>,
) -> Result<()> {
    let mut parser = Parser::new(data, tree);
    let mut tables = Vec::new();

    let magic = parser.peek::<FontMagic>()?;
    match magic {
        FontMagic::TrueType | FontMagic::OpenType => {
            parse_header(0, &mut tables, &mut parser)?;
        }
        FontMagic::FontCollection => {
            parser.begin_group("Header");
            parser.read::<FontMagic>("Magic")?;
            let major_version = parser.read::<u16>("Major version")?;
            parser.read::<u16>("Minor version")?;
            let num_fonts = parser.read::<u32>("Number of fonts")?;

            let mut offsets = Vec::new();
            if num_fonts != 0 {
                parser.begin_group("Offsets");
                for _ in 0..num_fonts {
                    offsets.push(parser.read::<Offset32>("Offset")?.to_usize());
                }
                parser.end_group();
            }

            offsets.sort();
            offsets.dedup();

            if major_version == 2 {
                parser.read::<Tag>("DSIG tag")?;
                parser.read::<u32>("DSIG table length")?;
                parser.read::<OptionalOffset32>("DSIG table offset")?;
            }

            parser.end_group();

            for (i, offset) in offsets.iter().enumerate() {
                parser.jump_to(*offset)?;
                parser.begin_group("Font");
                parse_header(i as u32, &mut tables, &mut parser)?;
                parser.end_group();
            }
        }
    }

    tables.sort_by_key(|v| v.range.start);

    let mut parsed_offsets = Vec::new();
    for table in &tables {
        // There can be multiple tables pointing to the same data.
        // Mainly in Font Collections.
        // We cannon simply dedup the `tables`, because tables have different indexes
        // and it will break the `find_table` closure.
        // So we have to check for duplicates manually.
        if parsed_offsets.contains(&table.range.start) {
            continue;
        }

        parsed_offsets.push(table.range.start);

        let node_id = parser.current_group_node();

        parser.jump_to(table.range.start)?;
        parser.begin_group(&table.title);

        if let Err(e) = parse_table(data, &tables, table, &mut parser) {
            use std::fmt::Write;
            writeln!(warnings, "Failed to parse a '{}' table cause {}.", table.tag, e).unwrap();

            while parser.current_group_node() != node_id {
                parser.end_group();
            }
        } else {
            parser.end_group();
        }
    }

    Ok(())
}

fn parse_table(
    data: &[u8],
    tables: &[FontTable],
    table: &FontTable,
    parser: &mut Parser,
) -> Result<()> {
    let find_table = |index, tag: &[u8]| {
        let table = tables.iter()
            .find(|t| t.index == index && &t.tag.to_bytes() == tag)
            .ok_or(Error::MissingTable)?;
        data.try_slice(table.range.clone())
    };

    match &table.tag.to_bytes() {
        b"avar" => tables::avar::parse(parser)?,
        b"bdat" => {
            let cblc_data = find_table(table.index, b"bloc")?;
            let indices = tables::cblc::collect_indices(cblc_data)?;

            tables::cbdt::parse(&indices, parser)?;
        }
        b"bloc" => tables::cblc::parse(parser)?,
        b"CBDT" => {
            let cblc_data = find_table(table.index, b"CBLC")?;
            let indices = tables::cblc::collect_indices(cblc_data)?;

            tables::cbdt::parse(&indices, parser)?;
        }
        b"CBLC" => tables::cblc::parse(parser)?,
        b"CFF " => tables::cff::parse(parser)?,
        b"CFF2" => tables::cff2::parse(parser)?,
        b"cmap" => tables::cmap::parse(parser)?,
        b"cvt " => parser.read_array::<i16>("Values", "Value", table.range.len() / 2)?,
        b"EBDT" => {
            let cblc_data = find_table(table.index, b"EBLC")?;
            let indices = tables::cblc::collect_indices(cblc_data)?;

            tables::cbdt::parse(&indices, parser)?;
        }
        b"EBLC" => tables::cblc::parse(parser)?,
        b"fpgm" => {
            parser.read_bytes(table.range.len(), "Instructions")?;
        }
        b"fvar" => tables::fvar::parse(parser)?,
        b"GDEF" => tables::gdef::parse(parser)?,
        b"glyf" => {
            let head_data = find_table(table.index, b"head")?;
            let maxp_data = find_table(table.index, b"maxp")?;
            let loca_data = find_table(table.index, b"loca")?;

            let index = tables::head::parse_index_format(head_data)?;
            let number_of_glyphs = tables::maxp::parse_number_of_glyphs(maxp_data)?;
            let ranges = tables::loca::collect_ranges(number_of_glyphs, index, loca_data)?;

            tables::glyf::parse(&ranges, parser)?;
        }
        b"gvar" => tables::gvar::parse(parser)?,
        b"head" => tables::head::parse(parser)?,
        b"hhea" => tables::hhea::parse(parser)?,
        b"HVAR" => tables::hvar::parse(parser)?,
        b"hmtx" => {
            let hhea_data = find_table(table.index, b"hhea")?;
            let maxp_data = find_table(table.index, b"maxp")?;

            let number_of_metrics = tables::hhea::parse_number_of_metrics(hhea_data)?;
            let number_of_glyphs = tables::maxp::parse_number_of_glyphs(maxp_data)?;

            tables::hmtx::parse(number_of_metrics, number_of_glyphs, parser)?;
        }
        b"kern" => tables::kern::parse(parser)?,
        b"loca" => {
            let head_data = find_table(table.index, b"head")?;
            let maxp_data = find_table(table.index, b"maxp")?;

            let index = tables::head::parse_index_format(head_data)?;
            let number_of_glyphs = tables::maxp::parse_number_of_glyphs(maxp_data)?;

            tables::loca::parse(number_of_glyphs, index, parser)?;
        }
        b"maxp" => tables::maxp::parse(parser)?,
        b"MVAR" => tables::mvar::parse(parser)?,
        b"name" => tables::name::parse(parser)?,
        b"OS/2" => tables::os_2::parse(parser)?,
        b"sbix" => {
            let maxp_data = find_table(table.index, b"maxp")?;
            let number_of_glyphs = tables::maxp::parse_number_of_glyphs(maxp_data)?;

            tables::sbix::parse(number_of_glyphs, parser)?;
        }
        b"post" => tables::post::parse(table.range.end, parser)?,
        b"STAT" => tables::stat::parse(parser)?,
        b"SVG " => tables::svg::parse(parser)?,
        b"vhea" => tables::vhea::parse(parser)?,
        b"vmtx" => {
            let vhea_data = find_table(table.index, b"vhea")?;
            let maxp_data = find_table(table.index, b"maxp")?;

            let number_of_metrics = tables::vhea::parse_number_of_metrics(vhea_data)?;
            let number_of_glyphs = tables::maxp::parse_number_of_glyphs(maxp_data)?;

            tables::vmtx::parse(number_of_metrics, number_of_glyphs, parser)?;
        }
        b"VORG" => tables::vorg::parse(parser)?,
        b"VVAR" => tables::vvar::parse(parser)?,
        _ => {}
    }

    Ok(())
}

fn parse_header(font_index: u32, tables: &mut Vec<FontTable>, parser: &mut Parser) -> Result<()> {
    parser.begin_group("Header");
    parser.read::<FontMagic>("Magic")?;
    let num_tables = parser.read::<u16>("Number of tables")?;
    parser.read::<u16>("Search range")?;
    parser.read::<u16>("Entry selector")?;
    parser.read::<u16>("Range shift")?;
    parser.end_group();

    parser.begin_group("Table Records");
    for _ in 0..num_tables {
        parser.begin_group("");

        let tag: Tag = parser.read("Tag")?;
        parser.read::<u32>("Checksum")?;
        let offset: u32 = parser.read("Offset")?;
        let length: u32 = parser.read("Length")?;

        let table_name = match &tag.to_bytes() {
            b"avar" => "Axis Variations Table",
            b"bdat" => "Bitmap Data Table",
            b"bloc" => "Bitmap Data Table",
            b"CBDT" => "Color Bitmap Data Table",
            b"CBLC" => "Color Bitmap Location Table",
            b"CFF " => "Compact Font Format Table",
            b"CFF2" => "Compact Font Format 2 Table",
            b"cmap" => "Character to Glyph Index Mapping Table",
            b"cvt " => "Control Value Table",
            b"EBDT" => "Embedded Bitmap Data Table",
            b"EBLC" => "Embedded Bitmap Location Table",
            b"fpgm" => "Font Program Table",
            b"fvar" => "Font Variations Table",
            b"GDEF" => "Glyph Definition Table",
            b"glyf" => "Glyph Data Table",
            b"gvar" => "Glyph Variations Table",
            b"head" => "Font Header Table",
            b"hhea" => "Horizontal Header Table",
            b"hmtx" => "Horizontal Metrics Table",
            b"HVAR" => "Horizontal Metrics Variations Table",
            b"kern" => "Kerning Table",
            b"loca" => "Index to Location Table",
            b"maxp" => "Maximum Profile Table",
            b"MVAR" => "Metrics Variations Table",
            b"name" => "Naming Table",
            b"OS/2" => "OS/2 and Windows Metrics Table",
            b"post" => "PostScript Table",
            b"sbix" => "Standard Bitmap Graphics Table",
            b"STAT" => "Style Attributes Table",
            b"SVG " => "Scalable Vector Graphics Table",
            b"vhea" => "Vertical Header Table",
            b"vmtx" => "Vertical Metrics Table",
            b"VORG" => "Vertical Origin Table",
            b"VVAR" => "Vertical Metrics Variations Table",
            _ => "Unknown Table",
        };

        parser.end_group_with_title_and_value(table_name, std::str::from_utf8(&tag.to_bytes()).unwrap());

        if table_name == "Unknown Table" {
            continue;
        }

        if length == 0 {
            continue;
        }

        tables.push(FontTable {
            index: font_index,
            title: table_name.into(),
            tag,
            range: offset as usize .. offset as usize + length as usize,
        });
    }
    parser.end_group();

    Ok(())
}
