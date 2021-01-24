#include <rstartree/node.h>

namespace rstartree
{
	Node::Node()
	{
        // FIXME: this is not actually a valid m, M setting per R* paper.
        // Should be 2 <= m <= M/2.
		minBranchFactor = 3;
		maxBranchFactor = 5;
		parent = nullptr;
	}

	Node::Node(unsigned minBranchFactor, unsigned maxBranchFactor, Node *p)
	{
		this->minBranchFactor = minBranchFactor;
		this->maxBranchFactor = maxBranchFactor;
		this->parent = p;
	}

	void Node::deleteSubtrees()
	{
        if (entries.empty() || !std::holds_alternative<Branch>(entries[0]))
        {
            return;
        }
        for (const auto &entry : entries) {
            const Branch &b = std::get<Branch>(entry);
            b.child->deleteSubtrees();
            delete b.child;
        }
	}

	Rectangle Node::boundingBox()
	{
        assert( !this->entries.empty() );
		Rectangle boundingBox( boxFromNodeEntry( this->entries[0] ) );

        for( auto iter = this->entries.begin()+1; iter != this->entries.end(); iter++ ) {
            boundingBox.expand( boxFromNodeEntry( *iter ) );
        }
		return boundingBox;
	}

	// TODO: Optimize maybe
	void Node::updateBoundingBox(Node *child, Rectangle updatedBoundingBox)
	{
        for( auto &entry : entries ) {
            Branch &b = std::get<Branch>( entry );
            if( b.child == child ) {
                b.boundingBox = updatedBoundingBox;
                return;
            }
        }
	}

	// TODO: Optimize maybe
	void Node::removeChild(Node *child)
	{
        for( auto iter = entries.begin(); iter != entries.end(); iter++ ) {
            const Branch &b = std::get<Branch>( *iter );
            if( b.child == child ) {
                entries.erase( iter );
                return;
            }
        }
	}

	// TODO: Optimize maybe
	void Node::removeData(Point givenPoint)
	{
        for( auto iter = entries.begin(); iter != entries.end(); iter++ ) {
            const Point &p = std::get<Point>( *iter );
            if( p == givenPoint ) {
                entries.erase( iter );
                return;
            }
        }
	}

