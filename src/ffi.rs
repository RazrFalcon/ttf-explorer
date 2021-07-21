use std::os::raw::{c_char, c_void};

use crate::{NodeData, TitleKind, ValueType};

#[allow(non_camel_case_types)]
#[repr(C)]
pub struct ttfcore_tree {
    tree: crate::Tree,
    warnings: String,
}


#[inline]
fn to_tree(tree: *const ttfcore_tree) -> &'static crate::Tree {
    unsafe { &(*tree).tree }
}


#[no_mangle]
pub extern "C" fn ttfcore_parse_data(data: *const c_char, len: i32, tree: *mut *mut ttfcore_tree) -> bool {
    let data = unsafe { std::slice::from_raw_parts(data as *const u8, len as usize) };

    let mut rtree = crate::Tree::new(NodeData {
        title: "root".into(),
        index: None,
        value: String::new(),
        value_type: ValueType::None,
        range: 0..len as u32,
    });
    let mut warnings = String::new();

    std::panic::catch_unwind(|| {
        match crate::parse(data, &mut warnings, &mut rtree) {
            Ok(_) => {
                let raw_tree = ttfcore_tree { tree: rtree, warnings };
                unsafe { *tree = Box::into_raw(Box::new(raw_tree)) as *mut _ };
                true
            }
            Err(e) => {
                // TODO: show in gui
                eprintln!("Error: {}.", e);
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
pub extern "C" fn ttfcore_tree_item_parent(tree: *const ttfcore_tree, id: u32) -> u32 {
    let id = unsafe { crate::NodeId::new_unchecked(id) };
    let run = || -> Option<u32> {
        Some(to_tree(tree).node(id)?.parent()?.id().get_raw())
    };

    try_opt_or!(run(), 0)
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_item_child_at(tree: *const ttfcore_tree, parent_id: u32, row: u32) -> u32 {
    let parent_id = unsafe { crate::NodeId::new_unchecked(parent_id) };
    let run = || -> Option<u32> {
        Some(to_tree(tree).node(parent_id)?.child(row as usize)?.id().get_raw())
    };

    try_opt_or!(run(), 0)
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_item_child_index(tree: *const ttfcore_tree, id: u32) -> u32 {
    let id = unsafe { crate::NodeId::new_unchecked(id) };
    let run = || -> Option<u32> {
        let first = to_tree(tree).node(id)?.parent()?.first_child()?.id().index();
        Some(id.index().checked_sub(first)? as u32)
    };

    try_opt_or!(run(), 0)
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_item_children_count(tree: *const ttfcore_tree, id: u32) -> u32 {
    let id = unsafe { crate::NodeId::new_unchecked(id) };
    let run = || -> Option<u32> {
        Some(to_tree(tree).node(id)?.children_count() as u32)
    };

    try_opt_or!(run(), 0)
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_item_has_children(tree: *const ttfcore_tree, id: u32) -> bool {
    let id = unsafe { crate::NodeId::new_unchecked(id) };
    let run = || -> Option<bool> {
        Some(to_tree(tree).node(id)?.has_children())
    };

    try_opt_or!(run(), false)
}

#[inline]
fn get_item_str<P>(tree: *const ttfcore_tree, id: u32, p: P, len: *mut usize) -> *const c_char
    where P: FnOnce(&NodeData) -> &str
{
    unsafe {
        let id: crate::NodeId = std::mem::transmute(id);
        let title: &str = p(&(*tree).tree.node(id).unwrap().value());
        *len = title.len();
        title.as_ptr() as _
    }
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_item_title(tree: *const ttfcore_tree, id: u32, len: *mut usize) -> *const c_char {
    get_item_str(tree, id, |d| &d.title.as_str(), len)
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_item_title_type(tree: *const ttfcore_tree, id: u32) -> u8 {
    unsafe {
        let id: crate::NodeId = std::mem::transmute(id);
        match (*tree).tree.node(id).unwrap().value().title {
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
pub extern "C" fn ttfcore_tree_item_index(tree: *const ttfcore_tree, id: u32) -> i32 {
    unsafe {
        let id: crate::NodeId = std::mem::transmute(id);
        (*tree).tree.node(id).unwrap().value().index.map(|i| i as i32).unwrap_or(-1)
    }
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_item_value(tree: *const ttfcore_tree, id: u32, len: *mut usize) -> *const c_char {
    get_item_str(tree, id, |d| &d.value, len)
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_item_value_type(tree: *const ttfcore_tree, id: u32) -> u8 {
    unsafe {
        let id: crate::NodeId = std::mem::transmute(id);
        (*tree).tree.node(id).unwrap().value().value_type as u8
    }
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_item_range(tree: *const ttfcore_tree, id: u32, start: *mut u32, end: *mut u32) {
    unsafe {
        let id: crate::NodeId = std::mem::transmute(id);
        let range = &(*tree).tree.node(id).unwrap().value().range.clone();
        *start = range.start;
        *end = range.end;
    }
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_collect_ranges(tree: *const ttfcore_tree, data: *mut c_void, p: extern "C" fn(*mut c_void, u32, u32)) {
    let root = unsafe { &*tree }.tree.root();
    collect_ranges(&root, data, p);
}

fn collect_ranges(parent: &crate::Node, data: *mut c_void, p: extern "C" fn(*mut c_void, u32, u32)) {
    let mut child_opt = parent.first_child();
    while let Some(child) = child_opt {
        if child.has_children() {
            collect_ranges(&child, data, p);
        } else {
            p(
                data,
                child.value().range.start,
                child.value().range.end,
            );
        }

        child_opt = child.next_sibling();
    }
}

#[no_mangle]
pub extern "C" fn ttfcore_tree_item_at_byte(tree: *const ttfcore_tree, offset: u32) -> u32 {
    let root = unsafe { &*tree }.tree.root();
    find_at_byte(&root, offset).unwrap_or(0)
}

fn find_at_byte(parent: &crate::Node, offset: u32) -> Option<u32> {
    let mut child_opt = parent.first_child();
    while let Some(child) = child_opt {
        if child.value().range.contains(&offset) {
            return if child.has_children() {
                find_at_byte(&child, offset)
            } else {
                Some(child.id().get_raw())
            };
        }

        child_opt = child.next_sibling();
    }

    None
}
