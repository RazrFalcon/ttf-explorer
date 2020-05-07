use crate::parser::*;
use crate::Result;

pub fn parse(parser: &mut Parser) -> Result<()> {
    let start = parser.offset();

    parser.read::<u16>("Version")?;
    let list_offset = parser.read::<Offset32>("Offset to the SVG Document List")?.to_usize();
    parser.read::<u32>("Reserved")?;

    parser.jump_to(start + list_offset)?;
    parser.begin_group("SVG Document List");
    let count = parser.read::<u16>("Number of records")?;
    let mut ranges = Vec::new();
    for i in 0..count {
        parser.begin_group(format!("Record {}", i));
        parser.read::<u16>("First glyph ID")?;
        parser.read::<u16>("Last glyph ID")?;
        let offset = parser.read::<Offset32>("Offset to an SVG Document")?.to_usize();
        let size = parser.read::<u32>("SVG Document length")? as usize;
        parser.end_group();

        let doc_start = start + list_offset + offset;
        let doc_end = doc_start + size;
        ranges.push(doc_start..doc_end);
    }
    parser.end_group();

    ranges.sort_by_key(|r| r.start);
    ranges.dedup_by_key(|r| r.start);

    for range in ranges {
        parser.jump_to(range.start)?;

        if parser.peek::<u16>()? == 8075 {
            // Read gzip compressed SVG as is.
            // TODO: decompress
            parser.read_bytes(range.len(), "SVGZ")?;
        } else {
            // Otherwise read as string.
            // According to the spec, it must be in UTF-8, so we are fine.
            parser.read_string(range.len(), "SVG", None)?;
        }
    }

    Ok(())
}
