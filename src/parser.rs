use std::ops::Range;
use std::convert::TryInto;

use crate::{NodeData, TitleKind, ValueType, Error, Result};


pub trait TrySlice<'a> {
    fn try_slice(&self, range: Range<usize>) -> Result<&'a [u8]>;
}

impl<'a> TrySlice<'a> for &'a [u8] {
    #[inline]
    fn try_slice(&self, range: Range<usize>) -> Result<&'a [u8]> {
        self.get(range.clone()).ok_or_else(|| Error::ReadOutOfBounds)
    }
}


/// A trait for parsing raw binary data.
///
/// This is a low-level, internal trait that should not be used directly.
pub trait FromData: std::fmt::Display + Sized {
    /// Stores an object size in raw data.
    ///
    /// `mem::size_of` by default.
    ///
    /// Override when size of `Self` != size of a raw data.
    /// For example, when you are parsing `u16`, but storing it as `u8`.
    /// In this case `size_of::<Self>()` == 1, but `FromData::SIZE` == 2.
    const SIZE: usize = std::mem::size_of::<Self>();

    const NAME: ValueType;

    /// Parses an object from a raw data.
    fn parse(data: &[u8]) -> Result<Self>;
}

impl FromData for u8 {
    const NAME: ValueType = ValueType::UInt8;

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        Ok(data[0])
    }
}

impl FromData for i8 {
    const NAME: ValueType = ValueType::Int8;

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        Ok(data[0] as i8)
    }
}

impl FromData for u16 {
    const NAME: ValueType = ValueType::UInt16;

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        Ok(u16::from_be_bytes(data.try_into().unwrap()))
    }
}

impl FromData for i16 {
    const NAME: ValueType = ValueType::Int16;

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        Ok(i16::from_be_bytes(data.try_into().unwrap()))
    }
}

impl FromData for u32 {
    const NAME: ValueType = ValueType::UInt32;

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        Ok(u32::from_be_bytes(data.try_into().unwrap()))
    }
}

impl FromData for i32 {
    const NAME: ValueType = ValueType::Int32;

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        Ok(i32::from_be_bytes(data.try_into().unwrap()))
    }
}


// https://docs.microsoft.com/en-us/typography/opentype/spec/otff#data-types
#[derive(Clone, Copy, Debug)]
pub struct U24(pub u32);

impl FromData for U24 {
    const SIZE: usize = 3;
    const NAME: ValueType = ValueType::UInt24;

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        let data: [u8; 3] = data.try_into().unwrap();
        Ok(U24(u32::from_be_bytes([0, data[0], data[1], data[2]])))
    }
}

impl std::fmt::Display for U24 {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.0.fmt(f)
    }
}


#[derive(Clone, Copy, Debug)]
pub struct F2DOT14(pub i16);

impl F2DOT14 {
    #[inline]
    pub fn to_f32(&self) -> f32 {
        f32::from(self.0) / 16384.0
    }
}

impl FromData for F2DOT14 {
    const NAME: ValueType = ValueType::F2DOT14;

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        i16::parse(data).map(F2DOT14)
    }
}

impl std::fmt::Display for F2DOT14 {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.to_f32().fmt(f)
    }
}


#[derive(Clone, Copy, Debug)]
pub struct Fixed(pub f32);

impl FromData for Fixed {
    const SIZE: usize = 4;
    const NAME: ValueType = ValueType::Fixed;

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        // TODO: is it safe to cast?
        i32::parse(data).map(|n| Fixed(n as f32 / 65536.0))
    }
}

impl std::fmt::Display for Fixed {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.0.fmt(f)
    }
}


#[derive(Clone, Copy, Debug)]
pub struct LongDateTime(pub u64);

impl FromData for LongDateTime {
    const NAME: ValueType = ValueType::LongDateTime;

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        Ok(LongDateTime(u64::from_be_bytes(data.try_into().unwrap())))
    }
}

impl std::fmt::Display for LongDateTime {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.0.fmt(f) // TODO: this
    }
}


#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Debug)]
pub struct Offset16(pub u16);

impl Offset16 {
    #[inline]
    pub fn to_usize(&self) -> usize {
        usize::from(self.0)
    }
}

impl FromData for Offset16 {
    const NAME: ValueType = ValueType::Offset16;

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        u16::parse(data).map(Offset16)
    }
}

impl std::fmt::Display for Offset16 {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.0.fmt(f)
    }
}


