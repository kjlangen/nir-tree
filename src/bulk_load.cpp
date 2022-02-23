#include <bulk_load.h>

template <class TreeType>
void make_all_rects_disjoint(
    TreeType *treeRef,
    std::vector<Rectangle> &rects_a,
    tree_node_handle a_node,
    std::vector<Rectangle> &rects_b,
    tree_node_handle b_node
){
    std::vector<Rectangle> a_output;

    std::stack<Rectangle, std::vector<Rectangle>>
        remaining_a_rects( rects_a );

    while( not remaining_a_rects.empty() ) {
        Rectangle a = remaining_a_rects.top();
        remaining_a_rects.pop();
        bool did_split = false;
        for( uint64_t i = 0; i < rects_b.size(); i++ ) {
            Rectangle &b = rects_b.at(i);
            // If there is no intersection with this rectangle, keep
            // going
            if( not a.intersectsRectangle( b ) ) {
                continue;
            }
            // If there is, we need to split it.
            auto ret = nirtreedisk::make_rectangles_disjoint_accounting_for_region_ownership(
                treeRef,
                a,
                a_node,
                b,
                b_node
            );

            IsotheticPolygon poly1;
            poly1.basicRectangles = ret.first;
            poly1.recomputeBoundingBox();

            IsotheticPolygon poly2;
            poly2.basicRectangles = ret.second;
            poly2.recomputeBoundingBox();

            if( not poly1.disjoint( poly2 ) ) {
                std::cout << "A: " << a << std::endl;
                std::cout << "B: " << b << std::endl;
                std::cout << "Poly1: " << poly1 << std::endl;
                std::cout << "Poly2: " << poly2 << std::endl;

                std::cout << "Intersection." << std::endl;
                poly1.intersection( poly2 );
                std::cout << poly1 << std::endl;

                assert( false );
            }
            assert( poly1.disjoint( poly2 ) );

            for( auto &ret_a_rect : ret.first ) {
                remaining_a_rects.push( ret_a_rect );
            }
            rects_b.erase( rects_b.begin() + i );
            for( auto &ret_b_rect : ret.second ) {
                rects_b.push_back( ret_b_rect );
            }
            // Need to loop around because we broke the iterator, and
            // both a and b have new sets of rectangles
            did_split = true;
            break;
        }
        if( not did_split ) {
            a_output.push_back( a );
        }
    }
    rects_a = a_output;
}

template <>
void fill_branch(
    nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy> *treeRef,
    pinned_node_ptr<nirtreedisk::BranchNode<5,9,nirtreedisk::ExperimentalStrategy>> branch_node,
    tree_node_handle node_handle,
    std::vector<std::pair<Point,tree_node_handle>> &node_point_pairs,
    uint64_t &offset,
    unsigned branch_factor,
    nirtreedisk::LeafNode<5,9,nirtreedisk::ExperimentalStrategy> *leaf_type
) {
    using LN =
        nirtreedisk::LeafNode<5,9,nirtreedisk::ExperimentalStrategy>;
    using BN =
        nirtreedisk::BranchNode<5,9,nirtreedisk::ExperimentalStrategy>;

    std::vector<std::pair<IsotheticPolygon, tree_node_handle>>
        fixed_bb_and_handles;
    tree_node_allocator *allocator = treeRef->node_allocator_.get();

    // Add up to branch factor items to it
    for( uint64_t i = 0; i < branch_factor; i++ ) {
        tree_node_handle child_handle =
            node_point_pairs[offset++].second;
        Rectangle bbox;
        // Adjust parent
        if( child_handle.get_type() == LEAF_NODE ) {
            auto node =
                allocator->get_tree_node<LN>(
                        child_handle );
            node->parent = node_handle;
            bbox = node->boundingBox();
        } else {
            auto node =
                allocator->get_tree_node<BN>(
                        child_handle );
            node->parent = node_handle;
            bbox = node->boundingBox();
        }

        fixed_bb_and_handles.push_back( std::make_pair(
                    IsotheticPolygon( bbox ), child_handle ) );

        if( offset == node_point_pairs.size() ) {
            break;
        }
    }

    for( uint64_t i = 0; i < fixed_bb_and_handles.size(); i++ ) {
        for( uint64_t j = i+1; j < fixed_bb_and_handles.size(); j++ ) {

            std::vector<Rectangle> &existing_rects_a =
                fixed_bb_and_handles.at(i).first.basicRectangles;
            std::vector<Rectangle> &existing_rects_b =
                fixed_bb_and_handles.at(j).first.basicRectangles;
            make_all_rects_disjoint(
                treeRef,
                existing_rects_a,
                fixed_bb_and_handles.at(i).second,
                existing_rects_b,
                fixed_bb_and_handles.at(j).second
            );
        }
    }

    // Now we have made all the BoundingRegions disjoint.
    // It is time to add our children
    for( uint64_t i = 0; i < fixed_bb_and_handles.size(); i++ ) {
        nirtreedisk::Branch b;
        b.child = fixed_bb_and_handles.at(i).second;
        IsotheticPolygon &constructed_poly =
            fixed_bb_and_handles.at(i).first;
        if( constructed_poly.basicRectangles.size() <=
                MAX_RECTANGLE_COUNT ) {
            b.boundingPoly = InlineBoundedIsotheticPolygon();
            std::get<InlineBoundedIsotheticPolygon>( b.boundingPoly
                    ).push_polygon_to_disk( constructed_poly );
        } else {
            unsigned rect_size = std::min(
                    InlineUnboundedIsotheticPolygon::maximum_possible_rectangles_on_first_page(),
                    constructed_poly.basicRectangles.size() );
            auto alloc_data =
                allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                        compute_sizeof_inline_unbounded_polygon(
                            rect_size ),
                        NodeHandleType( BIG_POLYGON ) );
            new (&(*alloc_data.first)) InlineUnboundedIsotheticPolygon( allocator, rect_size );
            alloc_data.first->push_polygon_to_disk( constructed_poly );
            b.boundingPoly = alloc_data.second;
        }
        branch_node->addBranchToNode(b);
        //branch_node->bounding_box_validate();
    }
}

