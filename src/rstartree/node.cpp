#include <rstartree/node.h>
#include <rstartree/rstartree.h>

namespace rstartree
{
	template <typename N, typename functor>
	void treeWalker(N root, functor &f)
	{
		static_assert(std::is_pointer<N>::value, "Expected a pointer for N.");

		std::stack<N> context;
		context.push(root);
		N currentContext;

		while (!context.empty())
		{
			currentContext = context.top();
			context.pop();
			
			// Apply the general function to this node
			f(currentContext);

			// Recurse through the rest of the tree
			bool isLeaf = currentContext->isLeafNode();
			if (!isLeaf)
			{
				for (const auto &entry : currentContext->entries)
				{
					context.push(std::get<Node::Branch>(entry).child);
				}
			}
		}
	}

	template <typename functor>
	void adjustNodeLevels(Node *root, functor f)
	{
		struct bumpNodeLevels
		{
			bumpNodeLevels(functor f) : f(f) {}

			void operator()(Node *node)
			{
				node->level = f(node->level);
			}

			functor f;
		};

		bumpNodeLevels b(f);
		treeWalker(root, b);
	}

	bool Node::Branch::operator==(const Branch &o) const
	{
		return child == o.child && boundingBox == o.boundingBox;
	}

	Node::Node(RStarTree &treeRef, Node *parent, unsigned level) : 
		treeRef(treeRef),
		parent(parent),
		level(level)
	{
		entries.reserve(treeRef.maxBranchFactor);
	}

	void Node::deleteSubtrees()
	{
		if (entries.empty() || !std::holds_alternative<Branch>(entries[0]))
		{
			return;
		}

		for (const auto &entry : entries)
		{
			const Branch &b = std::get<Branch>(entry);
			b.child->deleteSubtrees();

			delete b.child;
		}
	}

	Rectangle Node::boundingBox() const
	{
		assert(!entries.empty());
		Rectangle boundingBox(boxFromNodeEntry(entries[0]));

		for (auto iter = entries.begin() + 1; iter != entries.end(); iter++)
		{
			boundingBox.expand(boxFromNodeEntry(*iter));
		}

		return boundingBox;
	}

	bool Node::updateBoundingBox(Node *child, Rectangle updatedBoundingBox)
	{
		for (auto &entry : entries)
		{
			Branch &b = std::get<Branch>(entry);
			if (b.child == child)
			{
				if (b.boundingBox != updatedBoundingBox)
				{
					b.boundingBox = updatedBoundingBox;
					return true;
				}
				return false;
			}
		}

#ifndef NDEBUG
		print();
		assert(false);
#endif
		return false;
	}

	void Node::removeChild(Node *child)
	{
		for (auto iter = entries.begin(); iter != entries.end(); iter++)
		{
			if (std::get<Branch>(*iter).child == child)
			{ 
				entries.erase(iter);
				return;
			}
		}
	}

	void Node::removeData(const Point &givenPoint)
	{
		for (auto iter = entries.begin(); iter != entries.end(); iter++)
		{
			if (std::get<Point>(*iter) == givenPoint)
			{
				entries.erase(iter);
				return;
			}
		}
	}

	void Node::exhaustiveSearch(const Point &requestedPoint, std::vector<Point> &accumulator) const
	{
		// Am I a leaf?
		bool isLeaf = isLeafNode();
		if (isLeaf)
		{
			for (const auto &entry : entries)
			{
				const Point &p = std::get<Point>(entry);

				if (p == requestedPoint)
				{
					accumulator.push_back(p);
				}
			}
		}
		else
		{
			for (const auto &entry : entries)
			{
				std::get<Branch>(entry).child->exhaustiveSearch(requestedPoint, accumulator);
			}
		}
	}

	void Node::searchSub(const Point &requestedPoint, std::vector<Point> &accumulator) CONST_IF_NOT_STAT
	{

		std::stack<const Node *> context;
		context.push(this);
		while (!context.empty())
		{
			const Node *curNode = context.top();
			context.pop();
			// Am I a leaf?
			bool isLeaf = curNode->isLeafNode();
			if (isLeaf)
			{
#ifdef STAT
				treeRef.stats.markLeafSearched();
#endif
				for (const auto &entry : curNode->entries)
				{
					const Point &p = std::get<Point>(entry);

					if (p == requestedPoint)
					{
						accumulator.push_back(p);
					}
				}
			}
			else
			{
#ifdef STAT
				treeRef.stats.markNonLeafNodeSearched();
#endif
				for (const auto &entry : curNode->entries)
				{

					const Branch &b = std::get<Branch>(entry);

					if (b.boundingBox.containsPoint(requestedPoint))
					{
						context.push(b.child);
					}
				}
			}
		}
	}

