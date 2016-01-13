#ifndef SVO_TREE_HPP
#define SVO_TREE_HPP 1

#include "svo_tree.capi.h"
#include "svo_tree.fwd.hpp"
#include "svo_buffer.hpp"
#include "svo_formatters.hpp"
#include <cassert>
#include <bitset>
#include <iostream>
#include <vector>
#include <array>
#include <tuple>
#include <cstddef>
#include <cstdint>
#include <list>
#include <map>
#include <set>
#include <memory>
#include <string>



namespace svo{


struct svo_bad_alloc : std::runtime_error{
    svo_bad_alloc()
        : std::runtime_error("Could not allocate enough contiguous memory in the GPU buffer")
    {}
};

struct svo_block_full : std::runtime_error{
    svo_block_full()
        : std::runtime_error("Block is unexpectedly full")
    {}
};



////////////////////////////////////////////////////////////////////////////////
//// Utility.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//// Slice maintenance.
////////////////////////////////////////////////////////////////////////////////


svo_slice_t* svo_init_slice(std::size_t level, vside_t side, void* userdata=0);
svo_slice_t* svo_clone_slice(const svo_slice_t* slice, bool recursive=true);
void svo_uninit_slice(svo_slice_t* slice, bool recursive=true);
/**
 * Attaches a child slice to a parent slice.
 *
 * @param parent
 *      the parent slice.
 * @param child
 *      the child slice.
 * @param position
 *      the "lower" position of the child slice cube-space within the parent's cube-space.
 *
 * The child slice must be one level higher than the parent slice.
 * The child slice must have no current parent; @c child->parent_slice must be 0.
 * The child's bounds within the parent's cube-space are determined by the child's @c side property,
 *  and the @c position paremeter.
 * This function ignores and sets/overwrites the child's @c parent_vcurve_begin parameter to
 *  @c position; they are equivalent in meaning.
 * The child must not overlap with any other child's cube-space within the parent slice.
 *  
 */
void svo_slice_attach_child(svo_slice_t* parent, svo_slice_t* child, vcurve_t position);

void svo_slice_detatch(svo_slice_t* slice);

void svo_split_slice(const svo_slice_t* src_slice, std::vector<svo_slice_t*>& dst_slices);
/**
 * Joins 8 sibling slices into one larger slice.
 *
 * The siblings, must be of the same size. They must be next to eachother, and their combined
 *  new node must be properly aligned. See @c svo_find_joinable_groups_of_children() for
 *  methods of finding such groups of children.
 *
 * @param dst_slice
 *          A newly initialized slice that will replace the siblings
 * @param src_slices
 *          A list of siblings that will be replaced. Can be of varying size, but cannot be
 *           greater than 8. Also, they must be in morton order of their position within the parent.
 *          Also, for convenience, null src slices will be ignored (in case those siblings are empty
 *           for example).
 *
 * Note that the src slices are not freed; you must do this yourself after calling this function.
 */
void svo_join_slices(svo_slice_t* dst_slice, const std::array<svo_slice_t*, 8>& src_slices);




bool svo_is_slice_loading(svo_slice_t* slice);
bool svo_is_slice_loaded(svo_slice_t* slice);


///find the children among the children of @c slice that are candidates for
/// combination, by virtue of being next to other candidates for combination
/// (or such other candidates being empty)
std::list<svo_slice_child_group_t> svo_find_joinable_groups_of_children(svo_slice_t* slice);


////////////////////////////////////////////////////////////////////////////////
//// Block maintenance.
////////////////////////////////////////////////////////////////////////////////


//svo_block_t* svo_allocate_block(svo_tree_t* tree, std::size_t size, svo_slice_t* slice, std::size_t root_level, vside_t side);
//void svo_deallocate_block(svo_tree_t* tree, svo_block_t* block);

//svo_block_t* svo_init_block(std::size_t root_level, vside_t side, void* userdata=0);
//void svo_uninit_block(svo_block_t* block);

///primitive method to append a page header to block-data. use at ur own risk.
goffset_t svo_append_ph(byte_t* address_space, svo_block_t* block);
///primitive method to append a page header to block-data. use at ur own risk.
goffset_t svo_append_cd(byte_t* address_space, svo_block_t* block, const child_descriptor_t* cd);
goffset_t svo_append_dummy_cd(byte_t* address_space, svo_block_t* block);
//svo_error_t svo_block_append_slice_data(byte_t* address_space, svo_block_t* block, svo_slice_t* slice);

svo_error_t svo_block_initialize_slice_data(std::vector<svo_block_t*>& new_leaf_blocks, svo_tree_t* tree, svo_block_t* block, svo_slice_t* slice);
svo_error_t svo_load_next_slice(std::vector<svo_block_t*>& new_leaf_blocks, svo_block_t* block);
//void load_next_slices(std::vector<svo_block_t*>& resulting_leaf_blocks, svo_tree_t* tree, svo_block_t* block);


template<typename svo_block_type, typename metadata_t, typename visitor_f>
inline void preorder_traverse_blocks(svo_block_type* block0, metadata_t metadata0, visitor_f visitor);

template<typename metadata_t, typename visitor_f>
inline void preorder_traverse_block_cds(const byte_t* address_space, const svo_block_t* block, metadata_t metadata0, visitor_f visitor);

/**
 * See http://cs.stackexchange.com/q/49319/2755
 */
template<typename metadata_t, typename visitor_f>
inline void z_preorder_traverse_block_cds(const byte_t* address_space, const svo_block_t* block, metadata_t metadata0, visitor_f visitor, bool debug=false);


////////////////////////////////////////////////////////////////////////////////
//// traversal functions.
////////////////////////////////////////////////////////////////////////////////



template<typename svo_slice_type, typename metadata_t, typename visitor_f>
inline void preorder_traverse_slices(svo_slice_type* root, metadata_t metadata0, visitor_f visitor);
template<typename svo_slice_type, typename visitor_f>
inline void preorder_traverse_slices(svo_slice_type* root, visitor_f visitor);

template<typename metadata_t, typename svo_slice_type, typename visitor_f>
inline metadata_t
postorder_traverse_slices(svo_slice_type* node0, visitor_f visitor);


/**
 * Callback for preorder_traverse_block_cds().
 * Use std::bind to bind the @c tree parameter.
 * Initialize preorder_traverse_block_cds() with metadata = level = 0.
 *
 */
inline std::size_t print_cd_tree(svo_tree_t* tree, goffset_t pcd_goffset, goffset_t cd_goffset, ccurve_t cd_ccurve, std::size_t level);

////////////////////////////////////////////////////////////////////////////////
//// Structures.
////////////////////////////////////////////////////////////////////////////////


struct svo_slice_child_group_t{
    vcurve_t parent_vcurve_begin;
    vside_t group_side_in_parent;
    vcurvesize_t group_size_in_parent;
    vside_t child_side;
    std::array<svo_slice_t*, 8> group_children;
    std::size_t count;
};



struct volume_of_slices_t{
    /**
     * @param volume_side
     *          Length of the volume-of-slices dimensions, in number of slices.
     * @param slice_side
     *          Length of each slice's volume-of-voxels, in number of voxels.
     */
    volume_of_slices_t(vside_t volume_side, vside_t slice_side)
        : volume_side(volume_side), slice_side(slice_side)
    {}