template <>
void fill_branch(
    rstartreedisk::RStarTreeDisk<5,9> *treeRef,
    pinned_node_ptr<rstartreedisk::BranchNode<5,9>> branch_node,
    tree_node_handle node_handle,
    std::vector<std::pair<Point,tree_node_handle>> &node_point_pairs,
    uint64_t &offset,
    unsigned branch_factor,
    rstartreedisk::LeafNode<5,9> *leaf_type
) {
    using LN = rstartreedisk::LeafNode<5,9>;
    using BN = rstartreedisk::BranchNode<5,9>;

    std::vector<std::pair<Rectangle, tree_node_handle>> bb_and_handles;
    tree_node_allocator *allocator = treeRef->node_allocator_.get();

    // Add up to branch factor items to it
    for( uint64_t i = 0; i < branch_factor; i++ ) {
        tree_node_handle child_handle =
            node_point_pairs[offset++].second;
        Rectangle bbox;
        // Adjust parent
        if( child_handle.get_type() == LEAF_NODE ) {
            auto node =
                allocator->get_tree_node<LN>(
                        child_handle );
            node->parent = node_handle;
            bbox = node->boundingBox();
        } else {
            auto node =
                allocator->get_tree_node<BN>(
                        child_handle );
            node->parent = node_handle;
            bbox = node->boundingBox();
        }

        bb_and_handles.push_back( std::make_pair(
                    bbox, child_handle ) );

        rstartreedisk::Branch b;
        b.child = child_handle;
        b.boundingBox = bbox;
        branch_node->addBranchToNode(b);

 
        if( offset == node_point_pairs.size() ) {
            break;
        }
    }
}


