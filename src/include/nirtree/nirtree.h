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

#ifndef __NIRTREE__
#define __NIRTREE__

#include <cassert>
#include <vector>
#include <stack>
#include <queue>
#include <string>
#include <iostream>
#include <utility>
#include <util/geometry.h>
#include <nirtree/node.h>
#include <index/index.h>
#include <util/bmpPrinter.h>
#include <util/statistics.h>

namespace nirtree
{
	class NIRTree: public Index
	{
		public:
			Node *root;
			Statistics stats;

			// Constructors and destructors
			NIRTree(unsigned minBranchFactor, unsigned maxBranchFactor);
			NIRTree(Node *root);
			~NIRTree();

			// Datastructure interface
			std::vector<Point> exhaustiveSearch(Point requestedPoint);
			std::vector<Point> search(Point requestedPoint);
			std::vector<Point> search(Rectangle requestedRectangle);
			void insert(Point givenPoint);
			void remove(Point givenPoint);

			// Miscellaneous
			unsigned checksum();
			bool validate();
			void stat();
			void print();
			void visualize();
	};
}

#endif
