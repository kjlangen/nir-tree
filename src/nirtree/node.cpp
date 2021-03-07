#include <nirtree/node.h>
#include <nirtree/nirtree.h>

namespace nirtree
{
	Node::Node(NIRTree &treeRef, Node *p, unsigned level) :
		treeRef(treeRef), parent(p), level(level)
	{
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
		for (Branch &branch : branches)
		{
			if (branch.child == child)
			{
				return branch;
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

		// Mark the child for deletion and remove it from our branches by overwriting
		treeRef.garbage.push_back(child);
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

	std::vector<Point> Node::search(const Point &requestedPoint)
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

			if (currentContext->branches.size() == 0)
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
					if (branch.boundingPoly.containsPoint(requestedPoint))
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
		treeRef.stats.resetSearchTracker<false>();
#endif

		return accumulator;
	}

	std::vector<Point> Node::search(const Rectangle &requestedRectangle)
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

			if (currentContext->branches.size() == 0)
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
					if (branch.boundingPoly.intersectsRectangle(requestedRectangle))
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
		treeRef.stats.resetSearchTracker<true>();
#endif

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
					evalExpansion = context->branches[i].boundingPoly.computeExpansionArea(givenPoint);
					if (evalExpansion.area < smallestExpansion.area && evalExpansion.area != 0.0)
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
							subsetPolygon.increaseResolution(givenPoint, context->branches[i].boundingPoly);
						}
					}

					if (context->parent != nullptr)
					{
						subsetPolygon.intersection(context->parent->branches[enclosingPolyIndex].boundingPoly);
					}

					context->branches[smallestExpansionBranchIndex].boundingPoly.remove(smallestExpansion.index);
					context->branches[smallestExpansionBranchIndex].boundingPoly.merge(subsetPolygon);

					if (context->branches[smallestExpansionBranchIndex].child->data.size() > 0)
					{
						context->branches[smallestExpansionBranchIndex].child->data.push_back(givenPoint);
						context->branches[smallestExpansionBranchIndex].boundingPoly.shrink(context->branches[smallestExpansionBranchIndex].child->data);
						context->branches[smallestExpansionBranchIndex].child->data.pop_back();
					}

