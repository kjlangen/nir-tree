# nir-tree++

This is a disk-backed version of the NIR-Tree. It also adds support for bulk loading, repacking of disk-nodes, and node compression.

## Architecture

Source code is in src/. Tests are in test/.

Within src/, each supported tree type has its own directory. These define a high level tree container that holds the root node and tree-level 
operations (<treetype>.h) and operations on the node types themselves (node.h). In include/, the *disk tree type folders contain implementations
with rudimentary disk support. Because there have been extensive changes to the code base, only nirtreedisk and rstartreedisk are well-supported.

The disk-trees are implemented using templates. These templates define the minimum and maximum branch factor of each node (how many entries they
contain) and in the nirtreedisk, what strategy is used for performing split operations. Templates were chosen because on disk, we want each node
to be of fixed size with the contents inline, which necessitates std::array. Since std::array needs to know its size at compile time, templates
are used to give it this information.

#### Legacy Code Choices
In the original NIR-Tree, we used a single `Node` class for each tree that implemented the logic for both leaf nodes and branch nodes.
When we ported the tree to disk, this strategy was no longer appropriate because BranchNodes in the NIRTree are much larger than LeafNodes ---
A single rectangle contains two points, and NIRTree branch nodes have polygons that may contain up to a fixed number of rectangles inline
(See MAX_RECTANGLE_COUNT in geometry.h). When the node class allocated on disk, it would need to provision enough space to hold all of these
rectangles in case the node was a branch node, which led to LeafNodes on disk being much larger than they needed to be and wasting a lot of
memory.

We addressed this issue by using seperate classes for `LeafNode`s and `BranchNode`s. Now, `LeafNode` allocations need only allocate enough space
for `max_branch_factor` points. Unfortunately, this led to a lot of code duplication between these node types because we can't use inheritance
for disk-backed nodes. Each virtual class hides a virtual pointer field in the object that is written to disk along with the rest of the objects
to quickly figure out which set of virtual methods to use. When we write the tree to disk and load it back up again in a subsequent execution of
the program, the pointer is invalid and causes segmentation faults. We should really do away with these classes entirely and just use C-structs 
and functions that take arbitrary pointers to node types to implement common functionality, but that's a lot of work so it hasn't been done yet.

#### Disk-Backing
The storage/ directory implements disk support for the tree types. The `buffer_pool` is responsible for determining which disk pages are mapped into which
main memory pages, pinning pages so that they cannot be evicted while in use, and writing data back to disk when pages are evicted. The `tree_node_allocator`
is responsible for finding space on disk pages to allocate new objects. These allocation locations are identified using `tree_node_handle`; this class
in turn defines a disk page number and an offset into the page for the allocation. It also identifies the type of object at that location --- LEAF_NODE,
BRANCH_NODE, POLYGON, etc. Trees use this type field to figure out how to interpret the bytes alloc'd there and what methods should be called to do searches
and inserts. To create a new object on disk, use `tree_node_allocator::create_new_tree_node<T>( NodeHandleType( type_code ) )`.

