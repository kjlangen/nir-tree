#include <catch2/catch.hpp>
#include <rtreedisk/rtreedisk.h>
#include <storage/page.h>
#include <util/geometry.h>
#include <iostream>
#include <unistd.h>

using NodeType = rtreedisk::Node<3, 6>;
using TreeType = rtreedisk::RTreeDisk<3, 6>;
using BranchType = NodeType::Branch;

static tree_node_handle
createFullLeafNode(TreeType &tree, tree_node_handle parent, Point p = Point::atOrigin)
{
    // Allocate new node
    std::pair<pinned_node_ptr<NodeType>, tree_node_handle> alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    tree_node_handle node_handle = alloc_data.second;
    auto node = alloc_data.first;
    new (&(*node)) NodeType(&tree, node_handle, tree_node_handle(nullptr));

    for (unsigned i = 0; i < 6; ++i)
    {
        auto new_handle = node->insert(p);
        REQUIRE(new_handle == node_handle);
    }

    node->parent = parent;

    return node_handle;
}

TEST_CASE("RTreeDisk: testRemoveData")
{

    unlink("rdiskbacked.txt");

    // Setup a rtree::Node with some data
    TreeType tree(4096, "rdiskbacked.txt");
    tree_node_handle root = tree.root;
    auto parentNode = tree.node_allocator_.get_tree_node<NodeType>(root);

    parentNode->insert(Point(9.0, -5.0));
    parentNode->insert(Point(14.0, -3.0));
    parentNode->insert(Point(11.0, 13.0));
    parentNode->insert(Point(13.0, 13.0));

    // Remove some of the data
    parentNode->removeData(Point(13.0, 13.0));

    // Test the removal
    REQUIRE(parentNode->cur_offset_ == 3);

    unlink("rdiskbacked.txt");
}

TEST_CASE("RTreeDisk:testBoundingBox")
{
    // Test set one
    unlink("rdiskbacked.txt");
    {
        TreeType tree(4096, "rdiskbacked.txt");
        tree_node_handle root = tree.root;

        pinned_node_ptr<rtreedisk::Node<3, 6>> rootNode =
            tree.node_allocator_.get_tree_node<rtreedisk::Node<3, 6>>(root);

        std::pair<pinned_node_ptr<rtreedisk::Node<3, 6>>, tree_node_handle> alloc_data =
            tree.node_allocator_.create_new_tree_node<rtreedisk::Node<3, 6>>();
        tree_node_handle child0 = alloc_data.second;
        new (&(*alloc_data.first)) NodeType(&tree, child0, root);
        rootNode->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(Rectangle(8.0, 1.0, 12.0, 5.0), child0));

        alloc_data =
            tree.node_allocator_.create_new_tree_node<rtreedisk::Node<3, 6>>();
        tree_node_handle child1 = alloc_data.second;
        new (&(*alloc_data.first)) NodeType(&tree, child1, root);
        rootNode->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(Rectangle(12.0, -4.0, 16.0, -2.0), child1));

        alloc_data =
            tree.node_allocator_.create_new_tree_node<rtreedisk::Node<3, 6>>();
        tree_node_handle child2 = alloc_data.second;
        new (&(*alloc_data.first)) NodeType(&tree, child2, root);
        rootNode->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(Rectangle(8.0, -6.0, 10.0, -4.0), child2));

        REQUIRE(rootNode->cur_offset_ == 3);
        REQUIRE(rootNode->boundingBox() == Rectangle(8.0, -6.0, 16.0, 5.0));
    }

    unlink("rdiskbacked.txt");
    {
        // Test set two
        TreeType tree(4096, "rdiskbacked.txt");
        tree_node_handle root = tree.root;

        pinned_node_ptr<rtreedisk::Node<3, 6>> rootNode =
            tree.node_allocator_.get_tree_node<rtreedisk::Node<3, 6>>(root);

        std::pair<pinned_node_ptr<rtreedisk::Node<3, 6>>, tree_node_handle> alloc_data =
            tree.node_allocator_.create_new_tree_node<rtreedisk::Node<3, 6>>();
        tree_node_handle child0 = alloc_data.second;
        new (&(*alloc_data.first)) NodeType(&tree, child0, root);
        rootNode->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(Rectangle(8.0, 12.0, 10.0, 14.0), child0));

        alloc_data =
            tree.node_allocator_.create_new_tree_node<rtreedisk::Node<3, 6>>();
        tree_node_handle child1 = alloc_data.second;
        new (&(*alloc_data.first)) NodeType(&tree, child1, root);
        rootNode->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(Rectangle(10.0, 12.0, 12.0, 14.0), child1));

        alloc_data =
            tree.node_allocator_.create_new_tree_node<rtreedisk::Node<3, 6>>();
        tree_node_handle child2 = alloc_data.second;
        new (&(*alloc_data.first)) NodeType(&tree, child2, root);
        rootNode->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(Rectangle(12.0, 12.0, 14.0, 14.0), child2));

        REQUIRE(rootNode->cur_offset_ == 3);

        REQUIRE(rootNode->boundingBox() == Rectangle(8.0, 12.0, 14.0, 14.0));
    }
    unlink("rdiskbacked.txt");
}