#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Debug)]
pub struct OptionalOffset16(pub u16);

impl OptionalOffset16 {
    #[inline]
    pub fn to_usize(self) -> Option<usize> {
        if self.0 == 0 { None } else { Some(self.0 as usize) }
    }
}

impl FromData for OptionalOffset16 {
    const NAME: ValueType = ValueType::Offset16;

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        u16::parse(data).map(OptionalOffset16)
    }
}

impl std::fmt::Display for OptionalOffset16 {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        if self.0 == 0 {
            f.write_str("NULL")
        } else {
            self.0.fmt(f)
        }
    }
}


#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Debug)]
pub struct Offset32(pub u32);

impl Offset32 {
    #[inline]
    pub fn to_usize(&self) -> usize {
        usize::num_from(self.0)
    }
}

impl FromData for Offset32 {
    const NAME: ValueType = ValueType::Offset32;

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        u32::parse(data).map(Offset32)
    }
}

impl std::fmt::Display for Offset32 {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.0.fmt(f)
    }
}


#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Debug)]
pub struct OptionalOffset32(pub u32);

impl OptionalOffset32 {
    #[inline]
    pub fn to_usize(self) -> Option<usize> {
        if self.0 == 0 { None } else { Some(self.0 as usize) }
    }
}

impl FromData for OptionalOffset32 {
    const NAME: ValueType = ValueType::Offset32;

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        u32::parse(data).map(OptionalOffset32)
    }
}

impl std::fmt::Display for OptionalOffset32 {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        if self.0 == 0 {
            f.write_str("NULL")
        } else {
            self.0.fmt(f)
        }
    }
}


#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Debug)]
pub struct GlyphId(pub u16);

impl FromData for GlyphId {
    const NAME: ValueType = ValueType::GlyphId;

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        u16::parse(data).map(GlyphId)
    }
}

impl std::fmt::Display for GlyphId {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.0.fmt(f)
    }
}


pub trait NumFrom<T>: Sized {
    fn num_from(_: T) -> Self;
}

// Rust doesn't implement `From<u32> for usize`,
// because it has to support 16 bit targets.
// We don't, so we can allow this.
impl NumFrom<u32> for usize {
    #[inline]
    fn num_from(v: u32) -> Self {
        debug_assert!(std::mem::size_of::<usize>() >= 4);
        v as usize
    }
}

pub struct Parser<'a> {
    data: &'a [u8],
    offset: usize,
    tree: &'a mut ego_tree::Tree<NodeData>,
    parent_id: ego_tree::NodeId,
}

impl<'a> Parser<'a> {
    #[inline]
    pub fn new(data: &'a [u8], tree: &'a mut ego_tree::Tree<NodeData>) -> Self {
        let parent_id = tree.root().id();
        Parser {
            data,
            offset: 0,
            tree,
            parent_id,
        }
    }

    #[inline]
    pub fn current_group_node(&self) -> ego_tree::NodeId {
        self.parent_id
    }

    pub fn begin_group<S: Into<TitleKind>>(&mut self, title: S) {
        self.parent_id = self.tree.get_mut(self.parent_id).unwrap().append(NodeData {
            title: title.into(),
            index: None,
            value: String::new(),
            value_type: ValueType::None,
            range: self.offset..self.offset, // End offset will be set later in `end_group()`.
        }).id();
    }

    pub fn begin_group_with_index(&mut self, title: TitleKind, index: u32) {
        self.parent_id = self.tree.get_mut(self.parent_id).unwrap().append(NodeData {
            title,
            index: Some(index),
            value: String::new(),
            value_type: ValueType::None,
            range: self.offset..self.offset, // End offset will be set later in `end_group()`.
        }).id();
    }

    pub fn begin_group_with_value<S1, S2>(&mut self, title: S1, value: S2)
        where S1: Into<TitleKind>,
              S2: Into<String>,
    {
        self.parent_id = self.tree.get_mut(self.parent_id).unwrap().append(NodeData {
            title: title.into(),
            index: None,
            value: value.into(),
            value_type: ValueType::None,
            range: self.offset..self.offset, // End offset will be set later in `end_group()`.
        }).id();
    }

    pub fn end_group(&mut self) {
        let mut node = self.tree.get_mut(self.parent_id).unwrap();
        node.value().range.end = self.offset;
        self.parent_id = node.parent().unwrap().id();
    }

