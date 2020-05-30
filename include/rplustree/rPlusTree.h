#ifndef __RPLUSTREE__
#define __RPLUSTREE__

#include <cassert>
#include <vector>
#include <util/geometry.h>
#include <rplustree/rPlusTreeNode.h>

class RPlusTree {

public:
    RPlusTree(unsigned minBranchFactor, unsigned maxBranchFactor);

    RPlusTree(RPlusTreeNode *root);

    ~RPlusTree();

    std::vector<Point> search(Point requestedPoint);

    std::vector<Point> search(Rectangle requestedRectangle);

    void insert(Point givenPoint);

    void remove(Point givenPoint);

    unsigned checksum();

    void print();
};

#endif // __RPLUSTREE__