	void Node::searchSub(const Rectangle &rectangle, std::vector<Point> &accumulator) CONST_IF_NOT_STAT
	{
		std::stack<const Node *> context;
		context.push(this);
		while (!context.empty())
		{
			const Node *curNode = context.top();
			context.pop();

			bool isLeaf = curNode->isLeafNode();
			if (isLeaf)
			{
#ifdef STAT
				treeRef.stats.markLeafSearched();
#endif
				for (const auto &entry : curNode->entries)
				{
					const Point &p = std::get<Point>(entry);

					if (rectangle.containsPoint(p))
					{
						accumulator.push_back(p);
					}
				}
			}
			else
			{
#ifdef STAT
				treeRef.stats.markNonLeafNodeSearched();
#endif
				for (const auto &entry : curNode->entries)
				{
					const Branch &b = std::get<Branch>(entry);

					if (b.boundingBox.intersectsRectangle(rectangle))
					{
						context.push(b.child);
					}
				}
			}
		}
	}


	std::vector<Point> Node::search(const Point &requestedPoint) CONST_IF_NOT_STAT
	{
		std::vector<Point> accumulator;

		searchSub(requestedPoint, accumulator);

#ifdef STAT
		treeRef.stats.resetSearchTracker<false>();
#endif
		return accumulator;
	}

	std::vector<Point> Node::search(const Rectangle &requestedRectangle) CONST_IF_NOT_STAT
	{
		std::vector<Point> matchingPoints;

		searchSub(requestedRectangle, matchingPoints);

#ifdef STAT
		treeRef.stats.resetSearchTracker<true>();
#endif
		return matchingPoints;
	}

	double computeOverlapGrowth(unsigned index, const std::vector<Node::NodeEntry> &entries, const Rectangle &givenBox)
	{
		// We cannot be a leaf
		assert(!entries.empty());
		assert(std::holds_alternative<Node::Branch>(entries[0]));
		
		// 1. Make a test rectangle we will use to not modify the original
		Rectangle newRectangle = std::get<Node::Branch>(entries[index]).boundingBox;
		
		// 2. Add the point to the copied Rectangle
		newRectangle.expand(givenBox);

		// 3. Compute the overlap expansion area 
		double overlapArea = 0;
		for (unsigned i = 0; i < entries.size(); ++i)
		{
			const auto &entry = entries[i];

			if (i == index)
			{
				continue;
			}

			overlapArea += newRectangle.computeIntersectionArea(std::get<Node::Branch>(entry).boundingBox);
		}

		return overlapArea;
	}