template <typename T, typename LN, typename BN>
std::vector<tree_node_handle> str_packing_branch(
    T* tree,
    std::vector<tree_node_handle> &child_nodes,
    unsigned branch_factor,
    LN *leaf_node_type,
    BN *branch_node_type
) {
    tree_node_allocator *allocator = tree->node_allocator_.get();

    // Get bbox once for everything so I'm not materializing it
    // constantly
    std::vector<std::pair<Point,tree_node_handle>> node_point_pairs;
    node_point_pairs.reserve( child_nodes.size() );
    for( tree_node_handle &child_handle : child_nodes ) {
        Rectangle bbox;
        if( child_handle.get_type() == LEAF_NODE ) {
            auto child =
                allocator->get_tree_node<LN>( child_handle );
            bbox = child->boundingBox();
        } else {
            auto child =
                allocator->get_tree_node<BN>( child_handle );
            bbox = child->boundingBox();
        }
        node_point_pairs.push_back( std::make_pair( bbox.centrePoint(), child_handle ) );
    }

    uint64_t P = node_point_pairs.size() / branch_factor;
    if( node_point_pairs.size() % branch_factor != 0  ) {
        P++;
    }

    double S_dbl = std::ceil( sqrt(P) );
    uint64_t S = (uint64_t) S_dbl;

    // Sort by X
    std::sort( node_point_pairs.begin(), node_point_pairs.end(), [](
                std::pair<Point, tree_node_handle> &l, std::pair<Point, tree_node_handle> &r
    ) { return l.first[0] < r.first[0]; });

    // There are |S| vertical stripes.
    // Each has |S|*branch_factor points
    for( uint64_t i = 0; i < S; i++ ) {
        uint64_t start_offset = i * (S*branch_factor);
        uint64_t stop_offset = (i+1) * (S*branch_factor);
        if( stop_offset > node_point_pairs.size() ) {
            stop_offset = node_point_pairs.size();
        }
        auto start_point = node_point_pairs.begin() + start_offset;
        auto stop_point = node_point_pairs.begin() + stop_offset;
        std::sort( start_point, stop_point, [](
                    std::pair<Point,tree_node_handle> &l,
                    std::pair<Point,tree_node_handle> &r ) {
            return l.first[1] < r.first[1]; } );
        if( stop_offset == node_point_pairs.size() ) {
            break;
        }
    }

    std::vector<tree_node_handle> branches;
    uint64_t offset = 0;
    while( offset < node_point_pairs.size() ) {

        // Create the branch node
        auto alloc_data =
            allocator->create_new_tree_node<BN>( NodeHandleType( BRANCH_NODE ) );
        new (&(*alloc_data.first))
            BN( tree, nullptr, alloc_data.second, 1 );
        auto branch_node = alloc_data.first;
        tree_node_handle branch_handle = alloc_data.second;
        
        fill_branch(
            tree,
            branch_node,
            branch_handle,
            node_point_pairs, 
            offset,
            branch_factor,
            (LN *) nullptr
        );
        branches.push_back( branch_handle );
    }
    return branches;
}

std::pair<uint64_t,double> compute_max_dist( const Point &point, std::vector<Point> &pts ) {
    uint64_t idx = 0;
    double max_dist = std::numeric_limits<double>::min();
    for( uint64_t i = 0; i < pts.size(); i++ ) {
        double dist = point.fast_distance( pts.at(i) );
        if( dist > max_dist ) {
            max_dist = dist;
            idx = i;
        }
    }
    return std::make_pair( idx, max_dist );
}

bool point_comparator( const Point &lhs, const Point &rhs ) {
    for( unsigned d = 0; d < dimensions; d++ ) {
        if( lhs[d] != rhs[d] ) {
            return lhs[d] < rhs[d];
        }
    }
    return false;
}

