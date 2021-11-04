#include <revisedrstartree/node.h>
#include <revisedrstartree/revisedrstartree.h>

namespace revisedrstartree
{
	Node::Node(RevisedRStarTree &treeRef, Node *p) : treeRef(treeRef)
	{
		parent = p;
		originalCentre = Point::atOrigin;
	}

	void Node::deleteSubtrees()
	{
		if (branches.size() == 0)
		{
			return;
		}
		else
		{
			for (Branch &branch : branches)
			{
				branch.child->deleteSubtrees();
				delete branch.child;
			}
		}
	}

	bool Node::isLeaf()
	{
		if (branches.size() == 0 && data.size() >= 0)
		{
			return true;
		}
		else if (branches.size() > 0 && data.size() == 0)
		{
			return false;
		}

		// Something has gone wrong, panic!
		assert(false);

		return false;
	}

	Rectangle Node::boundingBox()
	{
		Rectangle bb;

		if (isLeaf())
		{
			bb = Rectangle(data[0], data[0]);
			for (unsigned i = 1; i < data.size(); ++i)
			{
				bb.expand(data[i]);
			}
		}
		else
		{
			bb = branches[0].boundingBox;
			for (unsigned i = 1; i < branches.size(); ++i)
			{
				bb.expand(branches[i].boundingBox);
			}
		}

		return bb;
	}

	void Node::removeBranch(Node *child)
	{
		// Locate the child
		unsigned branchesSize = branches.size();
		unsigned childIndex;
		for (childIndex = 0; branches[childIndex].child != child && childIndex < branchesSize; ++childIndex) {}

		// Delete the child by deleting it and overwriting its branch
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
		if (isLeaf())
		{
			// We are a leaf so add our data points when they are the search point
			for (Point &dataPoint : data)
			{
				if (requestedPoint == dataPoint)
				{
					accumulator.push_back(dataPoint);
					break;
				}
			}
		}
		else
		{
			// Determine which branches we need to follow
			for (Branch &branch : branches)
			{
				// Recurse
				branch.child->exhaustiveSearch(requestedPoint, accumulator);
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

			if (currentContext->isLeaf())
			{
				// We are a leaf so add our data points when they are the search point
				for (Point &dataPoint : currentContext->data)
				{
					if (requestedPoint == dataPoint)
					{
						accumulator.push_back(dataPoint);
					}
				}
#ifdef STAT
				treeRef.stats.markLeafSearched();
#endif
			}
			else
			{
				// Determine which branches we need to follow
				for (Branch &branch : currentContext->branches)
				{
					if (branch.boundingBox.containsPoint(requestedPoint))
					{
						// Add to the nodes we will check
						context.push(branch.child);
					}
				}
#ifdef STAT
				treeRef.stats.markNonLeafNodeSearched();
#endif
			}
		}

#ifdef STAT
		treeRef.stats.resetSearchTracker( false );
#endif

		return accumulator;
	}

	std::vector<Point> Node::search(Rectangle &requestedRectangle)
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

			if (currentContext->isLeaf())
			{
				// We are a leaf so add our data points when they are within the search rectangle
				for (Point &dataPoint : currentContext->data)
				{
					if (requestedRectangle.containsPoint(dataPoint))
					{
						accumulator.push_back(dataPoint);
					}
				}

#ifdef STAT
				treeRef.stats.markLeafSearched();
#endif
			}
			else
			{
				// Determine which branches we need to follow
				for (Branch &branch : currentContext->branches)
				{
					if (branch.boundingBox.intersectsRectangle(requestedRectangle))
					{
						// Add to the nodes we will check
						context.push(branch.child);
					}
				}
#ifdef STAT
				treeRef.stats.markNonLeafNodeSearched();
#endif
			}
		}
#ifdef STAT
		treeRef.stats.resetSearchTracker( true );
#endif

