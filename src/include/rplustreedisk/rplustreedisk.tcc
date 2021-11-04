#define TREE_TEMPLATE_TYPES template <int min_branch_factor, int max_branch_factor>
#define TREE_CLASS_TYPES RPlusTreeDisk<min_branch_factor,max_branch_factor>

TREE_TEMPLATE_TYPES
TREE_CLASS_TYPES::~RPlusTreeDisk() {
    auto root_node = get_node( root_ );
    root_node->deleteSubtrees();

    // FIXME GC root;
}

TREE_TEMPLATE_TYPES
std::vector<Point> TREE_CLASS_TYPES::exhaustiveSearch(
    Point requestedPoint
) {
    std::vector<Point> v;
    auto root_node = get_node( root_ );
    root_node->exhaustiveSearch( requestedPoint, v );

    return v;
}

TREE_TEMPLATE_TYPES
std::vector<Point> TREE_CLASS_TYPES::search(
    Point requestedPoint
) {
    auto root_node = get_node( root_ );
    return root_node->search( requestedPoint );
}

TREE_TEMPLATE_TYPES
std::vector<Point> TREE_CLASS_TYPES::search(
    Rectangle requestedRectangle
) {
    auto root_node = get_node( root_ );
    return root_node->search( requestedRectangle );
}

TREE_TEMPLATE_TYPES
void TREE_CLASS_TYPES::insert(
    Point givenPoint
) {
    auto root_node = get_node( root_ );
    root_ = root_node->insert( givenPoint );
}

TREE_TEMPLATE_TYPES
void TREE_CLASS_TYPES::remove(
    Point givenPoint
) {
    auto root_node = get_node( root_ );
    root_ = root_node->remove( givenPoint );
}

TREE_TEMPLATE_TYPES
unsigned TREE_CLASS_TYPES::checksum()
{
    auto root_node = get_node( root_ );
    return root_node->checksum();
}

TREE_TEMPLATE_TYPES
bool TREE_CLASS_TYPES::validate()
{
    return true;
}

TREE_TEMPLATE_TYPES
void TREE_CLASS_TYPES::stat()
{
    auto root_node = get_node( root_ );
    root_node->stat();
}

TREE_TEMPLATE_TYPES
void TREE_CLASS_TYPES::print()
{
    auto root_node = get_node( root_ );
    root_node->printTree();
}

TREE_TEMPLATE_TYPES
void TREE_CLASS_TYPES::visualize()
{
    //BMPPrinter p( 1000, 1000 );
    //p.printToBMP( root_node );
}