    std::vector< std::tuple<vcurve_t, svo_slice_t*> > slices;
    vside_t volume_side;
    vside_t slice_side;

};



struct svo_block_t{
    svo_block_t();
    //~svo_block_t();
    void reset();
    std::size_t size() const{ return block_end - block_start; }
    std::size_t freesize() const{
        return (cdspace_end - cd_end) + (block_end - (info_goffset+sizeof(svo_info_section_t)));
    }

    bool is_in_block(goffset_t goffset, std::size_t size = 1) const;
    bool is_valid_cd_goffset(goffset_t cd_goffset) const;
    void reset_cd_data();

    void add_cd_count(goffset_t cd_goffset);
    void clear_cd_count(goffset_t cd_goffset);
    bool has_child_block(const svo_block_t* child_block) const;
    goffset_t root_children_goffset() const;
    bool has_root_children_goffset() const;
    bool check_parent_root_cd(std::vector<std::string>& issues) const;
    bool check_parent_root_cd() const;
    
    
    svo_tree_t* tree;
    svo_block_t* parent_block;

    bool trunk;

    ///the level of the root node in this block.
    std::size_t root_level;
    std::size_t height;

    ///length of the bounds of this block.
    vside_t side;

    goffset_t block_start;
    goffset_t block_end;

    goffset_t cd_start;
    goffset_t cd_end;
    goffset_t cdspace_end;