	Node *Node::chooseSubtree(const NodeEntry &givenNodeEntry)
	{
		// CS1: This is CAlled on the root! Just like above
		// CS2: If N is a leaf return N (same)
		// CS3: If the child pointers (bounding boxes) -> choose reactangle that needs least
		// 		overlap enlargment to fit the new Point/bounding rectangle if tie return smallest area
		// 		i.e. the rectangle that has the least overlap -> tbh I'm not sure we can just leave this
		// 	Else: 
		// 		If not child pointers (bounding boxes) -> choose reactangle that needs least
		// 		overlap enlargment to fit the new Point (same as before) if tie return smallest area (same)


		// CL1 [Initialize]
		Node *node = this;

		// Always called on root, this = root
		assert(parent == nullptr);

		unsigned stoppingLevel = 0;
		bool entryIsBranch = std::holds_alternative<Branch>(givenNodeEntry);
		if (entryIsBranch)
		{
			const Branch &b = std::get<Branch>(givenNodeEntry);
			stoppingLevel = b.child->level + 1;
		}
		Rectangle givenEntryBoundingBox = boxFromNodeEntry(givenNodeEntry);

		for (;;)
		{

			if (node->level == stoppingLevel)
			{
				return node;
			}

			assert(!node->isLeafNode());

			// Our children point to leaves
#ifndef NDEBUG
			const Branch &firstBranch = std::get<Branch>(node->entries[0]);
#endif
			assert(!firstBranch.child->entries.empty());

			unsigned descentIndex = 0;
			double smallestOverlapExpansion = std::numeric_limits<double>::max();
			double smallestExpansionArea = std::numeric_limits<double>::max();
			double smallestArea = std::numeric_limits<double>::max();
			double testOverlapExpansionArea, testExpansionArea, testArea;
			
			bool childrenAreLeaves = !std::holds_alternative<Branch>(std::get<Branch>(node->entries[0]).child->entries[0]);
			if (childrenAreLeaves)
			{
				// Choose the entry in N whose rectangle needs least overlap enlargement
				for (unsigned i = 0; i < node->entries.size(); ++i)
				{
					const NodeEntry &entry = node->entries[i];
					const Branch &b = std::get<Branch>(entry);

					// Compute overlap
					testOverlapExpansionArea = computeOverlapGrowth(i, node->entries, givenEntryBoundingBox);

					// Take largest overlap
					if (smallestOverlapExpansion > testOverlapExpansionArea)
					{
						descentIndex = i;
						smallestOverlapExpansion = testOverlapExpansionArea;

						// Update smallesExpansionArea if needed
						testExpansionArea = b.boundingBox.computeExpansionArea(givenEntryBoundingBox);
						if (smallestExpansionArea > testExpansionArea)
						{
							smallestExpansionArea = testExpansionArea;
						}

						// Update area if needed
						testArea = b.boundingBox.area();
						if (smallestArea > testArea)
						{
							smallestArea = testArea;
						}
					} 
					else if (smallestOverlapExpansion == testOverlapExpansionArea)
					{
						// Use expansion area to break tie
						testExpansionArea = b.boundingBox.computeExpansionArea(givenEntryBoundingBox);
						if (smallestExpansionArea > testExpansionArea)
						{
							descentIndex = i;
							smallestExpansionArea = testExpansionArea;
							
							// Update area if needed
							testArea = b.boundingBox.area();
							if (smallestArea > testArea)
							{
								smallestArea = testArea;
							}

						}
						else if (smallestExpansionArea == testExpansionArea)
						{
							// Use area to break tie
							testArea = b.boundingBox.area();
							if (smallestArea > testArea)
							{
								descentIndex = i;
								smallestArea = testArea;
							}
						}
					}
				}
			}
			else
			{
				// CL2 [Choose subtree]
				// Find the bounding box with least required expansion/overlap
				for (unsigned i = 0; i < node->entries.size(); ++i)
				{
					const NodeEntry &entry = node->entries[i];
					const Branch &b = std::get<Branch>(entry);

					testExpansionArea = b.boundingBox.computeExpansionArea(givenEntryBoundingBox);
					if (smallestExpansionArea > testExpansionArea)
					{
						descentIndex = i;
						smallestExpansionArea = testExpansionArea;

						// Potentially update smallest area
						testArea = b.boundingBox.area();
						if (smallestArea > testArea)
						{
							smallestArea = testArea;
						}
					}
					else if (smallestExpansionArea == testExpansionArea)
					{
						// Use area to break tie
						testArea = b.boundingBox.area();
						if (smallestArea > testArea)
						{
							descentIndex = i;
							smallestArea = testArea;
						}
					}
				}
			}

			// Descend
			node = std::get<Branch>(node->entries[descentIndex]).child;
		}
	}

	Node *Node::findLeaf(const Point &givenPoint)
	{
		assert(!entries.empty());

		// Am I a leaf?
		bool isLeaf = isLeafNode();
		if (isLeaf)
		{
			for (const auto &entry : entries)
			{
				if (std::get<Point>(entry) == givenPoint)
				{
					return this;
				}
			}

			return nullptr;
		}

		for (const auto &entry : entries)
		{
			const Branch &b = std::get<Branch>(entry);

			if (b.boundingBox.containsPoint(givenPoint))
			{
				Node *ptr = b.child->findLeaf(givenPoint);
				if (ptr != nullptr)
				{
					return ptr;
				}
				// Nope, keep looking...
			}
		}

		return nullptr;
	}


