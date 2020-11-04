#include <rplustree/node.h>

namespace rplustree
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
		DPRINT2("this = ", (void *)this);
		DPRINT2("branches.size() = ", branches.size());

		if (branches.size() == 0)
		{
			DPRINT1("deleteSubtrees leaf finished");
			return;
		}
		else
		{
			unsigned branchesSize = branches.size();
			for (unsigned i = 0; i < branchesSize; ++i)
			{
				branches[i].child->deleteSubtrees();
				DPRINT1("deleting branch child");
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
			bb = branches[0].boundingBox;
			for (unsigned i = 1; i < branches.size(); ++i)
			{
				bb.expand(branches[i].boundingBox);
			}
		}

		DPRINT1("boundingBox finished");
		return bb;
	}

	void Node::updateBranch(Node *child, Rectangle &boundingBox)
	{
		DPRINT1("updateBranch");

		// Locate the child
		unsigned childIndex;
		for (childIndex = 0; branches[childIndex].child != child && childIndex < branches.size(); ++childIndex) {}

		// Update the child
		branches[childIndex] = {child, boundingBox};

		DPRINT1("updateBranch finished");
	}

	void Node::removeBranch(Node *child)
	{
		DPRINT1("removeBranch");

		// Locate the child
		unsigned childIndex;
		for (childIndex = 0; branches[childIndex].child != child && childIndex < branches.size(); ++childIndex) {}

		// Delete the child deleting it and overwriting its branch
		delete child;
		branches[childIndex] = branches.back();
		branches.pop_back();

		DPRINT1("removeBranch finished");
	}

	void Node::removeData(Point givenPoint)
	{
		DPRINT1("removeData");

		// Locate the point
		unsigned pointIndex;
		for (pointIndex = 0; data[pointIndex] != givenPoint && pointIndex < data.size(); ++pointIndex) {}

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

		std::vector<Point> matchingPoints;

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

			if (currentContext->branches.size() == 0)
			{
				DPRINT3("searching ", currentContext->data.size(), "data points in leaf");
				// We are a leaf so add our data points when they are the search point
				for (unsigned i = 0; i < currentContext->data.size(); ++i)
				{
					if (requestedPoint == currentContext->data[i])
					{
						matchingPoints.push_back(currentContext->data[i]);
					}
				}
			}
			else
			{
				DPRINT3("searching ", currentContext->branches.size(), " branches in routing node");
				// Determine which branches we need to follow
				for (unsigned i = 0; i < currentContext->branches.size(); ++i)
				{
					DPRINT4(currentContext->branches[i].boundingBox, " contains ", requestedPoint, "?");
					if (currentContext->branches[i].boundingBox.containsPoint(requestedPoint))
					{
						DPRINT1("yes");
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
		return matchingPoints;
	}

	std::vector<Point> Node::search(Rectangle &requestedRectangle)
	{
		DPRINT1("search rectangle");

		std::vector<Point> matchingPoints;

		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;
		STATEXEC(unsigned branchesSearched = 0);

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
						STATEXEC(++branchesSearched);
					}
				}
			}
		}

		STATBRSR(branchesSearched);

		DPRINT1("search rectangle finished");
		return matchingPoints;
	}

	// Always called on root, this = root
	// This top-to-bottom sweep is only for adjusting bounding boxes to contain the point and
	// choosing a particular leaf
	Node *Node::chooseNode(Point givenPoint)
	{
		DPRINT1("chooseNode");
		// CL1 [Initialize]
		Node *context = this;

		for (;;)
		{

			// CL2 [Leaf check]
			if (context->branches.size() == 0)
			{
				DPRINT2("chose ", (void *)context);
				DPRINT1("chooseNode finished");
				return context;
			}
			else
			{
				// Compute the smallest expansion
				unsigned smallestExpansionIndex = 0;
				float smallestExpansionArea = context->branches[0].boundingBox.computeExpansionArea(givenPoint);
				float expansionArea;
				DPRINT2("smallestExpansionArea = ", smallestExpansionArea);
				for (unsigned i = 1; i < context->branches.size(); ++i)
				{
					expansionArea = context->branches[i].boundingBox.computeExpansionArea(givenPoint);
					DPRINT2("expansionArea = ", expansionArea);
					if (expansionArea < smallestExpansionArea)
					{
						smallestExpansionIndex = i;
						smallestExpansionArea = expansionArea;
					}
				}

				context->branches[smallestExpansionIndex].boundingBox.expand(givenPoint);

				// Descend
				DPRINT2("context ", (void *)context);
				context = context->branches[smallestExpansionIndex].child;
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

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();
			DPRINT2("Current find leaf context = ", (void *)currentContext);

			if (currentContext->branches.size() == 0)
			{
				DPRINT2("FL2 size = ", currentContext->branches.size());
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
				DPRINT2("FL1 size = ", currentContext->branches.size());
				// FL1 [Search subtrees]
				// Determine which branches we need to follow
				for (unsigned i = 0; i < currentContext->branches.size(); ++i)
				{
					if (currentContext->branches[i].boundingBox.containsPoint(givenPoint))
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
			std::vector<float> values;

			for (Point p : data)
			{
				values.push_back(p.x);
			}

			// Sort along x
			std::sort(values.begin(), values.end());

			// Pick split along x
			float location = values[values.size() / 2];

			DPRINT3("Partition {0, ", location, "}");
			DPRINT1("partitionNode finished");
			return {0, location};
		}
		else
		{
			std::vector<Rectangle> sortableBoundingBoxes;

			for (Branch b : branches)
			{
				sortableBoundingBoxes.push_back(b.boundingBox);
			}

			DASSERT(sortableBoundingBoxes.size() == branches.size());

			// Sort along x
			std::sort(sortableBoundingBoxes.begin(), sortableBoundingBoxes.end(), [](Rectangle a, Rectangle b){return a.lowerLeft.x < b.lowerLeft.x;});

			// Pick split along x
			float locationX = sortableBoundingBoxes[sortableBoundingBoxes.size() / 2 + 1].lowerLeft.x;

			// Compute # of splits if x is chosen
			unsigned inducedSplitsX = 0;

			for (unsigned i = 0; i < sortableBoundingBoxes.size(); ++i)
			{
				DPRINT4("sortableBoundingBoxes[", i, "] = ", sortableBoundingBoxes[i]);
				if (sortableBoundingBoxes[i].lowerLeft.x < locationX && locationX < sortableBoundingBoxes[i].upperRight.x)
				{
					inducedSplitsX++;
				}
			}

			// Sort along y
			std::sort(sortableBoundingBoxes.begin(), sortableBoundingBoxes.end(), [](Rectangle a, Rectangle b){return a.lowerLeft.y < b.lowerLeft.y;});

			// Pick split along y
			float locationY = sortableBoundingBoxes[sortableBoundingBoxes.size() / 2 + 1].lowerLeft.y;

			// Compute # of splits if y is chosen
			unsigned inducedSplitsY = 0;

			for (unsigned i = 0; i < sortableBoundingBoxes.size(); ++i)
			{
				DPRINT4("sortableBoundingBoxes[", i, "] = ", sortableBoundingBoxes[i]);
				if (sortableBoundingBoxes[i].lowerLeft.y < locationY && locationY < sortableBoundingBoxes[i].upperRight.y)
				{
					inducedSplitsY++;
				}
			}

			if (inducedSplitsY < inducedSplitsX)
			{
				DPRINT3("Partition {1, ", locationY, "}");
				DPRINT1("partitionNode finished");
				return {1, locationY};
			}
			else
			{
				DPRINT3("Partition {1, ", locationX, "}");
				DPRINT1("partitionNode finished");
				return {0, locationX};
			}
		}
	}

	// Splitting a node will remove it from its parent node and its memory will be freed
	Node::SplitResult Node::splitNode(Partition p)
	{
		DPRINT1("splitNode partition");
		DPRINT4("minBranchFactor = ", minBranchFactor, " maxBranchFactor = ", maxBranchFactor);
		DPRINT4("branches.size() = ", branches.size(), " data.size() = ", data.size());
		Node *left = new Node(minBranchFactor, maxBranchFactor, parent);
		Node *right = new Node(minBranchFactor, maxBranchFactor, parent);

		if (branches.size() == 0)
		{
			DPRINT1("leaf split");
			if (p.dimension == 0)
			{
				DPRINT2("partition line x = ", p.location);
				for (unsigned i = 0; i < data.size(); ++i)
				{
					DPRINT4("data[", i, "] = ", data[i]);
					if (data[i].x < p.location)
					{
						left->data.push_back(data[i]);
					}
					else
					{
						right->data.push_back(data[i]);
					}
				}
			}
			else if (p.dimension == 1)
			{
				DPRINT2("partition line y = ", p.location);
				for (unsigned i = 0; i < data.size(); ++i)
				{
					DPRINT4("data[", i, "] = ", data[i]);
					if (data[i].y < p.location)
					{
						left->data.push_back(data[i]);
					}
					else
					{
						right->data.push_back(data[i]);
					}
				}
			}
			DPRINT5("Left has ", left->branches.size(), " branches, ", left->data.size(), " data points");
			DPRINT5("Right has ", right->branches.size(), " branches, ", right->data.size(), " data points");
			DPRINT4("left side = ", (void *)left, " right side = ", (void *)right);
		}
		else
		{
			DPRINT1("routing split");
			if (p.dimension == 0)
			{
				DPRINT1("splitting along x");
				unsigned branchesSize = branches.size();
				for (unsigned i = 0; i < branchesSize;)
				{
					DPRINT4("branches[", i, "].boundingBox = ", branches[i].boundingBox);
					if (branches[i].boundingBox.upperRight.x <= p.location)
					{
						DPRINT1("placed left");
						branches[i].child->parent = left;
						left->branches.push_back(branches[i]);
						++i;
					}
					else if (branches[i].boundingBox.lowerLeft.x >= p.location)
					{
						DPRINT1("placed right");
						branches[i].child->parent = right;
						right->branches.push_back(branches[i]);
						++i;
					}
					else
					{
						DPRINT1("must split before placing");
						Node::SplitResult downwardSplit = branches[i].child->splitNode(p);

						DPRINT1("placing left");
						downwardSplit.leftBranch.child->parent = left;
						left->branches.push_back(downwardSplit.leftBranch);

						DPRINT1("placing right");
						downwardSplit.rightBranch.child->parent = right;
						right->branches.push_back(downwardSplit.rightBranch);

						// Do not advance since we have removed branches[i] and the vector is smaller
						branchesSize--;
					}
				}
			}
			else if (p.dimension == 1)
			{
				DPRINT1("splitting along y");
				unsigned branchesSize = branches.size();
				for (unsigned i = 0; i < branchesSize;)
				{
					DPRINT4("branches[", i, "].boundingBox = ", branches[i].boundingBox);
					if (branches[i].boundingBox.upperRight.y <= p.location)
					{
						DPRINT1("placed left");
						branches[i].child->parent = left;
						left->branches.push_back(branches[i]);
						++i;
					}
					else if (branches[i].boundingBox.lowerLeft.y >= p.location)
					{
						DPRINT1("placed right");
						branches[i].child->parent = right;
						right->branches.push_back(branches[i]);
						++i;
					}
					else
					{
						DPRINT1("must split before placing");
						Node::SplitResult downwardSplit = branches[i].child->splitNode(p);

						DPRINT1("placing left");
						downwardSplit.leftBranch.child->parent = left;
						left->branches.push_back(downwardSplit.leftBranch);

						DPRINT1("placing right");
						downwardSplit.rightBranch.child->parent = right;
						right->branches.push_back(downwardSplit.rightBranch);

						// Do not advance since we have removed branches[i] and the vector is smaller
						branchesSize--;
					}
				}
			}
		}

		for (unsigned i = 0; i < left->branches.size(); ++i)
		{
			DASSERT(left->branches[i].child->parent == left);
		}

		for (unsigned i = 0; i < right->branches.size(); ++i)
		{
			DASSERT(right->branches[i].child->parent == right);
		}

		DASSERT(left->parent == parent);
		DASSERT(right->parent == parent);

		// This node is split and no longer logically exists so clean it up
		if (parent == nullptr)
		{
			// Root special case
			DPRINT2("deleting ", (void *)this);
			delete this;
		}
		else
		{
			// Regular node case
			parent->removeBranch(this);
		}

		DASSERT(left->data.size() <= left->maxBranchFactor);
		DASSERT(right->data.size() <= right->maxBranchFactor);
		DPRINT5("Left has ", left->branches.size(), " branches, ", left->data.size(), " data points");
		DPRINT5("Right has ", right->branches.size(), " branches, ", right->data.size(), " data points");
		DPRINT4("left side = ", (void *)left, " right side = ", (void *)right);
		DPRINT1("splitNode partition finished");
		return {{left, left->boundingBox()}, {right, right->boundingBox()}};
	}

	// Splitting a node will remove it from its parent node and its memory will be freed
	Node::SplitResult Node::splitNode()
	{
		DPRINT1("splitNode");
		// STATSPLIT();

		Partition p = partitionNode();

		Node::SplitResult returnSplit = splitNode(p);

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
		Node::SplitResult propagationSplit = {{nullptr, Rectangle()}, {nullptr, Rectangle()}};

		for (;currentContext != nullptr;)
		{
			// If there was a split we were supposed to propagate then propagate it
			if (propagationSplit.leftBranch.child != nullptr && propagationSplit.rightBranch.child != nullptr)
			{
				DPRINT1("Propagating split");
				currentContext->branches.push_back(propagationSplit.leftBranch);
				currentContext->branches.push_back(propagationSplit.rightBranch);
			}

			// If there are too many branches or too much data at this level then create a split to
			// be propagated otherwise we may finish and return a non-split
			if (currentContext->data.size() > currentContext->maxBranchFactor || currentContext->branches.size() > currentContext->maxBranchFactor)
			{
				DPRINT6("maxBranchFactor = ", currentContext->maxBranchFactor, ", data.size() = ", currentContext->data.size(), ", branches.size() = ", currentContext->branches.size());
				propagationSplit = currentContext->splitNode();
				DPRINT4("left side = ", (void *)propagationSplit.leftBranch.child, " right side = ", (void *)propagationSplit.rightBranch.child);
				DPRINT5("Left has ", propagationSplit.leftBranch.child->branches.size(), " branches, ", propagationSplit.leftBranch.child->data.size(), " data points");
				DPRINT5("Right has ", propagationSplit.rightBranch.child->branches.size(), " branches, ", propagationSplit.rightBranch.child->data.size(), " data points");
				DASSERT(propagationSplit.leftBranch.child->branches.size() > 0 || propagationSplit.leftBranch.child->data.size() > 0);
				DASSERT(propagationSplit.rightBranch.child->branches.size() > 0 || propagationSplit.rightBranch.child->data.size() > 0);
			}
			else
			{
				DPRINT1("no split to propagate reseting and breaking");
				propagationSplit = {{nullptr, Rectangle()}, {nullptr, Rectangle()}};
				break;
			}
			
			// Ascend
			currentContext = propagationSplit.leftBranch.child->parent;
			DPRINT2("ascending to ", (void *)currentContext);
		}

		DPRINT1("adjustTree finished");
		return propagationSplit;
	}

	// Always called on root, this = root
	Node *Node::insert(Point givenPoint)
	{
		DPRINT1("insert");

		DPRINT2("Inserting ", givenPoint);

		// Find the appropriate position for the new point
		Node *adjustContext = chooseNode(givenPoint);

		// Adjust the tree
		DPRINT2("adding data ", givenPoint);
		// Add just the data
		adjustContext->data.push_back(givenPoint);

		// There is no guarantee that the root will still exist after adjustment so backup branch factors
		unsigned backupMinBranchFactor = minBranchFactor;
		unsigned backupMaxBranchFactor = maxBranchFactor;
		Node::SplitResult finalSplit = adjustContext->adjustTree();

		// Grow the tree taller if we need to
		if (finalSplit.leftBranch.child != nullptr && finalSplit.rightBranch.child != nullptr)
		{
			DPRINT5("Left has ", finalSplit.leftBranch.child->branches.size(), " branches, ", finalSplit.leftBranch.child->data.size(), " data points");
			DPRINT5("Right has ", finalSplit.rightBranch.child->branches.size(), " branches, ", finalSplit.rightBranch.child->data.size(), " data points");

			DPRINT1("CHKPT4");
			DPRINT5("Left has ", finalSplit.leftBranch.child->branches.size(), " branches, ", finalSplit.leftBranch.child->data.size(), " data points");
			DPRINT5("Right has ", finalSplit.rightBranch.child->branches.size(), " branches, ", finalSplit.rightBranch.child->data.size(), " data points");
			DPRINT4("minBranchFactor = ", minBranchFactor, " maxBranchFactor = ", maxBranchFactor);
			Node *newRoot = new Node(backupMinBranchFactor, backupMaxBranchFactor, nullptr);
			DPRINT9("finalSplit = {{", (void *)finalSplit.leftBranch.child, ", ", finalSplit.leftBranch.boundingBox, "}, {", (void *)finalSplit.rightBranch.child, ", ", finalSplit.rightBranch.boundingBox, "}}");
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
				else
				{
					Rectangle previousBox = previousContext->boundingBox();
					currentContext->updateBranch(previousContext, previousBox);
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
		DPRINT2("leaf data size = ", leaf->data.size());
		leaf->removeData(givenPoint);
		DPRINT2("leaf data size = ", leaf->data.size());
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
				sum += (unsigned)data[i].x;
				sum += (unsigned)data[i].y;
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
