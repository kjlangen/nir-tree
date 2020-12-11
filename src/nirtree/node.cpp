#include <nirtree/node.h>

namespace nirtree
{
	Node::Node()
	{
		minBranchFactor = 0;
		maxBranchFactor = 0;
		parent = nullptr;
	}

	Node::Node(unsigned minBranch, unsigned maxBranch, Node *p)
	{
		minBranchFactor = minBranch;
		maxBranchFactor = maxBranch;
		parent = p;
	}

	void Node::deleteSubtrees()
	{
		DPRINT1("deleteSubtrees");

		if (branches.size() == 0)
		{
			return;
		}
		else
		{
			for (unsigned i = 0; i < branches.size(); ++i)
			{
				branches[i].child->deleteSubtrees();
				delete branches[i].child;
			}
		}

		DPRINT1("deleteSubtrees finished");
	}

	Rectangle Node::boundingBox()
	{
		DPRINT1("boundingBox");

		Rectangle bb = Rectangle();

		DPRINT2("data.size() == ", data.size());
		DPRINT2("branches.size() == ", branches.size());

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
			bb = branches[0].boundingPoly.boundingBox();
			for (unsigned i = 1; i < branches.size(); ++i)
			{
				bb.expand(branches[i].boundingPoly.boundingBox());
			}
		}