    goffset_t data_start;
    goffset_t data_end;
    goffset_t dataspace_end;

    goffset_t info_goffset;


    ///is the root of this block valid?
    bool root_valid_bit;
    ///is the root of this block a leaf?
    bool root_leaf_bit;
    ///the position of the root voxel within its parent.
    ccurve_t root_ccurve;
    ///a pointer to a CD in this block that represents the root.
    /// this CD might shadow another CD in the parent block which is the "real" CD
    /// and which would be pointing to the children within this block. This CD is
    /// here for convenience, and every update to this root must be mirrored in the
    /// parent block (if there is one).
    goffset_t root_shadow_cd_goffset;

    ///this is a pointer to a CD within the parent block
    goffset_t parent_root_cd_goffset;


    std::size_t leaf_count;
    std::size_t cd_count;

    ///list of tuples, with (vcurve_t, child descriptor, parent child descriptor global offset) tuples.
    /// these voxels are the leafs of the previous level; the ones
    /// that were just added from the last slice. Because they are leaf
    /// voxels, they do not have a child_descriptor in the block. Their parents simply
    /// mark them as leaf voxels.
    ///the vcurve_t is the 3d location within and relative to the bounds of this block.
    //typedef std::vector< std::tuple<vcurve_t, child_descriptor_t, goffset_t> > bottom_fruit_t;
    //std::unique_ptr<bottom_fruit_t> bottom_fruit;


    ///list of tuples, with (vcurve_t, child descriptor global offset, parent child descriptor global offset) tuples.
    //typedef std::vector< std::tuple<vcurve_t, goffset_t, goffset_t> > neg_level_t;
    //std::unique_ptr<neg_level_t> neg_level_1;

    typedef std::vector< svo_block_t* > child_blocks_t;
    std::unique_ptr<child_blocks_t> child_blocks;
    
    typedef svo_gpu_buffers_t buffers_t;
    std::unique_ptr<buffers_t> buffers;

    svo_slice_t* slice;

    void* userdata;
};

struct svo_tree_t{
    byte_t* address_space_mem;
    byte_t* address_space;
    std::size_t size;

    svo_block_t* root_block;


    svo_tree_t(std::size_t size, std::size_t block_size);
    svo_block_t* allocate_block(std::size_t size);
    void deallocate_block(svo_block_t* block);

    void update_block_lookup_info(svo_block_t* block);

private:

    //////////////existing block management////////////////////////////////////////////////
    ///lookup indices
    ///indexed by the amount of unused space in the block.
    ///useful for compacting
    typedef std::set< std::pair<std::size_t, svo_block_t* > > freesize2block_t;
    typedef std::set< std::pair<std::size_t, svo_block_t* > > size2block_t;
    freesize2block_t freesize2block;
    size2block_t size2block;


    ///maps blocks back to their lookup indices
    struct block_lookup_info_t{
        freesize2block_t::iterator freesize2block_iterator;
        size2block_t::iterator size2block_iterator;
    };

public:
    ///set of all blocks, and maps them to their lookup indices.
    std::map<svo_block_t*, block_lookup_info_t> blocks;
private:

    //////////////free memory management////////////////////////////////////////////////
    
    typedef std::pair<std::size_t, std::size_t> mem_range_t;
    typedef std::set< std::pair<std::size_t, mem_range_t > > size2freemem_t;
    struct free_memory_range_info_t{
        free_memory_range_info_t() : left_block(nullptr), right_block(nullptr){}
        
        size2freemem_t::iterator size2freemem_iterator;
        svo_block_t* left_block;
        svo_block_t* right_block;
    };

