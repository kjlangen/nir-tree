#include <bench/randomPoints.h>
#include <nirtreedisk/node.h>
#include <nirtreedisk/nirtreedisk.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <unistd.h>
#include <random>

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
        for( unsigned i = 0; i < rects_b.size(); i++ ) {
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

template <typename T, typename LN, typename BN>
void fill_branch(
    T *treeRef,
    pinned_node_ptr<BN> branch_node,
    tree_node_handle node_handle,
    std::vector<std::pair<Point,tree_node_handle>> &node_point_pairs,
    unsigned &offset,
    unsigned branch_factor,
    LN *leaf_type
);

template <>
void fill_branch(
    nirtreedisk::NIRTreeDisk<15,25,nirtreedisk::ExperimentalStrategy> *treeRef,
    pinned_node_ptr<nirtreedisk::BranchNode<15,25,nirtreedisk::ExperimentalStrategy>> branch_node,
    tree_node_handle node_handle,
    std::vector<std::pair<Point,tree_node_handle>> &node_point_pairs,
    unsigned &offset,
    unsigned branch_factor,
    nirtreedisk::LeafNode<15,25,nirtreedisk::ExperimentalStrategy> *leaf_type
) {
    using LN =
        nirtreedisk::LeafNode<15,25,nirtreedisk::ExperimentalStrategy>;
    using BN =
        nirtreedisk::BranchNode<15,25,nirtreedisk::ExperimentalStrategy>;

    std::vector<std::pair<IsotheticPolygon, tree_node_handle>>
        fixed_bb_and_handles;
    tree_node_allocator *allocator = treeRef->node_allocator_.get();

    // Add up to branch factor items to it
    for( unsigned i = 0; i < branch_factor; i++ ) {
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

    for( unsigned i = 0; i < fixed_bb_and_handles.size(); i++ ) {
        for( unsigned j = i+1; j < fixed_bb_and_handles.size(); j++ ) {

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
    for( unsigned int i = 0; i < fixed_bb_and_handles.size(); i++ ) {
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
    rstartreedisk::RStarTreeDisk<15,25> *treeRef,
    pinned_node_ptr<rstartreedisk::BranchNode<15,25>> branch_node,
    tree_node_handle node_handle,
    std::vector<std::pair<Point,tree_node_handle>> &node_point_pairs,
    unsigned &offset,
    unsigned branch_factor,
    rstartreedisk::LeafNode<15,25> *leaf_type
) {
    using LN = rstartreedisk::LeafNode<15,25>;
    using BN = rstartreedisk::BranchNode<15,25>;

    std::vector<std::pair<Rectangle, tree_node_handle>> bb_and_handles;
    tree_node_allocator *allocator = treeRef->node_allocator_.get();

    // Add up to branch factor items to it
    for( unsigned i = 0; i < branch_factor; i++ ) {
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

template <typename T>
std::vector<tree_node_handle> str_packing_branch(
    T* tree,
    std::vector<tree_node_handle> &child_nodes,
    unsigned branch_factor
);


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
    unsigned c = 0;
    for( tree_node_handle &child_handle : child_nodes ) {
        c++;
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

    unsigned P = node_point_pairs.size() / branch_factor;
    if( node_point_pairs.size() % branch_factor != 0  ) {
        P++;
    }

    double S_dbl = std::ceil( sqrt(P) );
    unsigned S = (unsigned) S_dbl;

    // Sort by X
    std::sort( node_point_pairs.begin(), node_point_pairs.end(), [](
                std::pair<Point, tree_node_handle> &l, std::pair<Point, tree_node_handle> &r
    ) { return l.first[0] < r.first[0]; });

    // There are |S| vertical stripes.
    // Each has |S|*branch_factor points
    for( unsigned i = 0; i < S; i++ ) {
        unsigned start_offset = i * (S*branch_factor);
        unsigned stop_offset = (i+1) * (S*branch_factor);
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
    unsigned offset = 0;
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

template <>
std::vector<tree_node_handle> str_packing_branch(
    nirtreedisk::NIRTreeDisk<15,25,nirtreedisk::ExperimentalStrategy> *tree,
    std::vector<tree_node_handle> &child_nodes,
    unsigned branch_factor
) {
    nirtreedisk::LeafNode<15,25,nirtreedisk::ExperimentalStrategy> *targ
        = nullptr;
    nirtreedisk::BranchNode<15,25,nirtreedisk::ExperimentalStrategy>
        *targ2
        = nullptr;

    return str_packing_branch( tree, child_nodes, branch_factor, targ,
            targ2 );
}

template <>
std::vector<tree_node_handle> str_packing_branch(
    rstartreedisk::RStarTreeDisk<15,25> *tree,
    std::vector<tree_node_handle> &child_nodes,
    unsigned branch_factor
) {
    rstartreedisk::LeafNode<15,25> *targ = nullptr;
    rstartreedisk::BranchNode<15,25> *targ2 = nullptr;

    return str_packing_branch( tree, child_nodes, branch_factor, targ,
            targ2 );
}

template <typename T, typename LN, typename BN>
std::vector<tree_node_handle> str_packing_leaf(
    T *tree,
    std::vector<Point> &points,
    unsigned branch_factor,
    LN *ln_type,
    BN *bn_type
) {

    unsigned P = points.size() / branch_factor;
    if( points.size() % branch_factor != 0  ) {
        P++;
    }
    double S_dbl = std::ceil( sqrt(P) );
    unsigned S = (unsigned) S_dbl;

    // Sort on lower left
    std::sort( points.begin(), points.end(), []( Point &l, Point &r ) {
            return l[0] < r[0]; } );

    // There are |S| vertical stripes.
    // Each has |S|*branch_factor points
    for( unsigned i = 0; i < S; i++ ) {
        unsigned start_offset = i * (S*branch_factor);
        unsigned stop_offset = (i+1) * (S*branch_factor);
        auto start_point = points.begin() + start_offset;
        auto stop_point = (i==S-1)? points.end() : points.begin() + stop_offset;
        std::sort( start_point, stop_point, []( Point &l, Point &r ) {
            return l[1] < r[1]; } );
    }

    
    tree_node_allocator *allocator = tree->node_allocator_.get();
    std::vector<tree_node_handle> leaves;
    unsigned offset = 0;
    while( offset < points.size() ) {

        auto alloc_data =
            allocator->create_new_tree_node<LN>( NodeHandleType(LEAF_NODE) );

        new (&(*alloc_data.first))
            LN( tree, nullptr,
                alloc_data.second, 0 );

        auto leaf_node = alloc_data.first;
        tree_node_handle leaf_handle = alloc_data.second;
        for( unsigned i = 0; i < branch_factor; i++ ) {
            leaf_node->addPoint( points[ offset++ ] );
            if( offset == points.size() ) {
                break;
            }
        }
        leaves.push_back( leaf_handle );
    }
    return leaves;
}



template <typename T>
std::vector<tree_node_handle> str_packing_leaf(
    T *tree,
    std::vector<Point> &points,
    unsigned branch_factor
);

template <>
std::vector<tree_node_handle> str_packing_leaf(
    nirtreedisk::NIRTreeDisk<15,25,nirtreedisk::ExperimentalStrategy> *tree,
    std::vector<Point> &points,
    unsigned branch_factor
) {
    nirtreedisk::LeafNode<15,25,nirtreedisk::ExperimentalStrategy> *targ
        = nullptr;
    nirtreedisk::BranchNode<15,25,nirtreedisk::ExperimentalStrategy>
        *targ2
        = nullptr;

    return str_packing_leaf( tree, points, branch_factor,
            targ, targ2 );
}

template <>
std::vector<tree_node_handle> str_packing_leaf(
    rstartreedisk::RStarTreeDisk<15,25> *tree,
    std::vector<Point> &points,
    unsigned branch_factor
) {
    rstartreedisk::LeafNode<15,25> *targ = nullptr;
    rstartreedisk::BranchNode<15,25> *targ2 = nullptr;

    return str_packing_leaf( tree, points, branch_factor,
            targ, targ2 );
}

template <typename T>
void bulk_load_tree(
    T* tree,
    std::map<std::string,unsigned> &configU,
    std::vector<Point> &all_points,
    unsigned max_branch_factor
) {

    std::vector<tree_node_handle> leaves = str_packing_leaf(
        tree,
        all_points,
        25
    );

    std::vector<tree_node_handle> branches = str_packing_branch( tree, leaves, 25 );
    while( branches.size() > 1 ) {
        branches = str_packing_branch( tree, branches, 25 );
    }

    tree->root = branches.at(0);
}

void generate_tree( std::map<std::string, unsigned> &configU ) {

    assert( configU["distribution"] == CALIFORNIA );

    std::string backing_file = "bulkloaded_tree.txt";
    unlink( backing_file.c_str() );

    Index *spatialIndex;
    if( configU["tree"] == NIR_TREE ) {
        spatialIndex = new
            nirtreedisk::NIRTreeDisk<15,25,nirtreedisk::ExperimentalStrategy>(
                    4096*130000, backing_file );
        std::cout << "Created NIRTree." << std::endl;
    } else if( configU["tree"] == R_STAR_TREE ) {
        spatialIndex = new rstartreedisk::RStarTreeDisk<15,25>(
                    4096*130000, backing_file );
        std::cout << "Created R*Tree" << std::endl;
    } else {
        abort();
    }

    PointGenerator<BenchTypeClasses::California> points;
    std::vector<Point> all_points;
    std::optional<Point> next;
    while( (next = points.nextPoint() )) {
        all_points.push_back( next.value() );
    }

    std::cout << "Bulk Loading..." << std::endl;
    /*
    bulk_load_tree(
            (nirtreedisk::NIRTreeDisk<15,25,nirtreedisk::ExperimentalStrategy>
             *) spatialIndex, configU, all_points, 25 );
             */
    bulk_load_tree(
            (rstartreedisk::RStarTreeDisk<15,25> *) spatialIndex, configU, all_points, 25 );



    std::mt19937 g;
    g.seed(0);

    std::shuffle( all_points.begin(), all_points.end(), g );

    unsigned totalSearches  = 0;
	double totalTimeSearches = 0.0;
    for( Point p : all_points ) {
        std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
        std::vector<Point> out = spatialIndex->search(p);
        if( out.size() != 1 ) {
            std::cout << "Could not find " << p << std::endl;
            abort();
        }
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
        totalTimeSearches += delta.count();
        totalSearches += 1;
        std::cout << "Search OK." << std::endl;
        if( totalSearches >= 300 ) {
            break;
        }
    }

    spatialIndex->stat();

	std::cout << "Total time to search: " << totalTimeSearches << "s" << std::endl;
	std::cout << "Avg time to search: " << totalTimeSearches / totalSearches << "s" << std::endl;


    return;
}

int main( int argc, char **argv ) {

    std::map<std::string, unsigned> configU;
    configU.emplace( "tree", NIR_TREE );
    configU.emplace( "distribution", CALIFORNIA);

    int option;

    while( (option = getopt(argc,argv, "t:m:")) != -1 ) {
        switch( option ) {
            case 't': {
                configU["tree"] = (TreeType)atoi(optarg);
                break;
            }
            case 'm': {
                configU["distribution"] = (BenchType)atoi(optarg);
                break;
            }    
        }
    }

    generate_tree( configU );


    return 0;
}