	void Node::exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator)
	{
        if( entries.empty() ) {
            // I don't think this can happen, so assert in case it does for some reason.
            assert( false );
            return;
        }

        // Am I a leaf?
        bool isLeaf = std::holds_alternative<Point>( entries[0] );
        if( isLeaf ) {
            for( const auto &entry : entries ) {
                const Point &p = std::get<Point>( entry );
                if( p == requestedPoint ) {
                    accumulator.push_back( p );
                }
            }
        } else {
            for( const auto &entry : entries ) {
                const Branch &b = std::get<Branch>( entry );
                b.child->exhaustiveSearch( requestedPoint, accumulator );
            }
        }
	}

    void Node::searchSub(Point &requestedPoint, std::vector<Point> &accumulator) {
        if( entries.empty() ) {
            // I don't think this can happen, so assert in case it does for some reason.
            assert( false );
            return;
        }

        // Am I a leaf?
        bool isLeaf = std::holds_alternative<Point>( entries[0] );
        if( isLeaf ) {
            for( const auto &entry : entries ) {
                const Point &p = std::get<Point>( entry );
                if( p == requestedPoint ) {
                    accumulator.push_back( p );
                }
            }
        } else {
            for( const auto &entry : entries ) {
                const Branch &b = std::get<Branch>( entry );
                if( b.boundingBox.containsPoint( requestedPoint ) ) {
                    b.child->searchSub( requestedPoint, accumulator );
                }
            }
        }
    }

    void Node::searchSub(Rectangle &rectangle, std::vector<Point> &accumulator) {
        if( entries.empty() ) {
            // I don't think this can happen, so assert in case it does for some reason.
            assert( false );
            return;
        }

        // Am I a leaf?
        bool isLeaf = std::holds_alternative<Point>( entries[0] );
        if( isLeaf ) {
            for( const auto &entry : entries ) {
                const Point &p = std::get<Point>( entry );
                if( rectangle.containsPoint( p ) ) {
                    accumulator.push_back( p );
                }
            }
        } else {
            for( const auto &entry : entries ) {
                const Branch &b = std::get<Branch>( entry );
                if( b.boundingBox.intersectsRectangle( rectangle ) ) {
                    b.child->searchSub( rectangle, accumulator );
                }
            }
        }
    }


	std::vector<Point> Node::search(Point &requestedPoint)
	{
		STATEXEC(stat());
        std::vector<Point> accumulator;
        searchSub( requestedPoint, accumulator );
        return accumulator;
    }

	std::vector<Point> Node::search(Rectangle &requestedRectangle)
	{
		STATEXEC(stat());

		std::vector<Point> matchingPoints;
        searchSub( requestedRectangle, matchingPoints );
		return matchingPoints;
	}

	unsigned computeOverlapGrowth(unsigned int index, const std::vector<Node::NodeEntry> &entries, const Rectangle &givenBox )
	{
        // We cannot be a leaf.
        assert( !entries.empty() );
        assert( std::holds_alternative<Node::Branch>( entries[0] ) );
        
		// 1. make a test rectangle we will use to not modify the original
		Rectangle newRectangle = std::get<Node::Branch>( entries[index] ).boundingBox;
		
		// 2. Add the point to the copied Rectangle
		newRectangle.expand( givenBox );

		// 3. now compute the overlap expansion area 
		unsigned int overlapArea = 0;
        for( unsigned i = 0; i < entries.size(); i++ ) {
            const auto &entry = entries[i];
            if( i == index ) continue;
			overlapArea += newRectangle.computeIntersectionArea(
                std::get<Node::Branch>( entry ).boundingBox
            );
		}

		return overlapArea;
	}

	// Always called on root, this = root
	// TODO: Write the analogous chooseSubtree(Rectangle searchRectangle)
	Node *Node::chooseSubtree(Point givenPoint)
	{
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
        assert( this->parent == nullptr );

		for (;;)
		{

            std::cout << "Working on node: " << node << std::endl;

			// CL2 [Leaf check]
            bool isLeaf = node->entries.empty() or !std::holds_alternative<Branch>( node->entries[0] );
            
			if( isLeaf ) {
				return node;
			}
			// our children point to leaves
            const Branch &firstBranch = std::get<Branch>( node->entries[0] );
            assert( !firstBranch.child->entries.empty() );
            bool childrenAreLeaves = !std::holds_alternative<Branch>( std::get<Branch>( node->entries[0] ).child->entries[0] );
			if( childrenAreLeaves ) {
				// Choose the entry in N whose rectangle needs least overlap enlargement 
				unsigned smallestOverlapExpansionIndex = 0;
				float smallestOverlapExpansion = std::numeric_limits<float>::max();
				float smallestExpansionArea = std::numeric_limits<float>::max();
				float smallestArea = std::numeric_limits<float>::max();

                Rectangle pointBox = boxFromNodeEntry( givenPoint );
                std::cout << "PointBox: " << std::endl;
                for( unsigned i = 0; i < node->entries.size(); i++ ) {
                    const NodeEntry &entry = node->entries[i];
                    
					float testOverlapExpansionArea = computeOverlapGrowth(i, node->entries, pointBox);
					if (smallestOverlapExpansion > testOverlapExpansionArea)
					{
						smallestOverlapExpansionIndex = i;
						smallestOverlapExpansion = testOverlapExpansionArea;

                        const Branch &b = std::get<Branch>( entry );
						// update smallesExpansionArea if needed
						float testExpansionArea = b.boundingBox.computeExpansionArea(givenPoint);
						if (smallestExpansionArea > testExpansionArea)
						{
							smallestExpansionArea = testExpansionArea;
						}

						// update area if needed
						float testArea = b.boundingBox.area();
						if (smallestArea > testArea)
						{
							smallestArea = testArea;
						}
					} 
					else if (smallestOverlapExpansion == testOverlapExpansionArea)
					{
						// Use expansion area to break tie
                        const Branch &b = std::get<Branch>( entry );
						float testExpansionArea = b.boundingBox.computeExpansionArea(givenPoint);
						if (smallestExpansionArea > testExpansionArea)
						{
							smallestOverlapExpansionIndex = i;
							smallestExpansionArea = testExpansionArea;
							
							// update area if needed
							float testArea = b.boundingBox.area();
							if (smallestArea > testArea)
							{
								smallestArea = testArea;
							}

						}
						else if (smallestExpansionArea == testExpansionArea)
						{
							// use area to break tie
                            const Branch &b = std::get<Branch>( entry );
							float testArea = b.boundingBox.area();
							if (smallestArea > testArea)
							{
								smallestOverlapExpansionIndex = i;
								smallestArea = testArea;
							}
						}
					}
				}

				// Return Node since this is now a leaf
				return std::get<Branch>( node->entries[smallestOverlapExpansionIndex] ).child;
			} else {
				// CL2 [Choose subtree]
				// Find the bounding box with least required expansion/overlap?
				unsigned smallestExpansionIndex = 0;
				float smallestExpansionArea = std::numeric_limits<float>::max();
				float smallestArea = std::numeric_limits<float>::max();

                for( unsigned i = 0; i < node->entries.size(); i++ ) {
                    const NodeEntry &entry = node->entries[i];
                    const Branch &b = std::get<Branch>( entry );
                    std:: cout << "Entry has boundingBox: " << b.boundingBox << std::endl;
					float testExpansionArea = b.boundingBox.computeExpansionArea(givenPoint);
					if (smallestExpansionArea > testExpansionArea)
					{
						smallestExpansionIndex = i;
						smallestExpansionArea = testExpansionArea;

						// potentially update smallest area
						float testArea = b.boundingBox.area();
						if (smallestArea > testArea)
						{
							smallestArea = testArea;
						}	
					}
					else if (smallestExpansionArea == testExpansionArea)
					{
						// use area to break tie
						float testArea = b.boundingBox.area();
						if (smallestArea > testArea)
						{
							smallestExpansionIndex = i;
							smallestArea = testArea;
						}	
					}
				}

				// CL3 [Descend until a leaf is reached]
				node = std::get<Branch>( node->entries[smallestExpansionIndex] ).child;
			}
		}
	}

	// Always called on root, this = root
    // FIXME: read the chooseNode code again and see what the heck it is supposed to do b/c this
    // is most def wrong.
	Node *Node::chooseNode(ReinsertionEntry e)
	{
		// CL1 [Initialize]
		Node *node = this;
        assert( this->parent == nullptr );

		for (;;)
		{
            assert( !node->entries.empty() );
            bool isLeaf = !std::holds_alternative<Branch>( node->entries[0] );

			// CL2 [Leaf check]
			if( isLeaf ) {
                // Walk back up to the level at which we should insert.
                for( unsigned i = 0; i < e.level; i++ ) {
                    node = node->parent;
                }
                return node;
			}
			// our children point to leaves
            bool childrenAreLeaves = !std::holds_alternative<Branch>( std::get<Branch>( node->entries[0] ).child->entries[0] );
			if (childrenAreLeaves)
			{
				// Choose the entry in N whose rectangle needs least overlap enlargement 
				unsigned smallestOverlapExpansionIndex = 0;
				float smallestOverlapExpansion = std::numeric_limits<float>::max();
				float smallestExpansionArea = std::numeric_limits<float>::max();
				float smallestArea = std::numeric_limits<float>::max();


				// find smallest overlap 
				for( unsigned i = 0; i < node->entries.size(); ++i)
				{
                    const NodeEntry &entry = entries[i];
                    const Branch &b = std::get<Branch>( entry );
					float testOverlapExpansionArea = computeOverlapGrowth(i, node->entries, e.boundingBox);
					if (smallestOverlapExpansion > testOverlapExpansionArea)
					{
						smallestOverlapExpansionIndex = i;
						smallestOverlapExpansion = testOverlapExpansionArea;

						// potentially update smallest expansion area
						float testExpansionArea = b.boundingBox.computeExpansionArea(e.boundingBox);
						if (smallestExpansionArea > testExpansionArea)
						{
							smallestExpansionArea = testExpansionArea;
						}

						// potentially update smallest area
						float testArea = b.boundingBox.area();
						if (smallestArea > testArea)
						{
							smallestArea = testArea;
						}
					} 
					else if (smallestOverlapExpansion == testOverlapExpansionArea)
					{
						// Use expansion area to break tie
						float testExpansionArea = b.boundingBox.computeExpansionArea(e.boundingBox);
						if (smallestExpansionArea > testExpansionArea)
						{
							smallestOverlapExpansionIndex = i;
							smallestExpansionArea = testExpansionArea;

							// potentially update smallest area
							float testArea = b.boundingBox.area();
							if (smallestArea > testArea)
							{
								smallestArea = testArea;
							}
						}
						else if (smallestExpansionArea == testExpansionArea)
						{
							// use area to break tie
							float testArea = b.boundingBox.area();
							if (smallestArea > testArea)
							{
								smallestOverlapExpansionIndex = i;
								smallestArea = testArea;
							}
						}
					}
				}

				// we continue down to next level to properly do the leaf check
				node = std::get<Branch>(node->entries[smallestOverlapExpansionIndex] ).child;
			}
			else
			{
				// CL2 [Choose subtree]
				// Find the bounding box with least required expansion/overlap?
				unsigned smallestExpansionIndex = 0;
				float smallestExpansionArea = std::numeric_limits<float>::max();
				float smallestArea = std::numeric_limits<float>::max();

				for (unsigned i = 0; i < node->entries.size(); ++i)
				{
                    const NodeEntry &entry = node->entries[i];
                    const Branch &b = std::get<Branch>( entry );
					float testExpansionArea = b.boundingBox.computeExpansionArea(e.boundingBox);
					if (smallestExpansionArea > testExpansionArea)
					{
						smallestExpansionIndex = i;
						smallestExpansionArea = testExpansionArea;

						// potentially update smallestArea
						float testArea = b.boundingBox.area();
						if (smallestArea > testArea)
						{
							smallestArea = testArea;
						}
					}
					else if (smallestExpansionArea == testExpansionArea)
					{
						// use area to break tie
						float testArea = b.boundingBox.area();
						if (smallestArea > testArea)
						{
							smallestExpansionIndex = i;
							smallestArea = testArea;
						}
					}
				}

				// CL3 [Descend until a leaf is reached]
				node = std::get<Branch>( node->entries[smallestExpansionIndex] ).child;
			}
		}
	}

	Node *Node::findLeaf(Point givenPoint)
	{
        // Am I a leaf.
        assert( !this->entries.empty() );
        bool isLeaf = !std::holds_alternative<Branch>( this->entries[0] );
        if( isLeaf ) {
            for( const auto &entry : entries ) {
                const Point &p = std::get<Point>( entry );
                if( p == givenPoint ) {
                    return this;
                }
            }
            return nullptr;
        }

        for( const auto &entry : entries ) {
            const Branch &b = std::get<Branch>( entry );
            if( b.boundingBox.containsPoint( givenPoint ) ) {
                Node *ptr = b.child->findLeaf( givenPoint );
                if( ptr != nullptr ) {
                    return ptr;
                }
                // Nope, keep looking...
            }
        }
        return nullptr;
	}

	double Node::computeTotalMarginSumOfBoundingBoxes()
	{

        // I cannot be a leaf.
        assert( !this->entries.empty() );
        assert( std::holds_alternative<Branch>( this->entries[0] ) );

		// use the data to find total sum of all M - 2m +2 possible distributions

		// first split the vector into vectorA and vectorB with vectorA being the minimum
		const auto groupABegin = entries.begin();
		const auto groupAEnd = entries.begin() + minBranchFactor;
        const auto groupBBegin = entries.begin() + minBranchFactor;
		const auto groupBEnd = entries.end();

		std::vector<NodeEntry> groupA(groupABegin, groupAEnd);
		std::vector<NodeEntry> groupB(groupBBegin, groupBEnd);

		// Find the best size out of all the distributions
		double sumOfAllMarginValues = 0;

		// will test all M - 2m + 2 possible distributions
		while (groupA.size() <= maxBranchFactor && groupB.size() >= minBranchFactor)
		{
			// compute the margin of groupA and groupB
			Rectangle boundingBoxA( std::get<Branch>( groupA[0] ).boundingBox );
			for (unsigned int i = 1; i < groupA.size(); i++ )
			{
				boundingBoxA.expand( std::get<Branch>( groupA[i] ).boundingBox );
			}

			Rectangle boundingBoxB( std::get<Branch>( groupB[0] ).boundingBox );
			for (unsigned int i = 1; i < groupB.size(); i++ )
			{
				boundingBoxB.expand( std::get<Branch>( groupB[i] ).boundingBox );
			}

			sumOfAllMarginValues += boundingBoxA.margin() + boundingBoxB.margin();

			
			// Add one new value to groupA and remove one from groupB to get to next distribution
			NodeEntry transfer = groupB.front();
			groupB.erase(groupB.begin());
			groupA.push_back(transfer);
		}

		return sumOfAllMarginValues;

	}

    //FIXME: this is functionally the same as above. Remove later.
	std::vector<std::vector<unsigned int>> Node::chooseSplitIndexByRectangle(unsigned int axis)
	{

        // Not a leaf
        assert( !this->entries.empty() );
        assert( std::holds_alternative<Branch>( this->entries[0] ) );

		// split the sorted by axis childBoundingBoxes array into the ideal split
		const auto groupABegin = entries.begin();
		const auto groupAEnd = entries.begin() + minBranchFactor;
		const auto groupBBegin = entries.begin() + minBranchFactor;
		const auto groupBEnd = entries.end();

		std::vector<NodeEntry> groupA(groupABegin, groupAEnd);
		std::vector<unsigned int> groupAIndices(groupA.size());
		std::iota(groupAIndices.begin(), groupAIndices.end(), 0);

		std::vector<NodeEntry> groupB(groupBBegin, groupBEnd);
		std::vector<unsigned int> groupBIndices(groupB.size());
		std::iota(groupBIndices.begin(), groupBIndices.end(), minBranchFactor);

		// return value
		std::vector<std::vector<unsigned int>> groups;

		// Find the best size out of all the distributions
		float minOverlapValue = std::numeric_limits<float>::max();
		float minAreaValue = std::numeric_limits<float>::max();

		// will test all M - 2m + 2 possible groupings
		while (groupA.size() <= maxBranchFactor && groupB.size() >= minBranchFactor)
		{
			// compute the margin of groupA and groupB
			Rectangle boundingBoxA( std::get<Branch>( groupA[0] ).boundingBox );
			for (unsigned int i = 0; i < groupA.size(); i++ )
			{
				boundingBoxA.expand( std::get<Branch>( groupA[i] ).boundingBox );
			}

			Rectangle boundingBoxB( std::get<Branch>( groupB[0] ).boundingBox );
			for (unsigned int i = 0; i < groupB.size(); i++ )
			{
				boundingBoxB.expand( std::get<Branch>( groupB[i] ).boundingBox );
			}

			unsigned int currDistOverlapVal = boundingBoxA.computeIntersectionArea(boundingBoxB);

			if (currDistOverlapVal < minOverlapValue) {
				// we save this current distribution of indices to return
				minOverlapValue = currDistOverlapVal;
				groups.clear();
				groups.push_back(groupAIndices);
				groups.push_back(groupBIndices);

			}
			else if (currDistOverlapVal == minOverlapValue)
			{
				// if tied we use the minimum total value or bounding boxes to decide on grouping
				unsigned int currMinAreaVal = boundingBoxA.area() + boundingBoxB.area();

				if (currMinAreaVal < minAreaValue)
				{
					// we save this current distribution of indices to return
					minAreaValue = currMinAreaVal;
					groups.clear();
					groups.push_back(groupAIndices);
					groups.push_back(groupBIndices);
				}
			}
			
			// Add one new value to groupA and remove one from groupB to get a new grouping
			NodeEntry transferRectangle = groupB.front();
			unsigned int transferIndex = groupBIndices.front();
			groupB.erase(groupB.begin());
			groupBIndices.erase(groupBIndices.begin());
			groupA.push_back(transferRectangle);
			groupAIndices.push_back(transferIndex);
		}

		return groups;
	}

	unsigned int Node::chooseSplitAxis(Node *newChild)
	{

        // Not leaf.
        assert( !this->entries.empty() );
        assert( std::holds_alternative<Branch>( this->entries[0] ) );
		// find optimal index for grouping the bouding boxes by using total margin sums
		unsigned int optimalAxis = 0;
		
		// For now we will say there is only 2 axis; however, we can set up geometry.h to include an axis type
		// eventually we can make this a loop to work with multi dimensional data
		// Sort along x axis

		std::sort(entries.begin(), entries.end(), sortByXRectangleFirst());
		unsigned int marginsFromAxisX = computeTotalMarginSumOfBoundingBoxes();

		// Sort along y axis
		std::sort(entries.begin(), entries.end(), sortByYRectangleFirst());
		unsigned int marginsFromAxisY = computeTotalMarginSumOfBoundingBoxes();

		if (marginsFromAxisX < marginsFromAxisY)
		{
			// X axis is better
			// resort childBoundingBoxes
			std::sort(entries.begin(), entries.end(), sortByXRectangleFirst());
			optimalAxis = 0;
		}
		else
		{
			// Y axis is better
			optimalAxis = 1;
		}

		return optimalAxis;
	}

    // This is simlar to the other one, but relies on that confusing copy thing. FIXME.
    // Combine methods.
	Node *Node::splitNode(Node *newChild)
	{
		/*
			S1: Call chooseSplitAxis to determine the axis in which the split of the childBoundingBoxes will be performed
			S2: Invoke chooseSplitIndex given the access to determine the best distribution along a sorting on this axis
			S3: Distribute the entries among these two groups
		*/

		STATSPLIT();


		// We first insert the point into the data to do computations

        
        Branch b( newChild->boundingBox(), newChild );
        entries.emplace_back( std::move( b ) );

		newChild->parent = this; //	I hope we didn't do this already

		// Call chooseSplitAxis to determine the axis perpendicular to which the split is perfromed
		// 	For now we will save the axis as a int -> since this allows for room for growth in the future
		unsigned int axis = chooseSplitAxis(newChild);

		// Call ChooseSplitIndex to create a grouping of the vaues
		std::vector<std::vector<unsigned>> groups = chooseSplitIndexByRectangle(axis);

		// Take the two groups and modify the actual node
		// Setup the two groups which will be the entries in the two new nodes
		std::vector<unsigned> groupA = groups.at(0);
		std::vector<unsigned> groupB = groups.at(1);

		// Given split indices, Create the new node and fill it with groupB entries by doing complicated stuff
		Node *newSibling = new Node(minBranchFactor, maxBranchFactor, parent);
		unsigned groupASize = groupA.size();
		unsigned iGroupB;
		for (unsigned i = 0; i < groupB.size(); ++i)
		{
			iGroupB = groupB[i];
            NodeEntry &groupBEnt = entries[iGroupB];
            Branch &groupBBranch = std::get<Branch>( groupBEnt );
            groupBBranch.child->parent = newSibling;
			newSibling->entries.push_back(groupBEnt);
		}
        entries.resize(groupASize);

		// Return our newly minted sibling
		return newSibling;
	}

	/*
		Helper function that takes a pre-sorted data and then computes the sum
		of all margin values over all possible M - 2m + 2 distributions
	*/
	double Node::computeTotalMarginSum()
	{
		// use the data to find possible matches (1, M - 2m +2) possible distributions
		// first split the vector into vectorA and vectorB with vectorA being the minimum
		const auto groupABegin = entries.begin();
		const auto groupAEnd = entries.begin() + minBranchFactor;
		const auto groupBBegin = entries.begin() + minBranchFactor;
		const auto groupBEnd = entries.end();

		std::vector<NodeEntry> groupA(groupABegin, groupAEnd);
		std::vector<NodeEntry> groupB(groupBBegin, groupBEnd);

		// Find the total margin sum size out of all the distributions along given axis
		double sumOfAllMarginValues = 0;

		// will test all M - 2m + 2 groupings
		while (groupA.size() <= maxBranchFactor && groupB.size() >= minBranchFactor)
		{
			Rectangle boundingBoxA = boxFromNodeEntry( groupA[0] );
			for (unsigned int i = 1; i < groupA.size(); i++)
			{
				boundingBoxA.expand( boxFromNodeEntry( groupA[i] ) );
			}

			Rectangle boundingBoxB = boxFromNodeEntry( groupB[0] );
			for (unsigned int i = 1; i < groupB.size(); i++)
			{
				boundingBoxB.expand( boxFromNodeEntry( groupB[i] ) );
			}

			// Calculate new margin sum
			sumOfAllMarginValues += boundingBoxA.margin() + boundingBoxB.margin();
			
			// Add one new value to groupA and remove one from groupB
			// ok that's the next step and repeat
			NodeEntry transfer = groupB.front();
			groupB.erase(groupB.begin());
			groupA.push_back(transfer);
		}
        std::cout << "Computed Margin: " << sumOfAllMarginValues << std::endl;

		return sumOfAllMarginValues;
	}

	
	/*
		CSA1: Sort entries by lower and upper bound along each axis and compute S -> sum of all margin values for the different distributions
			This can be stored in a array of variable that we keep in a loop -> and the just compare to the others?
			// we can first call a helper function that returns an array of all possible distributions for it?
		CSA2: Return the Axis that has the minimum total sum of all the distributions
	*/
	unsigned int Node::chooseSplitAxis()
	{
		unsigned int optimalAxis = 0;
		
		// For now we will say there is only 2 axis; however, we can set up geometry.h to include an axis type
		// eventually we can make this a loop to work with multi dimensional data
		// Sort along x axis
		std::sort(entries.begin(), entries.end(), sortByXRectangleFirst());
		double marginsFromAxisX = computeTotalMarginSum();

		// Sort along y axis
		std::sort(entries.begin(), entries.end(), sortByYRectangleFirst());
		double marginsFromAxisY = computeTotalMarginSum();


		if (marginsFromAxisX < marginsFromAxisY)
		{
			// X axis is better and resort data
			std::sort(entries.begin(), entries.end(), sortByXRectangleFirst());
			optimalAxis = 0;
		}
		else
		{
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
	unsigned Node::chooseSplitIndex(unsigned int axis)
	{

		// We assume this is called after we have sorted this->data according to axis.
		(void) axis;

		const auto groupABegin = entries.begin();
		const auto groupAEnd = entries.begin() + minBranchFactor;
		const auto groupBBegin = entries.begin() + minBranchFactor;
		const auto groupBEnd = entries.end();

		std::vector<NodeEntry> groupA(groupABegin, groupAEnd);
		std::vector<NodeEntry> groupB(groupBBegin, groupBEnd);
		unsigned splitIndex;

		// Find the best size out of all the distributions
		unsigned int minOverlapValue = std::numeric_limits<unsigned int>::max();
		unsigned int minAreaValue = std::numeric_limits<unsigned int>::max();

		// For tracking what the current "cut" mark is.
		unsigned curSplitPoint = minBranchFactor;

		// this will try each of the M-2m + 2 groups
		while (groupA.size() <= maxBranchFactor && groupB.size() >= minBranchFactor)
		{
			// compute the margin of groupA and groupB
			Rectangle boundingBoxA = boxFromNodeEntry( groupA[0] );
			for (unsigned int i = 1; i < groupA.size(); i += 1)
			{
				boundingBoxA.expand( boxFromNodeEntry( groupA[i] ) );
			}

			Rectangle boundingBoxB = boxFromNodeEntry(groupB[0]);
			for (unsigned int i = 1; i < groupB.size(); i += 1)
			{
				boundingBoxB.expand( boxFromNodeEntry( groupB[i] ) );
			}

			// Computer intersection area to determine best grouping of data points
			unsigned int currDistOverlapVal = boundingBoxA.computeIntersectionArea(boundingBoxB);

			if (currDistOverlapVal < minOverlapValue) {
				// we save this current distribution of indices to return
				minOverlapValue = currDistOverlapVal;
				splitIndex = curSplitPoint;

				// Set this if we haven't already
				if( minAreaValue == std::numeric_limits<unsigned int>::max() ) {
					minAreaValue = boundingBoxA.area() + boundingBoxB.area();
				}
			}
			else if (currDistOverlapVal == minOverlapValue)
			{

				// if overlap is equal, we use the distribution that creates the smallest areas
				unsigned int currMinAreaVal = boundingBoxA.area() + boundingBoxB.area();

				if (currMinAreaVal < minAreaValue)
				{
					// we save this current distribution of indices to return
					minAreaValue = currMinAreaVal;
					splitIndex = curSplitPoint;
				}
			}
			
			// Add one new value to groupA and remove one from groupB to obtain next distribution
			NodeEntry transferPoint = groupB.front();
			groupB.erase(groupB.begin());
			groupA.push_back(transferPoint);

			// Push the split point forward.
			curSplitPoint++;
		}

		return splitIndex;
	}


	Node *Node::splitNode()
	{
		/*
			S1: Call chooseSplitAxis to determine the axis perpendicular to which the split is performed
			S2: Invoke chooseSplitIndex given the axis to determine the best distribution along this axis
			S3: Distribute the entries among these two groups
		*/

		STATSPLIT();

        std::cout << "Going to split Node." << std::endl;

		// Call chooseSplitAxis to determine the axis perpendicular to which the split is performed
		// 	For now we will save the axis as a int -> since this allows for room for growth in the future
		// Call ChooseSplitIndex to create optimal splitting of data array
		unsigned splitIndex = chooseSplitIndex(chooseSplitAxis());

		// Create the new node and fill it with groupB entries by doing really complicated stuff
		Node *newSibling = new Node(minBranchFactor, maxBranchFactor, parent);
		newSibling->entries.reserve(entries.size()-splitIndex);

		// Copy everything to the right of the splitPoint (inclusive) to the new sibling
		std::copy(entries.begin() + splitIndex, entries.end(), std::back_inserter(newSibling->entries));

		// Chop our node's data down.
		entries.resize(splitIndex);

		// Return our newly minted sibling
		return newSibling;
	}


	Node *Node::overflowTreatment(Node *nodeToInsert, std::vector<bool> &hasReinsertedOnLevel)
	{
		Node *node = this;
		if (hasReinsertedOnLevel.at(node->level)) {
			std::cout << "Calling split for node." << std::endl;
			return node->splitNode(nodeToInsert);
		} else {
			std::cout << "Calling reInsert for node." << std::endl;
			hasReinsertedOnLevel.at(node->level) = true;
			ReinsertionEntry e{};
			e.boundingBox = nodeToInsert->boundingBox();
			e.child = nodeToInsert;
			e.level = node->level;

			// In the Point variant of this code, we iterate over all the points in the thing
			// and reinsert them. In this variant, we iterate over all of the childBoundingBoxes
			// in the thing and reinsert those as children elsewhere.

			Node *root = this;
			while( root->parent ) {
				root = root->parent;
			}

			return root->reInsert(e, hasReinsertedOnLevel);
		}
		
	}

	Node *Node::adjustTree(Node *sibling, std::vector<bool> hasReinsertedOnLevel)
	{
		// AT1 [Initialize]
		Node *node = this;
		Node *siblingNode = sibling;

		for (;;)
		{
            std::cout << "In adjustTree, am root: " << (node->parent == nullptr) << std::endl;
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
                    Branch b( siblingNode->boundingBox(), siblingNode );
                    node->parent->entries.emplace_back( std::move( b ) );

					if( node->parent->entries.size() > node->parent->maxBranchFactor ) {
						std::cout << "We don't have enough room in parent to add this guy." << std::endl;
						Node *siblingParent = node->parent->overflowTreatment(hasReinsertedOnLevel);
						node = node->parent;
						siblingNode = siblingParent;

					}
					node = node->parent;
					siblingNode = nullptr;
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

	Node *Node::reInsert(std::vector<bool> &hasReinsertedOnLevel)
	{
		// 1. RI1 Compute distance between each of the points and the bounding box containing them.
		// 2. RI2 Sort the entries by DECREASING index -> ok let's define an
		// 		extra helper function that gets to do this and pass it into sort

		Point globalCenterPoint = boundingBox().centerPoint();

        // FIXME: what to do about rectangles here.
		std::sort(entries.begin(), entries.end(), [globalCenterPoint](NodeEntry a, NodeEntry b) {
			return boxFromNodeEntry( a ).centerPoint().distance(globalCenterPoint) > boxFromNodeEntry( b ).centerPoint().distance(globalCenterPoint);
		});

		// 3. RI3 Remove the first p entries from N and adjust the bounding box -> OK so we need to adjust the data model
		//		to include a specified "p" value -> this should be unique to the node -> so it's a node variable
		unsigned int numNodesToReinsert = p * entries.size();

		// 4. Insert the removed entries -> OK we can also specify a flag that is
		//		if you want to reinsert starting with largest values (i.e. start at index 0) or closest values (Start at index p)

        // Find the root node
        Node *root = this;
        while( root->parent != nullptr ) {
            root = root->parent;
        }

        // We need to reinsert these entries
        // We pop them all off before hand so that any reorganization of the tree during this recursive
        // insert does not affect which entries get popped off.
        std::vector<NodeEntry> entriesToReinsert;
        entriesToReinsert.reserve(numNodesToReinsert);
        
        std::copy(entries.begin(), entries.begin() + numNodesToReinsert, std::back_inserter(entriesToReinsert));
        entries.erase(entries.begin(), entries.begin() + numNodesToReinsert);

        // During this recursive insert (we are already in an insert, since we are 
        // reInserting), we may end up here again. If we do, we should still be using the same
        // hasReinsertedOnLevel vector because it corresponds to the activities we have performed
        // during a single point/rectangle insertion (the top level one).
        for(const auto &entry: entriesToReinsert) {
            // the root may change. Update it.
            // FIXME: I'm going to assume this a point for now, but it could also be a Branch.
            // If this is a branch, construct a reinsertion entry and throw it through.
            // Better --- make insert take a NodeEntry!
            root = root->insert( std::get<Point>( entry ), hasReinsertedOnLevel);
        }

		return nullptr;
	}

	Node *Node::reInsert(ReinsertionEntry e, std::vector<bool> hasReinsertedOnLevel) {
		// reinsert children of node that has overflowed

		// temporarily add the newly inserted child's boundingBox into your own
        Branch b( e.child->boundingBox(), e.child );
        entries.emplace_back( std::move( b ) );
		e.child->parent = this;
		e.child->level = this->level + 1; // temproarily set new level

		// 1. RI1 Compute distance between each of the boundBoxes.ceter and the 
		//		gloabl bounding box -> parent's bounding box at that index
		// 2. RI2 Sort the entries by DECREASING index
		Point globalCenterPoint = boundingBox().centerPoint();
	
		std::sort(entries.begin(), entries.end(), [globalCenterPoint](NodeEntry a, NodeEntry b) {
			return (boxFromNodeEntry(a).centerPoint().distance(globalCenterPoint) > boxFromNodeEntry(b).centerPoint().distance(globalCenterPoint));
		});

		// 3. RI3 Remove the first p entries from N and adjust the bounding box -> OK so we need to adjust the data model
		//		to include a specified "p" value -> this should be unique to the node -> so it's a node variable
		unsigned int nodesToReinsert = p * entries.size();

		// 4. Insert the removed entries -> OK we can also specify a flag that is
		//		if you want to reinsert starting with largest values (i.e. start at index 0) or closest values (Start at index p)
        std::vector<NodeEntry> entriesToReinsert(entries.begin(), entries.begin() + nodesToReinsert);
        entries.erase(entries.begin(), entries.begin()+nodesToReinsert);
        for( const auto &entry : entriesToReinsert ) {
            // FIXME: can probably just reinsert entries directly
			ReinsertionEntry e = {};
			e.boundingBox = std::get<Branch>( entry ).boundingBox;
			e.child = std::get<Branch>( entry ).child;
			e.level = level;
			insert(e, hasReinsertedOnLevel); // reinsert child
		}

		return nullptr;
	}

	
	/*
		overflow treatement for dealing with a leaf node that is too big (overflow)
	*/
	Node *Node::overflowTreatment(std::vector<bool> &hasReinsertedOnLevel)
	{

		if (hasReinsertedOnLevel.at(hasReinsertedOnLevel.size() - 1))
		{
			return splitNode();
		}
		else
		{
			hasReinsertedOnLevel.at(this->level) = true;
			return reInsert(hasReinsertedOnLevel);
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

	Node *Node::insert( NodeEntry nodeEntry, std::vector<bool> &hasReinsertedOnLevel)
	{

        // Always called on root, this = root
        assert( this->level == 0 );

        bool nodeEntryIsPoint = std::holds_alternative<Point>( nodeEntry );
		STATEXEC(stat());

        Node *insertionPoint;
		// I1 [Find position for new record]
        if( nodeEntryIsPoint ) {
            insertionPoint = chooseSubtree( std::get<Point>( nodeEntry ) );
        } else {
            ReinsertionEntry e;
            Branch &b = std::get<Branch>( nodeEntry );
            e.boundingBox = b.boundingBox;
            e.child = b.child;
            e.level = b.child->level+1;
            insertionPoint = chooseNode( e );
        }
		Node *sibling = nullptr;

		// I2 [Add record to leaf node]
	    insertionPoint->entries.push_back(nodeEntry);

        // If we exceed maxBranchFactor, need to do sometyhing about it.
        if (insertionPoint->entries.size() > maxBranchFactor) 
		{
			// we call overflow treatment to determine how our sibling node is treated
			// if we do forced reInsert siblingLeaf if nullptr and is properly dealt with in adjustTree
            std::cout << "It's time for overflow treatment!" << std::endl;
			sibling = insertionPoint->overflowTreatment(hasReinsertedOnLevel);
			std::cout << "Overflow treatment done." << std::endl;
		}

		// I3 [Propogate overflow treatment changes upward]
		Node *siblingNode = insertionPoint->adjustTree(sibling, hasReinsertedOnLevel);

		// I4 [Grow tree taller]
		if (siblingNode != nullptr)
		{
            std::cout << "Growing tree..." << std::endl;
			Node *newRoot = new Node(minBranchFactor, maxBranchFactor);

			this->parent = newRoot;
            Branch b1( this->boundingBox(), this );
            newRoot->entries.emplace_back( std::move( b1 ) );

			siblingNode->parent = newRoot;
            Branch b2( siblingNode->boundingBox(), siblingNode );
            newRoot->entries.emplace_back(std::move( b2 ) );

			// fix the reinserted length
			hasReinsertedOnLevel.push_back(false);

			// Adjust levels.
			newRoot->level = this->level;
			this->level = this->level+1;
			siblingNode->level = this->level;

			return newRoot;
		}
		else
		{
			// We might no longer be the parent.  If we hit overflowTreatment,
			// we may have triggered reInsert, which then triggered a split.
			// That insert will have returned newRoot, but because reInsert()
			// returns nullptr, we don't know about it.
			Node *root = this; 
			while(root->parent)
			{
				root = root->parent;
			}
			return root;
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
		if (node->entries.size() < node->maxBranchFactor)
		{
			e.child->parent = node;
			e.child->level = node->level + 1;
            Branch b( e.boundingBox, e.child );
			node->entries.emplace_back( std::move( b ) );
		}
		else
		{
			// use overflowTreatement to decide if we must do a reinsert or a splitNode on this level
			siblingNode = overflowTreatment(node, e, hasReinsertedOnLevel); // this is fine
		}

		// I3 [Propogate overflow treatment changes upward]
		siblingNode = node->adjustTree(siblingNode, hasReinsertedOnLevel);

		// I4 [Grow tree taller]
		if (siblingNode != nullptr)
		{
			Node *newRoot = new Node(minBranchFactor, maxBranchFactor);

			this->parent = newRoot;
            Branch b( this->boundingBox(), this );
            newRoot->entries.emplace_back( std::move( b ) );

			siblingNode->parent = newRoot;
            Branch b2( siblingNode->boundingBox(), siblingNode );
            newRoot->entries.emplace_back( std::move( b2 ) );

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
		unsigned entriesSize;
		while (node->parent != nullptr)
		{
            entriesSize = node->entries.size();

			// CT3 & CT4 [Eliminate under-full node. & Adjust covering rectangle.]
			if (entriesSize >= node->minBranchFactor ) {
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
                // FIXME: combine and just do an entry reinsert
                assert( !node->entries.empty() );
                bool isLeaf = !std::holds_alternative<Branch>( node->entries[0] );
                if( isLeaf ) {
                    for( const auto &entry : node->entries ) {
                        ReinsertionEntry e = {};
                        e.child = nullptr;
                        e.data = std::get<Point>( entry );
                        e.level = 0;
                        Q.push_back(e);
                    }
                } else {
                    for( const auto &entry : node->entries ) {
                        ReinsertionEntry e = {};
                        const Branch &b = std::get<Branch>( entry );
                        e.boundingBox = b.boundingBox;
                        e.child = b.child;
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
		}

		// CT6 [Re-insert oprhaned entries]
		for (unsigned i = 0; i < Q.size(); ++i) {
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
		if (root->entries.size() == 1)
		{
			// slice the hasReinsertedOnLevel
			// we are removing the root to shorten the tree so we then decide to remove the root
			hasReinsertedOnLevel.erase(hasReinsertedOnLevel.begin());
            Branch &b = std::get<Branch>( root->entries[0] );
            b.child->parent = nullptr;
			return b.child;
		}
		else
		{
			return root;
		}
	}

	void Node::print(unsigned n) const
	{
		std::string indentation(n * 4, ' ');
		std::cout << indentation << "Node " << (void *)this << std::endl;
		std::cout << indentation << "{" << std::endl;
		std::cout << indentation << "    Parent: " << (void *)parent << std::endl;
		std::cout << indentation << "    Entries: " << std::endl;
        bool isLeaf = !std::holds_alternative<Branch>( entries[0] );
        if( isLeaf ) {
            for( const auto &entry : entries ) {
                std::cout << indentation << "		" << std::get<Point>( entry ) << std::endl;
            }
        } else {
            for( const auto &entry : entries ) {
                const Branch &b = std::get<Branch>( entry );
                std::cout << indentation << "		" << b.boundingBox << ", ptr: " << b.child << std::endl;
            }
        }
		std::cout << std::endl << indentation << "}" << std::endl;
	}

	void Node::printTree(unsigned n) const
	{
		// Print this node first
		print(n);

		// Print any of our children with one more level of indentation
        if( std::holds_alternative<Branch>( entries[0] ) ) {
            for( const auto &entry : entries ) {
                const Branch &b = std::get<Branch>( entry );
                b.child->printTree(n + 1);
            }
        }
	}

	unsigned Node::checksum() const
	{
		unsigned sum = 0;

        bool isLeaf = !std::holds_alternative<Branch>( entries[0] );
        if( isLeaf ) {
            for( const auto &entry : entries ) {
                const Point &p = std::get<Point>( entry );
                sum += (unsigned)p[0];
                sum += (unsigned)p[1];
            }
		} else {
            for( const auto &entry : entries ) {
				// Recurse
                const Branch &b = std::get<Branch>( entry );
				sum += b.child->checksum();
            }
		}

		return sum;
	}

	unsigned Node::height() const
	{
		unsigned ret = 0;
		const Node *node = this;

		for (;;) {
			ret++;
            if( std::holds_alternative<Branch>( node->entries[0] ) ) {
                node = std::get<Branch>( node->entries[0] ).child;
            } else {
                return ret;
            }
		}
	}

	void Node::stat() const
	{
		STATHEIGHT(height());

		// Initialize our context stack
		std::stack<const Node *> context;
		context.push(this);
		const Node *currentContext;
		size_t memoryFootprint = 0;

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();

            bool isLeaf = !std::holds_alternative<Branch>( currentContext->entries[0] );
            if( isLeaf ) {
				STATBRANCH(currentContext->entries.size());
				memoryFootprint += sizeof(Node) + currentContext->entries.size() * sizeof(Point);
            } else {
				STATBRANCH(currentContext->entries.size());
				memoryFootprint += sizeof(Node) + currentContext->entries.size() * sizeof(Node *) + currentContext->entries.size() * sizeof(Rectangle);
				// Determine which branches we need to follow
                for( const auto &entry : currentContext->entries ) {
                    context.push( std::get<Branch>( entry ).child );
				}
			}
		}

		STATMEM(memoryFootprint);
	}

    Rectangle boxFromNodeEntry( const Node::NodeEntry &entry ) {
        if( std::holds_alternative<Node::Branch>( entry ) ) {
            return std::get<Node::Branch>( entry ).boundingBox;
        }
        const Point &p = std::get<Point>( entry );
        return Rectangle( p, p );
    }
}
