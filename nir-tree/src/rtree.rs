// ChooseLeaf algorithm
fn chooseLeaf(node: Node)
{
	// Initialize
	let n = node;
	// Leaf check
	if rootNode.isLeaf()
	{
		return n;
	}

	// Choose subtree
	let f = n.chooseNodeLeastEnlargement();

	// Descend until a leaf is reached
	return chooseLeaf(f);
}

// Insert algorithm
fn insert(entry: Node) {
	// Find position for new record
	let leaf = chooseLeaf(rootNode); // L

	// Add e to the leaf
	if leaf.hasSpace()
	{
		leaf.insert(entry);
	}
	else
	{
		// Split the overflowing node
		let newLeaf = leaf.splitNode(entry); // L and LL
		// Propogate changes upward
		adjustTree(leaf, new leaf);
	}
}