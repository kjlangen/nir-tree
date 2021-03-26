#include <quadtree/node.h>
#include <quadtree/quadtree.h>

namespace quadtree
{
	Node::Node(QuadTree &treeRef, Point &givenPoint, Node *p) : treeRef(treeRef)
	{
		data = givenPoint;
		parent = p;
		branches.resize(treeRef.quadrants);

		for (unsigned i = 0; i < treeRef.quadrants; ++i)
		{
			branches[i] = nullptr;
		}
	}

	void Node::deleteSubtrees()
	{
		for (Node *branch : branches)
		{
			if (branch != nullptr)
			{
				branch->deleteSubtrees();
				delete branch;
			}
		}
	}

	unsigned Node::nextBranch(Point &givenPoint)
	{
		unsigned branchIndex = 0;

		for (unsigned d = 0; d < dimensions; ++d)
		{
			unsigned dimensionOK = givenPoint[d] > data[d] ? 1 : 0;
			branchIndex |= (dimensionOK << d);
		}

		assert(branchIndex < treeRef.quadrants);

		return branchIndex;
	}

	std::vector<unsigned> Node::nextBranch(Rectangle &givenRectangle)
	{
		std::vector<unsigned> nextIndexes;

		for (unsigned i = 0; i < treeRef.quadrants; ++i)
		{
			// Go down this branch only if the rectangle intersects it
			bool placeInQuad = true;

			// In each dimension the bits of each quadrant number tell us whether to use a max or min
			// comparison. Ex. to test quadrant 1 base 10 = 01 base 2, axis 0 must be a max comparison while axis 1 must
			// be a min comparison.
			for (unsigned d = 0; d < dimensions; ++d)
			{
				unsigned maxComparison = (1 << d) & i;

				if (maxComparison)
				{
					placeInQuad = placeInQuad && givenRectangle.upperRight[d] > data[d];
				}
				else
				{
					placeInQuad = placeInQuad && givenRectangle.lowerLeft[d] <= data[d];
				}
			}

			if (placeInQuad)
			{
				nextIndexes.push_back(i);
			}
		}

		return nextIndexes;
	}

	bool Node::isLeaf()
	{
		for (Node *branch : branches)
		{
			if (branch != nullptr)
			{
				return false;
			}
		}

		return true;
	}

	void Node::exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator)
	{
		if (data == requestedPoint)
		{
			accumulator.push_back(data);
		}
		else
		{
			// Determine which branches we need to follow
			for (Node *branch : branches)
			{
				if (branch != nullptr)
				{
					// Recurse
					branch->exhaustiveSearch(requestedPoint, accumulator);
				}
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

#ifdef STAT
			if (currentContext->isLeaf())
			{
				treeRef.stats.markLeafSearched();
			}
			else
			{
				treeRef.stats.markNonLeafNodeSearched();
			}
#endif

			if (currentContext->data == requestedPoint)
			{
				// This node contains the data so we may stop
				accumulator.push_back(currentContext->data);
				break;
			}
			
			// Determine which branches we need to follow
			unsigned nextIndex = currentContext->nextBranch(requestedPoint);
			if (currentContext->branches[nextIndex] != nullptr)
			{
				context.push(currentContext->branches[nextIndex]);
			}
		}

#ifdef STAT
		treeRef.stats.resetSearchTracker<false>();
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

#ifdef STAT
			if (currentContext->isLeaf())
			{
				treeRef.stats.markLeafSearched();
			}
			else
			{
				treeRef.stats.markNonLeafNodeSearched();
			}
#endif

			if (requestedRectangle.containsPoint(currentContext->data))
			{
				// This data point is contained in our search area
				accumulator.push_back(currentContext->data);
			}

			for (unsigned nextIndex : currentContext->nextBranch(requestedRectangle))
			{
				if (currentContext->branches[nextIndex] != nullptr)
				{
					context.push(currentContext->branches[nextIndex]);
				}
			}
		}

#ifdef STAT
		treeRef.stats.resetSearchTracker<true>();
#endif

		return accumulator;
	}

	// Always called on root, this = root
	void Node::insert(Point givenPoint)
	{
		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();

			unsigned nextIndex = currentContext->nextBranch(givenPoint);

			if (currentContext->branches[nextIndex] == nullptr)
			{
				currentContext->branches[nextIndex] = new Node(treeRef, givenPoint, currentContext);
				break;
			}
			else
			{
				context.push(currentContext->branches[nextIndex]);
			}
		}
	}

	void Node::remove(Point givenPoint)
	{
		// Quadtrees don't support deletion!
	}

	unsigned Node::checksum()
	{
		unsigned sum = 0;

		for (unsigned d = 0; d < dimensions; ++d)
		{
			sum += (unsigned)data[d];
		}

		for (Node *branch : branches)
		{
			if (branch != nullptr)
			{
				sum += branch->checksum();
			}
		}

		return sum;
	}

	bool Node::validate(Node *expectedParent)
	{
		bool valid = true;

		if (parent != expectedParent)
		{
			std::cout << "node = " << (void *)this << std::endl;
			std::cout << "parent = " << (void *)parent << " expectedParent = " << (void *)expectedParent << std::endl;
			std::cout << "data = " << data << std::endl;
			assert(parent == expectedParent);
		}

		for (Node *branch : branches)
		{
			if (branch != nullptr)
			{
				valid = valid && branch->validate(this);
			}
		}

		return valid;
	}

	void Node::print(unsigned n)
	{
		std::string indendtation(n * 4, ' ');
		std::cout << indendtation << "Node " << (void *)this << std::endl;
		std::cout << indendtation << "    Parent: " << (void *)parent << std::endl;
		std::cout << indendtation << "    Data: " << data << std::endl;
		std::cout << indendtation << "    Branches: " << std::endl;
		for (Node *branch : branches)
		{
			std::cout << indendtation << "		" << (void *)branch << std::endl;
		}
		std::cout << std::endl;
	}

	void Node::printTree(unsigned n)
	{
		// Print this node first
		print(n);

		// Print all of our children with one more level of indentation
		std::string indendtation(n * 4, ' ');
		for (Node *branch : branches)
		{
			if (branch != nullptr)
			{
				// Recurse
				branch->printTree(n + 1);
			}
		}
		std::cout << std::endl;
	}

	unsigned Node::height()
	{
		return 5000;
	}

	void Node::stat()
	{
#ifdef STAT
		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;
		size_t memoryFootprint = 0;
		unsigned long singularBranches = 0;
		unsigned long totalLeaves = 0;

		std::vector<unsigned long> histogramFanout;
		histogramFanout.resize(dimensions + 3, 0);

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();

			memoryFootprint += sizeof(Node);
			unsigned fanout = 0;
			for (Node *branch : currentContext->branches)
			{
				if (branch != nullptr)
				{
					++fanout;
					context.push(branch);
				}
			}

			++histogramFanout[fanout];
			if (fanout == 0)
			{
				++totalLeaves;
			}
			else if (fanout == 1)
			{
				++singularBranches;
			}
		}

		// Print out what we have found
		STATEXEC(std::cout << "### Statistics ###" << std::endl);
		STATMEM(memoryFootprint);
		STATHEIGHT(height());
		STATSINGULAR(singularBranches);
		STATLEAF(totalLeaves);
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