TEST_CASE("RTreeDisk: testSearch")
{
    // Build the tree directly

    // Cluster 1, n = 7
    // (-8, 16), (-3, 16), (-5, 15), (-3, 15), (-6, 14), (-4, 13), (-5, 12)
    unlink("rdiskbacked.txt");
    {
        TreeType tree(4096 * 5, "rdiskbacked.txt");
        auto root = tree.root;
        auto rootNode = tree.node_allocator_.get_tree_node<NodeType>(
            root);

        auto alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        new (&(*alloc_data.first)) NodeType(&tree, alloc_data.second,
                                            tree_node_handle(nullptr));
        auto cluster1a_handle = alloc_data.second;
        auto cluster1a = alloc_data.first;
        cluster1a->addEntryToNode(Point(-3.0, 16.0));
        cluster1a->addEntryToNode(Point(-3.0, 15.0));
        cluster1a->addEntryToNode(Point(-4.0, 13.0));

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        new (&(*alloc_data.first)) NodeType(&tree, alloc_data.second,
                                            tree_node_handle(nullptr));
        auto cluster1b_handle = alloc_data.second;
        auto cluster1b = alloc_data.first;
        cluster1b->addEntryToNode(Point(-5.0, 12.0));
        cluster1b->addEntryToNode(Point(-5.0, 15.0));
        cluster1b->addEntryToNode(Point(-6.0, 14.0));
        cluster1b->addEntryToNode(Point(-8.0, 16.0));

        // Cluster 2, n = 8
        // (-14, 8), (-10, 8), (-9, 10), (-9, 9), (-8, 10), (-9, 7), (-8, 8), (-8, 9)
        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        new (&(*alloc_data.first)) NodeType(&tree, alloc_data.second,
                                            tree_node_handle(nullptr));
        auto cluster2a_handle = alloc_data.second;
        auto cluster2a = alloc_data.first;
        cluster2a->addEntryToNode(Point(-8.0, 10.0));
        cluster2a->addEntryToNode(Point(-9.0, 10.0));
        cluster2a->addEntryToNode(Point(-8.0, 9.0));
        cluster2a->addEntryToNode(Point(-9.0, 9.0));
        cluster2a->addEntryToNode(Point(-8.0, 8.0));

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        new (&(*alloc_data.first)) NodeType(&tree, alloc_data.second,
                                            tree_node_handle(nullptr));
        auto cluster2b_handle = alloc_data.second;
        auto cluster2b = alloc_data.first;
        cluster2b->addEntryToNode(Point(-14.0, 8.0));
        cluster2b->addEntryToNode(Point(-10.0, 8.0));
        cluster2b->addEntryToNode(Point(-9.0, 7.0));

        // Cluster 3, n = 9
        // (-5, 4), (-3, 4), (-2, 4), (-4, 3), (-1, 3), (-6, 2), (-4, 1), (-3, 0), (-1, 1)
        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        new (&(*alloc_data.first)) NodeType(&tree, alloc_data.second,
                                            tree_node_handle(nullptr));
        auto cluster3a_handle = alloc_data.second;
        auto cluster3a = alloc_data.first;
        cluster3a->addEntryToNode(Point(-3.0, 4.0));
        cluster3a->addEntryToNode(Point(-3.0, 0.0));
        cluster3a->addEntryToNode(Point(-2.0, 4.0));
        cluster3a->addEntryToNode(Point(-1.0, 3.0));
        cluster3a->addEntryToNode(Point(-1.0, 1.0));

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        new (&(*alloc_data.first)) NodeType(&tree, alloc_data.second,
                                            tree_node_handle(nullptr));
        auto cluster3b_handle = alloc_data.second;
        auto cluster3b = alloc_data.first;
        cluster3b->addEntryToNode(Point(-5.0, 4.0));
        cluster3b->addEntryToNode(Point(-4.0, 3.0));
        cluster3b->addEntryToNode(Point(-4.0, 1.0));
        cluster3b->addEntryToNode(Point(-6.0, 2.0));

        // High level rtree::Nodes
        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        new (&(*alloc_data.first)) NodeType(&tree, alloc_data.second,
                                            tree_node_handle(nullptr));
        auto left_handle = alloc_data.second;
        auto left = alloc_data.first;
        cluster1a->parent = left_handle;
        left->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(cluster1a->boundingBox(),
                             cluster1a_handle));
        cluster1b->parent = left_handle;
        left->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(cluster1b->boundingBox(),
                             cluster1b_handle));
        cluster2a->parent = left_handle;
        left->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(cluster2a->boundingBox(),
                             cluster2a_handle));
        cluster2b->parent = left_handle;
        left->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(cluster2b->boundingBox(),
                             cluster2b_handle));

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        new (&(*alloc_data.first)) NodeType(&tree, alloc_data.second,
                                            tree_node_handle(nullptr));
        auto right_handle = alloc_data.second;
        auto right = alloc_data.first;
        cluster3a->parent = right_handle;
        right->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(cluster3a->boundingBox(),
                              cluster3a_handle));
        cluster3b->parent = right_handle;
        right->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(cluster3b->boundingBox(),
                              cluster3b_handle));

        left->parent = root;
        rootNode->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(left->boundingBox(),
                                 left_handle));
        right->parent = root;
        rootNode->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(right->boundingBox(),
                                 right_handle));

        // Test search

        // Test set one
        Rectangle sr1 = Rectangle(-9.0, 9.5, -5.0, 12.5);
        std::vector<Point> v1 = rootNode->search(sr1);
        REQUIRE(v1.size() == 3);
        REQUIRE(std::find(v1.begin(), v1.end(), Point(-8.0, 10.0)) != v1.end());
        REQUIRE(std::find(v1.begin(), v1.end(), Point(-9.0, 10.0)) != v1.end());
        REQUIRE(std::find(v1.begin(), v1.end(), Point(-5.0, 12.0)) != v1.end());

        // Test set two
        Rectangle sr2 = Rectangle(-8.0, 4.0, -5.0, 8.0);
        std::vector<Point> v2 = rootNode->search(sr2);
        REQUIRE(v2.size() == 2);
        REQUIRE(std::find(v2.begin(), v2.end(), Point(-5.0, 4.0)) != v2.end());
        REQUIRE(std::find(v2.begin(), v2.end(), Point(-8.0, 8.0)) != v2.end());

        // Test set three
        Rectangle sr3 = Rectangle(-8.0, 0.0, -4.0, 16.0);
        std::vector<Point> v3 = rootNode->search(sr3);
        REQUIRE(v3.size() == 12);
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-5.0, 4.0)) != v3.end());
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-4.0, 3.0)) != v3.end());
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-4.0, 1.0)) != v3.end());
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-6.0, 2.0)) != v3.end());
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-8.0, 10.0)) != v3.end());
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-8.0, 9.0)) != v3.end());
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-8.0, 8.0)) != v3.end());
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-5.0, 12.0)) != v3.end());
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-5.0, 15.0)) != v3.end());
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-6.0, 14.0)) != v3.end());
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-8.0, 16.0)) != v3.end());
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-4.0, 13.0)) != v3.end());

        // Test set four
        Rectangle sr4 = Rectangle(2.0, -4.0, 4.0, -2.0);
        std::vector<Point> v4 = rootNode->search(sr4);
        REQUIRE(v4.size() == 0);

        // Test set five
        Rectangle sr5 = Rectangle(-3.5, 1.0, -1.5, 3.0);
        std::vector<Point> v5 = rootNode->search(sr5);
        REQUIRE(v5.size() == 0);
    }
    {
        // Re-read the tree from disk, doall the searches again
        TreeType tree(4096 * 5, "rdiskbacked.txt");
        auto rootNode = tree.node_allocator_.get_tree_node<NodeType>(
            tree.root);
        // Test set one
        Rectangle sr1 = Rectangle(-9.0, 9.5, -5.0, 12.5);
        std::vector<Point> v1 = rootNode->search(sr1);
        REQUIRE(v1.size() == 3);
        REQUIRE(std::find(v1.begin(), v1.end(), Point(-8.0, 10.0)) != v1.end());
        REQUIRE(std::find(v1.begin(), v1.end(), Point(-9.0, 10.0)) != v1.end());
        REQUIRE(std::find(v1.begin(), v1.end(), Point(-5.0, 12.0)) != v1.end());

        // Test set two
        Rectangle sr2 = Rectangle(-8.0, 4.0, -5.0, 8.0);
        std::vector<Point> v2 = rootNode->search(sr2);
        REQUIRE(v2.size() == 2);
        REQUIRE(std::find(v2.begin(), v2.end(), Point(-5.0, 4.0)) != v2.end());
        REQUIRE(std::find(v2.begin(), v2.end(), Point(-8.0, 8.0)) != v2.end());

        // Test set three
        Rectangle sr3 = Rectangle(-8.0, 0.0, -4.0, 16.0);
        std::vector<Point> v3 = rootNode->search(sr3);
        REQUIRE(v3.size() == 12);
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-5.0, 4.0)) != v3.end());
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-4.0, 3.0)) != v3.end());
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-4.0, 1.0)) != v3.end());
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-6.0, 2.0)) != v3.end());
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-8.0, 10.0)) != v3.end());
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-8.0, 9.0)) != v3.end());
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-8.0, 8.0)) != v3.end());
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-5.0, 12.0)) != v3.end());
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-5.0, 15.0)) != v3.end());
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-6.0, 14.0)) != v3.end());
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-8.0, 16.0)) != v3.end());
        REQUIRE(std::find(v3.begin(), v3.end(), Point(-4.0, 13.0)) != v3.end());

        // Test set four
        Rectangle sr4 = Rectangle(2.0, -4.0, 4.0, -2.0);
        std::vector<Point> v4 = rootNode->search(sr4);
        REQUIRE(v4.size() == 0);

        // Test set five
        Rectangle sr5 = Rectangle(-3.5, 1.0, -1.5, 3.0);
        std::vector<Point> v5 = rootNode->search(sr5);
        REQUIRE(v5.size() == 0);
    }
    unlink("rdiskbacked.txt");
}


