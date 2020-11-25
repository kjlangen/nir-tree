#include <rstartree/node.h>

namespace rstartree
{
	RStarTreeNode::RStarTreeNode()
	{
		minBranchFactor = 3;
		maxBranchFactor = 5;
		parent = nullptr;
		boundingBoxes.resize(0);
		children.resize(0);
		data.resize(0);
	}

	RStarTreeNode::RStarTreeNode(unsigned minBranchFactor, unsigned maxBranchFactor, RStarTreeNode *p)
	{
		this->minBranchFactor = minBranchFactor;
		this->maxBranchFactor = maxBranchFactor;
		this->parent = p;
		boundingBoxes.resize(0);
		children.resize(0);
		data.resize(0);
	}

	void RStarTreeNode::deleteSubtrees()
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

	Rectangle RStarTreeNode::boundingBox()
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
	void RStarTreeNode::updateBoundingBox(RStarTreeNode *child, Rectangle updatedBoundingBox)
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
	void RStarTreeNode::removeChild(RStarTreeNode *child)
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
	void RStarTreeNode::removeData(Point givenPoint)
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

	void RStarTreeNode::exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator)
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

	std::vector<Point> RStarTreeNode::search(Point &requestedPoint)
	{
		STATEXEC(stat());

		std::vector<Point> matchingPoints;

		// Initialize our context stack
		std::stack<RStarTreeNode *> context;
		context.push(this);
		RStarTreeNode *currentContext;

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

	std::vector<Point> RStarTreeNode::search(Rectangle &requestedRectangle)
	{
		STATEXEC(stat());

		std::vector<Point> matchingPoints;

		// Initialize our context stack
		std::stack<RStarTreeNode *> context;
		context.push(this);
		RStarTreeNode *currentContext;

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

	unsigned int RStarTreeNode::computeOverlapGrowth(unsigned int index, std::vector<Rectangle> boundingBoxes, Point givenPoint)
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
	RStarTreeNode *RStarTreeNode::chooseLeaf(Point givenPoint)
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
		RStarTreeNode *node = this;

		for (;;)
		{
			// CL2 [Leaf check]
			if (node->children.size() == 0)
			{
				return node;
			}
			// our children point to leaves - TODO: I am not sure if this is the best way to check for
			// 		children since not all point to children?
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
				// CL3 [Choose subtree]
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

				// CL4 [Descend until a leaf is reached] - TODO: I am unsure about this step
				node = node->children[smallestExpansionIndex];
			}
		}
	}

	// Always called on root, this = root
	RStarTreeNode *RStarTreeNode::chooseNode(ReinsertionEntry e)
	{
		// CL1 [Initialize]
		RStarTreeNode *node = this;

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
			else
			{
				// CL3 [Choose subtree]
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

				// CL4 [Descend until a leaf is reached]
				node = node->children[smallestExpansionIndex];
			}
		}
	}

	// TODO: Optimize
	RStarTreeNode *RStarTreeNode::findLeaf(Point givenPoint)
	{
		// Initialize our context stack
		std::stack<RStarTreeNode *> context;
		context.push(this);
		RStarTreeNode *currentContext;

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

	RStarTreeNode *RStarTreeNode::splitNode(RStarTreeNode *newChild)
	{
		/*
			I think this should be the same as the new splitNode -> I am unsure
			of why or else this needs to be called differently
			I think we should basically just copy base code except the newChild is a pointer intead of a passed Object
		*/
		STATSPLIT();

		unsigned boundingBoxesSize = boundingBoxes.size();

		// Setup the two groups which will be the entries in the two new nodes
		unsigned seedA = 0;
		std::vector<unsigned> groupA;

		unsigned seedB = boundingBoxesSize - 1;
		std::vector<unsigned> groupB;

		// Compute the first entry in each group based on PS1 & PS2
		float maxWasted = 0;
		Rectangle iBox, jBox;
		for (unsigned i = 0; i < boundingBoxesSize; ++i)
		{
			iBox = boundingBoxes[i];
			for (unsigned j = 0; j < boundingBoxesSize; ++j)
			{
				jBox = boundingBoxes[j];
				float xdist = iBox.lowerLeft.x - jBox.lowerLeft.x;
				float xdistPrime = iBox.upperRight.x - jBox.upperRight.x;
				float ydist = iBox.lowerLeft.y - jBox.lowerLeft.y;
				float ydistPrime = iBox.upperRight.y - jBox.upperRight.y;

				float wasted = (xdist * xdist + xdistPrime * xdistPrime + ydist * ydist + ydistPrime * ydistPrime) / 2;
				if (maxWasted < wasted)
				{
					maxWasted = wasted;

					seedA = i;
					seedB = j;
				}
			}
		}

		Rectangle boundingBoxA = boundingBoxes[seedA];
		Rectangle boundingBoxB = boundingBoxes[seedB];

		// Go through the remaining entries and add them to groupA or groupB
		for (unsigned i = 0; i < boundingBoxesSize; ++i)
		{
			if (i == seedA)
			{
				groupA.push_back(i);
				continue;
			}
			else if (i == seedB)
			{
				groupB.push_back(i);
				continue;
			}

			// Choose the group which will need to expand the least
			if (boundingBoxB.computeExpansionArea(boundingBoxes[i]) > boundingBoxA.computeExpansionArea(boundingBoxes[i]))
			{
				groupA.push_back(i);
				boundingBoxA.expand(boundingBoxes[i]);
			}
			else
			{
				groupB.push_back(i);
				boundingBoxB.expand(boundingBoxes[i]);
			}
		}

		// Create the new node and fill it with groupB entries by doing complicated stuff
		RStarTreeNode *newSibling = new RStarTreeNode(minBranchFactor, maxBranchFactor, parent);
		unsigned groupASize = groupA.size();
		unsigned groupALastIndex = groupASize - 1;
		unsigned iGroupB;
		for (unsigned i = 0; i < groupB.size(); ++i)
		{
			iGroupB = groupB[i];
			children[iGroupB]->parent = newSibling;
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

		// Choose the group which will need to expand the least
		if (boundingBoxB.computeExpansionArea(newBox) > boundingBoxA.computeExpansionArea(newBox))
		{
			newChild->parent = this;
			boundingBoxes.push_back(newBox);
			children.push_back(newChild);
		}
		else
		{
			newChild->parent = newSibling;
			newSibling->boundingBoxes.push_back(newBox);
			newSibling->children.push_back(newChild);
		}

		// Return our newly minted sibling
		return newSibling;
	}

	/*
		Helper function that takes a pre-sorted data and then computes the sum
		of all margin values over all possible M - 2m + 2 distributions
	*/
	unsigned int RStarTreeNode::computeTotalMarginSum()
	{
		// use the data to find possible matches (1, M - 2m +2) possible distributions
		// first split the vector into vectorA and vectorB with vectorA being the minimum
		std::vector<Point>::const_iterator groupABegin = data.begin();
		std::vector<Point>::const_iterator groupAEnd = data.begin() + minBranchFactor;
		std::vector<Point>::const_iterator groupBBegin = data.begin() + minBranchFactor + 1;
		std::vector<Point>::const_iterator groupBEnd = data.end();

		std::vector<Point> groupA(groupABegin, groupAEnd);
		std::vector<Point> groupB(groupBBegin, groupBEnd);

		// Find the best size out of all the distributions
		unsigned int sumOfAllMarginValues = 0;
		for (unsigned int offset = 1; offset < (maxBranchFactor - 2*minBranchFactor + 2); offset += 1)
		{
			// compute the margin of groupA and groupB -> track of the sum
			Rectangle boundingBoxA;
			for (unsigned int i = 0; i < groupA.size(); i += 1)
			{
				boundingBoxA.expand(groupA[i]);
			}

			Rectangle boundingBoxB;
			for (unsigned int i = 0; i < groupA.size(); i += 1)
			{
				boundingBoxB.expand(groupB[i]);
			}

			sumOfAllMarginValues += boundingBoxA.margin() + boundingBoxB.margin();

			
			// Add one new value to groupA and remove one from groupB
			// ok that's the next step and repeat
			Point transfer = groupA.back();
			groupA.pop_back();
			groupB.insert(groupB.begin(), transfer);
		}

		return sumOfAllMarginValues;
	}

	// NOTE follow up -> let's just change Point implementation here -> but I'm unsure of what changes for the type Reinsertion entry
	
	// Implement splitAxis here
	/*
		CSA1: Sort entries by lower and upper bound along each axis and compute S -> sum of all margin values for the different distributions
			This can be stored in a array of variable that we keep in a loop -> and the just compare to the others?
			// we can first call a helper function that returns an array of all possible distributions for it?
		CSA2: Return the Axis that has the minimum total sum of all the distributions
	*/


	unsigned int RStarTreeNode::splitAxis(Point newData)
	{
		unsigned int optimalAxis = 0;
		
		// For now we will say there is only 2 axis; however, we can set up geometry.h to include an axis type
		// eventually we can make this a loop to work with multi dimensional data
		// Sort along x axis
		std::sort(data.begin(), data.end(), RStarTreeNode::sortByXFirst());
		unsigned int marginsFromAxisX = computeTotalMarginSum();

		// Sort along y axis
		std::sort(data.begin(), data.end(), RStarTreeNode::sortByXFirst());
		unsigned int marginsFromAxisY = computeTotalMarginSum();

		if (marginsFromAxisX < marginsFromAxisY) {
			// X axis is better
			return 0;
		} else  {
			// Y axis is better
			return 1;
		}

		return optimalAxis;
	}


	// Implement ChooseSplitIndex here
	/*
		CSI1: Given the chosen split index -> basically perform chooseSubtree
			group all the entries into multiple groups and choose the one that has the least
			overlap value -> so I guess we'll need to make a helper that returns distributions?
			And resolve ties with the minimum area
			I guess this helper can just be tied with the first function and done along each axis
			the key is a tuple of the split
			The value is a tuple -> one containing the area of the split, one containing the overlap value and lastly
			one containing the margin value -> this can be shared by this and the one above.
	*/
	std::vector<std::vector<unsigned int>> RStarTreeNode::chooseSplitIndex(unsigned int axis)
	{
		// so this algorithm we are doing here will be repeated in the chooseSubtree part
		// as such we should think about making this more extensible else where
		// once again we have M - 2m + 2 possible distributions -> maybe we ought to
		//	have a function that generalizes this
		std::vector<Point>::const_iterator groupABegin = data.begin();
		std::vector<Point>::const_iterator groupAEnd = data.begin() + minBranchFactor;
		std::vector<Point>::const_iterator groupBBegin = data.begin() + minBranchFactor + 1;
		std::vector<Point>::const_iterator groupBEnd = data.end();

		std::vector<Point> groupA(groupABegin, groupAEnd);
		std::vector<unsigned int> groupAIndices(groupA.size());
		std::iota(groupAIndices.begin(), groupAIndices.end(), 0);

		std::vector<Point> groupB(groupBBegin, groupBEnd);
		std::vector<unsigned int> groupBIndices(groupB.size());
		std::iota(groupBIndices.begin(), groupBIndices.end(), minBranchFactor + 1);

		// return value
		std::vector<std::vector<unsigned int>> groups;

		// Find the best size out of all the distributions
		unsigned int minOverlapValue = std::numeric_limits<unsigned int>::max();
		unsigned int minAreaValue = std::numeric_limits<unsigned int>::max();

		for (unsigned int offset = 1; offset < (maxBranchFactor - 2*minBranchFactor + 2); offset += 1)
		{
			// compute the margin of groupA and groupB -> track of the sum
			Rectangle boundingBoxA;
			for (unsigned int i = 0; i < groupA.size(); i += 1)
			{
				boundingBoxA.expand(groupA[i]);
			}

			Rectangle boundingBoxB;
			for (unsigned int i = 0; i < groupA.size(); i += 1)
			{
				boundingBoxB.expand(groupB[i]);
			}

			unsigned int currDistOverlapVal = boundingBoxA.computeIntersectionArea(boundingBoxB);

			if (currDistOverlapVal < minOverlapValue) {
				minOverlapValue = currDistOverlapVal;
				groups.push_back(groupAIndices);
				groups.push_back(groupBIndices);
			} else if (currDistOverlapVal == minOverlapValue) {
				unsigned int currMinAreaVal = boundingBoxA.area() + boundingBoxB.area();
				if (currMinAreaVal < minAreaValue) {
					minAreaValue = currMinAreaVal;
					groups.push_back(groupAIndices);
					groups.push_back(groupBIndices);
				}
			}
			
			// Add one new value to groupA and remove one from groupB
			// ok that's the next step and repeat
			Point transferPoint = groupA.back();
			unsigned int transferIndex = groupAIndices.back();
			groupA.pop_back();
			groupAIndices.pop_back();
			groupB.insert(groupB.begin(), transferPoint);
			groupBIndices.insert(groupBIndices.begin(), transferIndex);
		}

		return groups;
	}


	// TODO: Because we're using vectors and didn't exactly implement the original R-Tree rewriting this
	// with sets is necessary and that will necessitate rewriting the entire R-Tree with sets.
	// Note split node is always called on a leaf -> so we can directly access it's data points
	RStarTreeNode *RStarTreeNode::splitNode(Point newData)
	{
		// This will now be modified
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

		// Call ChooseSplitIndex -> creates a grouping of the vaues
		std::vector<std::vector<unsigned>> groups = chooseSplitIndex(axis);

		// Take the two groups and modify the actual node
		// Setup the two groups which will be the entries in the two new nodes
		std::vector<unsigned> groupA = groups.at(0);
		std::vector<unsigned> groupB = groups.at(1);

		// Keep this all the same below

		// Create the new node and fill it with groupB entries by doing really complicated stuff
		RStarTreeNode *newSibling = new RStarTreeNode(minBranchFactor, maxBranchFactor, parent);
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

	RStarTreeNode *RStarTreeNode::adjustTree(RStarTreeNode *sibling)
	{
		// AT1 [Initialize]
		RStarTreeNode *node = this;
		RStarTreeNode *siblingNode = sibling;

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
						RStarTreeNode *siblingParent = node->parent->splitNode(siblingNode);

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

	RStarTreeNode *RStarTreeNode::reInsert(Point givenPoint)
	{
		// WAIT -> what I'm confused is is this for a specific node?

		// tbh given point should already be here
		// Follow reInsert algorithm here
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
			Point pointToReinsert = data.at(0);
			boundingBoxes.erase(boundingBoxes.begin());
			data.erase(data.begin()); // ok I don't think this is right
			// reinsert
			insert(pointToReinsert); // ok I am unsure if this correct
		}

		return nullptr;
	}
	
	/*
		overflow treatement -> this should be moved to a different place in code
			but let's keep it here for now
	*/
	RStarTreeNode *RStarTreeNode::overflowTreatment(RStarTreeNode *leaf, Point givenPoint)
	{
		// ok so for overflow treatement we need to save calls on the level
		if (false)
		{
			/* if this is the first call on the level */
			// this will return nullptr -> which is fine since we don't need to adjust tree
			// 	since we don't need to adjust Tree to change
			return leaf->reInsert(givenPoint);
		}
		else
		{
			/* code */
			return leaf->splitNode(givenPoint);
		}
		
	}

	// Always called on root, this = root
	RStarTreeNode *RStarTreeNode::insert(Point givenPoint)
	{
		STATEXEC(stat());

		// I1 [Find position for new record]
		RStarTreeNode *leaf = chooseLeaf(givenPoint); // this is now "chooseSubtree" we can rename
		RStarTreeNode *siblingLeaf = nullptr;

		// I2 [Add record to leaf node]
		if (leaf->data.size() < leaf->maxBranchFactor)
		{
			leaf->data.push_back(givenPoint);
		}
		else
		{
			// This call to splitNode will be udpated -> to use the new formula
			// so now this should actually call overflowTreatment which calls splitNode
			// siblingLeaf = leaf->splitNode(givenPoint);
			// tbh this isn't always correct -> we only get siblign leaf for split
			// hoenstly lets move overflow treatment in here
			// then else can be handled sepeatey - >since all the split stuff IS THE SAME
			// the rest is the same -> ok so that's just the last part

			// actually the one thing is that how do u get bounding boxes but also data -> there might be something I am missing
			siblingLeaf = overflowTreatment(leaf, givenPoint); // this is fine
		}

		// I3 [Propogate changes upward]
		RStarTreeNode *siblingNode = leaf->adjustTree(siblingLeaf);

		// I4 [Grow tree taller]
		if (siblingNode != nullptr)
		{
			RStarTreeNode *newRoot = new RStarTreeNode(minBranchFactor, maxBranchFactor);

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

	// Always called on root, this = root
	RStarTreeNode *RStarTreeNode::insert(ReinsertionEntry e)
	{
		// If reinserting a leaf then use normal insert - this should mimick basically the exact same insert as before
		if (e.level == 0)
		{
			return insert(e.data);
		}

		// I1 [Find position for new record]
		RStarTreeNode *node = chooseNode(e);
		RStarTreeNode *siblingNode = nullptr;

		// I2 [Add record to node]
		if (node->children.size() < node->maxBranchFactor)
		{
			e.child->parent = node;
			node->boundingBoxes.push_back(e.boundingBox);
			node->children.push_back(e.child);
		}
		else
		{
			siblingNode = node->splitNode(e.child);
		}

		// I3 [Propogate changes upward]
		siblingNode = node->adjustTree(siblingNode);

		// I4 [Grow tree taller]
		if (siblingNode != nullptr)
		{
			RStarTreeNode *newRoot = new RStarTreeNode(minBranchFactor, maxBranchFactor);

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
	RStarTreeNode *RStarTreeNode::condenseTree()
	{
		// CT1 [Initialize]
		RStarTreeNode *node = this;
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
				RStarTreeNode *garbage = node;

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
			node = node->insert(Q[i]);
		}

		return node;
	}

	// Always called on root, this = root
	RStarTreeNode *RStarTreeNode::remove(Point givenPoint)
	{
		STATEXEC(stat());

		// D1 [Find node containing record]
		RStarTreeNode *leaf = findLeaf(givenPoint);

		if (leaf == nullptr)
		{
			return nullptr;
		}

		// D2 [Delete record]
		leaf->removeData(givenPoint);

		// D3 [Propagate changes]
		RStarTreeNode *root = leaf->condenseTree();

		// D4 [Shorten tree]
		if (root->children.size() == 1)
		{
			root->children[0]->parent = nullptr;
			return root->children[0];
		}
		else
		{
			return root;
		}
	}

	void RStarTreeNode::print(unsigned n)
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

	void RStarTreeNode::printTree(unsigned n)
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

	unsigned RStarTreeNode::checksum()
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

	unsigned RStarTreeNode::height()
	{
		unsigned ret = 0;
		RStarTreeNode *node = this;

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

	void RStarTreeNode::stat()
	{
		STATHEIGHT(height());

		// Initialize our context stack
		std::stack<RStarTreeNode *> context;
		context.push(this);
		RStarTreeNode *currentContext;
		size_t memoryFootprint = 0;

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();

			if (currentContext->children.size() == 0)
			{
				STATBRANCH(currentContext->data.size());
				memoryFootprint += sizeof(RStarTreeNode) + currentContext->data.size() * sizeof(Point);
			}
			else
			{
				STATBRANCH(currentContext->children.size());
				memoryFootprint += sizeof(RStarTreeNode) + currentContext->children.size() * sizeof(RStarTreeNode *) + currentContext->boundingBoxes.size() * sizeof(Rectangle);
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
