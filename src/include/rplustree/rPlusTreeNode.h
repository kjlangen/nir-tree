#ifndef __RPLUSTREE_NODE__
#define __RPLUSTREE_NODE__

#include <vector>
#include <util/geometry.h>

class RPlusTreeNode {
public:
    std::vector<Rectangle> boundingBoxes;
    std::vector<RPlusTreeNode *> children;
    std::vector<Point> data;
    bool isLeaf() const;
    unsigned int numEntries() const;
};

#endif // __RPLUSTREE_NODE__
