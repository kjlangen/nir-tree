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
	}

	Rectangle Node::boundingBox()
	{
		Rectangle bb;

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
			bb = branches[0].boundingPoly.boundingBox;
			for (unsigned i = 1; i < branches.size(); ++i)
			{
				bb.expand(branches[i].boundingPoly.boundingBox);
			}
		}

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

		// If we are here, panic
		assert(false);

		return {nullptr, IsotheticPolygon()};
	}

	void Node::updateBranch(Node *child, IsotheticPolygon &boundingPoly)
	{
		// Locate the child
		unsigned branchesSize = branches.size();
		unsigned childIndex;
		for (childIndex = 0; branches[childIndex].child != child && childIndex < branchesSize; ++childIndex) {}

		// Update the child
		branches[childIndex] = {child, boundingPoly};
	}

	void Node::removeBranch(Node *child)
	{
		// Locate the child
		unsigned branchesSize = branches.size();
		unsigned childIndex;
		for (childIndex = 0; branches[childIndex].child != child && childIndex < branchesSize; ++childIndex) {}

		// Delete the child deleting it and overwriting its branch
		delete child;
		branches[childIndex] = branches.back();
		branches.pop_back();
	}

	void Node::removeData(Point givenPoint)
	{
		// Locate the point
		unsigned dataSize = data.size();
		unsigned pointIndex;
		for (pointIndex = 0; data[pointIndex] != givenPoint && pointIndex < dataSize; ++pointIndex) {}

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
		std::vector<Point> accumulator;

		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();

			unsigned branchesSize = currentContext->branches.size();
			unsigned dataSize = currentContext->data.size();

			if (branchesSize == 0)
			{
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
				// Determine which branches we need to follow
				for (unsigned i = 0; i < branchesSize; ++i)
				{
					if (currentContext->branches[i].boundingPoly.containsPoint(requestedPoint))
					{
						// Add to the nodes we will check
						context.push(currentContext->branches[i].child);
					}
				}
			}
		}

		return accumulator;
	}

	std::vector<Point> Node::search(Rectangle &requestedRectangle)
	{
		std::vector<Point> accumulator;

		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;
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
					}
				}
			}
		}

		return accumulator;
	}

	// Always called on root, this = root
	// This top-to-bottom sweep is only for adjusting bounding boxes to contain the point and
	// choosing a particular leaf
	Node *Node::chooseNode(Point givenPoint)
	{
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
				return context;
			}
			else
			{
				// Compute the smallest expansion
				unsigned smallestExpansionBranchIndex = 0;
				IsotheticPolygon::OptimalExpansion smallestExpansion = context->branches[0].boundingPoly.computeExpansionArea(givenPoint);
				IsotheticPolygon::OptimalExpansion evalExpansion;
				for (unsigned i = 1; i < branchesSize && smallestExpansion.area != -1.0; ++i)
				{
					// if (context->branches[i].boundingPoly.containsPoint(givenPoint))
					// {
					// 	smallestExpansionBranchIndex = i;
					// 	smallestExpansion = {0, -1.0};
					// 	break;
					// }
					// else 

					if ((evalExpansion = context->branches[i].boundingPoly.computeExpansionArea(givenPoint)).area < smallestExpansion.area)
					{
						smallestExpansionBranchIndex = i;
						smallestExpansion = evalExpansion;
					}
				}

				if (smallestExpansion.area != -1.0)
				{
					IsotheticPolygon subsetPolygon(context->branches[smallestExpansionBranchIndex].boundingPoly.basicRectangles[smallestExpansion.index]);
					subsetPolygon.expand(givenPoint);

					for (unsigned i = 0; i < branchesSize; ++i)
					{
						if (i != smallestExpansionBranchIndex)
						{
							subsetPolygon.increaseResolution(context->branches[i].boundingPoly);
						}
					}


					if (context->parent != nullptr)
					{
						subsetPolygon.intersection(context->parent->branches[enclosingPolyIndex].boundingPoly);
					}

					context->branches[smallestExpansionBranchIndex].boundingPoly.remove(smallestExpansion.index);
					context->branches[smallestExpansionBranchIndex].boundingPoly.merge(subsetPolygon);
				}

				// Descend
				context = context->branches[smallestExpansionBranchIndex].child;
				enclosingPolyIndex = smallestExpansionBranchIndex;
			}
		}
	}

	Node *Node::findLeaf(Point givenPoint)
	{
		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;
		unsigned branchesSize, dataSize;

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();

			branchesSize = currentContext->branches.size();
			dataSize = currentContext->data.size();

			if (branchesSize == 0)
			{
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
				// FL1 [Search subtrees]
				// Determine which branches we need to follow
				for (unsigned i = 0; i < branchesSize; ++i)
				{
					if (currentContext->branches[i].boundingPoly.containsPoint(givenPoint))
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
		nirtree::Node::Partition defaultPartition;
		double totalMass = 0.0;
		unsigned branchesSize = branches.size();
		unsigned costMetric = std::numeric_limits<unsigned>::max();

		if (branchesSize == 0)
		{
			// // Setup variance values
			// Point variance = Point::atOrigin;
			// Point average = Point::atOrigin;
			// Point sumOfSquares = Point::atOrigin;

			// for (Point d : data)
			// {
			// 	average += d;
			// 	sumOfSquares += d * d;
			// 	totalMass += 1.0;
			// }

			// // Compute final terms
			// average /= totalMass;
			// sumOfSquares /= totalMass;

			// // Compute final variance
			// variance = sumOfSquares - average * average;

			// // Choose most variate dimension
			// defaultPartition.dimension = 0;

			// for (unsigned i = 0; i < dimensions; ++i)
			// {
			// 	defaultPartition.dimension = variance[i] > variance[defaultPartition.dimension] ? i : defaultPartition.dimension;
			// }
			// defaultPartition.location = average[defaultPartition.dimension];

			unsigned dataSize = data.size();

			for (unsigned d = 0; d < dimensions; ++d)
			{
				// Sort along dimension d
				std::sort(data.begin(), data.end(), [d](Point a, Point b){return a[d] < b[d];});

				// Compute cost
				unsigned duplicates = 0;
				for (Point dataPoint : data)
				{
					if (dataPoint[d] == data[dataSize / 2 - 1][d])
					{
						++duplicates;
					}
				}

				// Compare cost
				if (duplicates < costMetric)
				{
					defaultPartition.dimension = d;
					defaultPartition.location = data[dataSize / 2 - 1][d];
					costMetric = duplicates;
				}
			}

			return defaultPartition;
		}
		else
		{
			std::vector<double> sortable;
			unsigned bestBalance = branchesSize;
			std::vector<double> deduplicated;

			for (unsigned d = 0; d < dimensions; ++d)
			{
				// Make rightmost limits sortable
				for (unsigned i = 0; i < branchesSize; ++i)
				{
					sortable.push_back(branches[i].boundingPoly.boundingBox.upperRight[d]);
				}

				// Sort along d
				std::sort(sortable.begin(), sortable.end());

				// Deduplicate
				deduplicated.push_back(sortable[0]);
				for (unsigned i = 1; i < branchesSize; ++i)
				{
					if (sortable[i] != sortable[i - 1])
					{
						deduplicated.push_back(sortable[i]);
					}
				}

				// Compute cost
				unsigned currentLeftSize = 0;
				for (Branch b : branches)
				{
					if (b.boundingPoly.boundingBox.upperRight[d] <= deduplicated[deduplicated.size() / 2])
					{
						++currentLeftSize;
					}
				}

				// Compare cost
				unsigned balance = branchesSize / 2 > currentLeftSize ? branchesSize / 2 - currentLeftSize : currentLeftSize - branchesSize / 2;
				if (balance < bestBalance)
				{
					defaultPartition.dimension = d;
					defaultPartition.location = deduplicated[deduplicated.size() / 2];
					bestBalance = balance;
				}

				deduplicated.clear();
				sortable.clear();
			}

			unsigned rightSize = 0;
			for (Branch b : branches)
			{
				if (b.boundingPoly.boundingBox.upperRight[defaultPartition.dimension] <= defaultPartition.location)
				{
					++rightSize;
				}
			}

			if (defaultPartition.dimension > dimensions || rightSize == 0 || rightSize > maxBranchFactor)
			{
				assert(false);
			}

			return defaultPartition;
		}
	}

	// Splitting a node will remove it from its parent node and its memory will be freed
	Node::SplitResult Node::splitNode(Partition p)
	{
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

		if (branchesSize == 0 && dataSize > 0)
		{
			for (unsigned i = 0; i < dataSize; ++i)
			{
				if (data[i][p.dimension] <= p.location && split.leftBranch.child->data.size() < maxBranchFactor)
				{
					split.leftBranch.child->data.push_back(data[i]);
				}
				else
				{
					split.rightBranch.child->data.push_back(data[i]);
				}
			}
			data.clear();
		}
		else
		{
			for (unsigned i = 0; i < branchesSize; ++i)
			{
				if (branches[i].boundingPoly.boundingBox.upperRight[p.dimension] <= p.location)
				{
					branches[i].child->parent = split.leftBranch.child;
					split.leftBranch.child->branches.push_back(branches[i]);
				}
				else if (branches[i].boundingPoly.boundingBox.lowerLeft[p.dimension] >= p.location)
				{
					branches[i].child->parent = split.rightBranch.child;
					split.rightBranch.child->branches.push_back(branches[i]);
				}
				else
				{
					Node::SplitResult downwardSplit = branches[i].child->splitNode(p);

					delete branches[i].child;

					if (downwardSplit.leftBranch.child->data.size() > 0 || downwardSplit.leftBranch.child->branches.size() > 0)
					{
						downwardSplit.leftBranch.child->parent = split.leftBranch.child;
						split.leftBranch.child->branches.push_back(downwardSplit.leftBranch);
					}

					if (downwardSplit.rightBranch.child->data.size() > 0 || downwardSplit.rightBranch.child->branches.size() > 0)
					{
						downwardSplit.rightBranch.child->parent = split.rightBranch.child;
						split.rightBranch.child->branches.push_back(downwardSplit.rightBranch);
					}
				}
			}
			branches.clear();
		}

		split.leftBranch.boundingPoly.maxLimit(p.location, p.dimension);
		split.rightBranch.boundingPoly.minLimit(p.location, p.dimension);

		if (split.leftBranch.child->data.size() > 0)
		{
			for (Point dataPoint : split.leftBranch.child->data)
			{
				DASSERT(split.leftBranch.boundingPoly.containsPoint(dataPoint));
			}
		}

		if (split.rightBranch.child->data.size() > 0)
		{
			for (Point dataPoint : split.rightBranch.child->data)
			{
				DASSERT(split.rightBranch.boundingPoly.containsPoint(dataPoint));
			}
		}

		return split;
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
		Node::SplitResult propagationSplit = {{nullptr, IsotheticPolygon()}, {nullptr, IsotheticPolygon()}};

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

	void Node::pushDown(Point givenPoint)
	{
		Node *pacerChild = branches.front().child;
		Node *pushChild = branches.back().child;

		for (; pacerChild->branches.size() > 0; pacerChild = pacerChild->branches.front().child)
		{
			pushChild->branches.push_back({new Node(minBranchFactor, maxBranchFactor, pushChild), IsotheticPolygon(Rectangle(givenPoint, givenPoint))});
			pushChild = pushChild->branches.front().child;
		}

		pushChild->data.push_back(givenPoint);
	}

	// Always called on root, this = root
	Node *Node::insert(Point givenPoint)
	{
		// Find the appropriate position for the new point
		Node *adjustContext = chooseNode(givenPoint);

		if (adjustContext->parent != nullptr)
		{
			DASSERT(adjustContext->parent->locateBranch(adjustContext).boundingPoly.containsPoint(givenPoint));
		}

		// Adjust the tree
		if (adjustContext->branches.size() == 0)
		{
			// Add just the data
			adjustContext->data.push_back(givenPoint);
		}

		// There is no guarantee that the root will still exist after adjustment so backup branch factors
		unsigned backupMinBranchFactor = minBranchFactor;
		unsigned backupMaxBranchFactor = maxBranchFactor;
		Node::SplitResult finalSplit = adjustContext->adjustTree();

		// Grow the tree taller if we need to
		if (finalSplit.leftBranch.child != nullptr && finalSplit.rightBranch.child != nullptr)
		{
			Node *newRoot = new Node(backupMinBranchFactor, backupMaxBranchFactor, nullptr);

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
			DEXEC(this->printTree());
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
		if (parent != expectedParent || branches.size() > maxBranchFactor || data.size() > maxBranchFactor)
		{
			std::cout << "node = " << (void *)this << std::endl;
			std::cout << "parent = " << (void *)parent << " expectedParent = " << (void *)expectedParent << std::endl;
			std::cout << "maxBranchFactor = " << maxBranchFactor << std::endl;
			std::cout << "branches.size() = " << branches.size() << std::endl;
			std::cout << "data.size() = " << data.size() << std::endl;
			assert(parent == expectedParent);
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
		std::cout << std::endl;
	}

	void Node::printTree(unsigned n)
	{
		// Print this node first
		print(n);

		// Print any of our children with one more level of indentation
		std::string indendtation(n * 4, ' ');
		if (branches.size() > 0)
		{
			for (unsigned i = 0; i < branches.size(); ++i)
			{
				// Recurse
				branches[i].child->printTree(n + 1);
			}
		}
		std::cout << std::endl;
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
		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;
		unsigned long branchesSize;
		unsigned long dataSize;
		unsigned long polygonSize;
		size_t memoryFootprint = 0;
		unsigned long totalNodes = 1;
		unsigned long singularBranches = 0;
		unsigned long totalLeaves = 0;

		std::vector<unsigned long> histogramPolygon;
		histogramPolygon.resize(10000, 0);
		std::vector<unsigned long> histogramFanout;
		histogramFanout.resize(maxBranchFactor, 0);

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();

			branchesSize = currentContext->branches.size();
			dataSize = currentContext->data.size();
			unsigned fanout = branchesSize == 0 ? dataSize : branchesSize;
			if (fanout > histogramFanout.size())
			{
				histogramFanout.resize(fanout, 0);
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
				memoryFootprint += sizeof(Node) + branchesSize * sizeof(Node::Branch);
				for (unsigned i = 0; i < branchesSize; ++i)
				{
					if (currentContext->branches[i].child->branches.size() == 1 || currentContext->branches[i].child->data.size() == 1)
					{
						singularBranches++;
					}

					polygonSize = currentContext->branches[i].boundingPoly.basicRectangles.size();
					++histogramPolygon[polygonSize];

					context.push(currentContext->branches[i].child);
				}
			}
		}

		// Print out what we have found
		STATEXEC(std::cout << "### Statistics ###" << std::endl);
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
		STATPOLYHIST();
		for (unsigned i = 0; i < histogramPolygon.size(); ++i)
		{
			if (histogramPolygon[i] > 0)
			{
				STATHIST(i, histogramPolygon[i]);
			}
		}
		STATEXEC(std::cout << "### ### ### ###" << std::endl);
	}
}