TEST_CASE("RTreeDisk:RemoveLeafNode")
{

    unlink("rdiskbacked.txt");
    {
        unsigned maxBranchFactor = 6;
        unsigned minBranchFactor = 3;
        TreeType tree(4096 * 5, "rdiskbacked.txt");

        for (unsigned i = 0; i < maxBranchFactor * maxBranchFactor + 1;
             i++)
        {
            tree.insert(Point(i, i));
        }

        for (unsigned i = 0; i < maxBranchFactor * maxBranchFactor + 1;
             i++)
        {
            Point p(i, i);
            REQUIRE(tree.search(p).size() == 1);
        }

        auto root = tree.root;
        auto node = tree.node_allocator_.get_tree_node<NodeType>(root);

        while (std::holds_alternative<NodeType::Branch>(node->entries[0]))
        {
            auto child = std::get<BranchType>( node->entries[0] ).child;
            node = tree.node_allocator_.get_tree_node<NodeType>(child);
        }

        REQUIRE( std::holds_alternative<Point>(node->entries[0]) );
        REQUIRE(node->cur_offset_ > 0);
        size_t cnt = node->cur_offset_;
        std::vector<NodeType::NodeEntry> nodesToRemove(node->entries.begin(), node->entries.begin() + (cnt - minBranchFactor + 1));
        for (const auto &entry : nodesToRemove)
        {
            const Point &p = std::get<Point>(entry);
            tree.remove(p);
        }

        for (unsigned i = 0; i < maxBranchFactor * maxBranchFactor + 1; ++i)
        {
            Point p(i, i);
            NodeType::NodeEntry ne = p;
            if (std::find(nodesToRemove.begin(), nodesToRemove.end(), ne) == nodesToRemove.end())
            {
                REQUIRE(tree.search(p).size() == 1);
            }
            else
            {
                REQUIRE(tree.search(p).size() == 0);
            }
        }
    }

    unlink("rdiskbacked.txt");
}

