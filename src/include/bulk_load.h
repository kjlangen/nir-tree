#pragma once

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
);

template <typename T, typename LN, typename BN>
void fill_branch(
    T *treeRef,
    pinned_node_ptr<BN> branch_node,
    tree_node_handle node_handle,
    std::vector<std::pair<Point,tree_node_handle>> &node_point_pairs,
    uint64_t &offset,
    unsigned branch_factor,
    LN *leaf_type
);

template <>
void fill_branch(
    nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy> *treeRef,
    pinned_node_ptr<nirtreedisk::BranchNode<5,9,nirtreedisk::ExperimentalStrategy>> branch_node,
    tree_node_handle node_handle,
    std::vector<std::pair<Point,tree_node_handle>> &node_point_pairs,
    uint64_t &offset,
    unsigned branch_factor,
    nirtreedisk::LeafNode<5,9,nirtreedisk::ExperimentalStrategy> *leaf_type
); 

template <>
void fill_branch(
    rstartreedisk::RStarTreeDisk<5,9> *treeRef,
    pinned_node_ptr<rstartreedisk::BranchNode<5,9>> branch_node,
    tree_node_handle node_handle,
    std::vector<std::pair<Point,tree_node_handle>> &node_point_pairs,
    uint64_t &offset,
    unsigned branch_factor,
    rstartreedisk::LeafNode<5,9> *leaf_type
); 

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
); 

std::pair<uint64_t,double> compute_max_dist( const Point &point, std::vector<Point> &pts );

bool point_comparator( const Point &lhs, const Point &rhs );

template <typename T, typename LN, typename BN>
std::vector<tree_node_handle> str_packing_branch_euclidean(
    T* tree,
    std::vector<tree_node_handle> &child_nodes,
    unsigned branch_factor,
    LN *leaf_node_type,
    BN *branch_node_type
);

template <>
std::vector<tree_node_handle> str_packing_branch(
    nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy> *tree,
    std::vector<tree_node_handle> &child_nodes,
    unsigned branch_factor
); 

template <>
std::vector<tree_node_handle> str_packing_branch(
    rstartreedisk::RStarTreeDisk<5,9> *tree,
    std::vector<tree_node_handle> &child_nodes,
    unsigned branch_factor
);

template <typename T, typename LN, typename BN>
std::vector<tree_node_handle> str_packing_leaf(
    T *tree,
    std::vector<Point>::iterator begin,
    std::vector<Point>::iterator end,
    unsigned branch_factor,
    LN *ln_type,
    BN *bn_type
); 

template <typename T, typename LN, typename BN>
std::vector<tree_node_handle> str_packing_leaf_euclidean(
    T *tree,
    std::vector<Point>::iterator begin,
    std::vector<Point>::iterator end,
    unsigned branch_factor,
    LN *ln_type,
    BN *bn_type
);

template <typename T>
std::vector<tree_node_handle> str_packing_leaf(
    T *tree,
    std::vector<Point>::iterator begin,
    std::vector<Point>::iterator end,
    unsigned branch_factor
);

template <>
std::vector<tree_node_handle> str_packing_leaf(
    nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy> *tree,
    std::vector<Point>::iterator begin,
    std::vector<Point>::iterator end,
    unsigned branch_factor
); 

template <>
std::vector<tree_node_handle> str_packing_leaf(
    rstartreedisk::RStarTreeDisk<5,9> *tree,
    std::vector<Point>::iterator begin,
    std::vector<Point>::iterator end,
    unsigned branch_factor
); 

std::vector<uint64_t> find_bounding_lines(
    std::vector<Point>::iterator start,
    std::vector<Point>::iterator stop,
    unsigned d,
    unsigned partitions
); 

tree_node_handle quad_tree_style_load(
    nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy> *tree,
    std::vector<Point>::iterator start,
    std::vector<Point>::iterator stop,
    unsigned branch_factor,
    unsigned cur_depth,
    unsigned max_depth,
    tree_node_handle parent_handle
); 

template <typename T>
void bulk_load_tree(
    T* tree,
    std::map<std::string,unsigned> &configU,
    std::vector<Point>::iterator begin,
    std::vector<Point>::iterator end,
    unsigned max_branch_factor
);

template <>
void bulk_load_tree(
    nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy>* tree,
    std::map<std::string,unsigned> &configU,
    std::vector<Point>::iterator begin,
    std::vector<Point>::iterator end,
    unsigned max_branch_factor
);

template <>
void bulk_load_tree(
    rstartreedisk::RStarTreeDisk<5,9>* tree,
    std::map<std::string,unsigned> &configU,
    std::vector<Point>::iterator begin,
    std::vector<Point>::iterator end,
    unsigned max_branch_factor
);
