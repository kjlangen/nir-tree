#![allow(non_snake_case)]
#![allow(dead_code)]
#![allow(unused_variables)]

use crate::geometry::Point;
use crate::geometry::Rectangle;


// Structure to define the R-Tree
pub struct Node
{
	pub id: u64,
	pub boundingPolygons: Vec<Rectangle>,
	pub children: Vec<Node>
}

impl Node
{
	// Constructor
	pub fn new(id: u64, boundingPolygons: Vec<Rectangle>, children: Vec<Node>) -> Node
	{
		return Node{id, boundingPolygons, children};
	}

	// Search. Recursively descend through the tree choosing subtrees if they contain the query or
	// if they intersect the query. Return a vector of all the leaf nodes that match
	pub fn searchRectangle(&self, requestedRectangle: &Rectangle) -> Vec<&Node>
	{
		// All the nodes in the R-Tree that match
		let mut matchingNodes: Vec<&Node> = Vec::new();

		// If we have no children then we have been selected by the node above us and therefore we
		// match. Otherwise, we are the ones testing and recursively asking the nodes below us for
		// matching nodes.
		if self.children.len() == 0_usize
		{
			println!("[searchRectangle] Leaf child with id {} hit, adding ourself.", self.id);
			matchingNodes.push(self);
		}
		else
		{
			for i in 0..self.children.len()
			{
				println!("[searchRectangle] Checking a child with id {}.", self.children[i].id);
				let intersecting = self.boundingPolygons[i].containsRectangle(requestedRectangle);
				println!("[searchRectangle] {:?}", intersecting);
				if intersecting
				{
					println!("[searchRectangle] Child matched. Adding all valid leaves. Recursing.");
					for node in &self.children[i].searchRectangle(requestedRectangle)
					{
						println!("[searchRectangle] Adding node {} to results at node {}", node.id, self.id);
						matchingNodes.push(node);
					}
				}
			}
		}

		return matchingNodes;
	}

	pub fn searchPoint(&self, requestedPoint: &Point) -> bool
	{
		// If we have no children then we have only to check our own bounding box
		if self.children.len() == 0_usize
		{
			println!("Checking ourself.");
			return self.boundingPolygons[0].containsPoint(requestedPoint);
		}
		else
		{
			// Check each child's bounding box to see if we should go to that subtree
			for i in 0..self.children.len()
			{
				println!("Checking a child.");
				if self.boundingPolygons[i].containsPoint(requestedPoint)
				{
					println!("Bounding polygon contained point. Recursing.");
					return self.children[i].searchPoint(requestedPoint);
				}
			}
		}

		return false;
	}

	pub fn chooseLeaf(&self, newPolygon: &Point) -> &Node
	{
		/*
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
		*/
		&self
	}

	pub fn insert(&self, newPolygon: &Point)
	{
		/*
		// ChooseLeaf, we need to be given the starting node
		let chosenLeaf = root.chooseLeaf(newPolygon);

		// Add record to the leaf node
		let overflow = false; // This should be dynamically figured out once we figure out leaves

		if overflow
		{
			// Couldn't fit newPolygon so call splitNode
			let L = chosenLeaf.splitNode(newPolygon);
		}
		else
		{
			
		}
		*/
	}
}

/*
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

#[test]
fn testSimpleSearch()
{
	// NOTE: This test does not describe a smallest-fitting-boxes R-Tree
	// The leaf nodes which have no children
	let leafRect1 = Rectangle::new(2, 2, 4, 5, 6);
	let leafRect2 = Rectangle::new(10, 18, 12, 20, 4);
	let leafRect3 = Rectangle::new(17, 5, 18, 18, 13);
	let leaf1 = Node::new(0, vec![leafRect1], Vec::new());
	let leaf2 = Node::new(1, vec![leafRect2], Vec::new());
	let leaf3 = Node::new(2, vec![leafRect3], Vec::new());

	// The node describing rectangles 1 and 2
	let rect1 = Rectangle::new(2, 2, 4, 5, 6);
	let rect2 = Rectangle::new(10, 18, 12, 20, 4);
	let node1 = Node::new(3, vec![rect1, rect2], vec![leaf1, leaf2]);

	// The node describing rectangle 3
	let rect3 = Rectangle::new(17, 5, 18, 18, 13);
	let node2 = Node::new(4, vec![rect3], vec![leaf3]);

	// The linking (root) node describing node1 and node2
	let rect4 = Rectangle::new(0, 0, 13, 21, 273);
	let rect5 = Rectangle::new(15, 4, 21, 21, 102);
	let root = Node::new(5, vec![rect4, rect5], vec![node1, node2]);

	// The first query rectangle
	let query1 = Rectangle::new(3, 3, 19, 19, 128);
	// Test that the first query gives us back all three leaves
	let queryResult1 = root.searchRectangle(&query1);
	assert!(queryResult1.len() == 3);

	// The second query rectangle
	let query2 = Rectangle::new(16, 4, 19, 21, 51);
	// Test that the second query gives us back only rectangle 3
	let queryResult2 = root.searchRectangle(&query2);
	assert!(queryResult2.len() == 1);
}
