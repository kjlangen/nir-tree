#ifndef __RPLUSTREE__
#define __RPLUSTREE__

#include <cassert>
#include <vector>
#include <utility>
#include <util/geometry.h>
#include <rplustree/rPlusTreeNode.h>

typedef std::pair<std::vector<Rectangle>, std::vector<Rectangle>> Partition;
typedef std::pair<float, float> Cost;

class RPlusTree {
    unsigned minBranchFactor;
    unsigned maxBranchFactor;

    enum Orientation {ALONG_X_AXIS, ALONG_Y_AXIS};

    RPlusTreeNode* root = new RPlusTreeNode();

    void removeSubtree(RPlusTreeNode* r);

    RPlusTreeNode* chooseLeaf(Point givenPoint);

    Cost sweep(Orientation orientation, Point lowestCoordinates, float fillFactor);

    Partition partition(RPlusTreeNode* node);

    void splitNode(RPlusTreeNode* node);

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