	// Helper function that takes a pre-sorted data and then computes the sum
	// of all margin values over all possible M - 2m + 2 distributions
	double Node::computeTotalMarginSum()
	{
		// Use the data to find possible matches (1, M - 2m +2) possible distributions
		// First split the vector into vectorA and vectorB with vectorA being the minimum
		const auto groupABegin = entries.begin();
		const auto groupAEnd = entries.begin() + treeRef.minBranchFactor;
		const auto groupBBegin = entries.begin() + treeRef.minBranchFactor;
		const auto groupBEnd = entries.end();

		std::vector<NodeEntry> groupA(groupABegin, groupAEnd);
		std::vector<NodeEntry> groupB(groupBBegin, groupBEnd);

		// Find the total margin sum size out of all the distributions along given axis
		double sumOfAllMarginValues = 0;

		// Test all M - 2m + 2 groupings
		while (groupA.size() <= treeRef.maxBranchFactor && groupB.size() >= treeRef.minBranchFactor)
		{
			Rectangle boundingBoxA = boxFromNodeEntry(groupA[0]);
			for (unsigned i = 1; i < groupA.size(); ++i)
			{
				boundingBoxA.expand(boxFromNodeEntry(groupA[i]));
			}

			Rectangle boundingBoxB = boxFromNodeEntry(groupB[0]);
			for (unsigned i = 1; i < groupB.size(); ++i)
			{
				boundingBoxB.expand(boxFromNodeEntry(groupB[i]));
			}

			// Calculate new margin sum
			sumOfAllMarginValues += boundingBoxA.margin() + boundingBoxB.margin();
			
			// Add one new value to groupA and remove one from groupB. Repeat.
			NodeEntry transfer = groupB.front();
			groupB.erase(groupB.begin());
			groupA.push_back(transfer);
		}

		return sumOfAllMarginValues;
	}

	void Node::entrySort(unsigned startingDimension)
	{
		assert(entries.size() > 0);
		bool isLeaf = isLeafNode();
		if (isLeaf)
		{
			std::sort(entries.begin(), entries.end(),
				[startingDimension](NodeEntry &a, NodeEntry &b)
				{
					return std::get<Point>(a).orderedCompare(std::get<Point>(b), startingDimension);
				});
		}
		else
		{
			std::sort(entries.begin(), entries.end(),
				[startingDimension](NodeEntry &a, NodeEntry &b)
				{
					Rectangle A = boxFromNodeEntry(std::get<Branch>(a));
					Rectangle B = boxFromNodeEntry(std::get<Branch>(b));
					return A.lowerLeft.orderedCompare(B.lowerLeft, startingDimension) && A.upperRight.orderedCompare(B.upperRight, startingDimension);
				});
		}
	}

	// CSA1: Sort entries by lower and upper bound along each axis and compute S -> sum of all
	//  margin values for the different distributions. This can be stored in a array of variable
	//  that we keep in a loop -> and the just compare to the others?
	// 	We can first call a helper function that returns an array of all possible distributions for it?
	// CSA2: Return the Axis that has the minimum total sum of all the distributions
	unsigned Node::chooseSplitAxis()
	{
		unsigned optimalAxis = 0;
		double optimalMargin = std::numeric_limits<double>::infinity();
		double evalMargin;

		for (unsigned d = 0; d < dimensions; ++d)
		{
			entrySort(d);
			evalMargin = computeTotalMarginSum();

			if (evalMargin < optimalMargin)
			{
				optimalAxis = d;
				optimalMargin = evalMargin;
			}
		}

		// Sort because optimalAxis may not have been the last considered
		entrySort(optimalAxis);

		return optimalAxis;
	}

	// CSI1: Given the chosen split index
	// 	group all the entries into multiple groups and choose the one that has the least
	// 	overlap value; resolve ties with the minimum area
	// 	returns tuple of best distribution group indices
	unsigned Node::chooseSplitIndex(unsigned axis)
	{
		// We assume this is called after we have sorted this->data according to axis.
		(void) axis;

		const auto groupABegin = entries.begin();
		const auto groupAEnd = entries.begin() + treeRef.minBranchFactor;
		const auto groupBBegin = entries.begin() + treeRef.minBranchFactor;
		const auto groupBEnd = entries.end();

		std::vector<NodeEntry> groupA(groupABegin, groupAEnd);
		std::vector<NodeEntry> groupB(groupBBegin, groupBEnd);
		unsigned splitIndex = entries.size() / 2;

		// Find the best size out of all the distributions
		unsigned minOverlap = std::numeric_limits<unsigned>::max();
		unsigned minArea = std::numeric_limits<unsigned>::max();

		// Tracking what the current "cut" mark is
		unsigned currentSplitPoint = treeRef.minBranchFactor;

		// Try each of the M-2m + 2 groups
		while (groupA.size() <= treeRef.maxBranchFactor && groupB.size() >= treeRef.minBranchFactor)
		{
			// Compute the margin of groupA and groupB
			Rectangle boundingBoxA = boxFromNodeEntry(groupA[0]);
			for (unsigned i = 1; i < groupA.size(); ++i)
			{
				boundingBoxA.expand(boxFromNodeEntry(groupA[i]));
			}

			Rectangle boundingBoxB = boxFromNodeEntry(groupB[0]);
			for (unsigned i = 1; i < groupB.size(); ++i)
			{
				boundingBoxB.expand(boxFromNodeEntry(groupB[i]));
			}

			// Compute intersection area to determine best grouping of data points
			unsigned evalDistOverlap = boundingBoxA.computeIntersectionArea(boundingBoxB);

			if (evalDistOverlap < minOverlap)
			{
				// We save this current distribution of indices to return
				minOverlap = evalDistOverlap;
				splitIndex = currentSplitPoint;

				// Set this if we haven't already
				if (minArea == std::numeric_limits<unsigned>::max())
				{
					minArea = boundingBoxA.area() + boundingBoxB.area();
				}
			}
			else if (evalDistOverlap == minOverlap)
			{
				// If overlap is equal, we use the distribution that creates the smallest areas
				unsigned evalMinArea = boundingBoxA.area() + boundingBoxB.area();

				if (evalMinArea < minArea)
				{
					// Save this current distribution of indices to return
					minArea = evalMinArea;
					splitIndex = currentSplitPoint;
				}
			}
			
			// Add one new value to groupA and remove one from groupB to obtain next distribution
			NodeEntry transferPoint = groupB.front();
			groupB.erase(groupB.begin());
			groupA.push_back(transferPoint);

			// Push the split point forward.
			currentSplitPoint++;
		}

		return splitIndex;
	}