    pub fn end_group_with_title<S: Into<TitleKind>>(&mut self, title: S) {
        let mut node = self.tree.get_mut(self.parent_id).unwrap();
        node.value().title = title.into();
        node.value().range.end = self.offset;
        self.parent_id = node.parent().unwrap().id();
    }

    pub fn end_group_with_title_and_value<S1, S2>(&mut self, title: S1, value: S2)
        where S1: Into<TitleKind>,
              S2: Into<String>,
    {
        let mut node = self.tree.get_mut(self.parent_id).unwrap();
        node.value().title = title.into();
        node.value().value = value.into();
        node.value().range.end = self.offset;
        self.parent_id = node.parent().unwrap().id();
    }

    #[inline]
    pub fn jump_to(&mut self, offset: usize) -> Result<()> {
        if offset <= self.data.len() {
            self.offset = offset;
            Ok(())
        } else {
            Err(Error::ReadOutOfBounds)
        }
    }

    #[inline]
    pub fn offset(&self) -> usize {
        self.offset
    }

    #[inline]
    pub fn advance(&mut self, len: usize) {
        self.offset += len;
    }

    #[inline]
    pub fn peek<T: FromData>(&mut self) -> Result<T> {
        let data = self.data.try_slice(self.offset..self.offset + T::SIZE)?;
        T::parse(data)
    }

    #[inline]
    pub fn peek_at<T: FromData>(&mut self, offset: usize) -> Result<T> {
        let start = self.offset + offset;
        let end = start + T::SIZE;
        let data = self.data.try_slice(start..end)?;
        T::parse(data)
    }

    #[inline]
    pub fn peek_bytes(&mut self, len: usize) -> Result<&'a [u8]> {
        self.data.try_slice(self.offset..self.offset + len)
    }

    #[inline]
    pub fn read<T: FromData>(&mut self, title: &'static str) -> Result<T> {
        let start = self.offset;
        let value = self.read_bytes_impl(T::SIZE).and_then(T::parse)?;
        self.add_child(title.into(), None, value.to_string(), T::NAME, start..self.offset);
        Ok(value)
    }

    #[inline]
    pub fn read2<T: FromData>(&mut self, title: String) -> Result<T> {
        let start = self.offset;
        let value = self.read_bytes_impl(T::SIZE).and_then(T::parse)?;
        self.add_child(title.into(), None, value.to_string(), T::NAME, start..self.offset);
        Ok(value)
    }

    #[inline]
    pub fn read_index<T: FromData>(&mut self, title: TitleKind, n: u32) -> Result<T> {
        let start = self.offset;
        let value = self.read_bytes_impl(T::SIZE).and_then(T::parse)?;
        self.add_child(title, Some(n), value.to_string(), T::NAME, start..self.offset);
        Ok(value)
    }

    #[inline]
    pub fn read_bytes<S: Into<TitleKind>>(&mut self, len: usize, title: S) -> Result<&'a [u8]> {
        let start = self.offset;
        let value = self.read_bytes_impl(len)?;
        self.add_child(title.into(), None, String::new(), ValueType::Bytes, start..self.offset);
        Ok(value)
    }

    #[inline]
    fn read_bytes_impl(&mut self, len: usize) -> Result<&'a [u8]> {
        let v = self.data.try_slice(self.offset..self.offset + len)?;
        self.advance(len);
        Ok(v)
    }

    #[inline]
    pub fn read_string<S: Into<TitleKind>>(&mut self, len: usize, title: S, index: Option<u32>) -> Result<()> {
        let start = self.offset;
        let bytes = self.read_bytes_impl(len)?;
        let str = match std::str::from_utf8(bytes) {
            Ok(s) => s.to_string(),
            Err(_) => {
                // Try decode as ASCII.
                let mut s = String::new();
                for b in bytes.iter() {
                    if b.is_ascii() {
                        s.push(*b as char);
                    }
                }
                s
            }
        };

        self.add_child(title.into(), index, str, ValueType::String, start..self.offset);

        Ok(())
    }

    #[inline]
    pub fn read_value<S1, S2>(&mut self, len: usize, title: S1, value: S2) -> Result<()>
        where S1: Into<TitleKind>,
              S2: Into<String>,
    {
        self.add_child(title.into(), None, value.into(), ValueType::Bytes, self.offset..self.offset+len);
        self.read_bytes_impl(len)?;
        Ok(())
    }

    #[inline]
    pub fn read_array<T: FromData>(&mut self, title: &'static str, item: TitleKind, len: usize) -> Result<()> {
        if len == 0 {
            return Ok(());
        }

        self.begin_group_with_value(title, len.to_string());
        for i in 0..len {
            self.read_index::<T>(item.clone(), i as u32)?;
        }
        self.end_group();
        Ok(())
    }

    #[inline]
    fn add_child(
        &mut self,
        title: TitleKind,
        index: Option<u32>,
        value: String,
        value_type: ValueType,
        range: Range<usize>,
    ) {
        self.tree.get_mut(self.parent_id).unwrap().append(NodeData {
            title,
            index,
            value,
            value_type,
            range,
        });
    }

    #[inline]
    pub fn to_simple(&self) -> SimpleParser {
        SimpleParser::new(&self.data[self.offset..])
    }
}


