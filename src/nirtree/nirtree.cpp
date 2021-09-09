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

#include <nirtree/nirtree.h>

namespace nirtree
{
	NIRTree::NIRTree(unsigned minBranchFactor, unsigned maxBranchFactor)
	{
		root = new Node(*this, minBranchFactor, maxBranchFactor);
	}

	NIRTree::NIRTree(Node *root)
	{
		this->root = root;
	}

	NIRTree::~NIRTree()
	{
		root->deleteSubtrees();
		delete root;
	}

	std::vector<Point> NIRTree::exhaustiveSearch(Point requestedPoint)
	{
		std::vector<Point> v;
		root->exhaustiveSearch(requestedPoint, v);

		return v;
	}

	std::vector<Point> NIRTree::search(Point requestedPoint)
	{
		return root->search(requestedPoint);
	}

	std::vector<Point> NIRTree::search(Rectangle requestedRectangle)
	{
		return root->search(requestedRectangle);
	}

	void NIRTree::insert(Point givenPoint)
	{
		root = root->insert(givenPoint);
	}

	void NIRTree::remove(Point givenPoint)
	{
		root = root->remove(givenPoint);
	}

	unsigned NIRTree::checksum()
	{
		return root->checksum();
	}

	bool NIRTree::validate()
	{
		return root->validate(nullptr, 0);
	}

	void NIRTree::stat()
	{
		root->stat();
	}

	void NIRTree::print()
	{
		root->printTree();
	}

	void NIRTree::visualize()
	{
		BMPPrinter p(1000, 1000);

		p.printToBMP(root);
	}
}