	Node *Node::splitNode()
	{
		// S1: Call chooseSplitAxis to determine the axis perpendicular to which the split is performed
		// S2: Invoke chooseSplitIndex given the axis to determine the best distribution along this axis
		// S3: Distribute the entries among these two groups

		// Call chooseSplitAxis to determine the axis perpendicular to which the split is performed
		// For now we will save the axis as a int -> since this allows for room for growth in the future
		// Call ChooseSplitIndex to create optimal splitting of data array
		unsigned splitIndex = chooseSplitIndex(chooseSplitAxis());

		Node *newSibling = new Node(treeRef, parent, level);

		assert((parent == nullptr) || (level + 1 == parent->level));

		// Copy everything to the right of the splitPoint (inclusive) to the new sibling
		std::copy(entries.begin() + splitIndex, entries.end(), std::back_inserter(newSibling->entries));

		if (std::holds_alternative<Branch>(newSibling->entries[0]))
		{
			for (const auto &entry : newSibling->entries)
			{
				// Update parents
				const Branch &b = std::get<Branch>(entry);
				b.child->parent = newSibling;

				assert(level == b.child->level + 1);
				assert(newSibling->level == b.child->level + 1);
			}
		}

		// Chop our node's data down
		entries.erase(entries.begin() + splitIndex, entries.end());

#ifndef NDEBUG
		if (std::holds_alternative<Branch>(entries[0]))
		{
			for (const auto &entry : entries)
			{
				// Update parents
				const Branch &b = std::get<Branch>(entry);
				assert(level == b.child->level + 1);
			}
		}
#endif

		assert(!entries.empty());
		assert(!newSibling->entries.empty());

		// Return our newly minted sibling
		return newSibling;
	}

	Node *Node::adjustTree(Node *sibling, std::vector<bool> &hasReinsertedOnLevel)
	{
		// AT1 [Initialize]
		Node *node = this;
		Node *siblingNode = sibling;

		for (;;)
		{
			assert(node != nullptr);

			// AT2 [If node is the root, stop]
			if (node->parent == nullptr)
			{
				break;
			}
			else
			{
				// AT3 [Adjust covering rectangle in parent entry]
				bool didUpdateBoundingBox =node->parent->updateBoundingBox(node, node->boundingBox());

				// If we have a split then deal with it otherwise move up the tree
				if (siblingNode != nullptr)
				{
					assert(node->level + 1 == node->parent->level);
					assert(siblingNode->level + 1 == node->parent->level);

					// AT4 [Propogate the node split upwards]
					Branch b(siblingNode->boundingBox(), siblingNode);
					node->parent->entries.emplace_back(std::move(b));
#ifndef NDEBUG
					for (const auto &entry : node->parent->entries)
					{
						assert(std::holds_alternative<Branch>(entry));
						assert(std::get<Branch>(entry).child->level + 1 == node->parent->level);
					}
#endif
					if (node->parent->entries.size() > node->parent->treeRef.maxBranchFactor)
					{
						Node *parentBefore = node->parent;
						Node *siblingParent = node->parent->overflowTreatment(hasReinsertedOnLevel);

						if (siblingParent)
						{
							// We split our parent, so now we have two (possible) parents
							assert(node->parent == siblingParent || node->parent == parentBefore);
							assert(siblingNode->parent == siblingParent || siblingNode->parent == parentBefore);

							// Need to keep traversing up
							node = parentBefore;
							siblingNode = siblingParent;
							assert(node != siblingNode);

							continue;
						}
					}

					node = node->parent;
					siblingNode = nullptr;
				}
				else
				{
					// AT5 [Move up to next level]
					if (didUpdateBoundingBox)
					{
						node = node->parent;
					} else {

						// If we didn't update our bounding box and there was no split, no reason to keep
						// going.
						return nullptr;
					}
				}
			}
		}

		return siblingNode;
	}

