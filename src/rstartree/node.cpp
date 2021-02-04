#include <rstartree/node.h>
#include <rstartree/rstartree.h>

namespace rstartree
{

    template <typename N, typename functor>
    void treeWalker( N root, functor &f ) {
        static_assert( std::is_pointer<N>::value, "Expected a pointer for N." );

		std::stack<N> context;
		context.push(root);
		N currentContext;

		while (!context.empty())
		{
			currentContext = context.top();
			context.pop();

            bool isLeaf = currentContext->entries.empty() or !std::holds_alternative<Node::Branch>( currentContext->entries[0] );
            f( currentContext );
            if( !isLeaf ) {
                for( const auto &entry : currentContext->entries ) {
                    context.push( std::get<Node::Branch>( entry ).child );
				}
            }
		}
    }

    template <typename functor>
    void adjustNodeLevels( Node *root, functor f ) {
        struct bumpNodeLevels {
            bumpNodeLevels( functor f ) : f(f) {}
            void operator()( Node *node ) {
                node->level = f( node->level );
            }
            functor f;
        };

        bumpNodeLevels b(f);
        treeWalker( root, b );
    }

	Node::Node( const RStarTree &treeRef, Node *parent) : treeRef( treeRef ), parent( parent )
	{
        if( parent == nullptr ) {
            level = 0;
        } else {
            level = parent->level + 1;
        }
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

	Rectangle Node::boundingBox() const
	{
        assert( !entries.empty() );
		Rectangle boundingBox( boxFromNodeEntry( entries[0] ) );

        for( auto iter = entries.begin()+1; iter != entries.end(); iter++ ) {
            boundingBox.expand( boxFromNodeEntry( *iter ) );
        }
		return boundingBox;
	}

	void Node::updateBoundingBox(Node *child, Rectangle updatedBoundingBox)
	{
        for(auto &entry : entries)
        {
            Branch &b = std::get<Branch>(entry);
            if (b.child == child)
            {
                b.boundingBox = updatedBoundingBox;
                return;
            }
        }
#if !defined( NDEBUG )
        print();
        assert( false );
#endif
	}

	void Node::removeChild(Node *child)
	{
        for(auto iter = entries.begin(); iter != entries.end(); iter++)
        {
            const Branch &b = std::get<Branch>(*iter);
            if (b.child == child)
            { 
                entries.erase(iter);
                return;
            }
        }
	}

	void Node::removeData(Point givenPoint)
	{
        for(auto iter = entries.begin(); iter != entries.end(); iter++)
        {
            const Point &p = std::get<Point>(*iter);
            if (p == givenPoint)
            {
                entries.erase(iter);
                return;
            }
        }
	}

	void Node::exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator)
	{
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

    void Node::searchSub(const Point &requestedPoint, std::vector<Point> &accumulator) const {
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

    void Node::searchSub(const Rectangle &rectangle, std::vector<Point> &accumulator) const {
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


	std::vector<Point> Node::search( const Point &requestedPoint ) const
	{
        std::vector<Point> accumulator;
        searchSub( requestedPoint, accumulator );
        return accumulator;
    }

	std::vector<Point> Node::search( const Rectangle &requestedRectangle ) const
	{

		std::vector<Point> matchingPoints;
        searchSub( requestedRectangle, matchingPoints );
		return matchingPoints;
	}

	double computeOverlapGrowth(unsigned int index, const std::vector<Node::NodeEntry> &entries, const Rectangle &givenBox )
	{
        // We cannot be a leaf.
        assert( !entries.empty() );
        assert( std::holds_alternative<Node::Branch>( entries[0] ) );
        
		// 1. make a test rectangle we will use to not modify the original
		Rectangle newRectangle = std::get<Node::Branch>( entries[index] ).boundingBox;
		
		// 2. Add the point to the copied Rectangle
		newRectangle.expand( givenBox );

		// 3. now compute the overlap expansion area 
		double overlapArea = 0;
        for( unsigned i = 0; i < entries.size(); i++ ) {
            const auto &entry = entries[i];
            if( i == index ) continue;
			overlapArea += newRectangle.computeIntersectionArea(
                std::get<Node::Branch>( entry ).boundingBox
            );
		}

		return overlapArea;
	}

	Node *Node::chooseSubtree(NodeEntry givenNodeEntry)
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

        // Always called on root, this = root
        assert( parent == nullptr );

        bool entryIsBranch = std::holds_alternative<Branch>( givenNodeEntry );
        Rectangle givenEntryBoundingBox = boxFromNodeEntry( givenNodeEntry );

		for (;;)
		{

			// CL2 [Leaf check]
            bool isLeaf = node->entries.empty() or !std::holds_alternative<Branch>( node->entries[0] );
            
			if( isLeaf ) {
                if( node->parent != nullptr ) {
                    assert( node->level > 0 );
                }

                // If this branch's node is at level 3, I need to get to level 2 to do an insert. 
                if( entryIsBranch ) {
                    const Branch &b = std::get<Branch>( givenNodeEntry );
                    for( unsigned i = node->level; i > b.child->level-1; i-- ) {
                        assert( node->level == i );
                        node = node->parent;
                        assert( node->level == i-1 );
                    }
                }
				return node;
			}

			// our children point to leaves
            const Branch &firstBranch = std::get<Branch>( node->entries[0] );
            assert( !firstBranch.child->entries.empty() );
            bool childrenAreLeaves = !std::holds_alternative<Branch>( std::get<Branch>( node->entries[0] ).child->entries[0] );

			if( childrenAreLeaves ) {
				// Choose the entry in N whose rectangle needs least overlap enlargement 
				unsigned smallestOverlapExpansionIndex = 0;
				double smallestOverlapExpansion = std::numeric_limits<double>::max();
				double smallestExpansionArea = std::numeric_limits<double>::max();
				double smallestArea = std::numeric_limits<double>::max();

                for( unsigned i = 0; i < node->entries.size(); i++ ) {
                    const NodeEntry &entry = node->entries[i];
                    const Branch &b = std::get<Branch>( entry );
                    
					double testOverlapExpansionArea = computeOverlapGrowth(i, node->entries, givenEntryBoundingBox);
					if (smallestOverlapExpansion > testOverlapExpansionArea)
					{
						smallestOverlapExpansionIndex = i;
						smallestOverlapExpansion = testOverlapExpansionArea;

						// update smallesExpansionArea if needed
						double testExpansionArea = b.boundingBox.computeExpansionArea(givenEntryBoundingBox);
						if (smallestExpansionArea > testExpansionArea)
						{
							smallestExpansionArea = testExpansionArea;
						}

						// update area if needed
						double testArea = b.boundingBox.area();
						if (smallestArea > testArea)
						{
							smallestArea = testArea;
						}
					} 
					else if (smallestOverlapExpansion == testOverlapExpansionArea)
					{
						// Use expansion area to break tie
						double testExpansionArea = b.boundingBox.computeExpansionArea(givenEntryBoundingBox);
						if (smallestExpansionArea > testExpansionArea)
						{
							smallestOverlapExpansionIndex = i;
							smallestExpansionArea = testExpansionArea;
							
							// update area if needed
							double testArea = b.boundingBox.area();
							if (smallestArea > testArea)
							{
								smallestArea = testArea;
							}

						}
						else if (smallestExpansionArea == testExpansionArea)
						{
							// use area to break tie
							double testArea = b.boundingBox.area();
							if (smallestArea > testArea)
							{
								smallestOverlapExpansionIndex = i;
								smallestArea = testArea;
							}
						}
					}
				}
                // Proceed to leaf check.
				node = std::get<Branch>( node->entries[smallestOverlapExpansionIndex] ).child;

			} else {
				// CL2 [Choose subtree]
				// Find the bounding box with least required expansion/overlap
				unsigned smallestExpansionIndex = 0;
				double smallestExpansionArea = std::numeric_limits<double>::max();
				double smallestArea = std::numeric_limits<double>::max();

                for( unsigned i = 0; i < node->entries.size(); i++ ) {
                    const NodeEntry &entry = node->entries[i];
                    const Branch &b = std::get<Branch>( entry );
					double testExpansionArea = b.boundingBox.computeExpansionArea(givenEntryBoundingBox);
					if (smallestExpansionArea > testExpansionArea)
					{
						smallestExpansionIndex = i;
						smallestExpansionArea = testExpansionArea;

						// potentially update smallest area
						double testArea = b.boundingBox.area();
						if (smallestArea > testArea)
						{
							smallestArea = testArea;
						}	
					}
					else if (smallestExpansionArea == testExpansionArea)
					{
						// use area to break tie
						double testArea = b.boundingBox.area();
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

	/*
		Helper function that takes a pre-sorted data and then computes the sum
		of all margin values over all possible M - 2m + 2 distributions
	*/
	double Node::computeTotalMarginSum()
	{
		// use the data to find possible matches (1, M - 2m +2) possible distributions
		// first split the vector into vectorA and vectorB with vectorA being the minimum
		const auto groupABegin = entries.begin();
		const auto groupAEnd = entries.begin() + treeRef.minBranchFactor;
		const auto groupBBegin = entries.begin() + treeRef.minBranchFactor;
		const auto groupBEnd = entries.end();

		std::vector<NodeEntry> groupA(groupABegin, groupAEnd);
		std::vector<NodeEntry> groupB(groupBBegin, groupBEnd);

		// Find the total margin sum size out of all the distributions along given axis
		double sumOfAllMarginValues = 0;

		// will test all M - 2m + 2 groupings
		while (groupA.size() <= treeRef.maxBranchFactor && groupB.size() >= treeRef.minBranchFactor)
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
		const auto groupAEnd = entries.begin() + treeRef.minBranchFactor;
		const auto groupBBegin = entries.begin() + treeRef.minBranchFactor;
		const auto groupBEnd = entries.end();

		std::vector<NodeEntry> groupA(groupABegin, groupAEnd);
		std::vector<NodeEntry> groupB(groupBBegin, groupBEnd);
		unsigned splitIndex;

		// Find the best size out of all the distributions
		unsigned int minOverlapValue = std::numeric_limits<unsigned int>::max();
		unsigned int minAreaValue = std::numeric_limits<unsigned int>::max();

		// For tracking what the current "cut" mark is.
		unsigned curSplitPoint = treeRef.minBranchFactor;

		// this will try each of the M-2m + 2 groups
		while (groupA.size() <= treeRef.maxBranchFactor && groupB.size() >= treeRef.minBranchFactor)
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

		// Call chooseSplitAxis to determine the axis perpendicular to which the split is performed
		// 	For now we will save the axis as a int -> since this allows for room for growth in the future
		// Call ChooseSplitIndex to create optimal splitting of data array
		unsigned splitIndex = chooseSplitIndex(chooseSplitAxis());

		Node *newSibling = new Node(treeRef, parent);
		newSibling->entries.reserve(entries.size()-splitIndex);
        assert( (this->parent == nullptr and this->level == 0) or this->level-1 == this->parent->level );
        assert( newSibling->level == this->level );

		// Copy everything to the right of the splitPoint (inclusive) to the new sibling
		std::copy(entries.begin() + splitIndex, entries.end(), std::back_inserter(newSibling->entries));
        if( std::holds_alternative<Branch>( newSibling->entries[0] ) ) {
            for( const auto &entry : newSibling->entries ) {
                // Update parents
                const Branch &b = std::get<Branch>( entry );
                b.child->parent = newSibling;
                assert( this->level == b.child->level-1 );
                assert( newSibling->level == b.child->level-1 );
            }
        }


		// Chop our node's data down.
		entries.erase(entries.begin()+splitIndex, entries.end());

#if !defined( NDEBUG )
        if( std::holds_alternative<Branch>( this->entries[0] ) ) {
            for( const auto &entry : this->entries ) {
                // Update parents
                const Branch &b = std::get<Branch>( entry );
                assert( this->level == b.child->level-1 );
            }
        }
#endif

        assert( !entries.empty() );
        assert( !newSibling->entries.empty() );

		// Return our newly minted sibling
		return newSibling;
	}

	Node *Node::adjustTree(Node *sibling, std::vector<bool> &hasReinsertedOnLevel)
	{
		// AT1 [Initialize]
		Node *node = this;
		Node *siblingNode = sibling;

		for (;;)
		{
            assert( node != nullptr );
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
                    assert( node->level-1 == node->parent->level );
                    assert( siblingNode->level-1 == node->parent->level );
                    node->parent->entries.emplace_back( std::move( b ) );

#if !defined( NDEBUG )
                    for (const auto &entry : node->parent->entries ) {
                        assert( std::holds_alternative<Branch>( entry ) );
                        assert( std::get<Branch>( entry ).child->level-1 == node->parent->level );
                    }
#endif

					if( node->parent->entries.size() > node->parent->treeRef.maxBranchFactor ) {
                        Node *parentBefore = node->parent;
						Node *siblingParent = node->parent->overflowTreatment(hasReinsertedOnLevel);

                        if( siblingParent ) {
                            
                            // We split our parent, so now we have two (possible) parents.
                            assert( node->parent == siblingParent || node->parent == parentBefore );
                            assert( siblingNode->parent == siblingParent || siblingNode->parent == parentBefore );

                            // Need to keep traversing up.
                            node = node->parent;
                            siblingNode = siblingParent;
                            assert( node != siblingNode );
                            continue;
                        }
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

		Point globalCenterPoint = boundingBox().centrePoint();

        assert( hasReinsertedOnLevel.at( level ) );

		std::sort(entries.begin(), entries.end(), [&globalCenterPoint](NodeEntry a, NodeEntry b) {
            Rectangle rectA = boxFromNodeEntry( a );
            Rectangle rectB = boxFromNodeEntry( b );
			return rectA.centrePoint().distance(globalCenterPoint) > rectB.centrePoint().distance(globalCenterPoint);
		});

		// 3. RI3 Remove the first p entries from N and adjust the bounding box -> OK so we need to adjust the data model
		//		to include a specified "p" value -> this should be unique to the node -> so it's a node variable
		unsigned int numNodesToReinsert = treeRef.p * entries.size();

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

        std::copy( entries.begin(), entries.begin() + numNodesToReinsert, std::back_inserter( entriesToReinsert ) );
        entries.erase(entries.begin(), entries.begin() + numNodesToReinsert);
        // During this recursive insert (we are already in an insert, since we are 
        // reInserting), we may end up here again. If we do, we should still be using the same
        // hasReinsertedOnLevel vector because it corresponds to the activities we have performed
        // during a single point/rectangle insertion (the top level one).
        for( const auto &entry : entriesToReinsert ) {
            assert( root->parent == nullptr );
            root = root->insert( entry, hasReinsertedOnLevel );
        }

		return nullptr;
	}
	
	/*
		overflow treatement for dealing with a leaf node that is too big (overflow)
	*/
	Node *Node::overflowTreatment(std::vector<bool> &hasReinsertedOnLevel)
	{

        assert( hasReinsertedOnLevel.size() > this->level );
		if( hasReinsertedOnLevel.at(this->level) ){
			return splitNode();
		}
		else
		{
			hasReinsertedOnLevel.at(this->level) = true;
			return reInsert(hasReinsertedOnLevel);
		}
		
	}

	Node *Node::insert( NodeEntry nodeEntry, std::vector<bool> &hasReinsertedOnLevel)
	{

        // Always called on root, this = root
        assert( this->level == 0 );

		// I1 [Find position for new record]
        Node *insertionPoint = chooseSubtree( nodeEntry );
		Node *sibling = nullptr;

		// I2 [Add record to leaf node]
        bool givenIsLeaf = std::holds_alternative<Point>( nodeEntry );
#if !defined( NDEBUG )
        bool firstIsPoint = entries.empty() or std::holds_alternative<Point>( insertionPoint->entries[0] );
        assert( (givenIsLeaf and firstIsPoint) or (!givenIsLeaf and !firstIsPoint) );
#endif
	    insertionPoint->entries.push_back(nodeEntry);
        if( !givenIsLeaf ) {
            const Branch &b = std::get<Branch>( nodeEntry );
            assert( insertionPoint->level == b.child->level-1);
            b.child->parent = insertionPoint;
        }

        // If we exceed treeRef.maxBranchFactor, need to do something about it.
        if (insertionPoint->entries.size() > treeRef.maxBranchFactor) 
		{
			// we call overflow treatment to determine how our sibling node is treated
			// if we do a reInsert, sibling is nullptr. This is properly dealt with in adjustTree
			sibling = insertionPoint->overflowTreatment(hasReinsertedOnLevel);
		}

		// I3 [Propogate overflow treatment changes upward]
		Node *siblingNode = insertionPoint->adjustTree(sibling, hasReinsertedOnLevel);

		// I4 [Grow tree taller]
		if (siblingNode != nullptr)
		{

            assert( this->level == 0 );
			Node *newRoot = new Node(treeRef);

			this->parent = newRoot;
            Branch b1( this->boundingBox(), this );
            newRoot->entries.emplace_back( std::move( b1 ) );

			siblingNode->parent = newRoot;
            Branch b2( siblingNode->boundingBox(), siblingNode );
            newRoot->entries.emplace_back( std::move( b2 ) );
            assert( newRoot->entries.size() == 2 );

			// fix the reinserted length
			hasReinsertedOnLevel.insert(hasReinsertedOnLevel.begin(), false);

			// Adjust levels.
			newRoot->level = this->level;
            adjustNodeLevels( siblingNode, []( int level ) { return level+1; } );
            adjustNodeLevels( this, []( int level ) { return level+1; } );

            assert( newRoot->level == 0 );
            assert( siblingNode->level == 1 );
            assert( this->level == 1 );


			return newRoot;
		}
		else
		{
			// We might no longer be the parent.  If we hit overflowTreatment,
			// we may have triggered reInsert, which then triggered a split.
			// That insert will have returned newRoot, but because reInsert()
			// returns nullptr, we don't know about it.
			Node *root = this; 
#if !defined( NDEBUG )
            unsigned curLevel = root->level;
#endif
			while(root->parent)
			{
				root = root->parent;
#if !defined( NDEBUG )
                assert( root->level == curLevel-1 );
                curLevel = root->level;
#endif
			}
            assert( root->level == 0 );
			return root;
		}
	}

	// To be called on a leaf
	Node *Node::condenseTree(std::vector<bool> &hasReinsertedOnLevel)
	{
		// CT1 [Initialize]
		Node *node = this;

        // Is Leaf
        assert( this->entries.empty() || std::holds_alternative<Point>(  this->entries[0] ) );

		std::vector<NodeEntry> Q;

		// CT2 [Find parent entry]
		unsigned entriesSize;
		while (node->parent != nullptr)
		{
            entriesSize = node->entries.size();

			// CT3 & CT4 [Eliminate under-full node. & Adjust covering rectangle.]
			if (entriesSize >= node->treeRef.minBranchFactor ) {

				node->parent->updateBoundingBox(node, node->boundingBox());

				// CT5 [Move up one level in the tree]
				// Move up a level without deleting ourselves
				node = node->parent;
			}
			else
			{
				// Remove ourselves from our parent
				node->parent->removeChild(node);
                assert( !node->entries.empty() );

                // push these entries into Q
                std::copy( node->entries.begin(), node->entries.end(), std::back_inserter( Q ) );

				// Prepare for garbage collection
				Node *garbage = node;

				// CT5 [Move up one level in the tree]
				// Move up a level before deleting ourselves
				node = node->parent;

				// Cleanup ourselves without deleting children b/c they will be reinserted
				delete garbage;
			}
		}

		// CT6 [Re-insert oprhaned entries]
		for ( const auto &entry : Q ) {
#if !defined( NDEBUG )
            if( std::holds_alternative<Branch>( entry ) ) {
                const Branch &b = std::get<Branch>( entry );
                assert( b.child->level-1 == node->level );
            }
#endif
            assert( node->parent == nullptr );
			node = node->insert(entry, hasReinsertedOnLevel);
		}

		return node;
	}

	// Always called on root, this = root
	Node *Node::remove(Point givenPoint, std::vector<bool> hasReinsertedOnLevel)
	{

        assert( this->level == 0 );
        assert( this->parent == nullptr );

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
            adjustNodeLevels( b.child, []( int level ) { return level-1; } );
            delete root;
			return b.child;
		}
		else
		{
			return root;
		}
	}

	void Node::print() const
	{
		std::string indentation(level * 4, ' ');
		std::cout << indentation << "Node " << (void *)this << std::endl;
		std::cout << indentation << "{" << std::endl;
        std::cout << indentation << "    BoundingBox: " << boundingBox() << std::endl;
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

	void Node::printTree() const
	{
		// Print this node first
        struct printer {
            void operator()( Node * const node ) {
                node->print();
            }
        };

        printer p;
        treeWalker( const_cast<Node * const>( this ), p );
	}

	unsigned Node::checksum() const
	{
        struct checksumFunctor {
            checksumFunctor() {
                checksum = 0;
            }

            void operator()( Node * const node ) {
                bool isLeaf = node->entries.empty() or std::holds_alternative<Point>( node->entries[0] );
                if( isLeaf ) {
                    for( const auto &entry : node->entries ) {
                        const Point &p = std::get<Point>( entry );
                        checksum += (unsigned)p[0];
                        checksum += (unsigned)p[1];
                    }
                } 
            }

            unsigned checksum;
        };

        checksumFunctor cf;
        treeWalker( const_cast<Node * const>( this ), cf );

		return cf.checksum;
	}

	unsigned Node::height() const
	{
        struct heightFunctor {
            heightFunctor() {
                maxDepth = 0;
            }

            void operator()( Node * const node ) {
                if( node->level > maxDepth ) {
                    maxDepth = node->level;
                }
            }
            unsigned maxDepth;
        };
        heightFunctor hf;
        treeWalker( const_cast<Node * const>( this ), hf );
        return hf.maxDepth+1;
    }


	void Node::stat() const
	{
		STATHEIGHT(height());

        struct memComsumptionFunctor {
            memComsumptionFunctor() {
                memoryFootprint = 0;
            }

            void operator()( Node * const node ) {
                bool isLeaf = node->entries.empty() or !std::holds_alternative<Point>( node->entries[0] );
                if( isLeaf ) {
                    STATBRANCH(node->entries.size());
                    memoryFootprint += sizeof(Node) + node->entries.size() * sizeof(Point);
                } else {
                    memoryFootprint += sizeof(Node) + node->entries.size() * sizeof(Node *) + node->entries.size() * sizeof(Rectangle);
                }
            }

            size_t memoryFootprint;
        };
        memComsumptionFunctor mf;
        treeWalker( const_cast<Node * const>( this ), mf );

		STATMEM(mf.memoryFootprint);
	}

    Rectangle boxFromNodeEntry( const Node::NodeEntry &entry ) {
        if( std::holds_alternative<Node::Branch>( entry ) ) {
            return std::get<Node::Branch>( entry ).boundingBox;
        }
        const Point &p = std::get<Point>( entry );
        return Rectangle( p, p );
    }
}
