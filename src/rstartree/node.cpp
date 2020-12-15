#include <rstartree/node.h>

namespace rstartree
{
	Node::Node()
	{
		minBranchFactor = 3;
		maxBranchFactor = 5;
		parent = nullptr;
		boundingBoxes.resize(0);
		children.resize(0);
		data.resize(0);
	}

	Node::Node(unsigned minBranchFactor, unsigned maxBranchFactor, Node *p)
	{
		this->minBranchFactor = minBranchFactor;
		this->maxBranchFactor = maxBranchFactor;
		this->parent = p;
		boundingBoxes.resize(0);
		children.resize(0);
		data.resize(0);
	}

	void Node::deleteSubtrees()
	{
		if (children.size() == 0)
		{
			return;
		}
		else
		{
			for (unsigned i = 0; i < children.size(); ++i)
			{
				children[i]->deleteSubtrees();
				delete children[i];
			}
		}
	}

	Rectangle Node::boundingBox()
	{
		Rectangle boundingBox;

		if (boundingBoxes.size() > 0)
		{
			boundingBox = boundingBoxes[0];
			for (unsigned i = 1; i < boundingBoxes.size(); ++i)
			{
				boundingBox.expand(boundingBoxes[i]);
			}
		}
		else
		{
			boundingBox = Rectangle(data[0], data[0]);
			for (unsigned i = 0; i < data.size(); ++i)
			{
				boundingBox.expand(data[i]);
			}
		}

		return boundingBox;
	}

	// TODO: Optimize maybe
	void Node::updateBoundingBox(Node *child, Rectangle updatedBoundingBox)
	{
		for (unsigned i = 0; i < children.size(); ++i)
		{
			if (children[i] == child)
			{
				boundingBoxes[i] = updatedBoundingBox;
				break;
			}
		}
	}

	// TODO: Optimize maybe
	void Node::removeChild(Node *child)
	{
		for (unsigned i = 0; i < children.size(); ++i)
		{
			if (children[i] == child)
			{
				boundingBoxes.erase(boundingBoxes.begin() + i);
				children.erase(children.begin() + i);
				break;
			}
		}
	}

	// TODO: Optimize maybe
	void Node::removeData(Point givenPoint)
	{
		for (unsigned i = 0; i < data.size(); ++i)
		{
			if (data[i] == givenPoint)
			{
				data.erase(data.begin() + i);
				break;
			}
		}
	}