TEST_CASE("RTreeDisk:testChooseLeaf")
{
    // Create rtree::Nodes
    unlink("rdiskbacked.txt");

    // Need a bunch of pages so we don't run out of memory while
    // everything is pinned
    TreeType tree(4096 * 4, "rdiskbacked.txt");
    tree_node_handle root = tree.root;

    auto rootNode = tree.node_allocator_.get_tree_node<NodeType>(root);

    auto alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto leftNode = alloc_data.first;
    tree_node_handle left = alloc_data.second;
    new (&(*leftNode)) NodeType(&tree, left, root);

    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto rightNode = alloc_data.first;
    tree_node_handle right = alloc_data.second;
    new (&(*rightNode)) NodeType(&tree, right, root);

    tree_node_handle leftChild0 = createFullLeafNode(tree, left);
    tree_node_handle leftChild1 = createFullLeafNode(tree, left);
    tree_node_handle leftChild2 = createFullLeafNode(tree, left);
    leftNode->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(
        Rectangle(8.0, 12.0, 10.0, 14.0), leftChild0));
    leftNode->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(
        Rectangle(10.0, 12.0, 12.0, 14.0), leftChild1));
    leftNode->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(
        Rectangle(12.0, 12.0, 14.0, 14.0), leftChild2));
    rootNode->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(
        Rectangle(8.0, 12.0, 14.0, 14.0), left));

    tree_node_handle rightChild0 = createFullLeafNode(tree, right);
    tree_node_handle rightChild1 = createFullLeafNode(tree, right);
    tree_node_handle rightChild2 = createFullLeafNode(tree, right);
    rightNode->parent = root;
    rightNode->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(
        Rectangle(8.0, 1.0, 12.0, 5.0), rightChild0));
    rightNode->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(
        Rectangle(12.0, -4.0, 16.0, -2.0), rightChild1));
    rightNode->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(
        Rectangle(8.0, -6.0, 10.0, -4.0), rightChild2));
    rootNode->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(
        Rectangle(8.0, -6.0, 16.0, 5.0), right));

    // Test that we get the correct child for the given point
    REQUIRE(rightChild1 == rootNode->chooseLeaf(Point(13.0, -3.0)));
    REQUIRE(leftChild0 == rootNode->chooseLeaf(Point(8.5, 12.5)));
    REQUIRE(leftChild2 == rootNode->chooseLeaf(Point(13.5, 13.5)));
    REQUIRE(rightChild0 == rootNode->chooseLeaf(Point(7.0, 3.0)));
    REQUIRE(leftChild1 == rootNode->chooseLeaf(Point(11.0, 15.0)));
    REQUIRE(leftChild0 == rootNode->chooseLeaf(Point(4.0, 8.0)));

    unlink("rdiskbacked.txt");
}

