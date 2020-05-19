use std::os::raw::c_char;

use crate::{NodeData, TitleKind, ValueType};

#[allow(non_camel_case_types)]
#[repr(C)]
pub struct ttfcore_tree {
    tree: ego_tree::Tree<NodeData>,
    warnings: String,
}


trait NodeIdExt {
    fn from_usize(_: usize) -> Self;
    fn to_usize(self) -> usize;
}

impl NodeIdExt for ego_tree::NodeId {
    #[inline]
    fn from_usize(id: usize) -> Self {
        debug_assert_ne!(id, 0);
        unsafe { std::mem::transmute(id) }
    }

    #[inline]
    fn to_usize(self) -> usize {
        unsafe { std::mem::transmute(self) }
    }
}


#[inline]
fn to_tree(tree: *const ttfcore_tree) -> &'static ego_tree::Tree<NodeData> {
    unsafe { &(*tree).tree }
}


#[no_mangle]
pub extern "C" fn ttfcore_parse_data(data: *const c_char, len: i32, tree: *mut *mut ttfcore_tree) -> bool {
    let data = unsafe { std::slice::from_raw_parts(data as *const u8, len as usize) };

    let mut rtree = ego_tree::Tree::with_capacity(NodeData {
        title: "root".into(),
        index: None,
        value: String::new(),
        value_type: ValueType::None,
        range: 0..len as usize,
    }, 0xFFFF);
    let mut warnings = String::new();

    std::panic::catch_unwind(|| {
        match crate::parse(data, &mut warnings, &mut rtree) {
            Ok(_) => {
                let raw_tree = ttfcore_tree { tree: rtree, warnings };
                unsafe { *tree = Box::into_raw(Box::new(raw_tree)) as *mut _ };
                true
            }
            Err(_) => {
                false
            }
        }
    }).unwrap_or(false)
}

#[no_mangle]
pub extern "C" fn ttfcore_free_tree(tree: *mut ttfcore_tree) {
    unsafe { Box::from_raw(tree) };
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_warnings(tree: *const ttfcore_tree, len: *mut usize) -> *const c_char {
    unsafe {
        let warnings = &(*tree).warnings;
        *len = warnings.len();
        warnings.as_ptr() as _
    }
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_item_parent(tree: *const ttfcore_tree, id: usize) -> usize {
    let id = ego_tree::NodeId::from_usize(id);
    let run = || -> Option<usize> {
        Some(to_tree(tree).get(id)?.parent()?.id().to_usize())
    };

    try_opt_or!(run(), 0)
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_item_child_at(tree: *const ttfcore_tree, parent_id: usize, row: usize) -> usize {
    let parent_id = ego_tree::NodeId::from_usize(parent_id);
    let run = || -> Option<usize> {
        Some(to_tree(tree).get(parent_id)?.children().nth(row)?.id().to_usize())
    };

    try_opt_or!(run(), 0)
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_item_child_index(tree: *const ttfcore_tree, id: usize) -> usize {
    let id = ego_tree::NodeId::from_usize(id);
    let run = || -> Option<usize> {
        to_tree(tree).get(id)?.parent()?.children().position(|n| n.id() == id)
    };

    try_opt_or!(run(), 0)
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_item_children_count(tree: *const ttfcore_tree, id: usize) -> usize {
    let id = ego_tree::NodeId::from_usize(id);
    let run = || -> Option<usize> {
        Some(to_tree(tree).get(id)?.children().count())
    };

    try_opt_or!(run(), 0)
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_item_has_children(tree: *const ttfcore_tree, id: usize) -> bool {
    let id = ego_tree::NodeId::from_usize(id);
    let run = || -> Option<bool> {
        Some(to_tree(tree).get(id)?.has_children())
    };

    try_opt_or!(run(), false)
}

#[inline]
fn get_item_str<P>(tree: *const ttfcore_tree, id: usize, p: P, len: *mut usize) -> *const c_char
    where P: FnOnce(&NodeData) -> &str
{
    unsafe {
        let id: ego_tree::NodeId = std::mem::transmute(id);
        let title: &str = p(&(*tree).tree.get(id).unwrap().value());
        *len = title.len();
        title.as_ptr() as _
    }
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_item_title(tree: *const ttfcore_tree, id: usize, len: *mut usize) -> *const c_char {
    get_item_str(tree, id, |d| &d.title.as_str(), len)
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_item_title_type(tree: *const ttfcore_tree, id: usize) -> u8 {
    unsafe {
        let id: ego_tree::NodeId = std::mem::transmute(id);
        match (*tree).tree.get(id).unwrap().value().title {
            TitleKind::StaticString(_)  => 0,
            TitleKind::OwnedString(_)   => 0,
            TitleKind::Action           => 1,
            TitleKind::Class            => 2,
            TitleKind::Code             => 3,
            TitleKind::Delta            => 4,
            TitleKind::Endpoint         => 5,
            TitleKind::Glyph            => 6,
            TitleKind::Index            => 7,
            TitleKind::Name             => 8,
            TitleKind::Number           => 9,
            TitleKind::Offset           => 10,
            TitleKind::String           => 11,
            TitleKind::StringId         => 12,
            TitleKind::Subroutine       => 13,
            TitleKind::Value            => 14,
        }
    }
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_item_index(tree: *const ttfcore_tree, id: usize) -> i32 {
    unsafe {
        let id: ego_tree::NodeId = std::mem::transmute(id);
        (*tree).tree.get(id).unwrap().value().index.map(|i| i as i32).unwrap_or(-1)
    }
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_item_value(tree: *const ttfcore_tree, id: usize, len: *mut usize) -> *const c_char {
    get_item_str(tree, id, |d| &d.value, len)
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_item_value_type(tree: *const ttfcore_tree, id: usize) -> u8 {
    unsafe {
        let id: ego_tree::NodeId = std::mem::transmute(id);
        (*tree).tree.get(id).unwrap().value().value_type as u8
    }
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_item_range(tree: *const ttfcore_tree, id: usize, start: *mut usize, end: *mut usize) {
    unsafe {
        let id: ego_tree::NodeId = std::mem::transmute(id);
        let range = &(*tree).tree.get(id).unwrap().value().range.clone();
        *start = range.start;
        *end = range.end;
    }
}