	Node *Node::reInsert(std::vector<bool> &hasReinsertedOnLevel)
	{
		// 1. RI1 Compute distance between each of the points and the bounding box containing them.
		// 2. RI2 Sort the entries by DECREASING index -> ok let's define an
		// 		extra helper function that gets to do this and pass it into sort

		Point globalCenterPoint = boundingBox().centrePoint();

		assert(hasReinsertedOnLevel.at(level));

		std::sort(entries.begin(), entries.end(),
			[&globalCenterPoint](NodeEntry &a, NodeEntry &b)
			{
				Rectangle rectA = boxFromNodeEntry(a);
				Rectangle rectB = boxFromNodeEntry(b);
				return rectA.centrePoint().distance(globalCenterPoint) > rectB.centrePoint().distance(globalCenterPoint);
			});

		// 3. RI3 Remove the first p entries from N and adjust the bounding box -> OK so we need to adjust the data model
		//		to include a specified "p" value -> this should be unique to the node -> so it's a node variable
		unsigned numNodesToReinsert = treeRef.p * entries.size();

		// 4. Insert the removed entries -> OK we can also specify a flag that is
		//		if you want to reinsert starting with largest values (i.e. start at index 0) or closest values (Start at index p)

		// Find the root node
		Node *root = this;
		while (root->parent != nullptr)
		{
			root = root->parent;
		}

		// We need to reinsert these entries
		// We pop them all off before hand so that any reorganization of the tree during this recursive
		// insert does not affect which entries get popped off
		std::vector<NodeEntry> entriesToReinsert;
		entriesToReinsert.reserve(numNodesToReinsert);

		std::copy(entries.begin(), entries.begin() + numNodesToReinsert, std::back_inserter(entriesToReinsert));
		entries.erase(entries.begin(), entries.begin() + numNodesToReinsert);

		// During this recursive insert (we are already in an insert, since we are reInserting), we
		// may end up here again. If we do, we should still be using the same hasReinsertedOnLevel
		// vector because it corresponds to the activities we have performed during a single
		// point/rectangle insertion (the top level one)

		for (const NodeEntry entry : entriesToReinsert)
		{
			assert(root->parent == nullptr);
			// TODO: Will this actually do a copy or assume we know what we are doing?
			root = root->insert(entry, hasReinsertedOnLevel);
		}
		return nullptr;
	}

	// Overflow treatement for dealing with a node that is too big (overflow)
	Node *Node::overflowTreatment(std::vector<bool> &hasReinsertedOnLevel)
	{
		assert(hasReinsertedOnLevel.size() > level);

		if (hasReinsertedOnLevel.at(level))
		{
			return splitNode();
		}
		else
		{
			hasReinsertedOnLevel.at(level) = true;
			return reInsert(hasReinsertedOnLevel);
		}
	}