pub struct SimpleParser<'a> {
    data: &'a [u8],
    offset: usize,
}

impl<'a> SimpleParser<'a> {
    #[inline]
    pub fn new(data: &'a [u8]) -> Self {
        SimpleParser { data, offset: 0 }
    }

    #[inline]
    pub fn jump_to(&mut self, offset: usize) -> Result<()> {
        if offset <= self.data.len() {
            self.offset = offset;
            Ok(())
        } else {
            Err(Error::ReadOutOfBounds)
        }
    }

    #[inline]
    pub fn offset(&self) -> usize {
        self.offset
    }

    #[inline]
    pub fn at_end(&self) -> bool {
        self.offset >= self.data.len()
    }

    #[inline]
    pub fn skip<T: FromData>(&mut self) {
        self.offset += T::SIZE;
    }

    #[inline]
    pub fn advance(&mut self, len: usize) {
        self.offset += len;
    }

    #[inline]
    pub fn read<T: FromData>(&mut self) -> Result<T> {
        self.read_bytes(T::SIZE).and_then(T::parse)
    }

    #[inline]
    pub fn read_bytes(&mut self, len: usize) -> Result<&'a [u8]> {
        let v = self.data.try_slice(self.offset..self.offset + len)?;
        self.advance(len);
        Ok(v)
    }
}


#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct Tag(pub u32);

impl Tag {
    #[inline]
    pub const fn to_bytes(self) -> [u8; 4] {
        [
            (self.0 >> 24 & 0xff) as u8,
            (self.0 >> 16 & 0xff) as u8,
            (self.0 >> 8 & 0xff) as u8,
            (self.0 >> 0 & 0xff) as u8,
        ]
    }
}

impl std::fmt::Debug for Tag {
    #[inline]
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Tag({})", self)
    }
}

impl std::fmt::Display for Tag {
    #[inline]
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f, "{}{}{}{}",
            (self.0 >> 24 & 0xff) as u8 as char,
            (self.0 >> 16 & 0xff) as u8 as char,
            (self.0 >> 8 & 0xff) as u8 as char,
            (self.0 >> 0 & 0xff) as u8 as char,
        )
    }
}

impl FromData for Tag {
    const NAME: ValueType = ValueType::GlyphId;

    #[inline]
    fn parse(data: &[u8]) -> Result<Self> {
        Ok(Tag(u32::parse(data).unwrap()))
    }
}


static TRUE: bool = true;
static FALSE: bool = false;

#[derive(Clone, Copy, Debug)]
pub struct U8Bits(pub u8);

impl std::ops::Index<usize> for U8Bits {
    type Output = bool;

    #[inline]
    fn index(&self, index: usize) -> &Self::Output {
        debug_assert!(index < 8);

        if (self.0 >> index as u8) & 1 != 0 {
            &TRUE
        } else {
            &FALSE
        }
    }
}

impl std::fmt::Display for U8Bits {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let n = self.0.reverse_bits();

        for i in 0..8 {
            write!(f, "{}", (n >> i) & 1)?;
        }

        Ok(())
    }
}


#[derive(Clone, Copy, Debug)]
pub struct U16Bits(pub u16);

impl std::ops::Index<usize> for U16Bits {
    type Output = bool;

    #[inline]
    fn index(&self, index: usize) -> &Self::Output {
        debug_assert!(index < 16);

        if (self.0 >> index as u16) & 1 != 0 {
            &TRUE
        } else {
            &FALSE
        }
    }
}

impl std::fmt::Display for U16Bits {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let n = self.0.reverse_bits();

        for i in 0..16 {
            write!(f, "{}", (n >> i) & 1)?;
        }

        Ok(())
    }
}
