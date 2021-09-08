#include <rstartreedisk/rstartreedisk.h>
#include <rstartreedisk/node.h>

namespace rstartreedisk {

    tree_node_allocator *get_node_allocator( RStarTreeDisk *treeRef ) {
        return &(treeRef->node_allocator_);
    }

    float get_p_value( RStarTreeDisk *treeRef ) {
        return treeRef->p;
    }

    tree_node_handle get_root_handle( RStarTreeDisk *treeRef ) {
        return treeRef->root;
    }
}