TEST_CASE("RTreeDisk: testSplitNonLeafNode")
{
    unlink("rdiskbacked.txt");
    {
        unsigned maxBranchFactor = 6;
        TreeType tree(4096 * 5, "rdiskbacked.txt");
        auto root_handle = tree.root;
        auto rootNode = tree.node_allocator_.get_tree_node<NodeType>(
            root_handle);

        for (unsigned i = 0; i < maxBranchFactor; i++)
        {
            auto child_handle = createFullLeafNode(tree, tree.root);
            auto childNode = tree.node_allocator_.get_tree_node<NodeType>(
                child_handle);
            rootNode->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(childNode->boundingBox(), child_handle));
        }

        std::vector<Point> accumulator = tree.search(Point::atOrigin);

        REQUIRE(accumulator.size() == maxBranchFactor * maxBranchFactor);

        REQUIRE(rootNode->cur_offset_ == maxBranchFactor);
        tree.insert(Point(0.0, 0.0));
        auto new_root_handle = tree.root;
        REQUIRE(new_root_handle != root_handle);
        auto newRootNode = tree.node_allocator_.get_tree_node<NodeType>(
            new_root_handle);

        // Confirm tree structure
        REQUIRE(newRootNode->cur_offset_ == 2);
        const auto bLeft = std::get<BranchType>( newRootNode->entries[0] ).child;
        const auto bRight = std::get<BranchType>( newRootNode->entries[1] ).child;
        auto left = tree.node_allocator_.get_tree_node<NodeType>(
            bLeft);
        auto right = tree.node_allocator_.get_tree_node<NodeType>(
            bRight);

        REQUIRE(left->cur_offset_ == 3);
        REQUIRE(right->cur_offset_ == 4);

        for (size_t i = 0; i < left->cur_offset_; i++)
        {

            auto child_handle = std::get<BranchType>( left->entries.at(i) ).child;
            auto childNode =
                tree.node_allocator_.get_tree_node<NodeType>(
                    child_handle);

            // These are all leaves
            REQUIRE(childNode->cur_offset_ > 0);
        }

        for (size_t i = 0; i < right->cur_offset_; i++)
        {
            auto child_handle = std::get<BranchType>( right->entries.at(i) ).child;
            auto childNode =
                tree.node_allocator_.get_tree_node<NodeType>(
                    child_handle);
        }

        // Count
        Point p = Point(0.0, 0.0);
        accumulator = newRootNode->search(p);
        REQUIRE(accumulator.size() == maxBranchFactor * maxBranchFactor + 1);
    }
    unlink("rdiskbacked.txt");
}