		return accumulator;
	}

	void Node::chooseNodeHelper(unsigned limitIndex, Point &givenPoint, unsigned &chosenIndex, bool &success, std::vector<bool> &candidates, std::vector<double> &deltas, unsigned startIndex, bool useMarginDelta)
	{
		candidates[startIndex] = true;
		deltas[startIndex] = 0.0;

		for (unsigned j = 0; j <= limitIndex; ++j)
		{
			if (j != startIndex)
			{
				unsigned additionalDelta = useMarginDelta ? branches[startIndex].boundingBox.marginDelta(givenPoint, branches[j].boundingBox) : branches[startIndex].boundingBox.areaDelta(givenPoint, branches[j].boundingBox);
				deltas[startIndex] += additionalDelta;

				if (additionalDelta != 0.0 && !candidates[j])
				{
					chooseNodeHelper(limitIndex, givenPoint, chosenIndex, success, candidates, deltas, j, useMarginDelta);
					if (success)
					{
						break;
					}
				}
			}
		}

		if (deltas[startIndex] == 0.0)
		{
			chosenIndex = startIndex;
			success = true;
		}
	}

	// Always called on root, this = root
	// This top-to-bottom sweep is only for adjusting bounding boxes to contain the point and
	// choosing a particular leaf
	Node *Node::chooseNode(Point givenPoint)
	{
		// CL1 [Initialize]
		Node *context = this;
		unsigned branchesSize = 0;

		for (;;)
		{
			branchesSize = context->branches.size();

			// CL2 [Leaf check]
			if (context->isLeaf())
			{
				return context;
			}
			else
			{
				unsigned optimalBranchIndex = 0;

				// Find all rectangles that completely cover the given point
				std::vector<unsigned> covers(0);
				for (unsigned i = 0; i < branchesSize; ++i)
				{
					if (context->branches[i].boundingBox.containsPoint(givenPoint))
					{
						covers.push_back(i);
					}
				}

				if (!covers.empty())
				{
					// If any rectangles cover the given point select the lowest volume, then lowest
					// margin rectangle among them
					double minVolume = std::numeric_limits<double>::infinity();
					double minMargin = std::numeric_limits<double>::infinity();
					unsigned minIndex = covers.front();

					for (unsigned i : covers)
					{
						double evalVolume = context->branches[i].boundingBox.computeExpansionArea(givenPoint);
						if (evalVolume < minVolume)
						{
							minVolume = evalVolume;
							minMargin = context->branches[i].boundingBox.computeExpansionMargin(givenPoint);
							minIndex = i;
						}
						else if (evalVolume == minVolume && evalVolume == 0)
						{
							// Tie break using perimeter
							double evalMargin = context->branches[i].boundingBox.computeExpansionMargin(givenPoint);
							if (evalMargin < minMargin)
							{
								minVolume = evalVolume;
								minMargin = evalMargin;
								minIndex = i;
							}
						}
					}

					optimalBranchIndex = minIndex;
				}
				else
				{
					// Sort the entries in ascending order of their margin delta
					std::sort(context->branches.begin(), context->branches.end(), [givenPoint](Branch &a, Branch &b){return a.boundingBox.computeExpansionMargin(givenPoint) <= b.boundingBox.computeExpansionMargin(givenPoint);});

					// Look at the first entry's intersection margin with all the others
					double deltaWithAll = 0.0;
					for (Branch &branch : context->branches)
					{
						deltaWithAll += branch.boundingBox.marginDelta(givenPoint, context->branches[0].boundingBox);
					}

					if (deltaWithAll == 0.0)
					{
						optimalBranchIndex = 0;
					}
					else
					{
						// Set limitIndex based on margin deltas that are not 0
						unsigned limitIndex = 0;
						double maxMarginDelta = - std::numeric_limits<double>::infinity();

						for (unsigned i = 1; i < branchesSize; ++i)
						{
							double evalMarginDelta = context->branches[0].boundingBox.marginDelta(givenPoint, context->branches[i].boundingBox);
							if (evalMarginDelta > maxMarginDelta)
							{
								maxMarginDelta = evalMarginDelta;
								limitIndex = i;
							}
						}

						// Consider branches only up to limitIndex
						std::vector<bool> candidate(limitIndex + 1, false);
						std::vector<double> deltas(limitIndex + 1, 0.0);
						bool success = false;
						unsigned chosenIndex;

						// Determine if there exists a rectangle with zero area containing given point
						bool zeroAreaContainer = false;
						for (unsigned i = 0; !zeroAreaContainer && i <= limitIndex; ++i)
						{
							zeroAreaContainer = 0.0 == context->branches[i].boundingBox.copyExpand(givenPoint).area();
						}

						if (zeroAreaContainer)
						{
							context->chooseNodeHelper(limitIndex, givenPoint, chosenIndex, success, candidate, deltas, 0, true);
						}
						else
						{
							context->chooseNodeHelper(limitIndex, givenPoint, chosenIndex, success, candidate, deltas, 0, false);
						}

						if (success)
						{
							optimalBranchIndex = chosenIndex;
						}
						else
						{
							double minDelta = std::numeric_limits<double>::infinity();

							for (unsigned i = 0; i <= limitIndex; ++i)
							{
								if (deltas[i] < minDelta && candidate[i])
								{
									minDelta = deltas[i];
									optimalBranchIndex = i;
								}
							}
						}
					}
				}

				// Descend
				context->branches[optimalBranchIndex].boundingBox.expand(givenPoint);
				context = context->branches[optimalBranchIndex].child;
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

			if (currentContext->isLeaf())
			{
				// FL2 [Search leaf node for record]
				// Check each entry to see if it matches E
				for (Point &dataPoint : currentContext->data)
				{
					if (dataPoint == givenPoint)
					{
						return currentContext;
					}
				}
			}
			else
			{
				// FL1 [Search subtrees]
				// Determine which branches we need to follow
				for (Branch &branch : currentContext->branches)
				{
					if (branch.boundingBox.containsPoint(givenPoint))
					{
						// Add the child to the nodes we will consider
						context.push(branch.child);
					}
				}
			}
		}

		return nullptr;
	}

	double Node::evaluateSplit(unsigned splitIndex, std::function<double(Rectangle &, Rectangle &)> evaluator)
	{
		Rectangle leftCandidate, rightCandidate;

		if (isLeaf())
		{
			// Build the left group bounding box
			leftCandidate = Rectangle(data[0], data[0]);
			for (unsigned i = 0; i < splitIndex; ++i)
			{
				leftCandidate.expand(data[i]);
			}

			// Build the right group bounding box
			rightCandidate = Rectangle(data[splitIndex], data[splitIndex]);
			for (unsigned i = splitIndex; i < data.size(); ++i)
			{
				rightCandidate.expand(data[i]);
			}
		}
		else
		{
			// Build the left group bounding box
			leftCandidate = branches[0].boundingBox;
			for (unsigned i = 1; i < splitIndex; ++i)
			{
				leftCandidate.expand(branches[i].boundingBox);
			}

			// Build the right group bounding box
			rightCandidate = branches[splitIndex].boundingBox;
			for (unsigned i = splitIndex + 1; i < branches.size(); ++i)
			{
				rightCandidate.expand(branches[i].boundingBox);
			}
		}

		// Evaluate the left and right groups based on their bounding boxes
		return evaluator(leftCandidate, rightCandidate);
	}

	unsigned Node::chooseSplitAxis()
	{
		unsigned axis = 0;
		bool minSort = true;
		double minMargin = std::numeric_limits<double>::infinity();

		for (unsigned d = 0; d < dimensions; ++d)
		{
			// Sort once for leaf
			if (isLeaf())
			{
				std::sort(data.begin(), data.end(), [d](Point &a, Point &b){return a[d] < b[d];});
				for (unsigned i = treeRef.minBranchFactor; i < 1 + treeRef.maxBranchFactor - treeRef.minBranchFactor; ++i)
				{
					// Compute margin on either side of the split
					double evalMargin = evaluateSplit(i, [](Rectangle &a, Rectangle &b){return a.margin() + b.margin();});
					if (evalMargin < minMargin)
					{
						axis = d;
						minMargin = evalMargin;
					}
				}
			}
			else
			{
				// Sort twice for routing
				// Lower left ordering
				std::sort(branches.begin(), branches.end(), [d](Branch &a, Branch &b){return a.boundingBox.lowerLeft[d] < b.boundingBox.lowerLeft[d];});
				for (unsigned i = treeRef.minBranchFactor; i < 1 + treeRef.maxBranchFactor - treeRef.minBranchFactor; ++i)
				{
					double evalMargin = evaluateSplit(i, [](Rectangle &a, Rectangle &b){return a.margin() + b.margin();});
					if (evalMargin < minMargin)
					{
						axis = d;
						minSort = true;
						minMargin = evalMargin;
					}
				}

				// Upper right ordering
				std::sort(branches.begin(), branches.end(), [d](Branch &a, Branch &b){return a.boundingBox.upperRight[d] < b.boundingBox.upperRight[d];});
				for (unsigned i = treeRef.minBranchFactor; i < 1 + treeRef.maxBranchFactor - treeRef.minBranchFactor; ++i)
				{
					double evalMargin = evaluateSplit(i, [](Rectangle &a, Rectangle &b){return a.margin() + b.margin();});
					if (evalMargin < minMargin)
					{
						axis = d;
						minSort = false;
						minMargin = evalMargin;
					}
				}
			}
		}

		// Sort the branches in the way requied by our chosen axis
		if (isLeaf())
		{
			std::sort(data.begin(), data.end(), [axis](Point &a, Point &b){return a[axis] < b[axis];});
		}
		else if (minSort)
		{
			std::sort(branches.begin(), branches.end(), [axis](Branch &a, Branch &b){return a.boundingBox.lowerLeft[axis] < b.boundingBox.lowerLeft[axis];});
		}
		else
		{
			std::sort(branches.begin(), branches.end(), [axis](Branch &a, Branch &b){return a.boundingBox.upperRight[axis] < b.boundingBox.upperRight[axis];});
		}

		return axis;
	}

	double Node::splitWeight(unsigned splitIndex, double ys, double y1, double u, double sigma, Rectangle &bb)
	{
		// Compute the bounding rectangles of each side
		double overlapOfCandidate = evaluateSplit(splitIndex, [](Rectangle &a, Rectangle &b){return a.computeIntersectionArea(b);});

		double weightGoal, weightFunction;

		// Evaluate wg(i)
		if (overlapOfCandidate == 0.0)
		{
			weightGoal = evaluateSplit(splitIndex, [](Rectangle &a, Rectangle &b){return a.margin() + b.margin();}) - bb.margin();
		}
		else
		{
			weightGoal = overlapOfCandidate;
		}

		// Evaluate wf(i)
		double M = (double) treeRef.maxBranchFactor;		
		double xi = ((2 * splitIndex) / (M + 1)) - 1;
		weightFunction = ys * (std::exp(-std::pow((xi - u) / sigma, 2)) - y1);

		return overlapOfCandidate == 0.0 ? weightGoal * weightFunction : weightGoal / weightFunction;
	}

	unsigned Node::chooseSplitIndex(unsigned splitAxis)
	{
		// Precompute the elements not dependant on candidateIndex
		Rectangle bb = boundingBox();
		double m = (double) treeRef.minBranchFactor;
		double M = (double) treeRef.maxBranchFactor;
		double asym = 2 * (bb.centrePoint()[splitAxis] - originalCentre[splitAxis]) / (std::fabs(bb.upperRight[splitAxis] - bb.lowerLeft[splitAxis]));
		double u = (1 - (2 * m) / (M + 1)) * asym;
		double sigma = treeRef.s * (1 + std::fabs(u));
		double y1 = std::exp(-1 / std::pow(treeRef.s, 2));
		double ys = 1 / (1 - y1);

		unsigned splitIndex = 0;
		double minWeight = std::numeric_limits<double>::infinity();

		for (unsigned candidateIndex = treeRef.minBranchFactor; candidateIndex < 1 + treeRef.maxBranchFactor - treeRef.minBranchFactor; ++candidateIndex)
		{
			double evalWeight = splitWeight(candidateIndex, ys, y1, u, sigma, bb);
			if (evalWeight < minWeight)
			{
				splitIndex = candidateIndex;
				minWeight = evalWeight;
			}
		}

		return splitIndex;
	}

	// Splitting a node will remove it from its parent node and its memory will be freed
	Node::SplitResult Node::splitNode()
	{
		SplitResult split = {{new Node(treeRef, parent), Rectangle::atOrigin}, {new Node(treeRef, parent), Rectangle::atOrigin}};

		unsigned splitIndex = chooseSplitIndex(chooseSplitAxis());

		assert(splitIndex < branches.size() || splitIndex < data.size());

		if (isLeaf())
		{
			for (unsigned i = 0; i < splitIndex; ++i)
			{
				split.leftBranch.child->data.push_back(data[i]);
			}

			for (unsigned i = splitIndex; i < data.size(); ++i)
			{
				split.rightBranch.child->data.push_back(data[i]);
			}

			data.clear();
		}
		else
		{
			for (unsigned i = 0; i < splitIndex; ++i)
			{
				branches[i].child->parent = split.leftBranch.child;
				split.leftBranch.child->branches.push_back(branches[i]);
			}

			for (unsigned i = splitIndex; i < branches.size(); ++i)
			{
				branches[i].child->parent = split.rightBranch.child;
				split.rightBranch.child->branches.push_back(branches[i]);
			}

			branches.clear();
		}

		split.leftBranch.boundingBox = split.leftBranch.child->boundingBox();
		split.leftBranch.child->originalCentre = split.leftBranch.boundingBox.centrePoint();

		split.rightBranch.boundingBox = split.rightBranch.child->boundingBox();
		split.rightBranch.child->originalCentre = split.rightBranch.boundingBox.centrePoint();

		return split;
	}

	// This bottom-to-top sweep is only for splitting bounding boxes as necessary
	Node::SplitResult Node::adjustTree()
	{
		Node *currentContext = this;
		unsigned branchesSize, dataSize;
		unsigned M = treeRef.maxBranchFactor;
		Node::SplitResult propagationSplit = {{nullptr, Rectangle::atInfinity}, {nullptr, Rectangle::atInfinity}};

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
			if (dataSize <= M && branchesSize <= M)
			{
				propagationSplit = {{nullptr, Rectangle::atInfinity}, {nullptr, Rectangle::atInfinity}};
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
		assert(adjustContext->isLeaf());

		// Add just the data
		adjustContext->data.push_back(givenPoint);

		// Deal with overflows in the tree
		Node::SplitResult finalSplit = adjustContext->adjustTree();

		// Grow the tree taller if we need to
		if (finalSplit.leftBranch.child != nullptr && finalSplit.rightBranch.child != nullptr)
		{
			Node *newRoot = new Node(treeRef);

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

		if (isLeaf())
		{
			for (Point &dataPoint : data)
			{
				for (unsigned d = 0; d < dimensions; ++d)
				{
					sum += (unsigned)dataPoint[d];
				}
			}
		}
		else
		{
			for (Branch &branch : branches)
			{
				// Recurse
				sum += branch.child->checksum();
			}
		}

		return sum;
	}

	bool Node::validate(Node *expectedParent, unsigned index)
	{
		if (parent != expectedParent || branches.size() > treeRef.maxBranchFactor || data.size() > treeRef.maxBranchFactor)
		{
			std::cout << "node = " << (void *)this << std::endl;
			std::cout << "parent = " << (void *)parent << " expectedParent = " << (void *)expectedParent << std::endl;
			std::cout << "maxBranchFactor = " << treeRef.maxBranchFactor << std::endl;
			std::cout << "branches.size() = " << branches.size() << std::endl;
			std::cout << "data.size() = " << data.size() << std::endl;
			assert(parent == expectedParent);
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
		for (Branch &branch : branches)
		{
			std::cout << indendtation << "		" << (void *)branch.child << std::endl;
			std::cout << indendtation << "		" << branch.boundingBox << std::endl;
		}
		std::cout << indendtation << "    Data: ";
		for (Point &dataPoint : data)
		{
			std::cout << dataPoint;
		}
		std::cout << std::endl;
	}

	void Node::printTree(unsigned n)
	{
		// Print this node first
		print(n);

		// Print any of our children with one more level of indentation
		std::string indendtation(n * 4, ' ');
		if (!isLeaf())
		{
			for (Branch &branch : branches)
			{
				// Recurse
				branch.child->printTree(n + 1);
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
			if (node->isLeaf())
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
#ifdef STAT
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

		std::vector<unsigned long> histogramFanout(treeRef.maxBranchFactor, 0);

		double coverage = 0.0;
		double intersectionArea = 0.0;

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

			// Compute the overlap and coverage of our children
			for (unsigned i = 0; i < branchesSize; ++i)
			{
				coverage += currentContext->branches[i].boundingBox.area();

				for (unsigned j = 0; j < branchesSize; ++j)
				{
					if (i != j)
					{
						intersectionArea += branches[i].boundingBox.computeIntersectionArea(branches[j].boundingBox);
					}
				}
			}

			if (currentContext->isLeaf())
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
		STATCOVER(coverage);
		STATOVERLAP(intersectionArea);
		STATFANHIST();
		for (unsigned i = 0; i < histogramFanout.size(); ++i)
		{
			if (histogramFanout[i] > 0)
			{
				STATHIST(i, histogramFanout[i]);
			}
		}
		std::cout << treeRef.stats;
		STATEXEC(std::cout << "### ### ### ###" << std::endl);
#else
    (void) 0;
#endif
	}
}