template <typename T, typename LN, typename BN>
std::vector<tree_node_handle> str_packing_branch_euclidean(
    T* tree,
    std::vector<tree_node_handle> &child_nodes,
    unsigned branch_factor,
    LN *leaf_node_type,
    BN *branch_node_type
) {

    std::vector<tree_node_handle> branches;
    branches.reserve( branch_factor );
    tree_node_allocator *allocator = tree->node_allocator_.get();

    // Get bbox once for everything so I'm not materializing it
    // constantly
    std::unordered_map<Point, tree_node_handle> point_to_handle_map;
    std::vector<Point> points;
    for( tree_node_handle &child_handle : child_nodes ) {
        Rectangle bbox;
        if( child_handle.get_type() == LEAF_NODE ) {
            auto child =
                allocator->get_tree_node<LN>( child_handle );
            bbox = child->boundingBox();
        } else {
            auto child =
                allocator->get_tree_node<BN>( child_handle );
            bbox = child->boundingBox();
        }

        Point pt = bbox.centrePoint();

        //FIXME I don't think this is actually guaranteed to be unique
        assert( point_to_handle_map.find( pt ) == point_to_handle_map.end() );

        point_to_handle_map[pt] = child_handle;
        points.push_back( pt );
    }

    // Sort by X, break ties with Y
    std::sort( points.begin(), points.end(), point_comparator );

    // Copy into list
    std::list<Point> loc_points( points.begin(), points.end() );

    // Group them into nodes.
    for( ;; ) {

        Point cur_point = loc_points.front();
        double max_allowed_distance =
            std::numeric_limits<double>::max();
        uint64_t worst_index = 0;
        std::vector<Point> pts_to_remove;
        pts_to_remove.reserve( branch_factor );
        pts_to_remove.push_back( cur_point );

        loc_points.pop_front();

        // Find closest points
        for( auto list_iter = loc_points.begin(); list_iter != loc_points.end(); list_iter++ ) {
            if( pts_to_remove.size() < branch_factor ) {
                pts_to_remove.push_back( *list_iter );
                if( pts_to_remove.size() == branch_factor ) {
                    // This might be expensive too
                    auto res = compute_max_dist( cur_point, pts_to_remove );
                    worst_index = res.first;
                    max_allowed_distance = res.second;
                }
            } else  {
                double dist = cur_point.fast_distance( *list_iter );
                if( dist < max_allowed_distance ) {
                    pts_to_remove.at( worst_index ) = *list_iter;
                    auto res = compute_max_dist( cur_point, pts_to_remove );
                    worst_index = res.first;
                    max_allowed_distance = res.second;
                }

                double val = ((*list_iter)[0] - cur_point[0]);
                val *= val;

                if( val > max_allowed_distance ) {
                    break;
                }

            }

        }

        // Create node
        auto alloc_data =
            allocator->create_new_tree_node<BN>(
                    NodeHandleType(BRANCH_NODE) );

        new (&(*alloc_data.first))
            BN( tree, nullptr,
                alloc_data.second, 0 );

        auto branch_node = alloc_data.first;
        std::vector<std::pair<IsotheticPolygon, tree_node_handle>> fixed_bb_and_handles;
        fixed_bb_and_handles.reserve( pts_to_remove.size() );

        // Figure out what the bbs are
        for( const auto &pt : pts_to_remove ) {
            auto search = point_to_handle_map.find( pt );
            assert( search != point_to_handle_map.end() );
            tree_node_handle handle = search->second;
            if( handle.get_type() == LEAF_NODE ) {
                auto node = allocator->get_tree_node<LN>( handle );
                fixed_bb_and_handles.push_back( std::make_pair(
                            IsotheticPolygon( node->boundingBox() ), handle
                            ) );
            } else {
                auto node = allocator->get_tree_node<BN>( handle );
                fixed_bb_and_handles.push_back( std::make_pair(
                            IsotheticPolygon( node->boundingBox() ), handle
                ) );
            }
        }

        // Make them all disjoint
        for( uint64_t i = 0; i < fixed_bb_and_handles.size(); i++ ) {
            for( uint64_t j = i+1; j < fixed_bb_and_handles.size(); j++ ) {

                std::vector<Rectangle> &existing_rects_a =
                    fixed_bb_and_handles.at(i).first.basicRectangles;
                std::vector<Rectangle> &existing_rects_b =
                    fixed_bb_and_handles.at(j).first.basicRectangles;
                make_all_rects_disjoint(
                    tree,
                    existing_rects_a,
                    fixed_bb_and_handles.at(i).second,
                    existing_rects_b,
                    fixed_bb_and_handles.at(j).second
                );
            }
        }

        for( uint64_t i = 0; i < fixed_bb_and_handles.size(); i++ ) {
            nirtreedisk::Branch b;
            b.child = fixed_bb_and_handles.at(i).second;
            IsotheticPolygon &constructed_poly =
                fixed_bb_and_handles.at(i).first;
            if( constructed_poly.basicRectangles.size() <=
                    MAX_RECTANGLE_COUNT ) {
                b.boundingPoly = InlineBoundedIsotheticPolygon();
                std::get<InlineBoundedIsotheticPolygon>( b.boundingPoly
                        ).push_polygon_to_disk( constructed_poly );
            } else {
                unsigned rect_size = std::min(
                        InlineUnboundedIsotheticPolygon::maximum_possible_rectangles_on_first_page(),
                        constructed_poly.basicRectangles.size() );
                auto alloc_data =
                    allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                            compute_sizeof_inline_unbounded_polygon(
                                rect_size ),
                            NodeHandleType( BIG_POLYGON ) );
                new (&(*alloc_data.first)) InlineUnboundedIsotheticPolygon( allocator, rect_size );
                alloc_data.first->push_polygon_to_disk( constructed_poly );
                b.boundingPoly = alloc_data.second;
            }
            branch_node->addBranchToNode(b);
        }

        branch_node->bounding_box_validate();
        std::cout << "Validated!" << std::endl;


        branches.push_back( branch_node->self_handle_ );

        // Remove used points
        if( pts_to_remove.size() != branch_factor and
                not loc_points.empty() ) {
            std::cout << pts_to_remove.size() << ", left: " <<
                loc_points.size() <<std::endl;
        }

        assert( pts_to_remove.size() == branch_factor or
                loc_points.empty() );

        for( const auto &pt : pts_to_remove ) {
            std::list<Point>::iterator iter = loc_points.begin();
            for( ; iter != loc_points.end() and *iter != pt; iter++ ) {}
            if( iter != loc_points.end() ) {
                loc_points.erase( iter );
            }
        }

        // Keep looping?
        if( loc_points.empty() ) {
            std::cout << "Done a branch layer." << std::endl;
            return branches;
        }
    }

    return branches;
}

