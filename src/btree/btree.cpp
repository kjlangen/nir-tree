#include "btree/btree.h"

void btree::insert(Node &root, Node &newLeaf) {
    // Should we go left or right?
    Node **child = nullptr;
    if (newLeaf.key < root.key) {
        child = &root.left;
    } else {
        child = &root.right;
    }

    // What recursive case are we in?
    if (*child == nullptr) {
        // Base Case, we have hit the bottom
        *child = &newLeaf;
    } else {
        // Recursive case, we must go deeper!
        insert(**child, newLeaf);
    }
}