TEST_CASE("RTreeDisk:ExhaustiveSearch")
{

    unlink("rdiskbacked.txt");
    {
        unsigned maxBranchFactor = 6;
        TreeType tree(4096 * 5, "rdiskbacked.txt");
        tree_node_handle root = tree.root;
        auto rootNode = tree.node_allocator_.get_tree_node<NodeType>(root);

        for (unsigned i = 0; i < maxBranchFactor * maxBranchFactor + 1;
             i++)
        {
            tree.insert(Point(i, i));
        }

        for (unsigned i = 0; i < maxBranchFactor * maxBranchFactor + 1;
             i++)
        {
            Point p(i, i);
            REQUIRE(tree.exhaustiveSearch(p).size() == 1);
        }
    }
}

TEST_CASE("RTreeDisk: testFindLeaf")
{
    // Setup the tree
    unlink("rdiskbacked.txt");

    TreeType tree(4096 * 5, "rdiskbacked.txt");
    tree_node_handle root = tree.root;
    auto rootNode = tree.node_allocator_.get_tree_node<NodeType>(root);

    auto alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster4aNode = alloc_data.first;
    tree_node_handle cluster4a = alloc_data.second;
    new (&(*cluster4aNode)) NodeType(&tree, cluster4a,
                                     tree_node_handle());
    cluster4aNode->addEntryToNode(Point(-10.0, -2.0));
    cluster4aNode->addEntryToNode(Point(-12.0, -3.0));
    cluster4aNode->addEntryToNode(Point(-11.0, -3.0));
    cluster4aNode->addEntryToNode(Point(-10.0, -3.0));

    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster4bNode = alloc_data.first;
    tree_node_handle cluster4b = alloc_data.second;
    new (&(*cluster4bNode)) NodeType(&tree, cluster4b,
                                     tree_node_handle());

    cluster4bNode->addEntryToNode(Point(-9.0, -3.0));
    cluster4bNode->addEntryToNode(Point(-7.0, -3.0));
    cluster4bNode->addEntryToNode(Point(-10.0, -5.0));

    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster4Node = alloc_data.first;
    tree_node_handle cluster4 = alloc_data.second;

    new (&(*cluster4Node)) NodeType(&tree, cluster4, root);
    cluster4aNode->parent = cluster4;
    cluster4Node->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(cluster4aNode->boundingBox(), cluster4a));
    cluster4bNode->parent = cluster4;
    cluster4Node->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(cluster4bNode->boundingBox(), cluster4b));

    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster5aNode = alloc_data.first;
    tree_node_handle cluster5a = alloc_data.second;
    new (&(*cluster5aNode)) NodeType(&tree, cluster5a,
                                     tree_node_handle());
    cluster5aNode->addEntryToNode(Point(-14.5, -13.0));
    cluster5aNode->addEntryToNode(Point(-14.0, -13.0));
    cluster5aNode->addEntryToNode(Point(-13.5, -13.5));
    cluster5aNode->addEntryToNode(Point(-15.0, -14.0));

    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster5bNode = alloc_data.first;
    tree_node_handle cluster5b = alloc_data.second;
    new (&(*cluster5bNode)) NodeType(&tree, cluster5b,
                                     tree_node_handle());
    cluster5bNode->addEntryToNode(Point(-14.0, -14.0));
    cluster5bNode->addEntryToNode(Point(-13.0, -14.0));
    cluster5bNode->addEntryToNode(Point(-12.0, -14.0));
    cluster5bNode->addEntryToNode(Point(-13.5, -16.0));

    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster5cNode = alloc_data.first;
    tree_node_handle cluster5c = alloc_data.second;
    new (&(*cluster5cNode)) NodeType(&tree, cluster5c,
                                     tree_node_handle());

    cluster5cNode->addEntryToNode(Point(-15.0, -14.5));
    cluster5cNode->addEntryToNode(Point(-14.0, -14.5));
    cluster5cNode->addEntryToNode(Point(-12.5, -14.5));
    cluster5cNode->addEntryToNode(Point(-13.5, -15.5));

    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster5dNode = alloc_data.first;
    tree_node_handle cluster5d = alloc_data.second;
    new (&(*cluster5dNode)) NodeType(&tree, cluster5d,
                                     tree_node_handle());
    cluster5dNode->addEntryToNode(Point(-15.0, -15.0));
    cluster5dNode->addEntryToNode(Point(-14.0, -15.0));
    cluster5dNode->addEntryToNode(Point(-13.0, -15.0));
    cluster5dNode->addEntryToNode(Point(-12.0, -15.0));
    cluster5dNode->addEntryToNode(Point(-15.0, -15.0));

    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster5Node = alloc_data.first;
    tree_node_handle cluster5 = alloc_data.second;
    new (&(*cluster5Node)) NodeType(&tree, cluster5, root);

    cluster5aNode->parent = cluster5;
    cluster5Node->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(cluster5aNode->boundingBox(), cluster5a));
    cluster5bNode->parent = cluster5;
    cluster5Node->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(cluster5bNode->boundingBox(), cluster5b));

    cluster5cNode->parent = cluster5;
    cluster5Node->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(cluster5cNode->boundingBox(), cluster5c));

    cluster5dNode->parent = cluster5;
    cluster5Node->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(cluster5dNode->boundingBox(), cluster5d));

    // Root
    rootNode->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(cluster4Node->boundingBox(), cluster4));
    rootNode->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(cluster5Node->boundingBox(), cluster5));

    // Test finding leaves
    REQUIRE(rootNode->findLeaf(Point(-11.0, -3.0)) == cluster4a);
    REQUIRE(rootNode->findLeaf(Point(-9.0, -3.0)) == cluster4b);
    REQUIRE(rootNode->findLeaf(Point(-13.5, -13.5)) == cluster5a);
    REQUIRE(rootNode->findLeaf(Point(-12.0, -14.0)) == cluster5b);
    REQUIRE(rootNode->findLeaf(Point(-12.5, -14.5)) == cluster5c);
    REQUIRE(rootNode->findLeaf(Point(-13.0, -15.0)) == cluster5d);

    unlink("rdiskbacked.txt");
}

