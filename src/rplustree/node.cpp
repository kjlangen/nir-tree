#include <rplustree/node.h>
#include <rplustree/rplustree.h>

namespace rplustree
{
	Node::Node(RPlusTree &treeRef) :
		treeRef(treeRef)
	{
		minBranchFactor = 0;
		maxBranchFactor = 0;
		parent = nullptr;
	}

	Node::Node(RPlusTree &treeRef, unsigned minBranch, unsigned maxBranch, Node *p) :
		treeRef(treeRef)
	{
		minBranchFactor = minBranch;
		maxBranchFactor = maxBranch;
		parent = p;
	}

	void Node::deleteSubtrees()
	{
		if (branches.size() == 0)
		{
			return;
		}
		else
		{
			unsigned branchesSize = branches.size();
			for (unsigned i = 0; i < branchesSize; ++i)
			{
				branches[i].child->deleteSubtrees();
				delete branches[i].child;
			}
		}
	}

	Rectangle Node::boundingBox()
	{
		Rectangle bb = Rectangle();

		if (data.size() != 0)
		{
			bb = Rectangle(data[0], data[0]);
			for (unsigned i = 1; i < data.size(); ++i)
			{
				bb.expand(data[i]);
			}
		}
		else if (branches.size() != 0)
		{
			bb = branches[0].boundingBox;
			for (unsigned i = 1; i < branches.size(); ++i)
			{
				bb.expand(branches[i].boundingBox);
			}
		}

		return bb;
	}

	void Node::updateBranch(Node *child, Rectangle &boundingBox)
	{
		// Locate the child
		unsigned childIndex;
		for (childIndex = 0; branches[childIndex].child != child && childIndex < branches.size(); ++childIndex) {}

		// Update the child
		branches[childIndex] = {child, boundingBox};
	}

	void Node::removeBranch(Node *child)
	{
		// Locate the child
		unsigned childIndex;
		for (childIndex = 0; branches[childIndex].child != child && childIndex < branches.size(); ++childIndex) {}

		// Delete the child deleting it and overwriting its branch
		delete child;
		branches[childIndex] = branches.back();
		branches.pop_back();
	}

	void Node::removeData(Point givenPoint)
	{
		// Locate the point
		unsigned pointIndex;
		for (pointIndex = 0; data[pointIndex] != givenPoint && pointIndex < data.size(); ++pointIndex) {}

		// Delete the point by overwriting it
		data[pointIndex] = data.back();
		data.pop_back();
	}

	void Node::exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator)
	{
		if (branches.size() == 0)
		{
			// We are a leaf so add our data points when they are the search point
			for (unsigned i = 0; i < data.size(); ++i)
			{
				if (requestedPoint == data[i])
				{
					accumulator.push_back(data[i]);
					break;
				}
			}
		}
		else
		{
			// Determine which branches we need to follow
			for (unsigned i = 0; i < branches.size(); ++i)
			{
				// Recurse
				branches[i].child->exhaustiveSearch(requestedPoint, accumulator);
			}
		}
	}

	std::vector<Point> Node::search(Point &requestedPoint)
	{
		std::vector<Point> matchingPoints;

		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();

			if (currentContext->branches.size() == 0)
			{
				// We are a leaf so add our data points when they are the search point
				for (unsigned i = 0; i < currentContext->data.size(); ++i)
				{
					if (requestedPoint == currentContext->data[i])
					{
						matchingPoints.push_back(currentContext->data[i]);
					}
				}

#if defined(STAT)
				treeRef.stats.markLeafSearched();
#endif
			}
			else
			{
				// Determine which branches we need to follow
				for (unsigned i = 0; i < currentContext->branches.size(); ++i)
				{
					if (currentContext->branches[i].boundingBox.containsPoint(requestedPoint))
					{
						// Add to the nodes we will check
						context.push(currentContext->branches[i].child);
					}
				}

#if defined(STAT)
				treeRef.stats.markNonLeafNodeSearched();
#endif
			}
		}

#if defined(STAT)
		treeRef.stats.resetSearchTracker<false>();