template <>
std::vector<tree_node_handle> str_packing_branch(
    nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy> *tree,
    std::vector<tree_node_handle> &child_nodes,
    unsigned branch_factor
) {
    nirtreedisk::LeafNode<5,9,nirtreedisk::ExperimentalStrategy> *targ
        = nullptr;
    nirtreedisk::BranchNode<5,9,nirtreedisk::ExperimentalStrategy>
        *targ2
        = nullptr;

    return str_packing_branch( tree, child_nodes, branch_factor, targ,
            targ2 );
}

template <>
std::vector<tree_node_handle> str_packing_branch(
    rstartreedisk::RStarTreeDisk<5,9> *tree,
    std::vector<tree_node_handle> &child_nodes,
    unsigned branch_factor
) {
    rstartreedisk::LeafNode<5,9> *targ = nullptr;
    rstartreedisk::BranchNode<5,9> *targ2 = nullptr;

    return str_packing_branch( tree, child_nodes, branch_factor, targ,
            targ2 );
}

template <typename T, typename LN, typename BN>
std::vector<tree_node_handle> str_packing_leaf(
    T *tree,
    std::vector<Point>::iterator begin,
    std::vector<Point>::iterator end,
    unsigned branch_factor,
    LN *ln_type,
    BN *bn_type
) {

    uint64_t count = (end-begin);
    uint64_t P = count / branch_factor;
    if( count % branch_factor != 0  ) {
        P++;
    }

    double S_dbl = std::ceil( sqrt(P) );
    uint64_t S = (uint64_t) S_dbl;

    // Sort on lower left
    std::sort( begin, end, []( Point &l, Point &r ) {
            return l[0] < r[0]; } );

    // There are |S| vertical stripes.
    // Each has |S|*branch_factor points
    for( uint64_t i = 0; i < S; i++ ) {
        uint64_t start_offset = i * (S*branch_factor);
        uint64_t stop_offset = (i+1) * (S*branch_factor);
        auto start_point = begin + start_offset;
        auto stop_point = stop_offset >= count ? end : begin + stop_offset;

        std::sort( start_point, stop_point, []( Point &l, Point &r ) {
            return l[1] < r[1]; } );

        if( stop_point == end ) {
            break;
        }
    }

    
    tree_node_allocator *allocator = tree->node_allocator_.get();
    std::vector<tree_node_handle> leaves;
    uint64_t offset = 0;
    while( offset < count ) {

        auto alloc_data =
            allocator->create_new_tree_node<LN>( NodeHandleType(LEAF_NODE) );

        new (&(*alloc_data.first))
            LN( tree, nullptr,
                alloc_data.second, 0 );

        auto leaf_node = alloc_data.first;
        tree_node_handle leaf_handle = alloc_data.second;
        for( uint64_t i = 0; i < branch_factor; i++ ) {
            leaf_node->addPoint( *(begin + offset) );
            offset++;
            if( offset == count ) {
                break;
            }
        }
        leaves.push_back( leaf_handle );
    }
    return leaves;
}

