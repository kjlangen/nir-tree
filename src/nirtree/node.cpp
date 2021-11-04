// Copyright 2021 Kyle Langendoen

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <nirtree/node.h>
#include <nirtree/nirtree.h>

namespace nirtree
{
	Node::Node(NIRTree &treeRef) :
		treeRef(treeRef)
	{
		minBranchFactor = 0;
		maxBranchFactor = 0;
		parent = nullptr;
	}

	Node::Node(NIRTree &treeRef, unsigned minBranch, unsigned maxBranch, Node *p) :
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
		treeRef.stats.resetSearchTracker( true );
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
			if (context->isLeaf())
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
						context->branches[smallestExpansionBranchIndex].boundingPoly.shrink(
                                context->branches[smallestExpansionBranchIndex].child->data
                        );
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
		unsigned costMetric = std::numeric_limits<unsigned>::max();
		double totalMass = 0.0;

		if (isLeaf())
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
		IsotheticPolygon referencePoly;
		if (parent != nullptr)
		{
			referencePoly = parent->locateBranch(this).boundingPoly;
		}
		else
		{
			referencePoly = IsotheticPolygon(boundingBox());
		}

		SplitResult split = {{new Node(treeRef, minBranchFactor, maxBranchFactor, parent), referencePoly}, {new Node(treeRef, minBranchFactor, maxBranchFactor, parent), referencePoly}};

		split.leftBranch.boundingPoly.maxLimit(p.location, p.dimension);
		split.rightBranch.boundingPoly.minLimit(p.location, p.dimension);

		split.leftBranch.boundingPoly.refine();
		split.rightBranch.boundingPoly.refine();

		if (isLeaf())
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

			split.leftBranch.boundingPoly.shrink(
                    split.leftBranch.child->data
            );
			split.rightBranch.boundingPoly.shrink(
                    split.rightBranch.child->data
            );
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

					delete branch.child;

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
				propagationSplit = {{nullptr, IsotheticPolygon()}, {nullptr, IsotheticPolygon()}};
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
		if (adjustContext->isLeaf())
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
		histogramFanout.resize(maxBranchFactor, 0);

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
