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
    if( root.get_type() == LEAF_NODE || root.get_type() == REPACKED_LEAF_NODE ) {
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
    return point_search( root, requestedPoint, this );
}

template <int min_branch_factor, int max_branch_factor, class strategy>
std::vector<Point>
NIRTreeDisk<min_branch_factor,max_branch_factor,strategy>::search( Rectangle requestedRectangle ) {
    return rectangle_search( root, requestedRectangle, this );
}

template <int min_branch_factor, int max_branch_factor, class strategy>
void NIRTreeDisk<min_branch_factor,max_branch_factor,strategy>::insert( Point givenPoint ) {
    std::fill( hasReinsertedOnLevel.begin(), hasReinsertedOnLevel.end(), false );
    bool root_repacked = root.get_type() == REPACKED_LEAF_NODE || root.get_type() == REPACKED_BRANCH_NODE;
    if( root.get_type() == LEAF_NODE || root.get_type() == REPACKED_LEAF_NODE ) {
        auto root_node = get_leaf_node( root , true );
        root = root_node->insert(givenPoint, hasReinsertedOnLevel);
        if ( root_repacked ) {
            root = root_node->repack_inplace( node_allocator_.get() );
        }
    } else {
        auto root_node = get_branch_node( root, true );
        std::variant<Branch,Point> entry = givenPoint;
        root = root_node->insert(entry, hasReinsertedOnLevel);
        if ( root_repacked ) {
            root = root_node->repack_inplace( node_allocator_.get(), node_allocator_.get()  );
        }
    }
}

template <int min_branch_factor, int max_branch_factor, class strategy>
void NIRTreeDisk<min_branch_factor,max_branch_factor,strategy>::remove( Point givenPoint ) {
    if( root.get_type() == LEAF_NODE || root.get_type() == REPACKED_LEAF_NODE ) {
        auto root_node = get_leaf_node( root );
        root = root_node->remove( givenPoint );
    } else {
        auto root_node = get_branch_node( root );
        root = root_node->remove( givenPoint );
    }
}

template <int min_branch_factor, int max_branch_factor, class strategy>
unsigned NIRTreeDisk<min_branch_factor,max_branch_factor,strategy>::checksum() {
    if( root.get_type() == LEAF_NODE || root.get_type() == REPACKED_LEAF_NODE ) {
        auto root_node = get_leaf_node( root );
        return root_node->checksum();
    } else {
        auto root_node = get_branch_node( root );
        return root_node->checksum();
    }
}

template <int min_branch_factor, int max_branch_factor, class strategy>
bool NIRTreeDisk<min_branch_factor,max_branch_factor,strategy>::validate() {
    if( root.get_type() == LEAF_NODE || root.get_type() == REPACKED_LEAF_NODE ) {
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
    stat_node( root, this );
}

template <int min_branch_factor, int max_branch_factor, class strategy>
void NIRTreeDisk<min_branch_factor,max_branch_factor,strategy>::print() {
    if( root.get_type() == LEAF_NODE || root.get_type() == REPACKED_LEAF_NODE ) {
        auto root_node = get_leaf_node( root, false );
        root_node->printTree();
    } else {
        auto root_node = get_branch_node( root, false );
        root_node->printTree();
    }
}

template <int min_branch_factor, int max_branch_factor, class strategy>
void NIRTreeDisk<min_branch_factor,max_branch_factor,strategy>::visualize() {
    //BMPPrinter p(1000, 1000);
   // p.printToBMP(root);
}
