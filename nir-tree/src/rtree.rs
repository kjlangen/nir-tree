#![allow(non_snake_case)]
#![allow(dead_code)]
#![allow(unused_variables)]

use crate::geometry::Point;
use crate::geometry::Rectangle;


// Structure to define the R-Tree routing nodes
#[derive(Debug)]
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

	pub fn getId(&self) -> u64
	{
		return self.id;
	}

	// Search functions
	pub fn searchRectangle(&self, requestedRectangle: &Rectangle) -> Vec<&Rectangle>
	{
		// All the nodes in the R-Tree that match
		let mut matchingRectangles: Vec<&Rectangle> = Vec::new();

		if self.children.len() == 0
		{
			// We are a leaf so check our rectangles
			for i in 0..self.boundingPolygons.len()
			{
				println!("[searchRectangle] Checking rectangle {}.", i);
				let intersecting = self.boundingPolygons[i].intersectsRectangle(requestedRectangle);
				println!("[searchRectangle] {:?}", intersecting);
				if intersecting
				{
					println!("[searchRectangle] Leaf {} adding rectangle {}", self.id, i);
					matchingRectangles.push(&self.boundingPolygons[i]);
				}
			}
		}
		else
		{
			// We are the ones testing and recursively asking the nodes below us for matching nodes.
			assert!(self.children.len() == self.boundingPolygons.len());
			for i in 0..self.children.len()
			{
				println!("[searchRectangle] Checking a child with id {}.", self.children[i].getId());
				let intersecting = self.boundingPolygons[i].intersectsRectangle(requestedRectangle);
				println!("[searchRectangle] {:?}", intersecting);
				if intersecting
				{
					println!("[searchRectangle] Child matched. Recursing to add all valid leaves.");
					for rectangle in &self.children[i].searchRectangle(requestedRectangle)
					{
						println!("[searchRectangle] Adding to results at node {}", self.id);
						matchingRectangles.push(rectangle);
					}
				}
			}
		}

		return matchingRectangles;
	}

	pub fn searchPoint(&self, requestedPoint: &Point) -> bool
	{
		if self.children.len() == 0
		{
			// We are a leaf so we have only to check our bounding boxes which represent the objects
			for i in 0..self.boundingPolygons.len()
			{
				if self.boundingPolygons[i].containsPoint(requestedPoint)
				{
					return true;
				}
			}
		}
		else
		{
			// Check each child's bounding box to see if we should go to that subtree
			assert!(self.children.len() == self.boundingPolygons.len());
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

	pub fn insert(self, newRectangle: Rectangle)
	{
		// Phase 1: Iteratively descend the tree to the proper leaf while keeping track of our path
		// by using the stack 'path'
		let mut path = Vec::new();

		let currentNode = &mut self;

		loop
		{
			// If we are a leaf stop
			if currentNode.children.len() == 0
			{
				unimplemented!();
			}
			else
			{
				// Pick a new current node based on least amount of expansion area
				let mut chosen = 0;
				let mut chosenMetrics = /**/.boundingPolygons[chosen].computeExpansionArea(&newRectangle);
				for i in 0../**/.children.len()
				{
					let competitorMetrics = /**/.boundingPolygons[i].computeExpansionArea(&newRectangle);
					if competitorMetrics.1 <= chosenMetrics.1 && competitorMetrics.0.area < chosenMetrics.0.area
					{
						chosen = i;
						chosenMetrics = competitorMetrics;
					}
				}
			}
		}

		// Print what node we've chosen
		println!("We have chosen node {:?}", self);

		// Phase 2: Analysis of the nodes, maybe we have to split and move backward to keep splitting
		/*
		if self.children.len() == 0
		{
			// So far we have no splitting just yet so jut enforce that there are less than 5 rect in a
			// leaf which is kinda arbitrary but w/e
			assert!(self.boundingPolygons.len() < 5);

			self.boundingPolygons.push(newRectangle);
		}
		else
		{
			// Choose a subtree based on smallest enlargement, breaking ties by area
			let mut chosen = 0;
			let mut chosenMetrics = self.boundingPolygons[chosen].computeExpansionArea(&newRectangle);
			for i in 0..self.children.len()
			{
				let competitorMetrics = self.boundingPolygons[i].computeExpansionArea(&newRectangle);
				if competitorMetrics.1 <= chosenMetrics.1 && competitorMetrics.0.area < chosenMetrics.0.area
				{
					chosen = i;
					chosenMetrics = competitorMetrics;
				}
			}

			// Descend into the chosen subtree
			self.children[chosen].insert(newRectangle);
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
	// NOTE: This test does not describe a tightest-fitting-boxes R-Tree but is a good simple test

	// The node describing rectangles 1 and 2
	let rect1 = Rectangle::new(2, 2, 4, 5, 6);
	let rect2 = Rectangle::new(10, 18, 12, 20, 4);
	let node1 = Node::new(1, vec![rect1, rect2], vec![]);

	// The node describing rectangle 3
	let rect3 = Rectangle::new(17, 5, 18, 18, 13);
	let node2 = Node::new(2, vec![rect3], vec![]);

	// The linking (root) node describing node1 and node2
	let rect4 = Rectangle::new(0, 0, 13, 21, 273);
	let rect5 = Rectangle::new(15, 4, 21, 21, 102);
	let root = Node::new(3, vec![rect4, rect5], vec![node1, node2]);

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

#[test]
fn testSimpleInsert()
{
	// NOTE: This test does not describe a tightest-fitting-boxes R-Tree but is a good simple test

	// The node describing rectangles 1 and 2
	let rect1 = Rectangle::new(2, 2, 4, 5, 6);
	let rect2 = Rectangle::new(10, 18, 12, 20, 4);
	let node1 = Node::new(1, vec![rect1, rect2], vec![]);

	// The node describing rectangle 3
	let rect3 = Rectangle::new(17, 5, 18, 18, 13);
	let node2 = Node::new(2, vec![rect3], vec![]);

	// The linking (root) node describing node1 and node2
	let rect4 = Rectangle::new(0, 0, 13, 21, 273);
	let rect5 = Rectangle::new(15, 4, 21, 21, 102);
	let mut root = Node::new(3, vec![rect4, rect5], vec![node1, node2]);

	// Rectangles to be inserted, they all fall under rect4 and should all be inserted under node1
	let insert1 = Rectangle::new(6, 3, 8, 4, 2);
	let insert2 = Rectangle::new(1, 13, 2, 14, 1);
	let insert3 = Rectangle::new(11, 16, 15, 18, 8);

	// Insert our rectangles
	root.insert(insert1);
	/*
	root.insert(insert2);
	root.insert(insert3);

	// Test that our rectangles were inserted into the expected node
	assert!(root.children[0].boundingPolygons.len() == 5);
	assert!(root.children[0].boundingPolygons[2].area == 2);
	assert!(root.children[0].boundingPolygons[3].area == 1);
	assert!(root.children[0].boundingPolygons[4].area == 8);

	// Test that the previous rectangles are still there
	assert!(root.children[0].boundingPolygons[0].area == 6);
	assert!(root.children[0].boundingPolygons[1].area == 4);
	*/
}