TEST_CASE("RTreeDisk: testFindLeaf ON DISK")
{
    // Setup the tree
    unlink("rdiskbacked.txt");

    // Same test as above, but shut down the tree and later read it from
    // disk for the search

    tree_node_handle root;
    tree_node_handle cluster4;
    tree_node_handle cluster4a;
    tree_node_handle cluster4b;
    tree_node_handle cluster4c;

    tree_node_handle cluster5;
    tree_node_handle cluster5a;
    tree_node_handle cluster5b;
    tree_node_handle cluster5c;
    tree_node_handle cluster5d;

    {
        TreeType tree(4096 * 5, "rdiskbacked.txt");
        root = tree.root;
        auto rootNode = tree.node_allocator_.get_tree_node<NodeType>(root);

        auto alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster4aNode = alloc_data.first;
        cluster4a = alloc_data.second;
        new (&(*cluster4aNode)) NodeType(&tree, cluster4a,
                                         tree_node_handle());
        cluster4aNode->addEntryToNode(Point(-10.0, -2.0));
        cluster4aNode->addEntryToNode(Point(-12.0, -3.0));
        cluster4aNode->addEntryToNode(Point(-11.0, -3.0));
        cluster4aNode->addEntryToNode(Point(-10.0, -3.0));

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster4bNode = alloc_data.first;
        cluster4b = alloc_data.second;
        new (&(*cluster4bNode)) NodeType(&tree, cluster4b, tree_node_handle());

        cluster4bNode->addEntryToNode(Point(-9.0, -3.0));
        cluster4bNode->addEntryToNode(Point(-7.0, -3.0));
        cluster4bNode->addEntryToNode(Point(-10.0, -5.0));

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster4Node = alloc_data.first;
        cluster4 = alloc_data.second;

        new (&(*cluster4Node)) NodeType(&tree, cluster4, root);
        cluster4aNode->parent = cluster4;
        cluster4Node->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(cluster4aNode->boundingBox(), cluster4a));
        cluster4bNode->parent = cluster4;
        cluster4Node->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(cluster4bNode->boundingBox(), cluster4b));

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster5aNode = alloc_data.first;
        cluster5a = alloc_data.second;
        new (&(*cluster5aNode)) NodeType(&tree, cluster5a,
                                         tree_node_handle());
        cluster5aNode->addEntryToNode(Point(-14.5, -13.0));
        cluster5aNode->addEntryToNode(Point(-14.0, -13.0));
        cluster5aNode->addEntryToNode(Point(-13.5, -13.5));
        cluster5aNode->addEntryToNode(Point(-15.0, -14.0));

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster5bNode = alloc_data.first;
        cluster5b = alloc_data.second;
        new (&(*cluster5bNode)) NodeType(&tree, cluster5b,
                                         tree_node_handle());
        cluster5bNode->addEntryToNode(Point(-14.0, -14.0));
        cluster5bNode->addEntryToNode(Point(-13.0, -14.0));
        cluster5bNode->addEntryToNode(Point(-12.0, -14.0));
        cluster5bNode->addEntryToNode(Point(-13.5, -16.0));

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster5cNode = alloc_data.first;
        cluster5c = alloc_data.second;
        new (&(*cluster5cNode)) NodeType(&tree, cluster5c,
                                         tree_node_handle());

        cluster5cNode->addEntryToNode(Point(-15.0, -14.5));
        cluster5cNode->addEntryToNode(Point(-14.0, -14.5));
        cluster5cNode->addEntryToNode(Point(-12.5, -14.5));
        cluster5cNode->addEntryToNode(Point(-13.5, -15.5));

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster5dNode = alloc_data.first;
        cluster5d = alloc_data.second;
        new (&(*cluster5dNode)) NodeType(&tree, cluster5d,
                                         tree_node_handle());
        cluster5dNode->addEntryToNode(Point(-15.0, -15.0));
        cluster5dNode->addEntryToNode(Point(-14.0, -15.0));
        cluster5dNode->addEntryToNode(Point(-13.0, -15.0));
        cluster5dNode->addEntryToNode(Point(-12.0, -15.0));
        cluster5dNode->addEntryToNode(Point(-15.0, -15.0));

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster5Node = alloc_data.first;
        cluster5 = alloc_data.second;
        new (&(*cluster5Node)) NodeType(&tree, cluster5, root);

        cluster5aNode->parent = cluster5;
        cluster5Node->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(cluster5aNode->boundingBox(), cluster5a));
        cluster5bNode->parent = cluster5;
        cluster5Node->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(cluster5bNode->boundingBox(), cluster5b));

        cluster5cNode->parent = cluster5;
        cluster5Node->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(cluster5cNode->boundingBox(), cluster5c));

        cluster5dNode->parent = cluster5;
        cluster5Node->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(cluster5dNode->boundingBox(), cluster5d));

        // Root
        rootNode->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(cluster4Node->boundingBox(), cluster4));
        rootNode->addEntryToNode(rtreedisk::createBranchEntry<NodeType::NodeEntry, BranchType>(cluster5Node->boundingBox(), cluster5));
    }

    TreeType tree(4096 * 5, "rdiskbacked.txt");
    REQUIRE(root == tree.root);
    auto rootNode = tree.node_allocator_.get_tree_node<NodeType>(root);

    // Test finding leaves
    REQUIRE(rootNode->findLeaf(Point(-11.0, -3.0)) == cluster4a);
    REQUIRE(rootNode->findLeaf(Point(-9.0, -3.0)) == cluster4b);
    REQUIRE(rootNode->findLeaf(Point(-13.5, -13.5)) == cluster5a);
    REQUIRE(rootNode->findLeaf(Point(-12.0, -14.0)) == cluster5b);
    REQUIRE(rootNode->findLeaf(Point(-12.5, -14.5)) == cluster5c);
    REQUIRE(rootNode->findLeaf(Point(-13.0, -15.0)) == cluster5d);

    unlink("rdiskbacked.txt");
}