template <typename T, typename LN, typename BN>
std::vector<tree_node_handle> str_packing_leaf_euclidean(
    T *tree,
    std::vector<Point>::iterator begin,
    std::vector<Point>::iterator end,
    unsigned branch_factor,
    LN *ln_type,
    BN *bn_type
) {
    std::cout << "Euclidean leaf pack." << std::endl;

    tree_node_allocator *allocator = tree->node_allocator_.get();

    std::vector<tree_node_handle> leaves;
    uint64_t count = (end-begin);
    leaves.reserve( count / branch_factor + 1 );

    // Sort on X, break ties with Y.
    std::sort( begin, end, point_comparator );
    
    // Copy
    std::list<Point> loc_points( begin, end );

    for( ;; ) {

        Point cur_point = loc_points.front();
        double max_allowed_distance =
            std::numeric_limits<double>::max();
        uint64_t worst_index = 0;
        std::vector<Point> pts_to_remove;
        pts_to_remove.reserve( branch_factor );
        pts_to_remove.push_back( cur_point );

        loc_points.pop_front();

        // Find closest points
        uint64_t i = 0;
        for( auto list_iter = loc_points.begin(); list_iter != loc_points.end(); list_iter++ ) {
            if( pts_to_remove.size() < branch_factor ) {
                pts_to_remove.push_back( *list_iter );
                if( pts_to_remove.size() == branch_factor ) {
                    // This might be expensive too
                    auto res = compute_max_dist( cur_point, pts_to_remove );
                    worst_index = res.first;
                    max_allowed_distance = res.second;
                }
            } else  {
                double x_val = ((*list_iter)[0] - cur_point[0]);
                x_val *= x_val;
                double dist = ((*list_iter)[1] - cur_point[1]);
                dist *= dist;
                dist += x_val;
 
                if( dist < max_allowed_distance ) {
                    pts_to_remove.at( worst_index ) = *list_iter;
                    auto res = compute_max_dist( cur_point, pts_to_remove );
                    worst_index = res.first;
                    assert( res.second <= max_allowed_distance );
                    max_allowed_distance = res.second;
                }

                if( x_val > max_allowed_distance ) {
                    std::cout << "Broke out after: " << i << std::endl;
                    break;
                }

            }
            i++;

        }

        // Create node
        auto alloc_data =
            allocator->create_new_tree_node<LN>( NodeHandleType(LEAF_NODE) );

        new (&(*alloc_data.first))
            LN( tree, nullptr,
                alloc_data.second, 0 );

        auto leaf_node = alloc_data.first;
        for( const auto &pt : pts_to_remove ) {
            leaf_node->addPoint( pt );
        }
        leaves.push_back( alloc_data.second );

        // Remove used points
        for( const auto &pt : pts_to_remove ) {
            if( pts_to_remove.size() != branch_factor and
                    not loc_points.empty() ) {
                std::cout << pts_to_remove.size() << ", left: " <<
                    loc_points.size() <<std::endl;
            }
            assert( pts_to_remove.size() == branch_factor or
                    loc_points.empty() );
            std::list<Point>::iterator iter = loc_points.begin();
            for( ; iter != loc_points.end() and *iter != pt; iter++ ) {}
            if( iter != loc_points.end() ) {
                loc_points.erase( iter );
            }
        }

        // Keep looping?
        if( loc_points.empty() ) {
            std::cout << "Done leaf layer!" << std::endl;
            return leaves;
        }
        std::cout << "Looping, remaining points: " << loc_points.size()
            << std::endl;
    }
}

template <>
std::vector<tree_node_handle> str_packing_leaf(
    nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy> *tree,
    std::vector<Point>::iterator begin,
    std::vector<Point>::iterator end,
    unsigned branch_factor
) {
    nirtreedisk::LeafNode<5,9,nirtreedisk::ExperimentalStrategy> *targ
        = nullptr;
    nirtreedisk::BranchNode<5,9,nirtreedisk::ExperimentalStrategy>
        *targ2
        = nullptr;

    return str_packing_leaf( tree, begin, end, branch_factor,
            targ, targ2 );
}

template <>
std::vector<tree_node_handle> str_packing_leaf(
    rstartreedisk::RStarTreeDisk<5,9> *tree,
    std::vector<Point>::iterator begin,
    std::vector<Point>::iterator end,
    unsigned branch_factor
) {
    rstartreedisk::LeafNode<5,9> *targ = nullptr;
    rstartreedisk::BranchNode<5,9> *targ2 = nullptr;

    return str_packing_leaf( tree, begin, end, branch_factor,
            targ, targ2 );
}

std::vector<uint64_t> find_bounding_lines(
    std::vector<Point>::iterator start,
    std::vector<Point>::iterator stop,
    unsigned d,
    unsigned partitions
) {
    std::sort( start, stop, [d]( const Point &p1, const
                Point &p2 ) { return p1[d] < p2[d]; } );

    std::vector<uint64_t> lines;
    lines.reserve( partitions+2 );
    lines.push_back( 0 );

    uint64_t count = stop-start;
    uint64_t rough_point_count_per_partition = count / partitions;
    uint64_t remainder = count % partitions;
    
    uint64_t line_bound = 0;
    for( uint64_t created_partitions = 0; created_partitions <
            partitions-1; created_partitions++ ) {

        // This is roughly where the line should go
        line_bound += rough_point_count_per_partition;
        // Adjusting for remainder
        if (remainder != 0) {
            remainder -= 1;
            line_bound += 1;
        }
        lines.push_back( line_bound ); 
    }
    lines.push_back( count ); 


    return lines;
}