    size2freemem_t size2freemem;
    std::map< mem_range_t, free_memory_range_info_t > freemems;


    mem_range_t mem_malloc(std::size_t size);

    std::size_t mem_range_size(mem_range_t mem_range);
    mem_range_t find_freemem_range(std::size_t size);

    free_memory_range_info_t default_freemem_lookup_info();
    void update_freemem_lookup_info(mem_range_t mem_range);
};



/**
 * Intro: there are two relationships each child slice has to a parent slice:
 * 1. the child slice is more resolute, and has 8X the volume.
 * 2. the child slice can take up the full space of the parent, or only a portion of it (like 1/8 in an octree).
 *
 * Here we want to allow a child to vary the amount of *space* it takes up.
 * This structure is useful for when a slice is not 1/8 of the parent slice in space.
 * Note that no matter how much space a child slice takes up, the child slice's resolution is exactly
 * double that of the parent's.
 * This leads to an unfortunate problem of needing an octree within this level just to specify its location/bounds
 * within the parent slice's bounds.
 * This structure allows this slice to be a tiny portion of the parent's space, much like a "compressed quadtree" (google).
 *
 * The format is as follows:
 *
 * @c levels tracks how far down this slice would be if it wouldn't be compressed. At 0 levels down,
 *      this slice takes up all the space of the parent. At 1 level down, this slice takes up 1/8th
 *      the space of its parent. At 2 levels down, this slice takes up 1/16th the space of the parent.
 *
 * @c
 */
/*
struct svo_slice_decendant_t{

    ///number of levels down this decendant would be to take up this amount of space
    /// if not for the "comressed quadtree".

    std::size_t levels;
    vcurve_t path;
    svo_slice_t* slice;
};

struct svo_slice_intralevel_tree_t{
    svo_slice_intralevel_tree_t* parent;
    svo_slice_intralevel_tree_t* children[8];
    svo_slice_t* slice;
};
*/


///A slice represents a cube of voxels at a particular level.
struct svo_slice_t{

    ///the slice for cube on the parent level that contains this slice. Can be NULL.
    svo_slice_t* parent_slice;

    ///the distance from the root slice for this slice. 
    std::size_t level;

    ///This value is the length of the cube, in number of voxels (at this cube's resolution).
    ///If this slice is the sole child of the parent slice that contains it, and takes up the same
    /// world-space as the parent cube, then @c side would be double of parent_slice->side,
    /// as this level has double the resolution of the parent level.
    ///This value must be a power of 2, and less than or equal to twice the side of the parent slice.
    ///Maximum value right now is also 256; any more and the morton encoding functions break.
    vside_t side;

    ///This is the position of this cube within the parent cube (at the parent slice's resolution).
    vcurve_t parent_vcurve_begin;

    ///voxel data type, including a curve/position within the cube-space (defined by side),
    /// and a voxel datum, for each voxel.
    typedef std::vector< vcurve_t > pos_data_t;

    ///voxel data.
    pos_data_t* pos_data;

    //svo_buffers_t& add_channel(const std::string& name, std::size_t element_size, const std::string& type_name);

    //svo_channel_t& get_channel(const std::string& name);
    //const svo_channel_t& get_channel(const std::string& name) const;
    //bool has_channel(const std::string& name) const;
    typedef svo_cpu_buffers_t buffers_t;
    //channels_t* channels;
    svo_cpu_buffers_t* buffers;

    typedef std::vector<svo_slice_t*> children_t;
    ///This is a list of children of this slice.
    ///Can be NULL to indicate no children, which means any possible children have not yet been loaded.
    ///Can contain 0 which means that this slice contains only leafs.
    ///Can contain 1 or more children, each of which covers a cubic subsection of this cube-space, but
    /// at double the resolution. The subsection size is implied by the child's @c side property.
    ///The subsection's position is specified by the child's @c parent_vcurve_begin property.
    ///Essentially, this allows for "compressed quadtree"-style children of different sizes, but within
    /// the same level, without having a complicated tree structure.
    children_t* children;

    void* userdata;
};


} //namespace svo

///inline implementations
#include "svo_tree.inl.hpp"

#endif