#endif

		return matchingPoints;
	}

	std::vector<Point> Node::search(Rectangle &requestedRectangle)
	{
		std::vector<Point> matchingPoints;

		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();

			if (currentContext->branches.size() == 0)
			{
				// We are a leaf so add our data points when they are within the search rectangle
				for (unsigned i = 0; i < currentContext->data.size(); ++i)
				{
					if (requestedRectangle.containsPoint(currentContext->data[i]))
					{
						matchingPoints.push_back(currentContext->data[i]);
					}
				}

#if defined(STAT)
				treeRef.stats.markLeafSearched();
#endif
			}
			else
			{
				// Determine which branches we need to follow
				for (unsigned i = 0; i < currentContext->branches.size(); ++i)
				{
					if (currentContext->branches[i].boundingBox.intersectsRectangle(requestedRectangle))
					{
						// Add to the nodes we will check
						context.push(currentContext->branches[i].child);
					}
				}
#if defined(STAT)
				treeRef.stats.markNonLeafNodeSearched();
#endif
			}
		}

#if defined(STAT)
		treeRef.stats.resetSearchTracker<true>();
#endif

		return matchingPoints;
	}

	// Always called on root, this = root
	// This top-to-bottom sweep is only for adjusting bounding boxes to contain the point and
	// choosing a particular leaf
	Node *Node::chooseNode(Point givenPoint)
	{
		// CL1 [Initialize]
		Node *context = this;

		for (;;)
		{
			// CL2 [Leaf check]
			if (context->branches.size() == 0)
			{
				return context;
			}
			else
			{
				// Compute the smallest expansion
				unsigned smallestExpansionIndex = 0;
				double smallestExpansionArea = context->branches[0].boundingBox.computeExpansionArea(givenPoint);
				double expansionArea;
				for (unsigned i = 1; i < context->branches.size() && smallestExpansionArea != -1.0; ++i)
				{
					expansionArea = context->branches[i].boundingBox.computeExpansionArea(givenPoint);
					if (expansionArea < smallestExpansionArea)
					{
						smallestExpansionIndex = i;
						smallestExpansionArea = expansionArea;
					}
				}

				if (smallestExpansionArea != -1.0)
				{
					context->branches[smallestExpansionIndex].boundingBox.expand(givenPoint);
				}

				// Descend
				context = context->branches[smallestExpansionIndex].child;
			}
		}
	}

	Node *Node::findLeaf(Point givenPoint)
	{
		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();

			if (currentContext->branches.size() == 0)
			{
				// FL2 [Search leaf node for record]
				// Check each entry to see if it matches E
				for (unsigned i = 0; i < currentContext->data.size(); ++i)
				{
					if (currentContext->data[i] == givenPoint)
					{
						return currentContext;
					}
				}
			}
			else
			{
				// FL1 [Search subtrees]
				// Determine which branches we need to follow
				for (unsigned i = 0; i < currentContext->branches.size(); ++i)
				{
					if (currentContext->branches[i].boundingBox.containsPoint(givenPoint))
					{
						// Add the child to the nodes we will consider
						context.push(currentContext->branches[i].child);
					}
				}
			}
		}

		return nullptr;
	}

	Node::Partition Node::partitionNode()
	{
		rplustree::Node::Partition defaultPartition;
		unsigned costMetric = std::numeric_limits<unsigned>::max();
		double location;

		if (branches.size() == 0)
		{
			unsigned dataSize = data.size();

			for (unsigned d = 0; d < dimensions; ++d)
			{
				// Sort along dimension d
				std::sort(data.begin(), data.end(), [d](Point a, Point b){return a[d] < b[d];});

				// Pick at least half the data
				location = data[dataSize / 2 - 1][d];

				// Compute cost, # of duplicates of this location
				unsigned duplicateCount = 0;
				for (Point dataPoint : data)
				{
					if (location == dataPoint[d])
					{
						++duplicateCount;
					}
				}

				// Compare cost
				if (duplicateCount < costMetric)
				{
					defaultPartition.dimension = d;
					defaultPartition.location = location;
					costMetric = duplicateCount;
				}
			}

			return defaultPartition;
		}
		else
		{
			std::vector<Rectangle> sortableBoundingBoxes;

			for (Branch b : branches)
			{
				sortableBoundingBoxes.push_back(b.boundingBox);
			}

			for (unsigned d = 0; d < dimensions; ++d)
			{
				// Sort along d
				std::sort(sortableBoundingBoxes.begin(), sortableBoundingBoxes.end(), [d](Rectangle a, Rectangle b){return a.upperRight[d] < b.upperRight[d];});

				// Pick at least half of the rectangles
				location = sortableBoundingBoxes[sortableBoundingBoxes.size() / 2 - 1].upperRight[d];

				// Compute cost, # of splits if d is chosen
				unsigned currentInducedSplits = 0;
				for (unsigned i = 0; i < sortableBoundingBoxes.size(); ++i)
				{
					if (sortableBoundingBoxes[i].lowerLeft[d] < location && location < sortableBoundingBoxes[i].upperRight[d])
					{
						++currentInducedSplits;
					}
				}

				// Compare cost
				if (currentInducedSplits < costMetric)
				{
					defaultPartition.dimension = d;
					defaultPartition.location = location;
					costMetric = currentInducedSplits;
				}
			}

			return defaultPartition;
		}
	}

	// Splitting a node will remove it from its parent node and its memory will be freed
	Node::SplitResult Node::splitNode(Partition p)
	{
		Node *left = new Node(treeRef, minBranchFactor, maxBranchFactor, parent);
		Node *right = new Node(treeRef, minBranchFactor, maxBranchFactor, parent);
		unsigned dataSize = data.size();
		unsigned branchesSize = branches.size();

		if (branchesSize == 0 && dataSize > 0)
		{
			for (Point dataPoint : data)
			{
				if (dataPoint[p.dimension] <= p.location && left->data.size() < maxBranchFactor)
				{
					left->data.push_back(dataPoint);
				}
				else
				{
					right->data.push_back(dataPoint);
				}
			}
			data.clear();
		}
		else
		{
			for (unsigned i = 0; i < branchesSize; ++i)
			{
				if (branches[i].boundingBox.upperRight[p.dimension] <= p.location)
				{
					branches[i].child->parent = left;
					left->branches.push_back(branches[i]);
				}
				else if (branches[i].boundingBox.lowerLeft[p.dimension] >= p.location)
				{
					branches[i].child->parent = right;
					right->branches.push_back(branches[i]);
				}
				else
				{
					Node::SplitResult downwardSplit = branches[i].child->splitNode(p);

					delete branches[i].child;

					if (downwardSplit.leftBranch.boundingBox != Rectangle::atInfinity)
					{
						downwardSplit.leftBranch.child->parent = left;
						left->branches.push_back(downwardSplit.leftBranch);
					}

					if (downwardSplit.rightBranch.boundingBox != Rectangle::atInfinity)
					{
						downwardSplit.rightBranch.child->parent = right;
						right->branches.push_back(downwardSplit.rightBranch);
					}
				}
			}
			branches.clear();
		}

		return {{left, left->boundingBox()}, {right, right->boundingBox()}};
	}

	// Splitting a node will remove it from its parent node and its memory will be freed
	Node::SplitResult Node::splitNode()
	{
		Node::SplitResult returnSplit = splitNode(partitionNode());

		return returnSplit;
	}

	// This bottom-to-top sweep is only for splitting bounding boxes as necessary
	Node::SplitResult Node::adjustTree()
	{
		Node *currentContext = this;
		unsigned branchesSize, dataSize;
		Node::SplitResult propagationSplit = {{nullptr, Rectangle()}, {nullptr, Rectangle()}};

		for (;currentContext != nullptr;)
		{
			branchesSize = currentContext->branches.size();
			dataSize = currentContext->data.size();

			// If there was a split we were supposed to propagate then propagate it
			if (propagationSplit.leftBranch.child != nullptr && propagationSplit.rightBranch.child != nullptr)
			{
				if (propagationSplit.leftBranch.child->data.size() > 0 || propagationSplit.leftBranch.child->branches.size() > 0)
				{
					currentContext->branches.push_back(propagationSplit.leftBranch);
					++branchesSize;
				}

				if (propagationSplit.rightBranch.child->data.size() > 0 || propagationSplit.rightBranch.child->branches.size() > 0)
				{
					currentContext->branches.push_back(propagationSplit.rightBranch);
					++branchesSize;
				}
			}

			// Early exit if this node does not overflow
			if (dataSize <= currentContext->maxBranchFactor && branchesSize <= currentContext->maxBranchFactor)
			{
				propagationSplit = {{nullptr, Rectangle()}, {nullptr, Rectangle()}};
				break;
			}

			// Otherwise, split node
			propagationSplit = currentContext->splitNode();

			// Cleanup before ascending
			if (currentContext->parent != nullptr)
			{
				currentContext->parent->removeBranch(currentContext);
			}

			// Ascend, propagating splits
			currentContext = propagationSplit.leftBranch.child->parent;
		}

		return propagationSplit;
	}

	// Always called on root, this = root
	Node *Node::insert(Point givenPoint)
	{
		// Find the appropriate position for the new point
		Node *adjustContext = chooseNode(givenPoint);

		// Adjust the tree
		// Add just the data
		adjustContext->data.push_back(givenPoint);

		// There is no guarantee that the root will still exist after adjustment so backup branch factors
		unsigned backupMinBranchFactor = minBranchFactor;
		unsigned backupMaxBranchFactor = maxBranchFactor;
		Node::SplitResult finalSplit = adjustContext->adjustTree();

		// Grow the tree taller if we need to
		if (finalSplit.leftBranch.child != nullptr && finalSplit.rightBranch.child != nullptr)
		{
			Node *newRoot = new Node(treeRef, backupMinBranchFactor, backupMaxBranchFactor, nullptr);

			finalSplit.leftBranch.child->parent = newRoot;
			newRoot->branches.push_back(finalSplit.leftBranch);
			finalSplit.rightBranch.child->parent = newRoot;
			newRoot->branches.push_back(finalSplit.rightBranch);

			delete this;

			return newRoot;
		}

		return this;
	}

	// To be called on a leaf
	void Node::condenseTree()
	{
		Node *currentContext = this;
		Node *previousContext = nullptr;

		for (; currentContext != nullptr; currentContext = currentContext->parent)
		{
			if (previousContext != nullptr)
			{
				if (previousContext->branches.size() == 0 && previousContext->data.size() == 0)
				{
					currentContext->removeBranch(previousContext);
				}
				else
				{
					Rectangle previousBox = previousContext->boundingBox();
					currentContext->updateBranch(previousContext, previousBox);
				}
			}

			previousContext = currentContext;
		}
	}

	// Always called on root, this = root
	Node *Node::remove(Point givenPoint)
	{
		// D1 [Find node containing record]
		Node *leaf = findLeaf(givenPoint);

		// Record not in the tree
		if (leaf == nullptr)
		{
			return this;
		}

		// D2 [Delete record]
		leaf->removeData(givenPoint);

		// D3 [Propagate changes]
		leaf->condenseTree();

		// D4 [Shorten tree]
		if (branches.size() == 1)
		{
			Node *newRoot = branches[0].child;
			delete this;
			newRoot->parent = nullptr;
			return newRoot;
		}

		return this;
	}

	unsigned Node::checksum()
	{
		unsigned sum = 0;

		if (branches.size() == 0)
		{
			for (unsigned i = 0; i < data.size(); ++i)
			{
				for (unsigned d = 0; d < dimensions; ++d)
				{
					sum += (unsigned)data[i][d];
				}
			}
		}
		else
		{
			for (unsigned i = 0; i < branches.size(); ++i)
			{
				// Recurse
				sum += branches[i].child->checksum();
			}
		}

		return sum;
	}

	bool Node::validate(Node *expectedParent, unsigned index)
	{
		if (parent != expectedParent || branches.size() > maxBranchFactor)
		{
			std::cout << "parent = " << (void *)parent << " expectedParent = " << (void *)expectedParent << std::endl;
			std::cout << "maxBranchFactor = " << maxBranchFactor << std::endl;
			std::cout << "branches.size() = " << branches.size() << std::endl;
			assert(parent == expectedParent);
			assert(branches.size() <= maxBranchFactor);
		}

		if (expectedParent != nullptr)
		{
			for (unsigned i = 0; i < data.size(); ++i)
			{
				if (!parent->branches[index].boundingBox.containsPoint(data[i]))
				{
					std::cout << parent->branches[index].boundingBox << " fails to contain " << data[i] << std::endl;
					assert(parent->branches[index].boundingBox.containsPoint(data[i]));
				}
			}
		}

		bool valid = true;
		for (unsigned i = 0; i < branches.size(); ++i)
		{
			valid = valid && branches[i].child->validate(this, i);
		}

		return valid;
	}

	void Node::print(unsigned n)
	{
		std::string indendtation(n * 4, ' ');
		std::cout << indendtation << "Node " << (void *)this << std::endl;
		std::cout << indendtation << "(" << std::endl;
		std::cout << indendtation << "    Parent: " << (void *)parent << std::endl;
		std::cout << indendtation << "    Branches: " << std::endl;
		for (unsigned i = 0; i < branches.size(); ++i)
		{
			std::cout << indendtation << "		" << (void *)branches[i].child << std::endl;
			std::cout << indendtation << "		" << branches[i].boundingBox << std::endl;
		}
		std::cout << indendtation << "    Data: ";
		for (unsigned i = 0; i < data.size(); ++i)
		{
			std::cout << data[i];
		}
		std::cout << std::endl << indendtation << ")" << std::endl;
	}

	void Node::printTree(unsigned n)
	{
		// Print this node first
		print(n);

		// Print any of our children with one more level of indentation
		std::string indendtation(n * 4, ' ');
		std::cout << std::endl << indendtation << "{" << std::endl;
		if (branches.size() > 0)
		{
			for (unsigned i = 0; i < branches.size(); ++i)
			{
				// Recurse
				branches[i].child->printTree(n + 1);
			}
		}
		std::cout << std::endl << indendtation << "}" << std::endl;
	}

	unsigned Node::height()
	{
		unsigned ret = 0;
		Node *node = this;

		for (;;)
		{
			ret++;
			if (node->branches.size() == 0)
			{
				return ret;
			}
			else
			{
				node = node->branches[0].child;
			}
		}
	}

	void Node::stat()
	{
#if defined(STAT)
		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;
		unsigned long branchesSize;
		unsigned long dataSize;
		size_t memoryFootprint = 0;
		unsigned long totalNodes = 1;
		unsigned long singularBranches = 0;
		unsigned long totalLeaves = 0;

		std::vector<unsigned long> histogramFanout;
		histogramFanout.resize(maxBranchFactor, 0);

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();

			branchesSize = currentContext->branches.size();
			dataSize = currentContext->data.size();
			unsigned fanout = branchesSize == 0 ? dataSize : branchesSize;
			if (unlikely(fanout >= histogramFanout.size()))
			{
				histogramFanout.resize(2*fanout, 0);
			}
			++histogramFanout[fanout];

			if (branchesSize == 0 && dataSize > 0)
			{
				++totalLeaves;
				memoryFootprint += sizeof(Node) + currentContext->data.size() * sizeof(Point);
			}
			else
			{
				totalNodes += branchesSize;
				memoryFootprint += sizeof(Node) + currentContext->branches.size() * sizeof(Node::Branch);
				for (unsigned i = 0; i < currentContext->branches.size(); ++i)
				{
					if (currentContext->branches[i].child->branches.size() == 1 || currentContext->branches[i].child->data.size() == 1)
					{
						++singularBranches;
					}

					context.push(currentContext->branches[i].child);
				}
			}
		}

		// Print out what we have found
		STATMEM(memoryFootprint);
		STATHEIGHT(height());
		STATSIZE(totalNodes);
		STATSINGULAR(singularBranches);
		STATLEAF(totalLeaves);
		STATBRANCH(totalNodes - 1);
		STATFANHIST();
		for (unsigned i = 0; i < histogramFanout.size(); ++i)
		{
			if (histogramFanout[i] > 0)
			{
				STATHIST(i, histogramFanout[i]);
			}
		}
		std::cout << treeRef.stats;
#else
		(void) 0;
#endif
	}
}