tree_node_handle quad_tree_style_load(
    nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy> *tree,
    std::vector<Point>::iterator start,
    std::vector<Point>::iterator stop,
    unsigned branch_factor,
    unsigned cur_depth,
    unsigned max_depth,
    tree_node_handle parent_handle,
    std::vector<Point> &overflow
) {
    uint64_t num_els = (stop-start);
    uint64_t tiles = std::floor(sqrt(branch_factor));
    tree_node_allocator *allocator = tree->node_allocator_.get();
    if( cur_depth == max_depth ) {
        if( num_els > branch_factor ) {
            std::copy( start + branch_factor, stop, std::back_inserter( overflow ) );
            stop = start + branch_factor;
        }
        num_els = (stop-start);
        if( num_els > branch_factor ) {
            abort();
        }
        auto alloc_data =
            allocator->create_new_tree_node<nirtreedisk::LeafNode<5,9,nirtreedisk::ExperimentalStrategy>>(
                    NodeHandleType( LEAF_NODE ) );
        new (&(*(alloc_data.first)))
                nirtreedisk::LeafNode<5,9,nirtreedisk::ExperimentalStrategy>(
                    tree, parent_handle, alloc_data.second, 0 );
        
        auto leaf_node = alloc_data.first;
        auto leaf_handle = alloc_data.second;
        for( auto iter = start; iter != stop; iter++ ) {
            leaf_node->addPoint(*iter);
        }

        return leaf_handle;
        // FIXME: re-enable once sergiu's code is done
        //auto repacked_handle = leaf_node->repack( allocator );
        //allocator->free( leaf_handle, sizeof(
        //            nirtreedisk::LeafNode<5,9,nirtreedisk::ExperimentalStrategy>) );
        //return repacked_handle;
    }

    // Return a tree node handle with pointers to all of its necessary
    // children.
    auto alloc_data =
        allocator->create_new_tree_node<nirtreedisk::BranchNode<5,9,nirtreedisk::ExperimentalStrategy>>(
                NodeHandleType( BRANCH_NODE ) );
    new (&(*(alloc_data.first)))
            nirtreedisk::BranchNode<5,9,nirtreedisk::ExperimentalStrategy>(
                tree, parent_handle, alloc_data.second, (max_depth-cur_depth) );

    auto branch_node = alloc_data.first;
    tree_node_handle branch_handle = alloc_data.second;


    std::vector<uint64_t> x_lines = find_bounding_lines( start, stop, 0, tiles );
    std::vector<Rectangle> existing_boxes;

    std::vector<std::pair<IsotheticPolygon, tree_node_handle>> branch_handles;
    branch_handles.reserve( 9 );
    for( uint64_t i = 0; i < x_lines.size()-1; i++ ) {
        uint64_t x_start = x_lines.at(i);
        uint64_t x_end = x_lines.at(i+1); /* not inclusive */
        std::vector<uint64_t> y_lines = find_bounding_lines(
                 start + x_start, start + x_end, 1, tiles );
        for( uint64_t j = 0; j < y_lines.size()-1; j++ ) {
            uint64_t y_start = y_lines.at(j);
            uint64_t y_end = y_lines.at(j+1); /* not inclusive */

            std::vector<Point>::iterator sub_start = start + x_start +
                y_start;
            std::vector<Point>::iterator sub_stop = start + x_start +
                y_end;
            if( sub_start == sub_stop ) {
                continue;
            }

            tree_node_handle child_handle =  quad_tree_style_load(
                tree,
                sub_start,
                sub_stop,
                branch_factor,
                cur_depth+1,
                max_depth,
                branch_handle,
                overflow
            );

            Rectangle bbox;
            if( child_handle.get_type() == LEAF_NODE ) {
                auto leaf_node = allocator->get_tree_node<nirtreedisk::LeafNode<5,9,nirtreedisk::ExperimentalStrategy>>( child_handle );
                bbox = leaf_node->boundingBox();
            } else {
                auto branch_node = allocator->get_tree_node<nirtreedisk::BranchNode<5,9,nirtreedisk::ExperimentalStrategy>>( child_handle );
                bbox = branch_node->boundingBox();
            }
            
            IsotheticPolygon ip = IsotheticPolygon(bbox);
            std::vector<Rectangle> handle_bounding = std::vector<Rectangle>{bbox};

            for(uint64_t i = 0; i < branch_handles.size(); i++) {
                std::vector<Rectangle> &existing_rects =
                    branch_handles.at(i).first.basicRectangles;
                for( Rectangle &existing : existing_rects ) {
                    if( bbox.intersectsRectangle( existing ) ) {
                        make_all_rects_disjoint(
                            tree,
                            existing_rects,
                            branch_handles.at(i).second,
                            handle_bounding,
                            child_handle
                        );
                    }
                }
            }
            
            branch_handles.push_back(std::make_pair(ip, child_handle));

#ifndef DNDEBUG
            // Double check non-intersection - Really inefficient, but I don't see a better way 
            // of doing this.
            for( uint64_t i = 0; i < branch_handles.size(); i++ ) {
                for( uint64_t j = i+1; j < branch_handles.size(); j++ ) {
                    std::vector<Rectangle> &existing_rects_a =
                        branch_handles.at(i).first.basicRectangles;
                    std::vector<Rectangle> &existing_rects_b =
                        branch_handles.at(j).first.basicRectangles;
                    for( Rectangle &rect_a : existing_rects_a ) {
                        for( Rectangle &rect_b : existing_rects_b ) {
                            if( rect_b.intersectsRectangle( rect_a ) ) { 
                                std::cout << rect_b << " intersects: " << rect_a <<
                                    std::endl;
                                std::cout << "Cur y_lo: " <<
                                    (*(start+x_start+y_start))[1] << std::endl;
                                std::cout << "Prev y: " <<
                                    (*(start+x_start+y_start-1))[1] << std::endl;
                                abort();
                            }
                        }
                    }
                }
            }
#endif
        }
    }
    for( uint64_t i = 0; i < branch_handles.size(); i++ ) {
        nirtreedisk::Branch b;
        b.child = branch_handles.at(i).second;
        IsotheticPolygon &constructed_poly =
                branch_handles.at(i).first;
        b.boundingPoly = InlineBoundedIsotheticPolygon();
        std::get<InlineBoundedIsotheticPolygon>( b.boundingPoly
                        ).push_polygon_to_disk( constructed_poly );
        branch_node->addBranchToNode( b );
    }

    return branch_handle;
    // FIXME: re-enable once sergiu's  code is done
    /*
    // Repack
    auto repacked_handle = branch_node->repack( allocator, allocator );
    allocator->free( branch_handle, sizeof(
                nirtreedisk::BranchNode<5,9,nirtreedisk::ExperimentalStrategy>) );
    return repacked_handle;
    */
}