		DPRINT1("boundingBox finished");
		return bb;
	}

	Node::Branch Node::locateBranch(Node *child)
	{
		for (unsigned i = 0; i < branches.size(); ++i)
		{
			if (branches[i].child == child)
			{
				return branches[i];
			}
		}

		// If we are here panic
		assert(false);
		return {nullptr, IsotheticPolygon()};
	}

	void Node::updateBranch(Node *child, IsotheticPolygon &boundingPoly)
	{
		DPRINT1("updateBranch");

		// Locate the child
		unsigned branchesSize = branches.size();
		unsigned childIndex;
		for (childIndex = 0; branches[childIndex].child != child && childIndex < branchesSize; ++childIndex) {}

		// Update the child
		branches[childIndex] = {child, boundingPoly};

		DPRINT1("updateBranch finished");
	}

	void Node::removeBranch(Node *child)
	{
		DPRINT1("removeBranch");

		// Locate the child
		unsigned branchesSize = branches.size();
		unsigned childIndex;
		for (childIndex = 0; branches[childIndex].child != child && childIndex < branchesSize; ++childIndex) {}

		// Delete the child deleting it and overwriting its branch
		DPRINT1("wat");
		delete child;
		DPRINT1("deleted child pointer");
		branches[childIndex] = branches.back();
		DPRINT1("swapppped branch with last branch in the vector");
		branches.pop_back();
		DPRINT1("poppppped last branch in vector");

		DPRINT1("removeBranch finished");
	}

	void Node::removeData(Point givenPoint)
	{
		DPRINT1("removeData");

		// Locate the point
		unsigned dataSize = data.size();
		unsigned pointIndex;
		for (pointIndex = 0; data[pointIndex] != givenPoint && pointIndex < dataSize; ++pointIndex) {}

		// Delete the point by overwriting it
		data[pointIndex] = data.back();
		data.pop_back();

		DPRINT1("removeData finished");
	}

	void Node::exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator)
	{
		DPRINT1("exhaustiveSearch");

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

		DPRINT1("exhaustiveSearch finished");
	}

	std::vector<Point> Node::search(Point &requestedPoint)
	{
		DPRINT1("search point");

		std::vector<Point> accumulator;

		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;
		STATEXEC(unsigned branchesSearched = 0);

		for (;!context.empty();)
		{
			DPRINT1("setting next context");
			currentContext = context.top();
			context.pop();

			unsigned branchesSize = currentContext->branches.size();
			unsigned dataSize = currentContext->data.size();

			if (branchesSize == 0)
			{
				DPRINT3("searching ", dataSize, "data points in leaf");
				// We are a leaf so add our data points when they are the search point
				for (unsigned i = 0; i < dataSize; ++i)
				{
					if (requestedPoint == currentContext->data[i])
					{
						accumulator.push_back(currentContext->data[i]);
					}
				}
			}
			else
			{
				DPRINT3("searching ", branchesSize, " branches in routing node");
				// Determine which branches we need to follow
				for (unsigned i = 0; i < branchesSize; ++i)
				{
					if (currentContext->branches[i].boundingPoly.containsPoint(requestedPoint))
					{
						// Add to the nodes we will check
						context.push(currentContext->branches[i].child);
						STATEXEC(++branchesSearched);
					}
				}
			}
			DPRINT1("done with current context");
		}

		STATBRSR(branchesSearched);

		DPRINT1("search point finished");
		return accumulator;
	}

	std::vector<Point> Node::search(Rectangle &requestedRectangle)
	{
		DPRINT1("search rectangle");

		std::vector<Point> accumulator;

		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;
		STATEXEC(unsigned branchesSearched = 0);
		unsigned branchesSize, dataSize;

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();

			branchesSize = currentContext->branches.size();
			dataSize = currentContext->data.size();

			if (branchesSize == 0)
			{
				// We are a leaf so add our data points when they are within the search rectangle
				for (unsigned i = 0; i < dataSize; ++i)
				{
					if (requestedRectangle.containsPoint(currentContext->data[i]))
					{
						accumulator.push_back(currentContext->data[i]);
					}
				}
			}
			else
			{
				// Determine which branches we need to follow
				for (unsigned i = 0; i < branchesSize; ++i)
				{
					if (currentContext->branches[i].boundingPoly.intersectsRectangle(requestedRectangle))
					{
						// Add to the nodes we will check
						context.push(currentContext->branches[i].child);
						STATEXEC(++branchesSearched);
					}
				}
			}
		}

		// STATBRSR(branchesSearched);

		DPRINT1("search rectangle finished");
		return accumulator;
	}

	// Always called on root, this = root
	// This top-to-bottom sweep is only for adjusting bounding boxes to contain the point and
	// choosing a particular leaf
	Node *Node::chooseNode(Point givenPoint)
	{
		DPRINT1("chooseNode");
		// CL1 [Initialize]
		Node *context = this;
		unsigned enclosingPolyIndex = 0;
		unsigned branchesSize = 0;

		for (;;)
		{
			branchesSize = context->branches.size();

			// CL2 [Leaf check]
			if (branchesSize == 0)
			{
				DPRINT2("chose ", (void *)context);
				DPRINT1("chooseNode finished");
				return context;
			}
			else
			{
				// Compute the smallest expansion
				DPRINT1("computing smallest expansion");
				unsigned smallestExpansionIndex = 0;
				float smallestExpansionArea = context->branches[0].boundingPoly.computeExpansionArea(givenPoint);
				float expansionArea;
				for (unsigned i = 1; i < branchesSize; ++i)
				{
					expansionArea = context->branches[i].boundingPoly.computeExpansionArea(givenPoint);
					if (expansionArea < smallestExpansionArea)
					{
						smallestExpansionIndex = i;
						smallestExpansionArea = expansionArea;
					}
				}

				DPRINT1("expanding polygon");
				if (context->parent != nullptr)
				{
					context->branches[smallestExpansionIndex].boundingPoly.expand(givenPoint, context->parent->branches[enclosingPolyIndex].boundingPoly);
				}
				else
				{
					context->branches[smallestExpansionIndex].boundingPoly.expand(givenPoint);
				}

				DPRINT1("trimming back polygon");
				for (unsigned i = 0; i < branchesSize; ++i)
				{
					if (i != smallestExpansionIndex)
					{
						context->branches[smallestExpansionIndex].boundingPoly.increaseResolution(context->branches[i].boundingPoly);
					}
				}

				// Descend
				DPRINT2("context ", (void *)context);
				context = context->branches[smallestExpansionIndex].child;
				enclosingPolyIndex = smallestExpansionIndex;
			}
		}
	}

	Node *Node::findLeaf(Point givenPoint)
	{
		DPRINT2("Given point = ", givenPoint);

		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;
		unsigned branchesSize, dataSize;

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();
			DPRINT2("Current find leaf context = ", (void *)currentContext);

			branchesSize = currentContext->branches.size();
			dataSize = currentContext->data.size();

			if (branchesSize == 0)
			{
				DPRINT2("FL2 size = ", currentContext->branches.size());
				// FL2 [Search leaf node for record]
				// Check each entry to see if it matches E
				for (unsigned i = 0; i < dataSize; ++i)
				{
					if (currentContext->data[i] == givenPoint)
					{
						return currentContext;
					}
				}
			}
			else
			{
				DPRINT2("FL1 size = ", branchesSize);
				// FL1 [Search subtrees]
				// Determine which branches we need to follow
				for (unsigned i = 0; i < branchesSize; ++i)
				{
					if (currentContext->branches[i].boundingPoly.containsPoint(givenPoint))
					{
						DPRINT2("Pushing ", (void *)currentContext->branches[i].child);
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
		DPRINT1("partitionNode");

		if (branches.size() == 0)
		{
			nirtree::Node::Partition defaultPartition;
			float leastTotalArea = std::numeric_limits<float>::infinity();
			float combinedArea = 0.0;

			for (unsigned d = 0; d < dimensions; ++d)
			{
				// Sort along dimension d
				std::sort(data.begin(), data.end(), [d](Point a, Point b){return a[d] < b[d];});

				// Compute size of left and right rectangles
				Rectangle left(data[0], data[0]);
				Rectangle right(data[data.size() / 2], data[data.size() / 2]);

				// Expand left
				for (unsigned i = 0; i < data.size() / 2; ++i)
				{
					left.expand(data[i]);
				}

				// Expand right
				for (unsigned i = data.size() / 2; i < data.size(); ++i)
				{
					right.expand(data[i]);
				}

				combinedArea = left.area() + right.area();

				if (combinedArea < leastTotalArea)
				{
					leastTotalArea = combinedArea;
					defaultPartition.dimension = d;
					defaultPartition.location = data[data.size() / 2][d];
				}
			}

			DPRINT5("Partition {", defaultPartition.dimension, ", ", defaultPartition.location, "}");
			DPRINT1("partitionNode finished");
			return defaultPartition;
		}
		else
		{
			std::vector<Rectangle> sortableBoundingBoxes;

			for (Branch b : branches)
			{
				sortableBoundingBoxes.insert(sortableBoundingBoxes.end(), b.boundingPoly.basicRectangles.begin(), b.boundingPoly.basicRectangles.end());
			}

			nirtree::Node::Partition defaultPartition;
			unsigned inducedSplits = std::numeric_limits<unsigned>::max();

			for (unsigned d = 0; d < dimensions; ++d)
			{
				// Sort along d
				std::sort(sortableBoundingBoxes.begin(), sortableBoundingBoxes.end(), [d](Rectangle a, Rectangle b){return a.lowerLeft[d] < b.lowerLeft[d];});

				// Pick split along d
				float locationD = sortableBoundingBoxes[sortableBoundingBoxes.size() / 2 + 1].lowerLeft[d];

				// Compute # of splits if d is chosen
				unsigned inducedSplitsD = 0;

				for (unsigned i = 0; i < sortableBoundingBoxes.size(); ++i)
				{
					DPRINT4("sortableBoundingBoxes[", i, "] = ", sortableBoundingBoxes[i]);
					if (sortableBoundingBoxes[i].lowerLeft[d] < locationD && locationD < sortableBoundingBoxes[i].upperRight[d])
					{
						inducedSplitsD++;
					}
				}

				// Decide if defaultPartition should be updated
				if (inducedSplitsD < inducedSplits)
				{
					defaultPartition.dimension = d;
					defaultPartition.location = locationD;
					inducedSplits = inducedSplitsD;
				}
			}

			return defaultPartition;
		}
	}

	// Splitting a node will remove it from its parent node and its memory will be freed
	Node::SplitResult Node::splitNode(Partition p)
	{
		DPRINT1("splitNode partition");
		DPRINT4("minBranchFactor = ", minBranchFactor, " maxBranchFactor = ", maxBranchFactor);
		DPRINT4("branches.size() = ", branches.size(), " data.size() = ", data.size());
		IsotheticPolygon referencePoly;
		if (parent != nullptr)
		{
			referencePoly = parent->locateBranch(this).boundingPoly;
		}
		else
		{
			referencePoly = IsotheticPolygon(boundingBox());
		}

		SplitResult split = {{new Node(minBranchFactor, maxBranchFactor, parent), referencePoly}, {new Node(minBranchFactor, maxBranchFactor, parent), referencePoly}};
		unsigned branchesSize = branches.size();
		unsigned dataSize = data.size();

		if (branchesSize == 0)
		{
			DPRINT1("leaf split");
			DPRINT4("partition line along", p.dimension, " = ", p.location);
			for (unsigned i = 0; i < dataSize; ++i)
			{
				DPRINT4("data[", i, "] = ", data[i]);
				if (data[i][p.dimension] < p.location)
				{
					split.leftBranch.child->data.push_back(data[i]);
				}
				else
				{
					split.rightBranch.child->data.push_back(data[i]);
				}
			}
			data.clear();
			DPRINT5("Left has ", split.leftBranch.child->branches.size(), " branches, ", split.leftBranch.child->data.size(), " data points");
			DPRINT5("Right has ", split.rightBranch.child->branches.size(), " branches, ", split.rightBranch.child->data.size(), " data points");
			DPRINT4("left side = ", (void *)split.leftBranch.child, " right side = ", (void *)split.rightBranch.child);
		}
		else
		{
			DPRINT1("routing split");
			DPRINT3("splitting along ", p.dimension, " dimension");
			for (unsigned i = 0; i < branchesSize; ++i)
			{
				DPRINT4("branches[", i, "].boundingPoly = ", branches[i].boundingPoly);
				if (branches[i].boundingPoly.max(p.dimension) <= p.location)
				{
					DPRINT1("placed left");
					branches[i].child->parent = split.leftBranch.child;
					split.leftBranch.child->branches.push_back(branches[i]);
				}
				else if (branches[i].boundingPoly.min(p.dimension) >= p.location)
				{
					DPRINT1("placed right");
					branches[i].child->parent = split.rightBranch.child;
					split.rightBranch.child->branches.push_back(branches[i]);
				}
				else
				{
					DPRINT1("must split before placing");
					Node::SplitResult downwardSplit = branches[i].child->splitNode(p);

					DPRINT1("cleaning up recursively split child");
					delete branches[i].child;

					DPRINT1("placing left");
					downwardSplit.leftBranch.child->parent = split.leftBranch.child;
					split.leftBranch.child->branches.push_back(downwardSplit.leftBranch);

					DPRINT1("placing right");
					downwardSplit.rightBranch.child->parent = split.rightBranch.child;
					split.rightBranch.child->branches.push_back(downwardSplit.rightBranch);
				}
			}
			branches.clear();

			split.leftBranch.boundingPoly.maxLimit(p.dimension, p.location);
			split.rightBranch.boundingPoly.minLimit(p.dimension, p.location);

			DPRINT5("Left has ", split.leftBranch.child->branches.size(), " branches, ", split.leftBranch.child->data.size(), " data points");
			DPRINT5("Right has ", split.rightBranch.child->branches.size(), " branches, ", split.rightBranch.child->data.size(), " data points");
			DPRINT4("left side = ", (void *)split.leftBranch.child, " right side = ", (void *)split.rightBranch.child);
		}

		for (unsigned i = 0; i < split.leftBranch.child->branches.size(); ++i)
		{
			DASSERT(split.leftBranch.child->branches[i].child->parent == split.leftBranch.child);
		}

		for (unsigned i = 0; i < split.rightBranch.child->branches.size(); ++i)
		{
			DASSERT(split.rightBranch.child->branches[i].child->parent == split.rightBranch.child);
		}

		DASSERT(split.rightBranch.child->parent == parent);
		DASSERT(split.rightBranch.child->parent == parent);
		DASSERT(split.rightBranch.child->data.size() <= split.rightBranch.child->maxBranchFactor);
		DASSERT(split.rightBranch.child->data.size() <= split.rightBranch.child->maxBranchFactor);
		DPRINT5("Left has ", split.rightBranch.child->branches.size(), " branches, ", split.rightBranch.child->data.size(), " data points");
		DPRINT5("Right has ", split.rightBranch.child->branches.size(), " branches, ", split.rightBranch.child->data.size(), " data points");
		DPRINT4("left side = ", (void *)split.rightBranch.child, " right side = ", (void *)split.rightBranch.child);
		DPRINT1("splitNode partition finished");
		return split;
	}

	// Splitting a node will remove it from its parent node and its memory will be freed
	Node::SplitResult Node::splitNode()
	{
		DPRINT1("splitNode");

		Node::SplitResult returnSplit = splitNode(partitionNode());

		DPRINT4("left side = ", (void *)returnSplit.leftBranch.child, " right side = ", (void *)returnSplit.rightBranch.child);
		DPRINT5("Left has ", returnSplit.leftBranch.child->branches.size(), " branches, ", returnSplit.leftBranch.child->data.size(), " data points");
		DPRINT5("Right has ", returnSplit.rightBranch.child->branches.size(), " branches, ", returnSplit.rightBranch.child->data.size(), " data points");
		DPRINT1("splitNode finished");
		return returnSplit;
	}

	// This bottom-to-top sweep is only for splitting bounding boxes as necessary
	Node::SplitResult Node::adjustTree()
	{
		DPRINT1("adjustTree");
		Node *currentContext = this;
		unsigned branchesSize, dataSize;
		Node::SplitResult propagationSplit = {{nullptr, IsotheticPolygon()}, {nullptr, IsotheticPolygon()}};

		for (;currentContext != nullptr;)
		{
			branchesSize = currentContext->branches.size();
			dataSize = currentContext->data.size();

			// If there was a split we were supposed to propagate then propagate it
			if (propagationSplit.leftBranch.child != nullptr && propagationSplit.rightBranch.child != nullptr)
			{
				DPRINT1("Propagating split");
				currentContext->branches.push_back(propagationSplit.leftBranch);
				currentContext->branches.push_back(propagationSplit.rightBranch);
			}

			// If there are too many branches or too much data at this level then create a split to
			// be propagated otherwise we may finish and return a non-split
			if (dataSize > currentContext->maxBranchFactor || branchesSize > currentContext->maxBranchFactor)
			{
				DPRINT6("maxBranchFactor = ", currentContext->maxBranchFactor, ", data.size() = ", dataSize, ", branches.size() = ", branchesSize);
				propagationSplit = currentContext->splitNode();
				DASSERT(propagationSplit.leftBranch.child->parent == currentContext->parent);
				DASSERT(propagationSplit.rightBranch.child->parent == currentContext->parent);
				if (currentContext->parent != nullptr)
				{
					currentContext->parent->removeBranch(currentContext);
				}
				DPRINT4("left side = ", (void *)propagationSplit.leftBranch.child, " right side = ", (void *)propagationSplit.rightBranch.child);
				DPRINT5("Left has ", propagationSplit.leftBranch.child->branches.size(), " branches, ", propagationSplit.leftBranch.child->data.size(), " data points");
				DPRINT5("Right has ", propagationSplit.rightBranch.child->branches.size(), " branches, ", propagationSplit.rightBranch.child->data.size(), " data points");
				DASSERT(propagationSplit.leftBranch.child->branches.size() > 0 || propagationSplit.leftBranch.child->data.size() > 0);
				DASSERT(propagationSplit.rightBranch.child->branches.size() > 0 || propagationSplit.rightBranch.child->data.size() > 0);
			}
			else
			{
				DPRINT1("no split to propagate reseting and breaking");
				propagationSplit = {{nullptr, IsotheticPolygon()}, {nullptr, IsotheticPolygon()}};
				break;
			}

			// Ascend
			currentContext = propagationSplit.leftBranch.child->parent;
			DPRINT2("ascending to ", (void *)currentContext);
		}

		DPRINT1("adjustTree finished");
		return propagationSplit;
	}

	void Node::pushDown(Point givenPoint)
	{
		DPRINT1("pushDown");
		DASSERT(branches.size() > 1);

		Node *pacerChild = branches.front().child;
		Node *pushChild = branches.back().child;

		for (; pacerChild->branches.size() > 0; pacerChild = pacerChild->branches.front().child)
		{
			DPRINT4("minBranchFactor = ", minBranchFactor, " maxBranchFactor = ", maxBranchFactor);
			pushChild->branches.push_back({new Node(minBranchFactor, maxBranchFactor, pushChild), IsotheticPolygon(Rectangle(givenPoint, givenPoint))});
			pushChild = pushChild->branches.front().child;
		}

		pushChild->data.push_back(givenPoint);
		DPRINT1("pushDown finished");
	}

	// Always called on root, this = root
	Node *Node::insert(Point givenPoint)
	{
		DPRINT1("insert");
		DPRINT2("Inserting ", givenPoint);

		// Find the appropriate position for the new point
		Node *adjustContext = chooseNode(givenPoint);

		// Adjust the tree
		if (adjustContext->branches.size() == 0)
		{
			DPRINT2("adding data ", givenPoint);
			// Add just the data
			adjustContext->data.push_back(givenPoint);
		}
		else
		{
			DPRINT2("pushing down data ", givenPoint);
			// Add a whole new branch
			adjustContext->pushDown(givenPoint);
		}
		// There is no guarantee that the root will still exist after adjustment so backup branch factors
		unsigned backupMinBranchFactor = minBranchFactor;
		unsigned backupMaxBranchFactor = maxBranchFactor;
		Node::SplitResult finalSplit = adjustContext->adjustTree();

		// Grow the tree taller if we need to
		if (finalSplit.leftBranch.child != nullptr && finalSplit.rightBranch.child != nullptr)
		{
			DPRINT1("CHKPT4");
			DPRINT5("Left has ", finalSplit.leftBranch.child->branches.size(), " branches, ", finalSplit.leftBranch.child->data.size(), " data points");
			DPRINT5("Right has ", finalSplit.rightBranch.child->branches.size(), " branches, ", finalSplit.rightBranch.child->data.size(), " data points");
			DPRINT4("minBranchFactor = ", minBranchFactor, " maxBranchFactor = ", maxBranchFactor);
			Node *newRoot = new Node(backupMinBranchFactor, backupMaxBranchFactor, nullptr);
			DPRINT9("finalSplit = {{", (void *)finalSplit.leftBranch.child, ", ", finalSplit.leftBranch.boundingPoly, "}, {", (void *)finalSplit.rightBranch.child, ", ", finalSplit.rightBranch.boundingPoly, "}}");
			DPRINT5("Left has ", finalSplit.leftBranch.child->branches.size(), " branches, ", finalSplit.leftBranch.child->data.size(), " data points");
			DPRINT5("Right has ", finalSplit.rightBranch.child->branches.size(), " branches, ", finalSplit.rightBranch.child->data.size(), " data points");
			DASSERT(finalSplit.leftBranch.child->branches.size() > 0 || finalSplit.leftBranch.child->data.size() > 0);
			DASSERT(finalSplit.rightBranch.child->branches.size() > 0 || finalSplit.rightBranch.child->data.size() > 0);

			finalSplit.leftBranch.child->parent = newRoot;
			newRoot->branches.push_back(finalSplit.leftBranch);
			DPRINT1("CHKPT5");
			finalSplit.rightBranch.child->parent = newRoot;
			DPRINT1("CHKPT6");
			newRoot->branches.push_back(finalSplit.rightBranch);
			DPRINT1("I4a complete");

			DPRINT1("cleanup old root");
			delete this;

			DPRINT1("insert finished");
			return newRoot;
		}

		DPRINT1("insert finished");
		return this;
	}

	// To be called on a leaf
	void Node::condenseTree()
	{
		DPRINT1("condenseTree");
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
			}

			previousContext = currentContext;
		}

		DPRINT1("condenseTree finish");
	}

	// Always called on root, this = root
	Node *Node::remove(Point givenPoint)
	{
		DPRINT1("remove");

		// D1 [Find node containing record]
		Node *leaf = findLeaf(givenPoint);

		DPRINT1("CHKPT1");
		// Record not in the tree
		if (leaf == nullptr)
		{
			DEXEC(this->printTree());
			DPRINT1("Leaf was nullptr");
			return this;
		}

		DPRINT1("CHKPT2");

		// D2 [Delete record]
		leaf->removeData(givenPoint);
		DPRINT1("CHKPT3");

		// D3 [Propagate changes]
		leaf->condenseTree();
		DPRINT1("CHKPT4");

		// D4 [Shorten tree]
		if (branches.size() == 1)
		{
			DPRINT1("CHKPT5");
			Node *newRoot = branches[0].child;
			delete this;
			newRoot->parent = nullptr;
			DPRINT1("remove finished");
			return newRoot;
		}

		DPRINT1("remove finished");
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
		DPRINT4("validating ", (void *)this, " parent = ", (void *)parent);
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
				if (!parent->branches[index].boundingPoly.containsPoint(data[i]))
				{
					std::cout << parent->branches[index].boundingPoly << " fails to contain " << data[i] << std::endl;
					assert(parent->branches[index].boundingPoly.containsPoint(data[i]));
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
			std::cout << indendtation << "		" << branches[i].boundingPoly << std::endl;
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
		DPRINT1("height");
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

		DPRINT1("height finished");
	}

	void Node::stat()
	{
		STATHEIGHT(height());

		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;
		size_t memoryFootprint = 0;
		unsigned branchesSize = 0;
		unsigned singularBranches = 0;

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();

			if (currentContext->branches.size() == 0)
			{
				memoryFootprint += sizeof(Node) + currentContext->data.size() * sizeof(Point);
			}
			else
			{
				STATBRANCH(currentContext->branches.size());
				branchesSize += currentContext->branches.size();
				memoryFootprint += sizeof(Node) + currentContext->branches.size() * sizeof(Node::Branch);
				for (unsigned i = 0; i < currentContext->branches.size(); ++i)
				{
					if (currentContext->branches[i].child->branches.size() == 1 || currentContext->branches[i].child->data.size() == 1)
					{
						singularBranches++;
					}
					context.push(currentContext->branches[i].child);
				}
			}
		}

		STATSIZE(branchesSize);
		STATSIZE(singularBranches);
		STATMEM(memoryFootprint);
	}
}