	void Node::exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator)
	{
		if (children.size() == 0)
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
			assert(children.size() == boundingBoxes.size());
			for (unsigned i = 0; i < boundingBoxes.size(); ++i)
			{
				// Recurse
				children[i]->exhaustiveSearch(requestedPoint, accumulator);
			}
		}
	}

	std::vector<Point> Node::search(Point &requestedPoint)
	{
		STATEXEC(stat());

		std::vector<Point> matchingPoints;

		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();

			if (currentContext->children.size() == 0)
			{
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
				// Determine which branches we need to follow
				for (unsigned i = 0; i < currentContext->boundingBoxes.size(); ++i)
				{
					if (currentContext->boundingBoxes[i].containsPoint(requestedPoint))
					{
						// Add to the nodes we will check
						context.push(currentContext->children[i]);
					}
				}
			}
		}

		return matchingPoints;
	}

	std::vector<Point> Node::search(Rectangle &requestedRectangle)
	{
		STATEXEC(stat());

		std::vector<Point> matchingPoints;

		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();

			if (currentContext->children.size() == 0)
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
				for (unsigned i = 0; i < currentContext->boundingBoxes.size(); ++i)
				{
					if (currentContext->boundingBoxes[i].intersectsRectangle(requestedRectangle))
					{
						// Add to the nodes we will check
						context.push(currentContext->children[i]);
					}
				}
			}
		}

		return matchingPoints;
	}

	unsigned int Node::computeOverlapGrowth(unsigned int index, std::vector<Rectangle> boundingBoxes, Rectangle givenRectangle)
	{
		// ok so now that we have the bounding boxes what we need to do is this 
		// 1. make a test rectangle we will use to not modify the original
		Rectangle newRectangle = boundingBoxes[index];
		
		// 2. Add the point to the copied Rectangle
		newRectangle.expand(givenRectangle);

		// 3. now compute the overlap expansion area 
		unsigned int overlapArea = 0;
		for (unsigned int i = 0; i < boundingBoxes.size(); i += 1)
		{
			if (i == index) continue;
			overlapArea += newRectangle.computeIntersectionArea(boundingBoxes[i]);
		}

		return overlapArea;
	}

	unsigned int Node::computeOverlapGrowth(unsigned int index, std::vector<Rectangle> boundingBoxes, Point givenPoint)
	{
		// ok so now that we have the bounding boxes what we need to do is this 
		// 1. make a test rectangle we will use to not modify the original
		Rectangle newRectangle = boundingBoxes[index];
		
		// 2. Add the point to the copied Rectangle
		newRectangle.expand(givenPoint);

		// 3. now compute the overlap expansion area 
		unsigned int overlapArea = 0;
		for (unsigned int i = 0; i < boundingBoxes.size(); i += 1)
		{
			if (i == index) continue;
			overlapArea += newRectangle.computeIntersectionArea(boundingBoxes[i]);
		}

		return overlapArea;
	}

	// Always called on root, this = root
	// TODO: Write the analogous chooseLeaf(Rectangle searchRectangle)
	Node *Node::chooseLeaf(Point givenPoint)
	{
		// This gets changed here

		// Basically we need to implememt the choose subtree algorithm here
		// Honestly not much has changed so we don't really need to do much
		/*
			CS1: This is CAlled on the root! Just like above
			CS2: If N is a leaf return N (same)
			CS3: If the child pointers (bounding boxes) -> choose reactangle that needs least
					overlap enlargment to fit the new Point/bounding rectangle if tie return smallest area
					i.e. the rectangle that has the least overlap -> tbh I'm not sure we can just leave this
				Else: 
					If not child pointers (bounding boxes) -> choose reactangle that needs least
					overlap enlargment to fit the new Point (same as before) if tie return smallest area (same)
			
		*/

		// CL1 [Initialize]
		Node *node = this;

		for (;;)
		{
			// CL2 [Leaf check]
			if (node->children.size() == 0)
			{
				return node;
			}
			// our children point to leaves
			else if (node->children[0]->children.size() == 0)
			{
				// Choose the entry in N whose rectangle needs least overlap enlargement 
				unsigned smallestOverlapExpansionIndex = 0;
				float smallestOverlapExpansion = computeOverlapGrowth(0, node->boundingBoxes, givenPoint);
				float smallestExpansionArea = node->boundingBoxes[0].computeExpansionArea(givenPoint);
				float smallestArea = node->boundingBoxes[0].area();

				// float smallestOverlapExpansion = node->boundingBoxes[0].computeExpansionArea(givenPoint);
				for (unsigned i = 0; i < node->boundingBoxes.size(); ++i)
				{
					float testOverlapExpansionArea = computeOverlapGrowth(i, node->boundingBoxes, givenPoint);
					if (smallestOverlapExpansion > testOverlapExpansionArea)
					{
						smallestOverlapExpansionIndex = i;
						smallestOverlapExpansion = testOverlapExpansionArea;
					} 
					else if (smallestOverlapExpansion == testOverlapExpansionArea)
					{
						// Use expansion area to break tie
						float testExpansionArea = node->boundingBoxes[i].computeExpansionArea(givenPoint);
						if (smallestExpansionArea > testExpansionArea)
						{
							smallestOverlapExpansionIndex = i;
							smallestExpansionArea = testExpansionArea;
						}
						else if  (smallestExpansionArea > testExpansionArea)
						{
							// use area to break tie
							float testArea = node->boundingBoxes[i].area();
							if (smallestArea > testArea)
							{
								smallestOverlapExpansionIndex = i;
								smallestArea = testArea;
							}
						}
					}
				}

				// Return Node since this is now a leaf
				return node->children[smallestOverlapExpansionIndex];;
			}
			else
			{
				// CL2 [Choose subtree]
				// Find the bounding box with least required expansion/overlap?
				// TODO: Break ties by using smallest area
				unsigned smallestExpansionIndex = 0;
				float smallestExpansionArea = node->boundingBoxes[0].computeExpansionArea(givenPoint);
				for (unsigned i = 0; i < node->boundingBoxes.size(); ++i)
				{
					float testExpansionArea = node->boundingBoxes[i].computeExpansionArea(givenPoint);
					if (smallestExpansionArea > testExpansionArea)
					{
						smallestExpansionIndex = i;
						smallestExpansionArea = testExpansionArea;
					}
				}

				// CL3 [Descend until a leaf is reached]
				node = node->children[smallestExpansionIndex];
			}
		}
	}

	// Always called on root, this = root
	Node *Node::chooseNode(ReinsertionEntry e)
	{
		// CL1 [Initialize]
		Node *node = this;

		for (;;)
		{
			// CL2 [Leaf check]
			if (node->children.size() == 0)
			{
				for (unsigned i = 0; i < e.level; ++i)
				{
					node = node->parent;
				}

				return node;
			}
			// our children point to leaves - TODO: I am not sure if this is the best way to check for
			// 		children since not all point to children? I should be fine since tree should be balenced
			else if (node->children[0]->children.size() == 0)
			{
				// Choose the entry in N whose rectangle needs least overlap enlargement 
				unsigned smallestOverlapExpansionIndex = 0;
				float smallestOverlapExpansion = computeOverlapGrowth(0, node->boundingBoxes, e.boundingBox);
				float smallestExpansionArea = node->boundingBoxes[0].computeExpansionArea(e.boundingBox);
				float smallestArea = node->boundingBoxes[0].area();

				// find smallest overlap 
				for (unsigned i = 0; i < node->boundingBoxes.size(); ++i)
				{
					float testOverlapExpansionArea = computeOverlapGrowth(i, node->boundingBoxes, e.boundingBox);
					if (smallestOverlapExpansion > testOverlapExpansionArea)
					{
						smallestOverlapExpansionIndex = i;
						smallestOverlapExpansion = testOverlapExpansionArea;
					} 
					else if (smallestOverlapExpansion == testOverlapExpansionArea)
					{
						// Use expansion area to break tie
						float testExpansionArea = node->boundingBoxes[i].computeExpansionArea(e.boundingBox);
						if (smallestExpansionArea > testExpansionArea)
						{
							smallestOverlapExpansionIndex = i;
							smallestExpansionArea = testExpansionArea;
						}
						else if  (smallestExpansionArea > testExpansionArea)
						{
							// use area to break tie
							float testArea = node->boundingBoxes[i].area();
							if (smallestArea > testArea)
							{
								smallestOverlapExpansionIndex = i;
								smallestArea = testArea;
							}
						}
					}
				}

				// we continue down to next level to properly do the leaf check
				node = node->children[smallestOverlapExpansionIndex];;
			}
			else
			{
				// CL2 [Choose subtree]
				// Find the bounding box with least required expansion/overlap?
				// TODO: Break ties by using smallest area
				unsigned smallestExpansionIndex = 0;
				float smallestExpansionArea = node->boundingBoxes[0].computeExpansionArea(e.boundingBox);
				for (unsigned i = 0; i < node->boundingBoxes.size(); ++i)
				{
					float testExpansionArea = node->boundingBoxes[i].computeExpansionArea(e.boundingBox);
					if (smallestExpansionArea > testExpansionArea)
					{
						smallestExpansionIndex = i;
						smallestExpansionArea = testExpansionArea;
					}
				}

				// CL3 [Descend until a leaf is reached]
				node = node->children[smallestExpansionIndex];
			}
		}
	}

	// TODO: Optimize
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

			if (currentContext->children.size() == 0)
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
				for (unsigned i = 0; i < currentContext->boundingBoxes.size(); ++i)
				{
					if (currentContext->boundingBoxes[i].containsPoint(givenPoint))
					{
						// Add the child to the nodes we will consider
						context.push(currentContext->children[i]);
					}
				}
			}
		}

		return nullptr;
	}

	unsigned int Node::computeTotalMarginSumOfBoundingBoxes()
	{

		// use the data to find total sum of all M - 2m +2 possible distributions

		// first split the vector into vectorA and vectorB with vectorA being the minimum
		std::vector<Rectangle>::const_iterator groupABegin = boundingBoxes.begin();
		std::vector<Rectangle>::const_iterator groupAEnd = boundingBoxes.begin() + minBranchFactor;
		std::vector<Rectangle>::const_iterator groupBBegin = boundingBoxes.begin() + minBranchFactor;
		std::vector<Rectangle>::const_iterator groupBEnd = boundingBoxes.end();

		std::vector<Rectangle> groupA(groupABegin, groupAEnd);
		std::vector<Rectangle> groupB(groupBBegin, groupBEnd);

		// Find the best size out of all the distributions
		unsigned int sumOfAllMarginValues = 0;

		// will test all M - 2m + 2 possible distributions
		while (groupA.size() <= maxBranchFactor && groupB.size() >= minBranchFactor)
		{
			// compute the margin of groupA and groupB
			Rectangle boundingBoxA(groupA[0]); // we do this so some values exist
			for (unsigned int i = 1; i < groupA.size(); i += 1)
			{
				boundingBoxA.expand(groupA[i]);
			}

			Rectangle boundingBoxB(groupB[0]);
			for (unsigned int i = 1; i < groupB.size(); i += 1)
			{
				boundingBoxB.expand(groupB[i]);
			}

			sumOfAllMarginValues += boundingBoxA.margin() + boundingBoxB.margin();

			
			// Add one new value to groupA and remove one from groupB to get to next distribution
			Rectangle transfer = groupB.front();
			groupB.erase(groupB.begin());
			groupA.push_back(transfer);
		}

		return sumOfAllMarginValues;

	}

	std::vector<std::vector<unsigned int>> Node::chooseSplitIndexByRectangle(unsigned int axis)
	{
		// split the sorted by axis boundingBoxes array into the ideal split
		std::vector<Rectangle>::const_iterator groupABegin = boundingBoxes.begin();
		std::vector<Rectangle>::const_iterator groupAEnd = boundingBoxes.begin() + minBranchFactor;
		std::vector<Rectangle>::const_iterator groupBBegin = boundingBoxes.begin() + minBranchFactor;
		std::vector<Rectangle>::const_iterator groupBEnd = boundingBoxes.end();

		std::vector<Rectangle> groupA(groupABegin, groupAEnd);
		std::vector<unsigned int> groupAIndices(groupA.size());
		std::iota(groupAIndices.begin(), groupAIndices.end(), 0);

		std::vector<Rectangle> groupB(groupBBegin, groupBEnd);
		std::vector<unsigned int> groupBIndices(groupB.size());
		std::iota(groupBIndices.begin(), groupBIndices.end(), minBranchFactor);

		// return value
		std::vector<std::vector<unsigned int>> groups;

		// Find the best size out of all the distributions
		unsigned int minOverlapValue = std::numeric_limits<unsigned int>::max();
		unsigned int minAreaValue = std::numeric_limits<unsigned int>::max();

		// will test all M - 2m + 2 possible groupings
		while (groupA.size() <= maxBranchFactor && groupB.size() >= minBranchFactor)
		{
			// compute the margin of groupA and groupB
			Rectangle boundingBoxA(groupA[0]);
			for (unsigned int i = 0; i < groupA.size(); i += 1)
			{
				boundingBoxA.expand(groupA[i]);
			}

			Rectangle boundingBoxB(groupB[0]);
			for (unsigned int i = 0; i < groupB.size(); i += 1)
			{
				boundingBoxB.expand(groupB[i]);
			}

			unsigned int currDistOverlapVal = boundingBoxA.computeIntersectionArea(boundingBoxB);

			if (currDistOverlapVal < minOverlapValue) {
				// we save this current distribution of indices to return
				minOverlapValue = currDistOverlapVal;
				groups.clear();
				groups.push_back(groupAIndices);
				groups.push_back(groupBIndices);

			} else if (currDistOverlapVal == minOverlapValue) {
				// if tied we use the minimum total value or bounding boxes to decide on grouping
				unsigned int currMinAreaVal = boundingBoxA.area() + boundingBoxB.area();

				if (currMinAreaVal < minAreaValue) {
					// we save this current distribution of indices to return
					minAreaValue = currMinAreaVal;
					groups.clear();
					groups.push_back(groupAIndices);
					groups.push_back(groupBIndices);
				}
			}
			
			// Add one new value to groupA and remove one from groupB to get a new grouping
			Rectangle transferRectangle = groupB.front();
			unsigned int transferIndex = groupBIndices.front();
			groupB.erase(groupB.begin());
			groupBIndices.erase(groupBIndices.begin());
			groupA.push_back(transferRectangle);
			groupAIndices.push_back(transferIndex);
		}

		return groups;
	}

	unsigned int Node::splitAxis(Node *newChild)
	{
		// find optimal index for grouping the bouding boxes by using total margin sums
		unsigned int optimalAxis = 0;
		
		// For now we will say there is only 2 axis; however, we can set up geometry.h to include an axis type
		// eventually we can make this a loop to work with multi dimensional data
		// Sort along x axis
		std::sort(boundingBoxes.begin(), boundingBoxes.end(), Node::sortByXRectangleFirst());
		unsigned int marginsFromAxisX = computeTotalMarginSumOfBoundingBoxes();

		// Sort along y axis
		std::sort(boundingBoxes.begin(), boundingBoxes.end(), Node::sortByYRectangleFirst());
		unsigned int marginsFromAxisY = computeTotalMarginSumOfBoundingBoxes();

		if (marginsFromAxisX < marginsFromAxisY) {
			// X axis is better
			// resort boundingBoxes
			std::sort(boundingBoxes.begin(), boundingBoxes.end(), Node::sortByXRectangleFirst());
			optimalAxis = 0;
		} else  {
			// Y axis is better
			optimalAxis = 1;
		}

		return optimalAxis;
	}

	Node *Node::splitNode(Node *newChild)
	{
		/*
			S1: Call splitAxis to determine the axis in which the split of the boundingBoxes will be performed
			S2: Invoke chooseSplitIndex given the access to determine the best distribution along a sorting on this axis
			S3: Distribute the entries among these two groups
		*/

		STATSPLIT();

		float dataSize = data.size();

		// We first insert the point into the data to do computations
		children.push_back(newChild);
		boundingBoxes.push_back(newChild->boundingBox());
		newChild->parent = this; //	I hope we didn't do this already

		// Call splitAxis to determine the axis perpendicular to which the split is perfromed
		// 	For now we will save the axis as a int -> since this allows for room for growth in the future
		unsigned int axis = splitAxis(newChild);

		// Call ChooseSplitIndex to create a grouping of the vaues
		std::vector<std::vector<unsigned>> groups = chooseSplitIndexByRectangle(axis);

		// Take the two groups and modify the actual node
		// Setup the two groups which will be the entries in the two new nodes
		std::vector<unsigned> groupA = groups.at(0);
		std::vector<unsigned> groupB = groups.at(1);

		// Given split indices, Create the new node and fill it with groupB entries by doing complicated stuff
		Node *newSibling = new Node(minBranchFactor, maxBranchFactor, parent);
		unsigned groupASize = groupA.size();
		unsigned groupALastIndex = groupASize - 1;
		unsigned iGroupB;
		for (unsigned i = 0; i < groupB.size(); ++i)
		{
			iGroupB = groupB[i];
			children[iGroupB]->parent = newSibling;	// this may include our new child so its parent is reset from this
			newSibling->boundingBoxes.push_back(boundingBoxes[iGroupB]);
			newSibling->children.push_back(children[iGroupB]);

			boundingBoxes[iGroupB] = boundingBoxes[groupA[groupALastIndex]];
			children[iGroupB] = children[groupA[groupALastIndex]];

			groupALastIndex = groupALastIndex == 0 ? 0 : groupALastIndex - 1;
		}
		boundingBoxes.resize(groupASize);
		children.resize(groupASize);

		// Add newChild which caused this split in the first place
		Rectangle newBox = newChild->boundingBox();

		// Return our newly minted sibling
		return newSibling;
	}

	/*
		Helper function that takes a pre-sorted data and then computes the sum
		of all margin values over all possible M - 2m + 2 distributions
	*/
	unsigned int Node::computeTotalMarginSum()
	{
		// use the data to find possible matches (1, M - 2m +2) possible distributions
		// first split the vector into vectorA and vectorB with vectorA being the minimum
		std::vector<Point>::const_iterator groupABegin = data.begin();
		std::vector<Point>::const_iterator groupAEnd = data.begin() + minBranchFactor;
		std::vector<Point>::const_iterator groupBBegin = data.begin() + minBranchFactor;
		std::vector<Point>::const_iterator groupBEnd = data.end();

		std::vector<Point> groupA(groupABegin, groupAEnd);
		std::vector<Point> groupB(groupBBegin, groupBEnd);

		// Find the total margin sum size out of all the distributions along given axis
		unsigned int sumOfAllMarginValues = 0;

		// will test all M - 2m + 2 groupings
		while (groupA.size() <= maxBranchFactor && groupB.size() >= minBranchFactor)
		{
			Rectangle boundingBoxA(groupA[0], groupA[0]); // we do this so some values exist
			for (unsigned int i = 1; i < groupA.size(); i += 1)
			{
				boundingBoxA.expand(groupA[i]);
			}

			Rectangle boundingBoxB(groupB[0], groupB[0]);
			for (unsigned int i = 1; i < groupB.size(); i += 1)
			{
				boundingBoxB.expand(groupB[i]);
			}

			// Calculate new margin sum
			sumOfAllMarginValues += boundingBoxA.margin() + boundingBoxB.margin();
			
			// Add one new value to groupA and remove one from groupB
			// ok that's the next step and repeat
			Point transfer = groupB.front();
			groupB.erase(groupB.begin());
			groupA.push_back(transfer);
		}

		return sumOfAllMarginValues;
	}

	
	/*
		CSA1: Sort entries by lower and upper bound along each axis and compute S -> sum of all margin values for the different distributions
			This can be stored in a array of variable that we keep in a loop -> and the just compare to the others?
			// we can first call a helper function that returns an array of all possible distributions for it?
		CSA2: Return the Axis that has the minimum total sum of all the distributions
	*/
	unsigned int Node::splitAxis(Point newData)
	{
		unsigned int optimalAxis = 0;

		// Note we first insert the point into the data to do computations
		data.push_back(newData);
		
		// For now we will say there is only 2 axis; however, we can set up geometry.h to include an axis type
		// eventually we can make this a loop to work with multi dimensional data
		// Sort along x axis
		std::sort(data.begin(), data.end(), Node::sortByXFirst());
		unsigned int marginsFromAxisX = computeTotalMarginSum();

		// Sort along y axis
		std::sort(data.begin(), data.end(), Node::sortByYFirst());
		unsigned int marginsFromAxisY = computeTotalMarginSum();


		if (marginsFromAxisX < marginsFromAxisY) {
			// X axis is better and resort data
			std::sort(data.begin(), data.end(), Node::sortByXFirst());
			optimalAxis = 0;
		} else  {
			// Y axis is better
			optimalAxis = 1;
		}

		return optimalAxis;
	}


	// Implement ChooseSplitIndex here
	/*
		CSI1: Given the chosen split index
			group all the entries into multiple groups and choose the one that has the least
			overlap value; resolve ties with the minimum area
			returns tuple of best distribution group indices
	*/
	std::vector<std::vector<unsigned int>> Node::chooseSplitIndex(unsigned int axis)
	{
		std::vector<Point>::const_iterator groupABegin = data.begin();
		std::vector<Point>::const_iterator groupAEnd = data.begin() + minBranchFactor;
		std::vector<Point>::const_iterator groupBBegin = data.begin() + minBranchFactor;
		std::vector<Point>::const_iterator groupBEnd = data.end();

		std::vector<Point> groupA(groupABegin, groupAEnd);
		std::vector<unsigned int> groupAIndices(groupA.size());
		std::iota(groupAIndices.begin(), groupAIndices.end(), 0);

		std::vector<Point> groupB(groupBBegin, groupBEnd);
		std::vector<unsigned int> groupBIndices(groupB.size());
		std::iota(groupBIndices.begin(), groupBIndices.end(), minBranchFactor);

		// return value
		std::vector<std::vector<unsigned int>> groups;

		// Find the best size out of all the distributions
		unsigned int minOverlapValue = std::numeric_limits<unsigned int>::max();
		unsigned int minAreaValue = std::numeric_limits<unsigned int>::max();


		// this will try each of the M-2m + 2 groups
		while (groupA.size() <= maxBranchFactor && groupB.size() >= minBranchFactor)
		{
			// compute the margin of groupA and groupB
			Rectangle boundingBoxA(groupA[0], groupA[0]);
			for (unsigned int i = 1; i < groupA.size(); i += 1)
			{
				boundingBoxA.expand(groupA[i]);
			}

			Rectangle boundingBoxB(groupB[0], groupB[0]);
			for (unsigned int i = 1; i < groupB.size(); i += 1)
			{
				boundingBoxB.expand(groupB[i]);
			}

			// Computer intersection area to determine best grouping of data points
			unsigned int currDistOverlapVal = boundingBoxA.computeIntersectionArea(boundingBoxB);

			if (currDistOverlapVal < minOverlapValue) {
				// we save this current distribution of indices to return
				minOverlapValue = currDistOverlapVal;
				groups.clear();
				groups.push_back(groupAIndices);
				groups.push_back(groupBIndices);

			} else if (currDistOverlapVal == minOverlapValue) {
				// if overlap is equal, we use the distribution that creates the smallest areas
				unsigned int currMinAreaVal = boundingBoxA.area() + boundingBoxB.area();

				if (currMinAreaVal < minAreaValue) {
					// we save this current distribution of indices to return
					minAreaValue = currMinAreaVal;
					groups.clear();
					groups.push_back(groupAIndices);
					groups.push_back(groupBIndices);
				}
			}
			
			// Add one new value to groupA and remove one from groupB to obtain next distribution
			Point transferPoint = groupB.front();
			unsigned int transferIndex = groupBIndices.front();
			groupB.erase(groupB.begin());
			groupBIndices.erase(groupBIndices.begin());
			groupA.push_back(transferPoint);
			groupAIndices.push_back(transferIndex);
		}

		return groups;
	}


	Node *Node::splitNode(Point newData)
	{
		/*
			S1: Call splitAxis to determine the axis perpendicular to which the split is performed
			S2: Invoke chooseSplitIndex given the access to determine the best distribution along this axis
			S3: Distribute the entries among these two groups
		*/

		STATSPLIT();

		float dataSize = data.size();

		// Call splitAxis to determine the axis perpendicular to which the split is perfromed
		// 	For now we will save the axis as a int -> since this allows for room for growth in the future
		unsigned int axis = splitAxis(newData);

		// Call ChooseSplitIndex to create optimal splitting of data array
		std::vector<std::vector<unsigned>> groups = chooseSplitIndex(axis);

		// Take the two groups and modify the actual node
		// Setup the two groups which will be the entries in the two new nodes
		std::vector<unsigned> groupA = groups.at(0);
		std::vector<unsigned> groupB = groups.at(1);

		// Create the new node and fill it with groupB entries by doing really complicated stuff
		Node *newSibling = new Node(minBranchFactor, maxBranchFactor, parent);
		unsigned groupALastIndex = groupA.size() - 1;
		unsigned iGroupB;
		for (unsigned i = 0; i < groupB.size(); ++i)
		{
			iGroupB = groupB[i];
			newSibling->data.push_back(data[iGroupB]);
			data[iGroupB] = data[groupA[groupALastIndex]];
			groupALastIndex = groupALastIndex == 0 ? 0 : groupALastIndex - 1;
		}
		data.resize(groupA.size());

		// Return our newly minted sibling
		return newSibling;
	}


	Node *Node::overflowTreatment(Node *node, Node *nodeToInsert, std::vector<bool> hasReinsertedOnLevel)
	{
		if (hasReinsertedOnLevel.at(node->level)) {
			return node->splitNode(nodeToInsert); // this is right -> we call splitNode on the new child
		} else {
			hasReinsertedOnLevel.at(node->level) = true;
			ReinsertionEntry e{};
			e.boundingBox = nodeToInsert->boundingBox();
			e.child = nodeToInsert;
			e.level = node->level;
			return node->reInsert(e, hasReinsertedOnLevel); // ok yeaa that's right -> that's fine since it's ALWAYS just a point
		}
		
	}

	Node *Node::adjustTree(Node *sibling, std::vector<bool> hasReinsertedOnLevel)
	{
		// AT1 [Initialize]
		Node *node = this;
		Node *siblingNode = sibling;

		for (;;)
		{
			// AT2 [If node is the root, stop]
			if (node->parent == nullptr)
			{
				break;
			}
			else
			{
				// AT3 [Adjust covering rectangle in parent entry]
				node->parent->updateBoundingBox(node, node->boundingBox());

				// If we have a split then deal with it otherwise move up the tree
				if (siblingNode != nullptr)
				{
					// AT4 [Propogate the node split upwards]
					if (node->parent->children.size() < node->parent->maxBranchFactor)
					{
						node->parent->boundingBoxes.push_back(siblingNode->boundingBox());
						node->parent->children.push_back(siblingNode);
						siblingNode->parent = node->parent;

						node = node->parent;
						siblingNode = nullptr;
					}
					else
					{
						Node *siblingParent = overflowTreatment(node->parent, siblingNode, hasReinsertedOnLevel);
						node = node->parent;
						siblingNode = siblingParent;
					}
				}
				else
				{
					// AT5 [Move up to next level]
					node = node->parent;
				}
			}
		}

		return siblingNode;
	}

	/*
		Reinsert leaf node values based off their distance to the center node
	*/
	Node *Node::reInsert(Point givenPoint, std::vector<bool> hasReinsertedOnLevel)
	{
		// reInsert points in data given the addition of givenPoint

		// 1. RI1 Compute distance between each of the boundBoxes.ceter and the 
		//		gloabl bounding box -> parent's bounding box at that index

		struct sortByDistanceToBoundingBoxMidpoint {
			Point globalCenterPoint;

			sortByDistanceToBoundingBoxMidpoint(Point globalCenterPoint): globalCenterPoint(globalCenterPoint) {}
			inline bool operator() (const Point& pointA, const Point& pointB)
			{
				return (pointA.distance(globalCenterPoint) > pointB.distance(globalCenterPoint));
			}
		};

		// 2. RI2 Sort the entries by DECREASING index -> ok let's define an
		// 		extra helper function that gets to do this and pass it into sort
		std::sort(data.begin(), data.end(), sortByDistanceToBoundingBoxMidpoint(boundingBox().centerPoint()));

		// 3. RI3 Remove the first p entries from N and adjust the bounding box -> OK so we need to adjust the data model
		//		to include a specified "p" value -> this should be unique to the node -> so it's a node variable
		unsigned int nodesToReinsert = p * boundingBoxes.size();

		// 4. Insert the removed entries -> OK we can also specify a flag that is
		//		if you want to reinsert starting with largest values (i.e. start at index 0) or closest values (Start at index p)
		for (unsigned int i = 0; i < nodesToReinsert; i += 1)
		{
			// For now we are working with deleting from the largest values
			// Remove from bounding boxes
			Point pointToReinsert = data.at(0);
			boundingBoxes.erase(boundingBoxes.begin());
			data.erase(data.begin());
			insert(pointToReinsert, hasReinsertedOnLevel); // reinsert point
		}

		return nullptr;
	}

	Node *Node::reInsert(ReinsertionEntry e, std::vector<bool> hasReinsertedOnLevel) {
		// reinsert children of node that has overflowed

		// 1. RI1 Compute distance between each of the boundBoxes.ceter and the 
		//		gloabl bounding box -> parent's bounding box at that index

		struct sortByRectangleMidpoints{
			Point globalCenterPoint;

			sortByRectangleMidpoints(Point globalCenterPoint): globalCenterPoint(globalCenterPoint) {}
			inline bool operator() (const Rectangle& boundingBoxA, const Rectangle& boundingBoxB)
			{
				return (boundingBoxA.centerPoint().distance(globalCenterPoint) > boundingBoxB.centerPoint().distance(globalCenterPoint));
			}

		};

		// temporarily add the newly inserted child's boundingBox into your own
		boundingBoxes.push_back(e.child->boundingBox());
		children.push_back(e.child);
		e.child->parent = this;
		e.child->level = this->level + 1; // temproarily set new level

		// 2. RI2 Sort the entries by DECREASING index -> ok let's define an
		// 		extra helper function that gets to do this and pass it into sort
		std::sort(boundingBoxes.begin(), boundingBoxes.end(), sortByRectangleMidpoints(boundingBox().centerPoint()));

		// 3. RI3 Remove the first p entries from N and adjust the bounding box -> OK so we need to adjust the data model
		//		to include a specified "p" value -> this should be unique to the node -> so it's a node variable
		unsigned int nodesToReinsert = p * boundingBoxes.size();

		// 4. Insert the removed entries -> OK we can also specify a flag that is
		//		if you want to reinsert starting with largest values (i.e. start at index 0) or closest values (Start at index p)
		for (unsigned int i = 0; i < nodesToReinsert; i += 1)
		{
			// For now we are working with deleting from the largest values
			// Remove from bounding boxes
			boundingBoxes.erase(boundingBoxes.begin());
			children.erase(children.begin());

			ReinsertionEntry e = {};
			e.boundingBox = boundingBoxes[i];
			e.child = children[i];
			e.level = level;
			insert(e, hasReinsertedOnLevel); // reinsert child
		}

		return nullptr;
	}

	
	/*
		overflow treatement for dealing with a leaf node where givenPoint will cause data to overflow
			but let's keep it here for now
	*/
	Node *Node::overflowTreatment(Node *leaf, Point givenPoint, std::vector<bool> hasReinsertedOnLevel)
	{
		// if not root and we have already done a forced reinsert on this level we split the node to deal with overflow
		if (hasReinsertedOnLevel.size() == 0 || hasReinsertedOnLevel.at(hasReinsertedOnLevel.size() - 1))
		{
			return leaf->splitNode(givenPoint);
		}
		else	// this is our first time overflowing this level and we do a forced reInsert
		{
			hasReinsertedOnLevel.at(hasReinsertedOnLevel.size() - 1) = true;
			return leaf->reInsert(givenPoint, hasReinsertedOnLevel);
		}
		
	}

	/*
		overflow treatement for reinserting an entry/subtree, e, to node
		TODO: we need to modify this to acutally pass in a level
	*/
	Node *Node::overflowTreatment(Node *node, ReinsertionEntry e, std::vector<bool> hasReinsertedOnLevel)
	{
		// if not root and we have already done a forced reinsert on this level we split the node to deal with overflow
		if (hasReinsertedOnLevel.at(node->level))
		{
			return node->splitNode(e.child);
		}
		else	// this is our first time overflowing this level and we do a forced reInsert
		{
			hasReinsertedOnLevel.at(node->level) = true;
			return node->reInsert(e, hasReinsertedOnLevel);
		}
	}

	// Always called on root, this = root
	Node *Node::insert(Point givenPoint, std::vector<bool> hasReinsertedOnLevel)
	{
		STATEXEC(stat());

		// I1 [Find position for new record]
		Node *leaf = chooseLeaf(givenPoint); // this is now "chooseSubtree" we can rename -> tho tbh it's kinda useless to rename imo.
		Node *siblingLeaf = nullptr;

		// I2 [Add record to leaf node]
		if (leaf->data.size() < leaf->maxBranchFactor)
		{
			leaf->data.push_back(givenPoint);
		}
		else
		{
			// we call overflow treatment to determine how our sibling node is treated
			// if we do forced reInsert siblingLeaf if nullptr and is properly dealt with in adjustTree
			siblingLeaf = overflowTreatment(leaf, givenPoint, hasReinsertedOnLevel);
		}

		// I3 [Propogate changes upward]
		Node *siblingNode = leaf->adjustTree(siblingLeaf, hasReinsertedOnLevel);

		// I4 [Grow tree taller]
		if (siblingNode != nullptr)
		{
			Node *newRoot = new Node(minBranchFactor, maxBranchFactor);

			this->parent = newRoot;
			newRoot->boundingBoxes.push_back(this->boundingBox());
			newRoot->children.push_back(this);

			siblingNode->parent = newRoot;
			newRoot->boundingBoxes.push_back(siblingNode->boundingBox());
			newRoot->children.push_back(siblingNode);

			// fix the reinserted length
			hasReinsertedOnLevel.push_back(false);
			return newRoot;
		}
		else
		{
			return this;
		}
	}

	// Attempt to insert subtree e back into the tree.
	Node *Node::insert(ReinsertionEntry e, std::vector<bool> hasReinsertedOnLevel)
	{
		// If reinserting a leaf then use normal insert - this should mimick basically the exact same insert as before
		if (e.level == 0)
		{
			return insert(e.data, hasReinsertedOnLevel);
		}

		// I1 [Find position for new record]
		Node *node = chooseNode(e);
		Node *siblingNode = nullptr;

		// I2 [Add record to node]
		if (node->children.size() < node->maxBranchFactor)
		{
			e.child->parent = node;
			e.child->level = node->level + 1;
			node->boundingBoxes.push_back(e.boundingBox);
			node->children.push_back(e.child);
		}
		else
		{
			// use overflowTreatement to decide if we must do a reinsert or a splitNode on this level
			siblingNode = overflowTreatment(node, e, hasReinsertedOnLevel); // this is fine
		}

		// I3 [Propogate changes upward]
		siblingNode = node->adjustTree(siblingNode, hasReinsertedOnLevel);

		// I4 [Grow tree taller]
		if (siblingNode != nullptr)
		{
			Node *newRoot = new Node(minBranchFactor, maxBranchFactor);

			this->parent = newRoot;
			newRoot->boundingBoxes.push_back(this->boundingBox());
			newRoot->children.push_back(this);

			siblingNode->parent = newRoot;
			newRoot->boundingBoxes.push_back(siblingNode->boundingBox());
			newRoot->children.push_back(siblingNode);

			return newRoot;
		}
		else
		{
			return this;
		}
	}

	// To be called on a leaf
	Node *Node::condenseTree(std::vector<bool> hasReinsertedOnLevel)
	{
		// CT1 [Initialize]
		Node *node = this;
		unsigned level = 0;

		std::vector<ReinsertionEntry> Q;

		// CT2 [Find parent entry]
		unsigned nodeBoundingBoxesSize, nodeDataSize;
		while (node->parent != nullptr)
		{
			nodeBoundingBoxesSize = node->boundingBoxes.size();
			nodeDataSize = node->data.size();
			// CT3 & CT4 [Eliminate under-full node. & Adjust covering rectangle.]
			if (nodeBoundingBoxesSize >= node->minBranchFactor || nodeDataSize >= node->minBranchFactor)
			{
				STATSHRINK();

				node->parent->updateBoundingBox(node, node->boundingBox());

				// CT5 [Move up one level in the tree]
				// Move up a level without deleting ourselves
				node = node->parent;
				level++;
			}
			else
			{
				STATSHRINK();

				// Remove ourselves from our parent
				node->parent->removeChild(node);

				// Add a reinsertion entry for each data point or branch of this node
				for (unsigned i = 0; i < nodeDataSize; ++i)
				{
					ReinsertionEntry e = {};
					e.child = nullptr;
					e.data = node->data[i];
					e.level = 0;
					Q.push_back(e);
				}
				for (unsigned i = 0; i < nodeBoundingBoxesSize; ++i)
				{
					ReinsertionEntry e = {};
					e.boundingBox = node->boundingBoxes[i];
					e.child = node->children[i];
					e.level = level;
					Q.push_back(e);
				}

				// Prepare for garbage collection
				Node *garbage = node;

				// CT5 [Move up one level in the tree]
				// Move up a level before deleting ourselves
				node = node->parent;
				level++;

				// Cleanup ourselves without deleting children b/c they will be reinserted
				delete garbage;
			}
		}

		// CT6 [Re-insert oprhaned entries]
		for (unsigned i = 0; i < Q.size(); ++i)
		{
			node = node->insert(Q[i], hasReinsertedOnLevel);
		}

		return node;
	}

	// Always called on root, this = root
	Node *Node::remove(Point givenPoint, std::vector<bool> hasReinsertedOnLevel)
	{
		STATEXEC(stat());

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
		if (root->children.size() == 1)
		{
			// slice the hasReinsertedOnLevel
			// we are removing the root to shorten the tree so we then decide to remove the root
			hasReinsertedOnLevel.erase(hasReinsertedOnLevel.begin());
			root->children[0]->parent = nullptr;
			return root->children[0];
		}
		else
		{
			return root;
		}
	}

	void Node::print(unsigned n)
	{
		std::string indendtation(n * 4, ' ');
		std::cout << indendtation << "Node " << (void *)this << std::endl;
		std::cout << indendtation << "{" << std::endl;
		std::cout << indendtation << "    Parent: " << (void *)parent << std::endl;
		std::cout << indendtation << "    Bounding Boxes: " << std::endl;
		for (unsigned i = 0; i < boundingBoxes.size(); ++i)
		{
			std::cout << indendtation << "		" << boundingBoxes[i] << std::endl;
		}
		std::cout << std::endl << indendtation << "    Children: ";
		for (unsigned i = 0; i < children.size(); ++i)
		{
			std::cout << (void *)children[i] << ' ';
		}
		std::cout << std::endl << indendtation << "    Data: ";
		for (unsigned i = 0; i < data.size(); ++i)
		{
			std::cout << data[i];
		}
		std::cout << std::endl << indendtation << "}" << std::endl;
	}

	void Node::printTree(unsigned n)
	{
		// Print this node first
		print(n);

		// Print any of our children with one more level of indentation
		if (children.size() > 0)
		{
			for (unsigned i = 0; i < boundingBoxes.size(); ++i)
			{
				// Recurse
				children[i]->printTree(n + 1);
			}
		}
	}

	unsigned Node::checksum()
	{
		unsigned sum = 0;

		if (children.size() == 0)
		{
			for (unsigned i = 0; i < data.size(); ++i)
			{
				sum += (unsigned)data[i].x;
				sum += (unsigned)data[i].y;
			}
		}
		else
		{
			for (unsigned i = 0; i < boundingBoxes.size(); ++i)
			{
				// Recurse
				sum += children[i]->checksum();
			}
		}

		return sum;
	}

	unsigned Node::height()
	{
		unsigned ret = 0;
		Node *node = this;

		for (;;)
		{
			ret++;
			if (node->children.size() == 0)
			{
				return ret;
			}
			else
			{
				node = node->children[0];
			}
		}
	}

	void Node::stat()
	{
		STATHEIGHT(height());

		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;
		size_t memoryFootprint = 0;

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();

			if (currentContext->children.size() == 0)
			{
				STATBRANCH(currentContext->data.size());
				memoryFootprint += sizeof(Node) + currentContext->data.size() * sizeof(Point);
			}
			else
			{
				STATBRANCH(currentContext->children.size());
				memoryFootprint += sizeof(Node) + currentContext->children.size() * sizeof(Node *) + currentContext->boundingBoxes.size() * sizeof(Rectangle);
				// Determine which branches we need to follow
				for (unsigned i = 0; i < currentContext->boundingBoxes.size(); ++i)
				{
					context.push(currentContext->children[i]);
				}
			}
		}

		STATMEM(memoryFootprint);
	}
}