	Node *Node::insert(NodeEntry nodeEntry, std::vector<bool> &hasReinsertedOnLevel)
	{
		// Always called on root, this = root
		assert(parent == nullptr);

		// I1 [Find position for new record]
		Node *insertionPoint = chooseSubtree(nodeEntry);
		Node *sibling = nullptr;

		// I2 [Add record to leaf node]
		bool givenIsLeaf = std::holds_alternative<Point>(nodeEntry);
#ifndef NDEBUG
		bool firstIsPoint = entries.empty() || std::holds_alternative<Point>(insertionPoint->entries[0]);
		assert((givenIsLeaf && firstIsPoint) || (!givenIsLeaf && !firstIsPoint));
#endif
		insertionPoint->entries.push_back(nodeEntry);
		if (!givenIsLeaf)
		{
			const Branch &b = std::get<Branch>(nodeEntry);
			assert(insertionPoint->level == b.child->level + 1);
			b.child->parent = insertionPoint;
		}

		// If we exceed treeRef.maxBranchFactor we need to do something about it
		if (insertionPoint->entries.size() > treeRef.maxBranchFactor) 
		{
			// We call overflow treatment to determine how our sibling node is treated if we do a
			// reInsert, sibling is nullptr. This is properly dealt with in adjustTree
			sibling = insertionPoint->overflowTreatment(hasReinsertedOnLevel);
		}

		// I3 [Propogate overflow treatment changes upward]
		Node *siblingNode = insertionPoint->adjustTree(sibling, hasReinsertedOnLevel);

		// I4 [Grow tree taller]
		if (siblingNode != nullptr)
		{
			assert(this->parent == nullptr);

			Node *newRoot = new Node(treeRef, nullptr, this->level+1);
			this->parent = newRoot;

			// Make the existing root a child of newRoot
			Branch b1(boundingBox(), this);
			newRoot->entries.emplace_back(std::move(b1));

			// Make the new sibling node a child of newRoot
			siblingNode->parent = newRoot;
			Branch b2(siblingNode->boundingBox(), siblingNode);
			newRoot->entries.emplace_back(std::move(b2));

			// Ensure newRoot has both children
			assert(newRoot->entries.size() == 2);
			assert(siblingNode->level+1 == newRoot->level);

			// Fix the reinserted length
			hasReinsertedOnLevel.push_back(false);

			return newRoot;
		}
		else
		{
			// We might no longer be the parent.  If we hit overflowTreatment, we may have triggered
			// reInsert, which then triggered a split. That insert will have returned newRoot, but
			// because reInsert() returns nullptr, we don't know about it
			Node *root = this; 
#ifndef NDEBUG
			unsigned currentLevel = root->level;
#endif
			while (root->parent)
			{
				root = root->parent;
#ifndef NDEBUG
				assert(root->level == currentLevel + 1);
				currentLevel = root->level;
#endif
			}

			return root;
		}
	}

	// To be called on a leaf
	Node *Node::condenseTree(std::vector<bool> &hasReinsertedOnLevel)
	{
		// CT1 [Initialize]
		Node *node = this;

		// Is Leaf
		assert(entries.empty() || std::holds_alternative<Point>(entries[0]));

		std::vector<NodeEntry> Q;

		// CT2 [Find parent entry]
		unsigned entriesSize;
		while (node->parent != nullptr)
		{
			entriesSize = node->entries.size();

			// CT3 & CT4 [Eliminate under-full node. & Adjust covering rectangle.]
			if (entriesSize >= node->treeRef.minBranchFactor)
			{
				node->parent->updateBoundingBox(node, node->boundingBox());

				// CT5 [Move up one level in the tree]
				// Move up a level without deleting ourselves
				node = node->parent;
			}
			else
			{
				// Remove ourselves from our parent
				node->parent->removeChild(node);
				assert(!node->entries.empty());

				// Push these entries into Q
				std::copy(node->entries.begin(), node->entries.end(), std::back_inserter(Q));

				// Prepare for garbage collection
				Node *garbage = node;

				// CT5 [Move up one level in the tree]
				// Move up a level before deleting ourselves
				node = node->parent;

				// Cleanup ourselves without deleting children b/c they will be reinserted
				delete garbage;
			}
		}

		// CT6 [Re-insert oprhaned entries]
		for (const auto &entry : Q)
		{
			assert(node->parent == nullptr);
			node = node->insert(entry, hasReinsertedOnLevel);
		}

		return node;
	}

	// Always called on root, this = root
	Node *Node::remove(Point &givenPoint, std::vector<bool> hasReinsertedOnLevel)
	{
		assert(parent == nullptr);

		// D1 [Find node containing record]
		Node *leaf = findLeaf(givenPoint);

		if (leaf == nullptr)
		{
			return nullptr;
		}

		// D2 [Delete record]
		leaf->removeData(givenPoint);

		// D3 [Propagate changes]
		Node *root = leaf->condenseTree(hasReinsertedOnLevel);

		// D4 [Shorten tree]
		if (root->entries.size() == 1 and !root->isLeafNode())
		{
			// Slice the hasReinsertedOnLevel
			hasReinsertedOnLevel.pop_back();

			// We are removing the root to shorten the tree so we then decide to remove the root
			Branch &b = std::get<Branch>(root->entries[0]);

			// Get rid of the old root
			Node *child = b.child;
			delete root;

			// I'm the root now!
			child->parent = nullptr;
			return child;
		}
		else
		{
			return root;
		}
	}