Allocations also return a `pinned_node_ptr<ObjectType>`, which can be thought of as a special pointer to the allocated object. You can dereference this pointer
to get the actual object, or do `&(*ptr)` to get a raw pointer to the object. You can also obtain one of these pointers for a given `tree_node_handle` using
`tree_node_allocator::get_tree_node<T>( tree_node_handle )`, which will tell the buffer pool to bring in the appropriate disk page and hand you an appropriate
pointer. pinned_node_ptr<T>` uses RAII to ensure that the buffer page underlying the pointer remains pinned in memory for the lifetime of the `pinned_node_ptr`. 
If you use raw pointers, and the pinned_node_ptr is destroyed, then the buffer pool may swap out the disk page mapped to the main memory page underlying the
pointer location. In this case, the pointer will refer to whatever is at that specific offset in that disk page, causing gnarly bugs. Pay careful attention
to pointer scopes.

#### Repacking
Each Tree implements an `insert` method, which performs a sequential insert. The trees assume that an insert is not a one-off operation, and that more inserts
to follow --- in the past, the only way to construct a tree was to use these insert operations. Therefore, inserts will "over-provision" resources when
they allocate so that future inserts will not have to do so. Concretely, if an insert operation needs to construct a new tree node, it will create a tree
node with the maximum possible space that tree node could use (e.g. enough space for `max_branch_factor` points in a `LeafNode`, or enough space for
`max_branch_factor` polygons with `MAX_RECTANGLE_COUNT` rectangles for a `BranchNode` in the NIRTree). This avoids lots of reallocations if we don't
provision enough sapce during the load stage.

However, once the tree is done loading, there will be many underfull nodes. `LeafNode`s may not contain `max_branch_points`, and most polygons contain only
a single rectangle. This means that all that extra allocated space is wasted. This means less tree data can fit in main memory because the disk pages are sparsely
utilized --- most of the tree is just dead space!

Repacking (`repacking.h`) addresses this issue by compacting tree nodes to use as little space as possible. It figures out exactly how many entries the node has, and
the precise number of rectangles it needs to represent, and allocates exactly the amount of space required. It serializes the data into a bytestream and
encodes these bytes at the allocated location. During search, the trees interpret this byte stream to determine if nodes contain the data they are interested
in.

Because the data is essentially a byte buffer, there is special object type, `packed_node`, to encapsulate the data. Because trees need different data to
implement their operations, the exact encodings differ by tree --- each tree implements their own version. Since `packed_node` may refer to either `LeafNode`
or `BranchNode`, we use NodeHandleType( REPACKED_[LEAF|BRANCH]_NODE ) so that trees know how to interpret the bytes at the allocation location. Since
`packed_node` may also be of arbitrary size, you must use the version of `create_new_tree_node<packed_node>( ... )` that allows you to supply an allocation
size. You do not need to supply this size on `get_tree_node<packed_node>` because it is assumed that an allocation never crosses page boundaries --- so the
pointer must be valid wherever the allocator brings it in.

#### Compression

Rectangle counts in polygons follow a Zipfian distribution --- most polygons contain only a few rectangles, but some polygons contain a very large number of
rectangles. When performing sequential inserts with millions of data points, it was common to have a few `BranchNode`s with hundreds of rectangles in them.
Storing all of these rectangles took a large amount of space, making the tree less memory efficient, and also took a long time to search. Compression is used
to reduce the number of bytes needed to store these polygons.

I'll elide the details of how compression works for now since it isn't necessary for the immediate objectives. At a high level, `compress_polygon` in
`compression.h` takes an iterator over a polygon's rectangles and outputs a byte buffer and integer describing the buffer's length. `decompress_polygon`
consumes this byte buffer, returns a decompressed polygon, and updates an offset pointer to where the polygon's data in the byte buffer ended.

#### Unpacking

After repacking and compression, we end up with a highly space-efficient tree. However, inserting new information into this tree is challenging --- we may need
to add more data to nodes that don't have space to fit it, or adjust the compressed bytes of a polygon. While it is theoretically possible to do this over
the encoded information, doing so is cumbersome and error-prone. Moreover, all of the existing insert and geometric operation functionality relies on the
data being in the "original" format. For these reasons, we implemented a function to "unpack" repacked nodes. This is performed as part of
`nirtreedisk::get_[leaf|branch]_node()`, so it is transparent to the insert functions. If you don't want to unpack nodes, you should use
`tree_node_allocator::get_tree_node<packed_node>()` directly.

#### Polygons and Geometric Operations

`geometry.h` defines the polygon types (and rectangles), along with all the geometric operations necessary. R-Trees use Rectangles --- NIR-Trees use
*polygon types, which may actually consist of only a single rectangle. There are three polygon types, which I have done a poor job of naming: `IsotheticPolygon`,
`InlineBoundedIsotheticPolygon`, and `InlineUnboundedIsotheticPolygon`.

`IsotheticPolygon` is the classic, in-memory polygon we used in the original NIR-Tree. They support a wide range of geometric operations, and are generally the
workhorse polygon for complex functionality. If you are fragmenting polygons around each other, you want to get a polygon in this format. The polygon's
rectangles are stored in an `std::vector`, which makes it easy to work with --- but it is important to remember that an `IsotheticPolygon` is NOT persisted
to disk. You need to write the polygon to disk in one of the following formats.

`InlineBoundedIsotheticPolygon` is a polygon that can support a fixed number of rectangles (`MAX_RECTANGLE_COUNT`). This is the type used in NIR-Tree `BranchNode`
by default. It implements the ability to write an `IsotheticPolygon` to disk at the `InlineBoundedIsotheticPolygon`'s allocation location (`push_polygon_to_disk`),
and some simple search operations so we don't have to copy every polygon into memory during search. If you have a polygon with < `MAX_RECTANGLE_COUNT` rectangles
in it, you want to use `std::get<InlineBoundedIsotheticPolygon>(b.boundingPoly).push_polygon_to_disk( &poly )` to write it to a `Branch` b.

`InlineUnboundedIsotheticPolygon` is a polygon larger than `MAX_RECTANGLE_COUNT` rectangles. This is supported in the NIR-Tree by putting a `tree_node_handle`
in the `Branch` instead of a `InlineBoundedIsotheticPolygon`, and storing the `InlineUnboundedIsotheticPolygon` at where the `tree_node_handle` points.
`InlineUnboundedIsotheticPolygon` supports multi-page polygons --- at the end of each page, a tail pointer points to the next page is stored. Each of these
follow-up pages is filled entirely with polygons, but the first page may contain less than that --- the allocation size of the original `tree_node_handle`
determines how many rectangles can be stored. See `node.tcc` in `src/include/nirtreedisk/node.tcc` for examples of how to create and store these objects.

#### Bulk Loading
  
Bulk loading support is defined in `src/gen_tree.cpp`. Note that for now, the templates hard code the tree `min_branch_factor` and `max_branch_factor`
(<5,9>) respectively.
  
There are two types of bulk loading. The first is an R-Tree-style load, which uses Sort-Tile Recursive, and is bottom-up. The second is a quad-tree-style
load, which is top-down. In the past, the NIR-Tree used an R-Tree-style load and then uses make_rectangles_disjoint to avoid intersections as we went up
the tree. This took forever and made hugely complex polygons. So, the NIRTree load is now configured to use a quad-tree style load, which for the most
part results in single-rectangle polygons.
  
The entry point for NIR-Tree bulk loading is here: https://github.com/bglasber/nir-tree/blob/master/src/gen_tree.cpp#L992

The core functionality underlying this load is `find_bounding_lines`, which determines a position along a given axis to place lines in the points array such
that the points are roughly evenly partitioned into `max_branch_factor` groups. I say roughly because the lines may need to be moved a little bit from their
ideal position so that the point immediately before the line does not share the same axis-value as the point that comes after (e.g., if we partition on x, then
point before does not have the same x-value as the point after). The reason for this is that otherwise the rectangles generated from the split would overlap,
which defeeats the non-intersection property of the NIRTree. The process of finding split lines is repeated along each access to get `max_branch_factor` total
tiles. This split process is then recursively repeated until we hit the leaf layer.
  
The end-effect of this line-movement is that some nodes at the leaf-layer may contain more than the maximum number of points than they can contain. For now,
  we take these extra overflow points, and sequential insert them afterward. This is prohibitively slow, and the key thing we wish to address.
  
The first task is to devise an algorithm that enables an even split of points at each layer by using polygons rather than simple line placements to construct
tiles. It is guaranteed that each point is unique, which means that there is necessarily a combination of rectangles that captures all the points in each node
and do not interesect with the polygons describing the regions of the other nodes.
  
  