					context->branches[smallestExpansionBranchIndex].boundingPoly.refine();
				}

				// Descend
				context = context->branches[smallestExpansionBranchIndex].child;
				enclosingPolyIndex = smallestExpansionBranchIndex;
			}
		}
	}

	Node *Node::chooseNode(Branch orphanedBranch)
	{
		// CL1 [Initialize]
		Node *context = this;
		unsigned enclosingPolyIndex = 0;
		unsigned branchesSize = 0;

		for (;;)
		{
			branchesSize = context->branches.size();

			// CL2 [Level check]
			if (context->level == orphanedBranch.child->level + 1)
			{
				return context;
			}
			else
			{
				assert(context->level > orphanedBranch.child->level);
				// Compute the smallest expansion
				unsigned smallestExpansionBranchIndex = 0;
				IsotheticPolygon::OptimalExpansion smallestExpansion = context->branches[0].boundingPoly.computeExpansionArea(orphanedBranch.boundingPoly.boundingBox);
				IsotheticPolygon::OptimalExpansion evalExpansion;
				for (unsigned i = 1; i < branchesSize && smallestExpansion.area != -1.0; ++i)
				{
					evalExpansion = context->branches[i].boundingPoly.computeExpansionArea(orphanedBranch.boundingPoly.boundingBox);
					if (evalExpansion.area < smallestExpansion.area && evalExpansion.area != 0.0)
					{
						smallestExpansionBranchIndex = i;
						smallestExpansion = evalExpansion;
					}
				}

				if (smallestExpansion.area != -1.0)
				{
					IsotheticPolygon subsetPolygon(context->branches[smallestExpansionBranchIndex].boundingPoly.basicRectangles[smallestExpansion.index]);
					subsetPolygon.basicRectangles[0].expand(orphanedBranch.boundingPoly.boundingBox);

					for (unsigned i = 0; i < branchesSize; ++i)
					{
						if (i != smallestExpansionBranchIndex)
						{
							subsetPolygon.increaseResolution(orphanedBranch.boundingPoly, context->branches[i].boundingPoly);
						}
					}

					if (context->parent != nullptr)
					{
						subsetPolygon.intersection(context->parent->branches[enclosingPolyIndex].boundingPoly);
					}

					context->branches[smallestExpansionBranchIndex].boundingPoly.remove(smallestExpansion.index);
					context->branches[smallestExpansionBranchIndex].boundingPoly.merge(subsetPolygon);
					context->branches[smallestExpansionBranchIndex].boundingPoly.refine();
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

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();

			if (currentContext->branches.size() == 0)
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
					if (branch.boundingPoly.containsPoint(givenPoint))
					{
						// Add the child to the nodes we will consider
						context.push(branch.child);
					}
				}
			}
		}

		return nullptr;
	}

	Node::Partition Node::partitionNode()
	{
		nirtree::Node::Partition defaultPartition;
		unsigned branchesSize = branches.size();
		unsigned costMetric = std::numeric_limits<unsigned>::max();
		double totalMass = 0.0;

		if (branchesSize == 0)
		{
			// Setup variance values
			Point variance = Point::atOrigin;
			Point average = Point::atOrigin;
			Point sumOfSquares = Point::atOrigin;

			for (Point &dataPoint : data)
			{
				average += dataPoint;
				sumOfSquares += dataPoint * dataPoint;
				totalMass += 1.0;
			}

			// Compute final terms
			average /= totalMass;
			sumOfSquares /= totalMass;

			// Compute final variance
			variance = sumOfSquares - average * average;

			// Choose most variate dimension
			defaultPartition.dimension = 0;
			for (unsigned d = 0; d < dimensions; ++d)
			{
				if (variance[d] > variance[defaultPartition.dimension])
				{
					defaultPartition.dimension = d;
				}
			}
			defaultPartition.location = average[defaultPartition.dimension];

			return defaultPartition;
		}
		else
		{
			Point centreOfMass = Point::atOrigin;
			std::vector<Rectangle> sortable;
			double location;

			for (Branch &b : branches)
			{
				sortable.insert(sortable.end(), b.boundingPoly.basicRectangles.begin(), b.boundingPoly.basicRectangles.end());
			}

			for (Rectangle &boundingBox : sortable)
			{
				centreOfMass += boundingBox.lowerLeft;
				centreOfMass += boundingBox.upperRight;
				totalMass += 2.0;
			}

			centreOfMass /= totalMass;

			for (unsigned d = 0; d < dimensions; ++d)
			{
				location = centreOfMass[d];

				// Compute cost, # of splits if d is chosen
				unsigned currentInducedSplits = 0;
				for (Rectangle &s : sortable)
				{
					if (s.lowerLeft[d] < location && location < s.upperRight[d])
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
		assert(treeRef.root->parent == nullptr);
		IsotheticPolygon referencePoly;
		if (parent != nullptr)
		{
			referencePoly = parent->locateBranch(this).boundingPoly;
		}
		else
		{
			referencePoly = IsotheticPolygon(boundingBox());
		}

		SplitResult split = {{new Node(treeRef, parent, level), referencePoly}, {new Node(treeRef, parent, level), referencePoly}};
		unsigned branchesSize = branches.size();
		unsigned dataSize = data.size();

		split.leftBranch.boundingPoly.maxLimit(p.location, p.dimension);
		split.rightBranch.boundingPoly.minLimit(p.location, p.dimension);

		split.leftBranch.boundingPoly.refine();
		split.rightBranch.boundingPoly.refine();

		if (branchesSize == 0 && dataSize > 0)
		{
			bool containedLeft, containedRight;
			for (Point &dataPoint : data)
			{
				containedLeft = split.leftBranch.boundingPoly.containsPoint(dataPoint);
				containedRight = split.rightBranch.boundingPoly.containsPoint(dataPoint);

				if (containedLeft && !containedRight)
				{
					split.leftBranch.child->data.push_back(dataPoint);
				}
				else if (!containedLeft && containedRight)
				{
					split.rightBranch.child->data.push_back(dataPoint);
				}
				else if (split.rightBranch.child->data.size() < split.leftBranch.child->data.size())
				{
					split.rightBranch.child->data.push_back(dataPoint);
				}
				else
				{
					split.leftBranch.child->data.push_back(dataPoint);
				}
			}
			data.clear();

			split.leftBranch.boundingPoly.shrink(split.leftBranch.child->data);
			split.rightBranch.boundingPoly.shrink(split.rightBranch.child->data);
		}
		else
		{
			for (Branch &branch : branches)
			{
				if (branch.boundingPoly.boundingBox.upperRight[p.dimension] <= p.location)
				{
					branch.child->parent = split.leftBranch.child;
					split.leftBranch.child->branches.push_back(branch);
				}
				else if (branch.boundingPoly.boundingBox.lowerLeft[p.dimension] >= p.location)
				{
					branch.child->parent = split.rightBranch.child;
					split.rightBranch.child->branches.push_back(branch);
				}
				else
				{
					Node::SplitResult downwardSplit = branch.child->splitNode(p);

					treeRef.garbage.push_back(branch.child);

					if (downwardSplit.leftBranch.child->data.size() > 0 || downwardSplit.leftBranch.child->branches.size() > 0)
					{
						downwardSplit.leftBranch.child->parent = split.leftBranch.child;
						split.leftBranch.child->branches.push_back(downwardSplit.leftBranch);
					}
					else
					{
						treeRef.garbage.push_back(downwardSplit.leftBranch.child);
					}

					if (downwardSplit.rightBranch.child->data.size() > 0 || downwardSplit.rightBranch.child->branches.size() > 0)
					{
						downwardSplit.rightBranch.child->parent = split.rightBranch.child;
						split.rightBranch.child->branches.push_back(downwardSplit.rightBranch);
					}
					else
					{
						treeRef.garbage.push_back(downwardSplit.rightBranch.child);
					}
				}
			}
			branches.clear();
		}

		assert(level == split.leftBranch.child->level);
		assert(level == split.rightBranch.child->level);
		assert(treeRef.root->parent == nullptr);
		assert(split.leftBranch.child->treeRef.root->parent == nullptr);
		assert(split.rightBranch.child->treeRef.root->parent == nullptr);

		return split;
	}

	// Splitting a node will remove it from its parent node and its memory will be freed
	Node::SplitResult Node::splitNode()
	{
		assert(treeRef.root->parent == nullptr);
		Node::SplitResult returnSplit = splitNode(partitionNode());

		assert(level == returnSplit.leftBranch.child->level);
		assert(level == returnSplit.rightBranch.child->level);
		assert(treeRef.root->parent == nullptr);

		return returnSplit;
	}

	void Node::resizeBoundingPoly()
	{
		if (parent == nullptr)
		{
			return;
		}

		IsotheticPolygon referencePoly(parent->locateBranch(this).boundingPoly);
		referencePoly.intersection(IsotheticPolygon(boundingBox()));
		parent->updateBranch(this, referencePoly);
	}

	Node::SplitResult Node::reInsert(std::vector<bool> &hasReinsertedOnLevel)
	{
		assert(hasReinsertedOnLevel.at(level));
		assert(treeRef.root->parent == nullptr);

		Partition bestDimension = partitionNode();
		std::vector<Point> orphanedData;
		std::vector<Branch> orphanedBranches;

		if (branches.size() == 0)
		{
			// Sort the entries somehow to determine which percentage gets reinserted
			std::sort(data.begin(), data.end(), [bestDimension](Point &a, Point &b){ return a[bestDimension.dimension] < b[bestDimension.dimension]; });

			assert(treeRef.root->parent == nullptr);

			// Remove those entries
			unsigned reinsertionCount = treeRef.p * data.size();
			orphanedData.reserve(reinsertionCount);
			std::copy(data.begin(), data.begin() + reinsertionCount, std::back_inserter(orphanedData));
			data.erase(data.begin(), data.begin() + reinsertionCount);
			assert(data.size() > 0);
			assert(treeRef.root->parent == nullptr);
		}
		else
		{
			// Sort the entries somehow to determine which percentage gets reinserted
			std::sort(branches.begin(), branches.end(), [bestDimension](Branch &a, Branch &b){ return a.boundingPoly.boundingBox.lowerLeft[bestDimension.dimension] < b.boundingPoly.boundingBox.lowerLeft[bestDimension.dimension]; });

			assert(treeRef.root->parent == nullptr);
			// Remove those entries
			unsigned reinsertionCount = treeRef.p * branches.size();
			orphanedBranches.reserve(reinsertionCount);
			std::copy(branches.begin(), branches.begin() + reinsertionCount, std::back_inserter(orphanedBranches));
			branches.erase(branches.begin(), branches.begin() + reinsertionCount);
			assert(branches.size() < treeRef.maxBranchFactor);
			assert(treeRef.root->parent == nullptr);
		}

		// Walk up the tree shrinking bounding polygons
		Node *context = this;
		while (context->parent != nullptr)
		{
			// TODO: This is the simple way to resize boundingPolygons to be smaller but there is potential to do more
			context->resizeBoundingPoly();
			context = context->parent;
		}

		// Loop over the removed entries and call insert again.
		for (const Point p : orphanedData)
		{
			assert(treeRef.root->parent == nullptr);
			treeRef.root->insert(p, hasReinsertedOnLevel);
			assert(treeRef.root->parent == nullptr);
		}
		for (const Branch b : orphanedBranches)
		{
			assert(treeRef.root->parent == nullptr);
			treeRef.root->insert(b, hasReinsertedOnLevel);
			assert(treeRef.root->parent == nullptr);
		}

		assert(treeRef.root->parent == nullptr);

		return {nullptr, IsotheticPolygon(), nullptr, IsotheticPolygon()};
	}

	Node::SplitResult Node::overflowTreatment(std::vector<bool> &hasReinsertedOnLevel)
	{
		assert(hasReinsertedOnLevel.size() > level);
		assert(treeRef.root->parent == nullptr);

		if (hasReinsertedOnLevel[level] || branches.size() > 0)
		{
			// If we've already reinserted then split and cleanup
			auto sr = splitNode();
			if (parent != nullptr)
			{
				parent->removeBranch(this);
			}

			assert(treeRef.root->parent == nullptr);
			return sr;
		}
		else
		{
			hasReinsertedOnLevel[level] = true;
			auto sr = reInsert(hasReinsertedOnLevel);
			assert(treeRef.root->parent == nullptr);
			return sr;
		}
	}

	// This bottom-to-top sweep is only for splitting bounding boxes as necessary
	Node::SplitResult Node::adjustTree(std::vector<bool> &hasReinsertedOnLevel)
	{
		assert(treeRef.root->parent == nullptr);
		Node *currentContext = this;
		unsigned branchesSize, dataSize;
		Node::SplitResult propagationSplit = {{nullptr, IsotheticPolygon()}, {nullptr, IsotheticPolygon()}};

		for (;currentContext != nullptr;)
		{
			branchesSize = currentContext->branches.size();
			dataSize = currentContext->data.size();

			// Node *currentContextAtTheTop = currentContext;

			assert(branchesSize < 100 && dataSize < 100);

			// If there was a split we were supposed to propagate then propagate it
			if (propagationSplit.leftBranch.child != nullptr && propagationSplit.rightBranch.child != nullptr)
			{
				if (propagationSplit.leftBranch.child->data.size() > 0 || propagationSplit.leftBranch.child->branches.size() > 0)
				{
					currentContext->branches.push_back(propagationSplit.leftBranch);
					++branchesSize;
				}
				else
				{
					treeRef.garbage.push_back(propagationSplit.leftBranch.child);
				}

				if (propagationSplit.rightBranch.child->data.size() > 0 || propagationSplit.rightBranch.child->branches.size() > 0)
				{
					currentContext->branches.push_back(propagationSplit.rightBranch);
					++branchesSize;
				}
				else
				{
					treeRef.garbage.push_back(propagationSplit.rightBranch.child);
				}
			}

			assert(currentContext == currentContextAtTheTop);
			assert(treeRef.root->parent == nullptr);

			// Early exit if this node does not overflow
			if (dataSize <= currentContext->treeRef.maxBranchFactor && branchesSize <= currentContext->treeRef.maxBranchFactor)
			{
				propagationSplit = {{nullptr, IsotheticPolygon()}, {nullptr, IsotheticPolygon()}};
				break;
			}

			assert(currentContext == currentContextAtTheTop);
			assert(treeRef.root->parent == nullptr);

			// Otherwise, split/reinsert node
			propagationSplit = currentContext->overflowTreatment(hasReinsertedOnLevel);

			assert(currentContext == currentContextAtTheTop);
			assert(treeRef.root->parent == nullptr);

			// Ascend, propagating splits
			currentContext = currentContext->parent;
		}

		assert(treeRef.root->parent == nullptr);
		return propagationSplit;
	}

	// Always called on root, this = root
	Node *Node::insert(PointOrOrphan given, std::vector<bool> &hasReinsertedOnLevel)
	{
		// assert(parent == nullptr);
		Node *adjustContext;
		assert(treeRef.root->parent == nullptr);
		if (std::holds_alternative<Point>(given))
		{
			// Find the appropriate position for the new point
			adjustContext = treeRef.root->chooseNode(std::get<Point>(given));
			assert(adjustContext->branches.size() == 0);

			// Add the data point
			adjustContext->data.push_back(std::get<Point>(given));
		}
		else
		{
			// Find the appropriate position for the orphan
			adjustContext = treeRef.root->chooseNode(std::get<Branch>(given));
			assert(adjustContext->data.size() == 0);

			// Add the orphan
			adjustContext->branches.push_back(std::get<Branch>(given));
			adjustContext->branches.back().child->parent = adjustContext;
		}

		assert(treeRef.root->parent == nullptr);

		// Adjust tree, possibly causing re-inserts
		Node::SplitResult finalSplit = adjustContext->adjustTree(hasReinsertedOnLevel);

		assert(treeRef.root->parent == nullptr);

		// Grow the tree taller if we need to
		if (finalSplit.leftBranch.child != nullptr && finalSplit.rightBranch.child != nullptr)
		{
			// assert(this->parent == nullptr);
			assert(finalSplit.leftBranch.child->level == finalSplit.rightBranch.child->level);

			Node *newRoot = new Node(treeRef, nullptr, finalSplit.leftBranch.child->level + 1);

			finalSplit.leftBranch.child->parent = newRoot;
			newRoot->branches.push_back(finalSplit.leftBranch);
			finalSplit.rightBranch.child->parent = newRoot;
			newRoot->branches.push_back(finalSplit.rightBranch);

			assert(newRoot->level == newRoot->branches[0].child->level + 1);
			assert(newRoot->level == newRoot->branches[1].child->level + 1);

			// Fix the reinserted length and notify the tree about the new root *before* we invalide
			// the treeRef
			hasReinsertedOnLevel.push_back(false);
			treeRef.root = newRoot;
			assert(treeRef.root->parent == nullptr);
			treeRef.garbage.push_back(this);
			assert(treeRef.root->parent == nullptr);

			return nullptr;
		}
		
		return nullptr;
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
			std::cout << "data.size() = " << data.size() << std::endl;
			assert(parent == expectedParent);
		}

		if (expectedParent != nullptr)
		{
			for (Point &dataPoint : data)
			{
				if (!parent->branches[index].boundingPoly.containsPoint(dataPoint))
				{
					std::cout << parent->branches[index].boundingPoly << " fails to contain " << dataPoint << std::endl;
					assert(parent->branches[index].boundingPoly.containsPoint(dataPoint));
				}
			}
		}

		for (unsigned i = 0; i < branches.size(); ++i)
		{
			for (unsigned j = 0; j < branches.size(); ++j)
			{
				if (i != j && !branches[i].boundingPoly.disjoint(branches[j].boundingPoly))
				{
					std::cout << "Branch " << i << " is not disjoint from sibling Branch " << j << std::endl;
					std::cout << "branches[" << i << "].boundingPoly = " << branches[i].boundingPoly << std::endl;
					std::cout << "branches[" << j << "].boundingPoly = " << branches[j].boundingPoly << std::endl;
					assert(branches[i].boundingPoly.disjoint(branches[j].boundingPoly));
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
		for (Branch &branch : branches)
		{
			std::cout << indendtation << "		" << (void *)branch.child << std::endl;
			std::cout << indendtation << "		" << branch.boundingPoly << std::endl;
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
		if (branches.size() > 0)
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
#ifdef STAT
		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;
		unsigned long branchesSize;
		unsigned long dataSize;
		unsigned long polygonSize;
		unsigned long totalPolygonSize = 0;
		unsigned long totalLines = 0;
		size_t memoryFootprint = 0;
		unsigned long totalNodes = 1;
		unsigned long singularBranches = 0;
		unsigned long totalLeaves = 0;

		std::vector<unsigned long> histogramPolygon;
		histogramPolygon.resize(10000, 0);
		std::vector<unsigned long> histogramFanout;
		histogramFanout.resize(treeRef.maxBranchFactor, 0);

		double coverage = 0.0;

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
				coverage += currentContext->branches[i].boundingPoly.area();
			}

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
					totalPolygonSize += polygonSize;

					for (Rectangle r : currentContext->branches[i].boundingPoly.basicRectangles)
					{
						if (r.area() == 0.0)
						{
							++totalLines;
						}
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
		STATFANHIST();
		for (unsigned i = 0; i < histogramFanout.size(); ++i)
		{
			if (histogramFanout[i] > 0)
			{
				STATHIST(i, histogramFanout[i]);
			}
		}
		STATLINES(totalLines);
		STATTOTALPOLYSIZE(totalPolygonSize);
		STATPOLYHIST();
		for (unsigned i = 0; i < histogramPolygon.size(); ++i)
		{
			if (histogramPolygon[i] > 0)
			{
				STATHIST(i, histogramPolygon[i]);
			}
		}
		std::cout << treeRef.stats;

		STATEXEC(std::cout << "### ### ### ###" << std::endl);
#else
    (void) 0;
#endif
	}
}