template <>
void bulk_load_tree(
    nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy>* tree,
    std::map<std::string,unsigned> &configU,
    std::vector<Point>::iterator begin,
    std::vector<Point>::iterator end,
    unsigned max_branch_factor
) {
    // Keep in mind there is a 0th level, so floor is correct
    uint64_t  num_els = (end - begin);
    std::cout << "Num els: " << num_els << std::endl;
    uint64_t max_depth =
        std::floor(log(num_els)/log(max_branch_factor));
    std::cout << "Max depth required: " << max_depth << std::endl;

    auto tree_ptr =
        (nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy>
         *) tree;

    std::chrono::high_resolution_clock::time_point begin_time = std::chrono::high_resolution_clock::now();
    std::vector<Point> overflow;
    tree_node_handle root = quad_tree_style_load( tree_ptr, begin, end, max_branch_factor, 0, max_depth, nullptr, overflow );
    tree->root = root;
    std::cout << "Root node: " << root << std::endl;
    std::cout << "Node Type: " << (unsigned) root.get_type() << std::endl;

    std::cout << "After bulk insert, need to insert: " << overflow.size() << " points sequentially." << std::endl;
    std::cout << "KNOWN BUG, not inserting right not, don't keep results." << std::endl;
    while( tree->hasReinsertedOnLevel.size() <= max_depth ) {
        tree->hasReinsertedOnLevel.push_back( false );
    }
    for( const auto &p : overflow ) {
        tree->insert(p);
    }
    std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - begin_time);
    std::cout << "Bulk loading NIRTree took: " << delta.count() << std::endl;


    std::string fname = "repacked_nirtree.txt";
    repack_tree( tree_ptr, fname,
            nirtreedisk::repack_subtree<5,9,nirtreedisk::ExperimentalStrategy>  );
    tree->write_metadata();
}

template <>
void bulk_load_tree(
    rstartreedisk::RStarTreeDisk<5,9>* tree,
    std::map<std::string,unsigned> &configU,
    std::vector<Point>::iterator begin,
    std::vector<Point>::iterator end,
    unsigned max_branch_factor
) {

    std::chrono::high_resolution_clock::time_point begin_time = std::chrono::high_resolution_clock::now();
    std::vector<tree_node_handle> leaves = str_packing_leaf(
        tree,
        begin,
        end,
        9
    );
    std::vector<tree_node_handle> branches = str_packing_branch( tree, leaves, 9 );
    while( branches.size() > 1 ) {
        branches = str_packing_branch( tree, branches, 9 );
    }
    tree->root = branches.at(0);
    std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - begin_time);
    std::cout << "Bulk loading tree took: " << delta.count() << std::endl;

    auto tree_ptr = (rstartreedisk::RStarTreeDisk<5,9> *) tree;
    std::string fname = "repacked_rstar.txt";
    repack_tree( tree_ptr, fname,
            rstartreedisk::repack_subtree<5,9> );
    tree->write_metadata(); 
}
