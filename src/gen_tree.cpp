#include <bench/randomPoints.h>
#include <nirtreedisk/node.h>
#include <nirtreedisk/nirtreedisk.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <unistd.h>

std::vector<tree_node_handle> str_packing_branch(
    nirtreedisk::NIRTreeDisk<15,25,nirtreedisk::ExperimentalStrategy>
    &tree, std::vector<tree_node_handle> &child_nodes, unsigned branch_factor
) {
    tree_node_allocator *allocator = tree.node_allocator_.get();

    std::cout << " Computing centerpoints. " << std::endl;
    // Get bbox once for everything so I'm not materializing it
    // constantly
    std::vector<std::pair<Point,tree_node_handle>> node_point_pairs;
    for( tree_node_handle &child_handle : child_nodes ) {
        Rectangle bbox;
        if( child_handle.get_type() == LEAF_NODE ) {
            auto child =
                allocator->get_tree_node<nirtreedisk::LeafNode<15,25,nirtreedisk::ExperimentalStrategy>>(
                    child_handle );
            bbox = child->boundingBox();
        } else {
            auto child =
                allocator->get_tree_node<nirtreedisk::BranchNode<15,25,nirtreedisk::ExperimentalStrategy>>(
                    child_handle );
            bbox = child->boundingBox();
        }
        node_point_pairs.push_back( std::make_pair( bbox.centrePoint(), child_handle ) );
    }

    unsigned P = node_point_pairs.size() / branch_factor;
    if( node_point_pairs.size() % branch_factor != 0  ) {
        P++;
    }
    std::cout << "Determined that there are " << P << "pages." <<
        std::endl;
    double S_dbl = std::ceil( sqrt(P) );
    unsigned S = (unsigned) S_dbl;
    std::cout << "This gives us an S value of " << S << std::endl;

    // Sort by X
    std::sort( node_point_pairs.begin(), node_point_pairs.end(), [](
                std::pair<Point, tree_node_handle> &l, std::pair<Point, tree_node_handle> &r
    ) { return l.first[0] < r.first[0]; });
    std::cout << " Done sorting by x. " << std::endl;

    std::cout << "S: " << S << std::endl;
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

    std::cout << " Done sorting by Y. " << std::endl;

    std::vector<tree_node_handle> branches;
    unsigned offset = 0;
    while( offset < node_point_pairs.size() ) {

        // Create the branch node
        auto alloc_data =
            allocator->create_new_tree_node<nirtreedisk::BranchNode<15,25,nirtreedisk::ExperimentalStrategy>>( NodeHandleType( BRANCH_NODE ) );
        new (&(*alloc_data.first))
            nirtreedisk::BranchNode<15,25,nirtreedisk::ExperimentalStrategy>(
                    &tree, nullptr, alloc_data.second, 1 );
        auto branch_node = alloc_data.first;
        tree_node_handle branch_handle = alloc_data.second;
        
        // Add up to branch factor items to it
        for( unsigned i = 0; i < branch_factor; i++ ) {
            tree_node_handle child_handle =
                node_point_pairs[offset++].second;
            Rectangle bbox;
            // Adjust parent
            if( child_handle.get_type() == LEAF_NODE ) {
                auto node =
                    allocator->get_tree_node<nirtreedisk::LeafNode<15,25,nirtreedisk::ExperimentalStrategy>>(
                            child_handle );
                node->parent = branch_handle;
                bbox = node->boundingBox();
            } else {
                auto node =
                    allocator->get_tree_node<nirtreedisk::BranchNode<15,25,nirtreedisk::ExperimentalStrategy>>(
                            child_handle );
                node->parent = branch_handle;
                bbox = node->boundingBox();
            }
            
            // Add to the branch
            branch_node->addBranchToNode( { bbox, child_handle } );
            if( offset == node_point_pairs.size() ) {
                break;
            }
        }
        branches.push_back( branch_handle );
    }
    return branches;
}


std::vector<tree_node_handle> str_packing_leaf(
        nirtreedisk::NIRTreeDisk<15,25,nirtreedisk::ExperimentalStrategy>
        &tree, std::vector<Point> &points, unsigned branch_factor ) {

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

    
    tree_node_allocator *allocator = tree.node_allocator_.get();
    std::vector<tree_node_handle> leaves;
    unsigned offset = 0;
    while( offset < points.size() ) {

        auto alloc_data =
            allocator->create_new_tree_node<nirtreedisk::LeafNode<15,25,nirtreedisk::ExperimentalStrategy>>( NodeHandleType(LEAF_NODE) );

        new (&(*alloc_data.first))
            nirtreedisk::LeafNode<15,25,nirtreedisk::ExperimentalStrategy>( &tree, nullptr,
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

int main( int argc, char **argv ) {

    PointGenerator<BenchTypeClasses::California> points;
    std::vector<Point> all_points;
    std::optional<Point> next;
    while( (next = points.nextPoint() )) {
        all_points.push_back( next.value() );
    }
    std::cout << "Point Count: " << all_points.size() << std::endl;
    std::string backing_file = "gen_nirtree.txt";
    unlink( backing_file.c_str() );
    nirtreedisk::NIRTreeDisk<15,25,nirtreedisk::ExperimentalStrategy> tree( 4096*13000, backing_file );

    std::vector<tree_node_handle> leaves = str_packing_leaf( tree, all_points, 25 );
    std::cout << "One round of packing done: " << leaves.size() << std::endl;
    std::vector<tree_node_handle> branches = str_packing_branch( tree, leaves, 25 );
    std::cout << "Two rounds of packing done: " << branches.size() << std::endl;
    branches = str_packing_branch( tree, branches, 25 );
    std::cout << "Three rounds of packing done: " << branches.size() << std::endl;
    branches = str_packing_branch( tree, branches, 25 );
    std::cout << "Four rounds of packing done: " << branches.size() << std::endl;

    return 0;
}
