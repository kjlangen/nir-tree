#![allow(non_snake_case)]

// Structures to define the rtree nodes
#[derive(Debug)]
pub struct Point {
	pub x: u64,
	pub y: u64,
}

pub struct Rectangle {
	pub lowerLeft: Point,
	pub upperRight: Point,
}

impl Rectangle {
	pub fn containsPoint(&self, requestedPoint: &Point) -> bool {
		let inXRange = self.lowerLeft.x <= requestedPoint.x && requestedPoint.x <= self.upperRight.x;
		let inYRange = self.lowerLeft.y <= requestedPoint.y && requestedPoint.y <= self.upperRight.y;

		return inXRange && inYRange;
	}
}

pub struct Node {
	pub boundingPolygons: Vec<Rectangle>,
	pub children: Vec<Node>
}

impl Node {
	// TODO: update this function for box intersection
	pub fn search(&self, requestedPoint: &Point) -> bool
	{
		// If we have no children then we have only to check our own bounding box
		if self.children.len() == 0_usize
		{
			println!("Checking ourself.");
			return self.boundingPolygons[0].containsPoint(requestedPoint); // TODO: update for box intersection
		}
		else
		{
			// Check each child's bounding box to see if we should go to that subtree
			for i in 0..self.children.len()
			{
				println!("Checking a child.");
				if self.boundingPolygons[i].containsPoint(requestedPoint) // TODO: update for box intersection
				{
					println!("Bounding polygon contained point. Recursing.");
					return self.children[i].search(requestedPoint);
				}
			}
		}

		return false;
	}

	pub fn chooseLeaf(&self, newPolygon: &Point) -> &Node
	{
		// If we are a leaf then return ourselves
		if self.children.len() == 0
		{
			return &self;
		}

		// Choose node whos bounding polygon will require the least expansion. Breaking ties by
		// smaller area
		for boundingPolygon in self.boundingPolygons
		{
			unimplemented!();
		}
	}

	pub fn insert(&self, newPolygon: &Point)
	{
		// ChooseLeaf, we need to be given the starting node
		let chosenLeaf = root.chooseLeaf(newPolygon);

		// Add record to the leaf node
		let overflow = false; // This should be dynamically figured out once we figure out leaves

		if overflow
		{
			// Couldn't fit newPolygon so call splitNode
			let L = chosenLeaf.splitNode(newPolygon);
		}
	}
}

/*
// Search
// 1. Search subtrees. If T is not a leaf , check each node to determine if there is box overlap.
// For all overlapping nodes recursively invoke search
// 2. Search leaf. If T is a leaf, check each entry for overlap, return these as qualifying records.

// Insert
// 1. Find position for the new record. Call ChooseLeaf to select a leaf L in which to put E.
// 2. Add record to the leaf node. If L has room for E then add it, otherwise call SplitNode to get
// L and LL containing E and all the old entries of L.
// 3. Propogate changes upwards. Call AdjustTree on L, also passing LL if created by step 2.
// 4. Grow the tree taller. If the root split, create a new root with the results of AdjustTree.

// ChooseLeaf
// 1. Initialize. Set N to be the root node.
// 2. Leaf check. If N is a leaf, return N.
// 3. Choose subtree. If N is not a leaf, let F be the entry in N whose rectangle requires least
// enlargement to include E. Break ties by using smallest area.
// 4. Descend until a leaf is reached. Set N to be the chosen child node and repeat from step 2.

// AdjustTree
// 1. Initialize. Set N to be the leaf L. If L was split then also set NN to be LL.
// 2. Check if done. If N is the root then stop.
// 3. Adjust covering rectangle in parent. Let P be the parent of N. Adjust P's bounding box for N
// so that it tightly encloses all entry rectangles in N.
// 4. Propogate upwards. If N has a partner NN then create a new entry pointing to NN enclosing all
// rectangles within NN. Add this to P if there is room. Otherwise call SplitNode to make P and PP.
// 5. Move up to next level. Set N = P and NN = PP. Repeat from step 2.

// Deletion
// 1. Find node containing record E. Call FindLeaf to locate the leaf node L containing E. Stop if
// the record was not found.
// 2. Delete record. Remove E from L.
// 3. Propagate changes. Call CondenseTree, passing L.
// 4. Shorten tree. If the root node has only one child after the tree has been adjusted, make the
// child the new root.

// FindLeaf
// 1. Search subtrees. If T is not a leaf, check each entry F in T to determine overlaps. For each
// such entry call FindLeaf on the subtrees until E is found or all subtrees have been checked.
// 2. Search leaf node for record E. If T is a leaf, check each entry to see if it matches E. If E
// is found return T.

// CondenseTree
// 1. Initialize. Set N = L. Set Q, the set of eliminated nodes, to be empty.
// 2. Find parent entry. If N is the root, go to step 6. Otherwise let P be the parent of N, and let
// EN be N's entry in P.
// 3. Eliminate under-full node. If N has fewer than m entries, delete EN from P and add N to set Q
// 4. Adjust covering rectangle. If N has not been eliminated, adjust EN.I to tightly contain all
// entries in N.
// 5. Move up one level in tree. Set N = P and repeat from step 2.
// 6. Re-insert orphaned entries. Re-insert all entries of nodes in set Q. Entries from eliminated
// leaf nodes are re-inserted in tree leaves as described in Insert, but entries from higher-level
// nodes must be placed higher in the tree, so that leaves of their dependent subtrees will be on
// the same level as leaves of the main tree.
*/
