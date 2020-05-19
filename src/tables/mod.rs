pub mod avar;
pub mod cbdt;
pub mod cblc;
pub mod cff;
pub mod cff2;
pub mod cmap;
pub mod fvar;
pub mod gdef;
pub mod glyf;
pub mod gvar;
pub mod head;
pub mod hhea;
pub mod hmtx;
pub mod hvar;
pub mod kern;
pub mod loca;
pub mod maxp;
pub mod mvar;
pub mod name;
pub mod os_2;
pub mod post;
pub mod sbix;
pub mod stat;
pub mod svg;
pub mod vhea;
pub mod vmtx;
pub mod vorg;
pub mod vvar;

#[derive(Clone, Copy, PartialEq, Debug)]
pub enum IndexToLocFormat {
    Short,
    Long,
}
