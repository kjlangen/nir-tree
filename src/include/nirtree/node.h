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

#ifndef __NIRNODE__
#define __NIRNODE__

#include <cassert>
#include <vector>
#include <stack>
#include <unordered_map>
#include <limits>
#include <list>
#include <queue>
#include <utility>
#include <cmath>
#include <cstring>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <omp.h>
#include <globals/globals.h>
#include <util/geometry.h>
#include <util/graph.h>
#include <util/debug.h>
#include <util/statistics.h>

namespace nirtree
{

    class NIRTree;

	class Node
	{
		private:
			struct ReinsertionEntry
			{
				Rectangle boundingBox;
				Point data;
				Node *child;
				unsigned level;
			};

			NIRTree &treeRef;
			unsigned minBranchFactor;
			unsigned maxBranchFactor;


		public:
			struct Branch
			{
				Node *child;
				IsotheticPolygon boundingPoly;
			};

			struct SplitResult
			{
				Branch leftBranch;
				Branch rightBranch;
			};

			struct Partition
			{
				unsigned dimension;
				double location;
			};

			Node *parent;
			std::vector<Branch> branches;
			std::vector<Point> data;

			// Constructors and destructors
			Node(NIRTree &treeRef);
			Node(NIRTree &treeRef, unsigned minBranch, unsigned maxBranch, Node *p=nullptr);
			void deleteSubtrees();

			// Helper functions
			bool isLeaf();
			Rectangle boundingBox();
			Branch locateBranch(Node *child);
			void updateBranch(Node *child, IsotheticPolygon &boundingPoly);
			void removeBranch(Node *child);
			void removeData(Point givenPoint);
			Node *chooseNode(Point givenPoint);
			Node *findLeaf(Point givenPoint);
			Partition partitionNode();
			SplitResult splitNode(Partition p);
			SplitResult splitNode();
			SplitResult adjustTree();
			void condenseTree();

			// Data structure interface functions
			void exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator);
			std::vector<Point> search(Point &requestedPoint);
			std::vector<Point> search(Rectangle &requestedRectangle);
			Node *insert(Point givenPoint);
			Node *remove(Point givenPoint);

			// Miscellaneous
			unsigned checksum();
			bool validate(Node *expectedParent, unsigned index);
			void print(unsigned n=0);
			void printTree(unsigned n=0);
			unsigned height();
			void stat();
	};
}

#endif
