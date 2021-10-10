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

template <int min_branch_factor, int max_branch_factor>
std::vector<Point> NIRTreeDisk<min_branch_factor,max_branch_factor>::exhaustiveSearch( Point requestedPoint ) {
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    std::vector<Point> v;
    auto root_node = node_allocator_.get_tree_node<NodeType>( root );
    root_node->exhaustiveSearch( requestedPoint, v );
    return v;
}

template <int min_branch_factor, int max_branch_factor>
std::vector<Point> NIRTreeDisk<min_branch_factor,max_branch_factor>::search( Point requestedPoint ) {
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    auto root_node = node_allocator_.get_tree_node<NodeType>( root );
    return root_node->search(requestedPoint);
}

template <int min_branch_factor, int max_branch_factor>
std::vector<Point> NIRTreeDisk<min_branch_factor,max_branch_factor>::search( Rectangle requestedRectangle ) {
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    auto root_node = node_allocator_.get_tree_node<NodeType>( root );
    return root_node->search( requestedRectangle );
}

template <int min_branch_factor, int max_branch_factor>
void NIRTreeDisk<min_branch_factor,max_branch_factor>::insert( Point givenPoint ) {
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    auto root_node = node_allocator_.get_tree_node<NodeType>( root );
    root = root_node->insert(givenPoint);
}

template <int min_branch_factor, int max_branch_factor>
void NIRTreeDisk<min_branch_factor,max_branch_factor>::remove( Point givenPoint ) {
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    auto root_node = node_allocator_.get_tree_node<NodeType>( root );
    root = root_node->remove( givenPoint );
}

template <int min_branch_factor, int max_branch_factor>
unsigned NIRTreeDisk<min_branch_factor,max_branch_factor>::checksum() {
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    auto root_node = node_allocator_.get_tree_node<NodeType>( root );
    return root_node->checksum();
}

template <int min_branch_factor, int max_branch_factor>
bool NIRTreeDisk<min_branch_factor,max_branch_factor>::validate() {
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    auto root_node = node_allocator_.get_tree_node<NodeType>( root );
/*    assert( root_node->bounding_box_validate() ); */
    return root_node->validate( tree_node_handle(nullptr), 0 );
}

template <int min_branch_factor, int max_branch_factor>
void NIRTreeDisk<min_branch_factor,max_branch_factor>::stat() {
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    auto root_node = node_allocator_.get_tree_node<NodeType>( root );
    root_node->stat();
}

template <int min_branch_factor, int max_branch_factor>
void NIRTreeDisk<min_branch_factor,max_branch_factor>::print() {
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    auto root_node = node_allocator_.get_tree_node<NodeType>( root );
    root_node->printTree();
}

template <int min_branch_factor, int max_branch_factor>
void NIRTreeDisk<min_branch_factor,max_branch_factor>::visualize() {
    //BMPPrinter p(1000, 1000);
   // p.printToBMP(root);
}
