use std::num::NonZeroU32;

pub struct Tree {
    nodes: Vec<Vec<InnerNodeData>>,
}

impl Tree {
    #[inline]
    pub fn new(root: crate::NodeData) -> Self {
        Tree {
            nodes: vec![
                vec![
                    InnerNodeData {
                        parent: None,
                        prev_sibling: None,
                        next_sibling: None,
                        first_child: None,
                        last_child: None,
                        data: root,
                    }
                ]
            ]
        }
    }

    #[inline]
    pub fn root(&self) -> Node {
        Node { id: NodeId::new(0, 0), d: &self.nodes[0][0], doc: self }
    }

    #[inline]
    pub fn node(&self, id: NodeId) -> Option<Node> {
        self.node_data(id).map(|d| Node { id, d, doc: self })
    }

    #[inline]
    fn node_data(&self, id: NodeId) -> Option<&InnerNodeData> {
        self.nodes
            .get(id.depth() as usize)
            .and_then(|data| data.get(id.index()))
    }

    #[inline]
    fn node_data_mut(&mut self, id: NodeId) -> Option<&mut InnerNodeData> {
        self.nodes
            .get_mut(id.depth() as usize)
            .and_then(|data| data.get_mut(id.index()))
    }

    #[inline]
    pub fn node_value_mut(&mut self, id: NodeId) -> Option<&mut crate::NodeData> {
        self.node_data_mut(id).map(|data| &mut data.data)
    }

    pub fn append(
        &mut self,
        parent_id: NodeId,
        data: crate::NodeData,
    ) -> NodeId {
        let depth = parent_id.depth() as usize + 1;
        if self.nodes.len() == depth {
            self.nodes.push(Vec::new());
        }

        let new_child_id = NodeId::new(depth as u8, self.nodes[depth].len() as u32);

        let last_child_id = self.node_data(parent_id).unwrap().last_child;

        self.nodes[depth].push(InnerNodeData {
            parent: Some(parent_id),
            prev_sibling: last_child_id,
            next_sibling: None,
            first_child: None,
            last_child: None,
            data,
        });

        if let Some(id) = last_child_id {
            self.node_data_mut(id).unwrap().next_sibling = Some(new_child_id);
        }

        {
            let parent = self.node_data_mut(parent_id).unwrap();
            if parent.first_child.is_none() {
                parent.first_child = Some(new_child_id);
            }

            parent.last_child = Some(new_child_id);
        }

        new_child_id
    }
}


#[repr(transparent)]
#[derive(Clone, Copy, PartialEq)]
pub struct NodeId(NonZeroU32);

impl NodeId {
    #[inline]
    pub fn new(depth: u8, index: u32) -> Self {
        NodeId(NonZeroU32::new(((depth as u32) << 24) | (index + 1)).unwrap())
    }

    #[inline]
    pub const unsafe fn new_unchecked(id: u32) -> Self {
        NodeId(NonZeroU32::new_unchecked(id))
    }

    #[inline]
    fn get(self) -> u32 {
        self.0.get() - 1
    }

    #[inline]
    pub fn depth(self) -> u8 {
        (self.get() >> 24) as u8
    }

    #[inline]
    pub fn index(self) -> usize {
        (self.get() & 0x00FFFFFF) as usize
    }

    #[inline]
    pub fn get_raw(self) -> u32 {
        self.0.get()
    }
}

impl std::fmt::Debug for NodeId {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("NodeId")
         .field("depth", &self.depth())
         .field("index", &self.index())
         .finish()
    }
}


struct InnerNodeData {
    parent: Option<NodeId>,
    prev_sibling: Option<NodeId>,
    next_sibling: Option<NodeId>,
    first_child: Option<NodeId>,
    last_child: Option<NodeId>,
    data: crate::NodeData,
}


#[derive(Clone, Copy)]
pub struct Node<'a> {
    id: NodeId,
    doc: &'a Tree,
    d: &'a InnerNodeData,
}

impl PartialEq for Node<'_> {
    #[inline]
    fn eq(&self, other: &Self) -> bool {
           self.id == other.id
        && self.doc as *const _ == other.doc as *const _
        && self.d as *const _ == other.d as *const _
    }
}

impl<'a> Node<'a> {
    #[inline]
    pub fn value(&self) -> &'a crate::NodeData {
        &self.d.data
    }

    #[inline]
    pub fn document(&self) -> &'a Tree {
        self.doc
    }

    #[inline]
    pub fn parent(&self) -> Option<Self> {
        self.d.parent.map(|id| self.doc.node(id).unwrap())
    }

    #[inline]
    pub fn prev_sibling(&self) -> Option<Self> {
        self.d.prev_sibling.map(|id| self.doc.node(id).unwrap())
    }

    #[inline]
    pub fn next_sibling(&self) -> Option<Self> {
        self.d.next_sibling.map(|id| self.doc.node(id).unwrap())
    }

    #[inline]
    pub fn first_child(&self) -> Option<Self> {
        self.d.first_child.map(|id| self.doc.node(id).unwrap())
    }

    #[inline]
    pub fn last_child(&self) -> Option<Self> {
        self.d.last_child.map(|id| self.doc.node(id).unwrap())
    }

    #[inline]
    pub fn has_siblings(&self) -> bool {
        self.prev_sibling().is_some() || self.next_sibling().is_some()
    }

    #[inline]
    pub fn has_children(&self) -> bool {
        self.d.first_child.is_some()
    }

    #[inline]
    pub fn children_count(&self) -> usize {
        match (self.d.first_child, self.d.last_child) {
            (Some(first_id), Some(last_id)) => last_id.index() - first_id.index() + 1,
            _ => 0,
        }
    }

    #[inline]
    pub fn child(&self, index: usize) -> Option<Self> {
        let first_id = self.d.first_child?;
        let new_id = NodeId::new(first_id.depth(), (first_id.index() + index) as u32);
        self.doc.node(new_id)
    }

    #[inline]
    pub fn id(&self) -> NodeId {
        self.id
    }
}
