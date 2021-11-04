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


template <int min_branch_factor, int max_branch_factor, class strategy>
std::vector<Point> NIRTreeDisk<min_branch_factor,max_branch_factor,strategy>::exhaustiveSearch( Point requestedPoint ) {
    std::vector<Point> v;
    if( root.get_type() == LEAF_NODE ) {
        auto root_node = get_leaf_node( root );
        root_node->exhaustiveSearch( requestedPoint, v );
    } else {
        auto root_node = get_branch_node( root );
        root_node->exhaustiveSearch( requestedPoint, v );
    }
    return v;
}

template <int min_branch_factor, int max_branch_factor, class strategy>
std::vector<Point>
NIRTreeDisk<min_branch_factor,max_branch_factor,strategy>::search( Point requestedPoint ) {
    if( root.get_type() == LEAF_NODE ) {
        auto root_node = get_leaf_node( root );
        assert( root_node->self_handle_ == root );
        return root_node->search(requestedPoint);
    } else {
        auto root_node = get_branch_node( root );
        assert( root_node->self_handle_ == root );
        return root_node->search(requestedPoint);
    }
}

template <int min_branch_factor, int max_branch_factor, class strategy>
std::vector<Point>
NIRTreeDisk<min_branch_factor,max_branch_factor,strategy>::search( Rectangle requestedRectangle ) {
    if( root.get_type() == LEAF_NODE ) {
        auto root_node = get_leaf_node( root );
        return root_node->search( requestedRectangle );
    } else {
        auto root_node = get_branch_node( root );
        return root_node->search( requestedRectangle );
    }
}

template <int min_branch_factor, int max_branch_factor, class strategy>
void NIRTreeDisk<min_branch_factor,max_branch_factor,strategy>::insert( Point givenPoint ) {
    if( root.get_type() == LEAF_NODE ) {
        auto root_node = get_leaf_node( root );
        root = root_node->insert(givenPoint);
    } else {
        auto root_node = get_branch_node( root );
        root = root_node->insert(givenPoint);
    }
}

template <int min_branch_factor, int max_branch_factor, class strategy>
void NIRTreeDisk<min_branch_factor,max_branch_factor,strategy>::remove( Point givenPoint ) {
    if( root.get_type() == LEAF_NODE ) {
        auto root_node = get_leaf_node( root );
        root = root_node->remove( givenPoint );
    } else {
        auto root_node = get_branch_node( root );
        root = root_node->remove( givenPoint );
    }
}

template <int min_branch_factor, int max_branch_factor, class strategy>
unsigned NIRTreeDisk<min_branch_factor,max_branch_factor,strategy>::checksum() {
    if( root.get_type() == LEAF_NODE ) {
        auto root_node = get_leaf_node( root );
        return root_node->checksum();
    } else {
        auto root_node = get_branch_node( root );
        return root_node->checksum();
    }
}

template <int min_branch_factor, int max_branch_factor, class strategy>
bool NIRTreeDisk<min_branch_factor,max_branch_factor,strategy>::validate() {
    if( root.get_type() == LEAF_NODE ) {
        auto root_node = get_leaf_node( root );
        root_node->bounding_box_validate();
        return root_node->validate( tree_node_handle(nullptr), 0 );
    } else {
        auto root_node = get_branch_node( root );
        root_node->bounding_box_validate();
        return root_node->validate( tree_node_handle(nullptr), 0 );
    }
}

template <int min_branch_factor, int max_branch_factor, class strategy>
void NIRTreeDisk<min_branch_factor,max_branch_factor,strategy>::stat() {
    if( root.get_type() == LEAF_NODE ) {
        auto root_node = get_leaf_node( root );
        root_node->stat();
    } else {
        auto root_node = get_branch_node( root );
        root_node->stat();
    }
}

template <int min_branch_factor, int max_branch_factor, class strategy>
void NIRTreeDisk<min_branch_factor,max_branch_factor,strategy>::print() {
    if( root.get_type() == LEAF_NODE ) {
        auto root_node = get_leaf_node( root );
        root_node->printTree();
    } else {
        auto root_node = get_branch_node( root );
        root_node->printTree();
    }
}

template <int min_branch_factor, int max_branch_factor, class strategy>
void NIRTreeDisk<min_branch_factor,max_branch_factor,strategy>::visualize() {
    //BMPPrinter p(1000, 1000);
   // p.printToBMP(root);
}