	void Node::print() const
	{
		unsigned max_level = treeRef.root->level;

		std::string indentation((max_level - level) * 4, ' ');
		std::cout << indentation << "Node " << (void *)this << std::endl;
		std::cout << indentation << "{" << std::endl;
		std::cout << indentation << "    BoundingBox: " << boundingBox() << std::endl;
		std::cout << indentation << "    Parent: " << (void *)parent << std::endl;
		std::cout << indentation << "    Entries: " << std::endl;
		
		bool isLeaf = isLeafNode();
		if (isLeaf)
		{
			for (const auto &entry : entries)
			{
				std::cout << indentation << "		" << std::get<Point>(entry) << std::endl;
			}
		}
		else
		{
			for (const auto &entry : entries)
			{
				const Branch &b = std::get<Branch>(entry);
				std::cout << indentation << "		" << b.boundingBox << ", ptr: " << b.child << std::endl;
			}
		}
		std::cout << std::endl << indentation << "}" << std::endl;
	}

	void Node::printTree() const
	{
		// Print this node first
		struct Printer
		{
			void operator()(Node * const node)
			{
				node->print();
			}
		};

		Printer p;
		treeWalker(const_cast<Node * const>(this), p);
	}

	unsigned Node::checksum() const
	{
		struct ChecksumFunctor
		{
			unsigned checksum;

			ChecksumFunctor()
			{
				checksum = 0;
			}

			void operator()(Node * const node)
			{
				bool isLeaf = node->entries.empty() || std::holds_alternative<Point>(node->entries[0]);
				if (isLeaf)
				{
					for (const auto &entry : node->entries)
					{
						const Point &p = std::get<Point>(entry);
						for (unsigned d = 0; d < dimensions; ++d)
						{
							checksum += (unsigned)p[d];
						}
					}
				} 
			}
		};

		ChecksumFunctor cf;
		treeWalker(const_cast<Node * const>(this), cf);

		return cf.checksum;
	}

	unsigned Node::height() const
	{
		assert( parent == nullptr );
		return level+1;
	}


	void Node::stat() const
	{
#ifdef STAT
		struct StatWalker
		{
			size_t memoryFootprint;
			unsigned long totalNodes;
			unsigned long singularBranches;
			unsigned long totalLeaves;
			std::vector<unsigned long> histogramFanout;

			StatWalker(unsigned long maxBranchFactor)
			{
				memoryFootprint = 0;
				totalNodes = 0;
				singularBranches = 0;
				totalLeaves = 0;
				histogramFanout.resize(maxBranchFactor, 0);
			}

			void operator()(Node * const node)
			{
				unsigned entriesSize = node->entries.size();

				if (entriesSize == 1)
				{
					++singularBranches;
				}

				++totalNodes;

				if (unlikely(entriesSize >= histogramFanout.size()))
				{
					//Avoid reallocing
					histogramFanout.resize(2*entriesSize, 0);
				}
				++histogramFanout[entriesSize];

				bool isLeaf = node->isLeafNode();
				if (isLeaf)
				{
					++totalLeaves;
					memoryFootprint += sizeof(Node) + entriesSize * sizeof(Point);
				}
				else
				{
					memoryFootprint += sizeof(Node) + entriesSize * sizeof(Node *) + entriesSize * sizeof(Rectangle);
				}
			}
		};

		StatWalker sw(treeRef.maxBranchFactor);
		treeWalker(const_cast<Node * const>(this), sw);

		// Print out what we have found
		STATMEM(sw.memoryFootprint);
		STATHEIGHT(height());
		STATSIZE(sw.totalNodes);
		STATSINGULAR(sw.singularBranches);
		STATLEAF(sw.totalLeaves);
		STATBRANCH(sw.totalNodes - 1);
		STATFANHIST();
		for (unsigned i = 0; i < sw.histogramFanout.size(); ++i)
		{
			if (sw.histogramFanout[i] > 0)
			{
				STATHIST(i, sw.histogramFanout[i]);
			}
		}
		std::cout << treeRef.stats;
#else
		(void) 0;
#endif
	}

	Rectangle boxFromNodeEntry(const Node::NodeEntry &entry)
	{
		if (std::holds_alternative<Node::Branch>(entry))
		{
			return std::get<Node::Branch>(entry).boundingBox;
		}

		const Point &p = std::get<Point>(entry);
		return Rectangle(p, p);
	}
}
