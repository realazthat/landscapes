
#include "landscapes/svo_tree.hpp"
#include "landscapes/svo_tree.block_mgmt.hpp"
#include "landscapes/cpputils.hpp"
#include "landscapes/svo_tree.sanity.hpp"

#include "landscapes/debug_macro.h"
#include "pempek_assert.h"

#include "format.h"

#include <iostream>
#include <bitset>
#include <algorithm>
#include <cstring>

#ifndef DEBUG_PRINT

//#define DEBUG_PRINT if(1)
#define DEBUG_PRINT if(0)

#endif

#ifndef NOOP
#define NOOP if (0)
#endif




std::ostream& operator<<(std::ostream& out, const svo::svo_error_t& error)
{
    using namespace svo;
    switch(error)
    {
        case(svo_error_t::OK):
            return out << "OK";
        case(svo_error_t::CHILDREN_TOO_FAR):
            return out << "CHILDREN_TOO_FAR";
        case(svo_error_t::BLOCK_IS_FULL):
            return out << "BLOCK_IS_FULL";
        default:
            return out << "UNKNOWN ERROR";
    }

    return out;
}


namespace svo{

/**
 * @param slice, should be a pointer to uninitialized memory.
 */
svo_slice_t* svo_init_slice(std::size_t level, vside_t side, void* userdata)
{
    svo_slice_t* slice = new svo_slice_t();

    slice->level = level;
    slice->side = side;
    slice->userdata = userdata;
    slice->pos_data = 0;
    slice->children = 0;
    slice->parent_slice = 0;
    slice->parent_vcurve_begin = 0;

    slice->pos_data = new svo_slice_t::pos_data_t();
    slice->children = new svo_slice_t::children_t();
    slice->buffers = new svo_cpu_buffers_t();



    return slice;
}




void svo_uninit_slice(svo_slice_t* slice, bool recursive)
{
    assert(slice);

    delete slice->pos_data;
    delete slice->buffers;
    
    slice->level = 0;
    slice->side = 0;
    slice->pos_data = 0;
    slice->buffers = 0;

    if (slice->children)
    {
        auto* children = slice->children;
        slice->children = 0;

        for (std::size_t child_index = 0; child_index < children->size(); ++child_index)
        {
            svo_slice_t* child = (*children)[child_index];

            if (recursive)
                svo_uninit_slice(child, true);
            (*children)[child_index] = 0;
        }

        delete children;
    }



    delete slice;
}
/*
svo_channel_t& svo_slice_t::add_channel(const std::string& name, std::size_t element_size, const std::string& type_name)
{
    assert(this->channels);

    assert(channels->count(name) == 0);

    auto success_tuple = (*channels).insert( std::make_pair(name, svo_channel_t(name, element_size, type_name)) );

    assert(success_tuple.second);

    auto w = success_tuple.first;

    auto& channel = w->second;

    return channel;
}

svo_channel_t& svo_slice_t::get_channel(const std::string& name)
{
    assert(this->channels);

    auto w = (*channels).find(name);
    assert(w != channels->end());

    return w->second;
}

const svo_channel_t& svo_slice_t::get_channel(const std::string& name) const
{
    assert(this->channels);

    auto w = (*channels).find(name);
    assert(w != channels->end());

    return w->second;
}

bool svo_slice_t::has_channel(const std::string& name) const
{
    assert(this->channels);

    auto w = (*channels).find(name);

    return w != (*channels).end();
}

*/

svo_slice_t* svo_clone_slice(const svo_slice_t* slice0, bool recursive)
{
    assert(slice0);
    assert(slice0->pos_data);
    assert(slice0->buffers);
    assert(slice0->children);
    assert(slice0->parent_slice == 0);

    DEBUG {
        if (auto error = svo_slice_sanity(slice0))
        {
            std::cerr << "error: " << error << std::endl;
            assert(false && "sanity fail");
        }
    }

    svo_slice_t* slice = svo_init_slice(slice0->level, slice0->side, slice0->userdata);

    slice->level = slice0->level;
    slice->side = slice0->side;
    slice->userdata = slice0->userdata;
    slice->parent_slice = slice0->parent_slice;
    slice->parent_vcurve_begin = slice0->parent_vcurve_begin;




    assert(slice0->pos_data);
    assert(slice0->children);
    assert(slice0->buffers);
    slice0->buffers->assert_invariants();
    assert(slice->pos_data);
    assert(slice->children);
    assert(slice->buffers);
    slice->buffers->assert_invariants();
    
    *slice->pos_data = *slice0->pos_data;
    *slice->children = *slice0->children;
    slice->buffers->reset();
    
    slice->buffers->assert_invariants();
    
    slice->buffers->copy_schema( *slice0->buffers, slice0->buffers->entries() );

    if (recursive)
    {
        for (svo_slice_t*& child : *slice->children)
        {
            child = svo_clone_slice(child, true);
        }
    }

    DEBUG {
        if (auto error = svo_slice_sanity(slice))
        {
            std::cerr << "error: " << error << std::endl;
            assert(false && "sanity fail");
        }
    }

    return slice;
}


void svo_slice_attach_child(svo_slice_t* parent, svo_slice_t* child, vcurve_t parent_vcurve_begin)
{
    DEBUG {
        if (auto err = svo_slice_sanity(parent, svo_sanity_type_t(svo_sanity_type_t::all & ~svo_sanity_type_t::all_data)))
        {
            std::cerr << err << std::endl;
            assert(false && "sanity fail");
        }
        if (auto err = svo_slice_sanity(child, svo_sanity_type_t(svo_sanity_type_t::all & ~svo_sanity_type_t::all_data & ~svo_sanity_type_t::levels)))
        {
            std::cerr << err << std::endl;
            assert(false && "sanity fail");
        }
    }

    assert(child->parent_slice == 0);
    assert(parent->level + 1 == child->level);
    assert(child->parent_vcurve_begin == 0);
    assert(child->side <= parent->side * 2);
    assert(child->side < 256);

    vcurvesize_t parent_size = vcurvesize(parent->side);
    vcurvesize_t child_size = vcurvesize(child->side);
    assert(child_size % 8 == 0);
    vcurvesize_t child_size_in_parent = child_size / 8;


    
    assert(parent_vcurve_begin < parent_size);

    child->parent_slice = parent;
    child->parent_vcurve_begin = parent_vcurve_begin;

    vcurve_t child_parent_vcurve_end = child->parent_vcurve_begin + child_size_in_parent;
    assert(child_parent_vcurve_end <= parent_size);

    assert(parent->children);

    ///where in the parent->children vector to insert this child; will be computed in a sec.
    std::size_t insertion_position = parent->children->size();

    ///for each sibling, we will check some sanity, and then find where we need to insert this child.
    ///children are stored in the morton order of the "lower" position of their cube-space within the
    /// parent slice (@c parent_vcurve_begin property).
    for (std::size_t i = 0; i < parent->children->size(); ++i)
    {
        svo_slice_t* sibling = (*parent->children)[i];

        ///sanity
        assert(sibling);
        assert(sibling->parent_slice);
        assert(sibling->parent_slice == parent);
        assert(sibling != child);

        vcurvesize_t sibling_size = vcurvesize(sibling->side);
        assert(sibling_size % 8 == 0);
        vcurvesize_t sibling_size_in_parent = sibling_size / 8;
        vcurve_t sibling_parent_vcurve_end = sibling->parent_vcurve_begin + sibling_size_in_parent;

        /// make sure the two ranges of the child and sibling do not overlap.
        assert(!overlap(sibling->parent_vcurve_begin, sibling_parent_vcurve_end-1,  child->parent_vcurve_begin, child_parent_vcurve_end-1 ));

        ///if this child's position (within the cube-space of the parent slice) comes after the position of this sibling
        if (child->parent_vcurve_begin < sibling->parent_vcurve_begin)
        {
            ///insert the child right before this particular sibling.
            insertion_position = i;
            break;
        }
    }

    ///do the insertion
    parent->children->insert(parent->children->begin() + insertion_position, child);

    DEBUG {
        if (auto err = svo_slice_sanity(parent, svo_sanity_type_t(svo_sanity_type_t::all & ~svo_sanity_type_t::all_data & ~svo_sanity_type_t::levels)))
        {
            std::cerr << err << std::endl;
            assert(false && "sanity fail");
        }
        if (auto err = svo_slice_sanity(child, svo_sanity_type_t(svo_sanity_type_t::all & ~svo_sanity_type_t::all_data & ~svo_sanity_type_t::levels)))
        {
            std::cerr << err << std::endl;
            assert(false && "sanity fail");
        }
    }
}

/*
svo_block_t* svo_init_block(std::size_t root_level, vside_t side, void* userdata)
{
    svo_block_t* block = new svo_block_t();

    block->root_level = root_level;
    block->height = 0;
    block->side = side;
    block->userdata = userdata;

    block->block_start = 0;
    block->block_end = 0;

    block->cd_start = 0;
    block->cd_end = 0;
    block->cdspace_end = 0;

    block->info_start = 0;
    block->info_end = 0;

    block->root_cd_goffset = 0;
    block->slice = 0;

    block->bottom_fruit = new svo_block_t::bottom_fruit_t();


    return block;
}*/

std::size_t svo_tree_t::mem_range_size(mem_range_t mem_range)
{
    return mem_range.second - mem_range.first;
}

svo_tree_t::mem_range_t svo_tree_t::find_freemem_range(std::size_t size)
{
    assert(size > 0);
    
    auto w = this->size2freemem.lower_bound(std::make_pair(size,mem_range_t(0,0)));
    auto wend = this->size2freemem.cend();

    for ( ; w != wend; ++w)
    {
        auto freememsize = w->first;
        auto freememrange = w->second;

        assert(mem_range_size(freememrange) == freememsize);

        if (freememsize < size)
            continue;

        
        return freememrange;
    }

    return mem_range_t(0,0);
}


svo_tree_t::mem_range_t svo_tree_t::mem_malloc(std::size_t size)
{
    assert(size % SVO_PAGE_SIZE == 0);

    auto freemem_range0 = find_freemem_range(size);

    if (freemem_range0 == mem_range_t(0,0))
        throw svo_bad_alloc();

    auto free_memory_range_info0 = freemems[freemem_range0];



    mem_range_t result = mem_range_t(freemem_range0.first, freemem_range0.first+size);
    assert(mem_range_size(result) == size);

    
    ///create the new free range
    auto freemem_range1 = freemem_range0;
    freemem_range1.first += size;

    if (mem_range_size(freemem_range1) == 0)
    {
        freemems.erase(freemem_range0);
        size2freemem.erase(free_memory_range_info0.size2freemem_iterator);
        return result;
    }


    auto free_memory_range_info1 = free_memory_range_info0;
    //free_memory_range_info1.right_block = nullptr;
    free_memory_range_info1.size2freemem_iterator = size2freemem.end();

    auto size2fremem_key = std::make_pair(mem_range_size(freemem_range1),freemem_range1);
    try{
        auto v = std::make_pair(mem_range_size(freemem_range1),freemem_range1);
        size2freemem.insert(v);
        free_memory_range_info1.size2freemem_iterator = size2freemem.find(v);

        freemems[freemem_range1] = free_memory_range_info1;
    } catch (...) {
        if (size2freemem.find(size2fremem_key) != size2freemem.end())
            size2freemem.erase(size2fremem_key);

        if (freemems.find(freemem_range1) != freemems.end())
            freemems.erase(freemem_range1);
        throw;
    }
    freemems.erase(freemem_range0);
    size2freemem.erase(free_memory_range_info0.size2freemem_iterator);

    return result;
}



svo_block_t::svo_block_t()
    : tree(nullptr)
{
    reset();
}

void svo_block_t::reset()
{
    if (tree)
        tree->deallocate_block(this);

    this->tree = nullptr;
    this->parent_block = nullptr;
    this->trunk = false;
    this->root_level = root_level;
    this->height = 0;
    this->side = side;
    this->userdata = 0;

    this->block_start = invalid_goffset;
    this->block_end = invalid_goffset;

    this->cd_start = invalid_goffset;
    this->cd_end = invalid_goffset;
    this->cdspace_end = invalid_goffset;

    this->info_goffset = invalid_goffset;

    this->root_shadow_cd_goffset = invalid_goffset;
    this->parent_root_cd_goffset = invalid_goffset;

    this->leaf_count = 0;
    this->cd_count = 0;
    
    this->slice = 0;
    this->child_blocks.reset(new child_blocks_t());
    this->buffers.reset(new svo_block_t::buffers_t(this));
}


void svo_block_t::add_cd_count(goffset_t cd_goffset)
{
    assert(is_valid_cd_goffset(cd_goffset));

    const auto* cd = svo_cget_cd(tree->address_space, cd_goffset);

    this->cd_count += 1;
    this->leaf_count += svo_get_cd_leaf_count(cd);
}


void svo_block_t::clear_cd_count(goffset_t cd_goffset)
{
    assert(is_valid_cd_goffset(cd_goffset));

    const auto* cd = svo_cget_cd(tree->address_space, cd_goffset);

    this->cd_count -= 1;
    this->leaf_count -= svo_get_cd_leaf_count(cd);
}


void svo_block_t::reset_cd_data()
{
    assert(this->tree);

    this->cd_end = this->cd_start;
    child_descriptor_t root_shadow_cd; svo_init_cd(&root_shadow_cd);

    this->root_shadow_cd_goffset = svo_append_cd(this->tree->address_space, this, &root_shadow_cd);

    assert(this->root_shadow_cd_goffset != 0);
    assert(this->root_shadow_cd_goffset != invalid_goffset);

    this->parent_root_cd_goffset = invalid_goffset;
    this->parent_block = 0;
    this->leaf_count = 0;
    this->cd_count = 0;
    this->root_valid_bit = 0;
    this->root_leaf_bit = 0;
}


bool svo_block_t::is_in_block(goffset_t goffset, std::size_t size) const
{
    assert(goffset != 0 && "THIS SHOULD NEVER HAPPEN");

    if (goffset < this->block_start)
        return false;

    if (goffset + size >= this->block_end)
        return false;
    return true;
}

bool svo_block_t::is_valid_cd_goffset(goffset_t goffset) const
{
    assert(goffset != 0 && "THIS SHOULD NEVER HAPPEN");

    ///not necessary
    NOOP {
        if (goffset == invalid_goffset)
            return false;
    }

    auto actual_imeplementation = [this, &goffset]()
    {
        if (goffset < this->cd_start )
            return false;
        if (goffset + sizeof(child_descriptor_t) > this->cd_end )
            return false;
        ///for the odd case when goffset is close to the max
        if (goffset > invalid_goffset - sizeof(child_descriptor_t) )
            return false;
        if (goffset == svo_get_ph_goffset(goffset))
            return false;
        return true;
    };

    auto result = actual_imeplementation();
    
    ///if goffset is invalid, then certainly we know the result is false.
    ///(goffset is invalid) => (goffset is out of bounds)
    assert(goffset != invalid_goffset || !result);

    return result;
}



bool svo_block_t::has_child_block(const svo_block_t* child_block) const
{
    for (const svo_block_t* child_block_i : *child_blocks)
    {
        if (child_block_i == child_block)
            return true;
    }

    return false;
}

bool svo_block_t::has_root_children_goffset() const
{
    assert(this->root_shadow_cd_goffset != 0);
    if (this->root_shadow_cd_goffset == invalid_goffset)
        return false;

    assert(this->is_valid_cd_goffset(this->root_shadow_cd_goffset));

    const auto* root_shadow_cd = svo_cget_cd(tree->address_space, this->root_shadow_cd_goffset);

    offset4_t offset4 = svo_get_child_ptr_offset4(root_shadow_cd);
    if (offset4 == 0)
        return false;
    goffset_t result = svo_get_child_ptr_goffset(tree->address_space, this->root_shadow_cd_goffset, root_shadow_cd);

    assert(result != 0);
    if (result == invalid_goffset)
        return false;
    assert(this->is_valid_cd_goffset(result));
    return true;
}

goffset_t svo_block_t::root_children_goffset() const
{
    assert(this->root_shadow_cd_goffset != 0);
    assert(this->root_shadow_cd_goffset != invalid_goffset);
    assert(this->is_valid_cd_goffset(this->root_shadow_cd_goffset));

    const auto* root_shadow_cd = svo_cget_cd(tree->address_space, this->root_shadow_cd_goffset);
    goffset_t result = svo_get_child_ptr_goffset(tree->address_space, this->root_shadow_cd_goffset, root_shadow_cd);

    assert(result > this->root_shadow_cd_goffset);
    assert(result != 0);
    assert(result != invalid_goffset);
    assert(this->is_valid_cd_goffset(result));

    return result;
}

bool svo_block_t::check_parent_root_cd(std::vector<std::string>& issues) const
{
    assert(root_shadow_cd_goffset != 0);
    assert(root_shadow_cd_goffset != invalid_goffset);
    assert(is_valid_cd_goffset(root_shadow_cd_goffset));

    if (!parent_block)
    {

        assert(parent_root_cd_goffset == invalid_goffset);
        return true;
    }

    assert(parent_root_cd_goffset != 0);
    assert(parent_root_cd_goffset != invalid_goffset);
    assert(parent_block->is_valid_cd_goffset(parent_root_cd_goffset));

    const auto* shadow_root_cd = svo_cget_cd(tree->address_space, root_shadow_cd_goffset);
    const auto* parent_root_cd = svo_cget_cd(tree->address_space, parent_root_cd_goffset);

    if (svo_get_valid_mask(shadow_root_cd) != svo_get_valid_mask(parent_root_cd))
        issues.push_back(fmt::format("svo_get_valid_mask(shadow_root_cd) != svo_get_valid_mask(parent_root_cd)"));

    if (svo_get_leaf_mask(shadow_root_cd) != svo_get_leaf_mask(parent_root_cd))
        issues.push_back(fmt::format("svo_get_leaf_mask(shadow_root_cd) != svo_get_leaf_mask(parent_root_cd)"));

    if (!has_root_children_goffset())
        issues.push_back(fmt::format("!has_root_children_goffset()"));

    if (issues.size())
        return false;

    goffset_t parent_root_children_goffset = svo_get_child_ptr_goffset(tree->address_space, parent_root_cd_goffset, parent_root_cd);

    if (parent_root_children_goffset != root_children_goffset())
        issues.push_back(fmt::format("parent_root_children_goffset != root_children_goffset()"));

    if (issues.size())
        return false;

    return true;
}

bool svo_block_t::check_parent_root_cd() const
{
    std::vector<std::string> issues;
    return check_parent_root_cd(issues);
}

svo_tree_t::svo_tree_t(std::size_t size, std::size_t block_size){


    this->address_space_mem = 0;
    this->address_space = 0;
    this->size = size;

    std::size_t alignment = SVO_PAGE_SIZE;
    ///make sure that everything is page aligned.
    assert( size % alignment == 0);

    byte_t* address_space_mem = this->address_space_mem = (byte_t*)malloc(size + alignment - 1);
    assert(address_space_mem);

    std::cout << "uintptr_t(address_space_mem): " << uintptr_t(address_space_mem) << std::endl;
    std::cout << "alignment: " << alignment << std::endl;

    uintptr_t misalignment = ifloor((uintptr_t(address_space_mem) + alignment - 1), alignment) - uintptr_t(address_space_mem);

    std::cout << "misalignment: " << misalignment << std::endl;

    assert( misalignment < alignment );

    byte_t* address_space = this->address_space = address_space_mem + misalignment;
    std::cout << "uintptr_t(address_space): " << uintptr_t(address_space) << std::endl;
    std::cout << "uintptr_t(address_space) % alignment: " << (uintptr_t(address_space) % alignment) << std::endl;

    ///initial free memory block
    ///Note, don't start from 0, because 0 is an undefined goffset. Skip a page.
    auto initialfreemem = mem_range_t(alignment, size);
    this->freemems[initialfreemem] = default_freemem_lookup_info();
    this->update_freemem_lookup_info(initialfreemem);

    assert( (uintptr_t(address_space) % alignment) == 0 );
    svo_block_t* root_block = this->root_block = this->allocate_block(block_size/*size*/);
    root_block->trunk = true;



    DEBUG {
        if (auto error = svo_block_sanity_check(root_block))
        {
            std::cerr << error << std::endl;
            assert(false && "sanity fail");
        }
    }
}


svo_block_t* svo_tree_t::allocate_block(std::size_t size)
{
    std::unique_ptr<svo_block_t> block(new svo_block_t());


    auto mem_range = this->mem_malloc(size);
    assert(mem_range_size(mem_range) == size);

    block->tree = this;






    block->side = 0;
    block->root_level = 0;
    block->slice = 0;
    block->parent_block = 0;
    block->block_start = mem_range.first;
    block->block_end = mem_range.second;
    block->cd_start = block->block_start;
    block->cd_end = block->block_start;
    ///block->cdspace_end = (1<<15)*sizeof(uint32_t) - sizeof(svo_info_section_t);
    block->cdspace_end = block->block_start + size / 8;

    block->info_goffset = block->cdspace_end;


    
    child_descriptor_t root_cd0; svo_init_cd(&root_cd0);

    block->root_shadow_cd_goffset = svo_append_cd(this->address_space, block.get(), &root_cd0);
    block->height = 0;
    block->side = 0;
    block->root_ccurve = 0;
    block->root_valid_bit = 0;
    block->root_leaf_bit = 0;

    auto* root_shadow_cd = svo_get_cd(this->address_space, block->root_shadow_cd_goffset);
    goffset_t dummy_cd_goffset = svo_append_dummy_cd(this->address_space, block.get());
    offset_t offset = dummy_cd_goffset - block->root_shadow_cd_goffset;
    offset4_t offset4 = offset / 4;
    svo_set_child_ptr(root_shadow_cd, offset4);
    assert(svo_get_child_ptr_goffset(this->address_space, block->root_shadow_cd_goffset, root_shadow_cd) == dummy_cd_goffset);
    
    
    
    ///push_back() might throw, so don't release the block from the unique_ptr yet.
    auto& block_lookup_info = this->blocks[block.get()];
    
    block_lookup_info.freesize2block_iterator = freesize2block.end();
    block_lookup_info.size2block_iterator = size2block.end();
    update_block_lookup_info(block.get());


    DEBUG {
        if (auto error = svo_block_sanity_check(block.get()))
        {
            std::cerr << error << std::endl;
            assert(false && "sanity fail");
        }
    }

    ///now that push_back() succeeded, we can release the block ptr.
    return block.release();
}

void svo_tree_t::deallocate_block(svo_block_t* block)
{
    assert(block);
    assert(block->child_blocks);
    assert(block->child_blocks->size() == 0);



    auto freemem_range = mem_range_t(block->block_start, block->block_end);
    auto& free_memory_range_info = freemems[freemem_range] = default_freemem_lookup_info();
    update_freemem_lookup_info(freemem_range);

    ///todo: join adjacent freemem ranges.
}

void svo_tree_t::update_freemem_lookup_info(mem_range_t freemem_range)
{
    assert(freemems.count(freemem_range));
    auto& free_memory_range_info = freemems[freemem_range];

    if (free_memory_range_info.size2freemem_iterator != size2freemem.end())
        size2freemem.erase(free_memory_range_info.size2freemem_iterator);
    

    std::tie(free_memory_range_info.size2freemem_iterator, std::ignore) = size2freemem.insert( std::make_pair(mem_range_size(freemem_range), freemem_range ) );

}


svo_tree_t::free_memory_range_info_t svo_tree_t::default_freemem_lookup_info()
{
    free_memory_range_info_t result;
    result.size2freemem_iterator = size2freemem.end();
    return result;
}

void svo_tree_t::update_block_lookup_info(svo_block_t* block)
{
    assert(block);
    assert(blocks.count(block));



    auto& block_lookup_info = blocks[block];

    if (block_lookup_info.size2block_iterator != size2block.end())
        size2block.erase(block_lookup_info.size2block_iterator);
    if (block_lookup_info.freesize2block_iterator != freesize2block.end())
        freesize2block.erase(block_lookup_info.freesize2block_iterator);

    std::tie(block_lookup_info.size2block_iterator, std::ignore) = size2block.insert( std::make_pair(block->size(), block) );
    std::tie(block_lookup_info.freesize2block_iterator, std::ignore) = freesize2block.insert( std::make_pair(block->freesize(), block) );

}


#if 0
void load_next_slices(std::vector<svo_block_t*>& resulting_leaf_blocks, svo_tree_t* tree, svo_block_t* block)
{
    DEBUG {
        if (auto error = svo_block_sanity_check(block))
        {
            std::cerr << error << std::endl;
            assert(false && "sanity fail");
        }
    }


    svo_slice_t* slice = block->slice;
    if (!slice)
        return;

    DEBUG {
        if (auto error = svo_slice_sanity_check_error(slice))
        {
            std::cerr << error << std::endl;
            assert(false && "sanity fail");
        }
        if (auto error = svo_slice_sanity_check_children_error(slice))
        {
            std::cerr << error << std::endl;
            assert(false && "sanity fail");
        }
        if (auto error = svo_slice_sanity_check_parent_error(slice))
        {
            std::cerr << error << std::endl;
            assert(false && "sanity fail");
        }
    }

    if (svo_is_slice_loading(slice))
    {
        resulting_leaf_blocks.push_back(block);
        return;
    }

    assert(!(block->trunk));
    assert(slice->children);
    auto& children = *slice->children;

    auto height0 = block->height;

    if (children.size() == 0 || children.size() == 1)
    {
        auto success = svo_block_insert_slice_data(tree, block, slice);
        PPK_ASSERT(success == svo_error_t::OK, fmt::format("sucess: {}", success).c_str());

        block->height += 1;
        assert(block->height == height0 + 1);

        if (children.size() == 0)
            block->slice = 0;
        else
            block->slice = children[0];

        DEBUG {
            if (auto error = svo_block_sanity_check(block))
            {
                std::cerr << error << std::endl;
                assert(false && "sanity fail");
            }
        }
    } else {

        std::vector< svo_block_t* > new_blocks;
        for (svo_slice_t* child_slice : children)
        {
            ///NOTE: fix level later
            auto* new_block = tree->allocate_block(block->size());
            assert(new_block);
            new_blocks.push_back(new_block);


            new_block->slice = child_slice;

        }


        auto success = svo_block_insert_slice_data(tree, block, slice);
    }


#if 0
    auto height0 = block->height;
    auto success = svo_block_append_slice_data(tree->address_space, block, slice);
    assert(success == svo_error_t::OK);
    assert(block->height == height0 + 1);

    auto& children = *slice->children;

    if (children.size() == 0)
    {
        block->slice = 0;
        return;
    }
    if (children.size() == 1)
    {
        assert(children[0]);

        std::cout << "block->slice->side: " << block->slice->side << ", children[0]->side: " << children[0]->side << std::endl;
        block->slice = children[0];
        return;
    }

    std::vector< svo_block_t* > new_blocks;
    for (svo_slice_t* child_slice : children)
    {
        ///NOTE: fix level later
        auto* new_block = tree->allocate_block(block->size()/*size*/, 0/*parent*/, 0/*slice*/, 0/*root_level*/, 1/*side*/);
        new_blocks.push_back(new_block);


        new_block->slice = child_slice;

    }



    std::vector< std::tuple<goffset_t> > uncolored_cds;




    auto block_classify_split_append = [&tree, &block, &slice, &new_blocks, &uncolored_cds](
                                                        goffset_t pcd_goffset
                                                      , goffset_t cd_goffset
                                                      , ccurve_t cd_ccurve /// the corner that this cd is in, relative to parent
                                                      , const std::tuple<std::size_t, std::size_t, vcurve_t, goffset_t>& classification)
    {
        ///this is a leaf voxel, not a cd.
        if (!cd_goffset)
            ///the return value of a leaf voxel is discarded, so just send back the same.
            return classification;

        ///get the slice that this CD belongs to.
        std::size_t child_slice_index = std::get<0>(classification);
        ///the relative level to the root of this block.
        std::size_t cd_inblock_level = std::get<1>(classification);
        assert( cd_inblock_level < block->root_level + block->height );
        ///the vcurve of the parent within its level.
        vcurve_t parent_cd_inlevel_vcurve = std::get<2>(classification);
        ///the vcurve of the parent within its level.
        goffset_t new_pcd_goffset = std::get<3>(classification);

        ///we can compute the the vcurve of the child using morton-order logic.
        vcurve_t cd_inlevel_vcurve = parent_cd_inlevel_vcurve*8 + cd_ccurve;

        ///side of this level, based on the level.
        vside_t inblock_level_side = 1 << cd_inblock_level;
        assert( inblock_level_side < block->side );

        ///Find the lowest corner in the slice where this CD would take up if projected to the slice level.
        ///First we find the projection multipler between the cd level and the slice level.
        vside_t cd_bound_side = vcurvesize(slice->side) / vcurvesize(inblock_level_side);
        vside_t cd_bound_vcurve_begin = cd_inlevel_vcurve*cd_bound_side + 0;

        goffset_t new_cd_goffset = 0;

        ///if this CD is not classified (in a classified subtree)
        if (child_slice_index == (std::size_t)-1)
        {
            ///try to classify it.

            assert(slice);
            assert(slice->children);
            auto& children = *slice->children;

            for (std::size_t candidate_child_slice_index = 0;
                 candidate_child_slice_index < children.size();
                 ++candidate_child_slice_index)
            {
                svo_slice_t* child = children[candidate_child_slice_index];

                auto child_bound_vcurve_begin = child->parent_vcurve_begin;
                assert(child->side % 2 == 0);
                auto child_bound_side = child->side / 2;

                if (cd_bound_vcurve_begin >= child_bound_vcurve_begin && cd_bound_side <= child_bound_side)
                {
                    ///cd on this level is totally contained within this child's bounds in space.

                    child_slice_index = candidate_child_slice_index;
                    break;
                }
            }

            ///if cd was JUST classified; then this is the root node.
            if (child_slice_index != (std::size_t)-1)
            {
                const child_descriptor_t* cd0 = svo_cget_cd(tree->address_space, cd_goffset);

                assert(slice);
                assert(slice->children);
                assert(child_slice_index < slice->children->size());
                assert(child_slice_index < new_blocks.size());

                //svo_slice_t* child_slice = (*slice->children)[child_slice_index];
                svo_block_t* new_block = new_blocks[child_slice_index];
                assert(new_block);
                
                new_cd_goffset = svo_append_cd(tree->address_space, new_block, cd0);
                
                child_descriptor_t* cd1 = svo_get_cd(tree->address_space, new_cd_goffset);

                assert(new_cd_goffset);

                new_block->root_cd_goffset = new_cd_goffset;

                svo_set_child_ptr(cd1, 0);
                svo_set_far(cd1, false);

            } else {
                ///if this CD is (still) not classified (in a classified subtree)
                ///this is one of the "near the root node" nodes that won't fit into the new block(s). 
                uncolored_cds.push_back( std::make_tuple(pcd_goffset, cd_goffset) );
            }
        } else {
            const child_descriptor_t* cd0 = svo_cget_cd(tree->address_space, cd_goffset);

            ///if this CD is classified.
            if (child_slice_index != (std::size_t)-1)
            {
                assert(slice);
                assert(slice->children);
                assert(child_slice_index < slice->children->size());
                assert(child_slice_index < new_blocks.size());

                //svo_slice_t* child_slice = (*slice->children)[child_slice_index];
                svo_block_t* new_block = new_blocks[child_slice_index];
                assert(new_block);
                
                new_cd_goffset = svo_append_cd(tree->address_space, new_block, cd0);

                DEBUG {
                    if (!new_cd_goffset)
                    {
                        std::cerr << "new_block->size(): " << new_block->size()
                                  << ", new_block->block_start: " << new_block->block_start
                                  << ", new_block->block_end: " << new_block->block_end
                                  << ", new_block->cd_start: " << new_block->cd_start
                                  << ", new_block->cd_end: " << new_block->cd_end
                                  << ", new_block->cdspace_end: " << new_block->cdspace_end
                                  << ", new_block->height: " << new_block->height
                                  << ", new_block->side: " << new_block->side
                                  << std::endl;
                    }
                }
                assert(new_cd_goffset);



                child_descriptor_t* cd1 = svo_get_cd(tree->address_space, new_cd_goffset);

                assert(svo_get_far(cd0) == false);


                assert(new_pcd_goffset);
                assert(new_block->is_in_block(new_pcd_goffset, sizeof(child_descriptor_t)));
                assert(new_pcd_goffset < new_cd_goffset);

                child_descriptor_t* pcd = svo_get_cd(tree->address_space, new_pcd_goffset);

                if (svo_get_child_ptr_offset4(pcd) == 0)
                {

                    offset_t new_cd_offset = new_cd_goffset - new_pcd_goffset;
                    assert( new_cd_offset % sizeof(child_descriptor_t) == 0 );
                    assert( new_cd_offset % 4 == 0 );
                    offset4_t new_cd_offset4 = new_cd_offset / 4;

                    assert( (new_cd_offset4 & SVO_CHILD_PTR_MASK) == new_cd_offset4);
                    svo_set_child_ptr(pcd, new_cd_offset4);
                    svo_set_far(cd1, false);
                }
            }
        }

        return std::make_tuple(child_slice_index, cd_inblock_level + 1, cd_inlevel_vcurve, new_cd_goffset);
    };

    preorder_traverse_block_cds(  tree->address_space
                                , block
                                , std::make_tuple(  (std::size_t)-1 /*classification to child*/
                                                  , 0/*level in block*/
                                                  , 0/*parent_cd_inlevel_vcurve*/
                                                  , 0/*new parent cd goffset*/)
                                , block_classify_split_append);




    if (block->trunk)
    {
        ///erase all the cd's in the original block
        block->cd_end = block->root_cd_goffset + sizeof(child_descriptor_t);


    }

#endif



    
    DEBUG {
        if (auto error = svo_block_sanity_check(block))
        {
            std::cerr << error << std::endl;
            assert(false && "sanity fail");
        }
    }
}
#endif

static const std::size_t svo_max_cd_space = (1<<15)*sizeof(uint32_t);



goffset_t svo_append_ph(byte_t* address_space, svo_block_t* block)
{
    if (!(block->cd_end + sizeof(child_descriptor_t) <  block->cdspace_end))
        return 0;
    goffset_t ph_goffset = block->cd_end;
    if (ph_goffset == svo_get_ph_goffset(ph_goffset))
    {
        
        svo_page_header_t* page_header = (svo_page_header_t*)(address_space + ph_goffset);

        page_header->info_offset = block->info_goffset - ph_goffset;

        assert( sizeof(child_descriptor_t) >= sizeof(svo_page_header_t) );

        block->cd_end += sizeof(child_descriptor_t);

        return ph_goffset;
    }

    return 0;
}

goffset_t svo_append_cd(byte_t* address_space, svo_block_t* block, const child_descriptor_t* cd)
{
    assert(block);
    svo_append_ph(address_space, block);

    if (!(block->cd_end + sizeof(child_descriptor_t) <  block->cdspace_end))
        return invalid_goffset;
    
    goffset_t cd_goffset = block->cd_end;

    ///child descriptor address should be aligned to the size of the child descriptor
    assert(cd_goffset % sizeof(child_descriptor_t) == 0);

    ///cd_end should never be left on a page header address.
    assert(cd_goffset != svo_get_ph_goffset(cd_goffset));

    child_descriptor_t* cd_dest = (child_descriptor_t*)(address_space + cd_goffset);

    svo_copy_cd(cd_dest, cd);


    block->cd_end += sizeof(child_descriptor_t);


    return cd_goffset;
}

child_descriptor_t generate_dummy_cd()
{
    child_descriptor_t dummy_cd; svo_init_cd(&dummy_cd);
    return dummy_cd;
}

goffset_t svo_append_dummy_cd(byte_t* address_space, svo_block_t* block)
{
    static const child_descriptor_t dummy_cd = generate_dummy_cd();

    return svo_append_cd(address_space, block, &dummy_cd);
}


svo_error_t svo_block_initialize_slice_data(std::vector<svo_block_t*>& new_leaf_blocks, svo_tree_t* tree, svo_block_t* block, svo_slice_t* slice)
{
    DEBUG {
        if (auto error = svo_block_sanity_check(block))
        {
            std::cerr << error << std::endl;
            assert(false && "sanity fail");
        }
        if (auto error = svo_slice_sanity(slice))
        {
            std::cerr << error << std::endl;
            assert(false && "sanity fail");
        }
    }
    
    ///the dummy root should be there
    assert(block->side == 0);
    assert(block->root_valid_bit == 0);
    assert(block->root_leaf_bit == 0);
    assert(slice->side == 1);

    assert(svo_is_slice_loaded(slice));

    assert(block->trunk == true);
    assert(block->child_blocks->size() == 0);

    assert(slice->pos_data->size() == 1);

    assert(block->root_shadow_cd_goffset != 0);
    assert(block->root_shadow_cd_goffset != invalid_goffset);
    assert(block->is_valid_cd_goffset(block->root_shadow_cd_goffset));

    
    block->reset_cd_data();
    block->side = 0; ///trunk
    block->height = 0; ///trunk
    block->root_level = 0;
    block->root_valid_bit = true;
    block->root_leaf_bit = true;
    block->leaf_count = 1;
    block->cd_count = 1;


    ///add a far ptr after the root
    {
        goffset_t root_far_ptr_goffset = svo_append_dummy_cd(tree->address_space, block);
        if (root_far_ptr_goffset == invalid_goffset)
            return svo_error_t::BLOCK_IS_FULL;

        offset_t far_ptr_offset = root_far_ptr_goffset - block->root_shadow_cd_goffset;
        assert( far_ptr_offset % sizeof(goffset_t) == 0);

        ///the far ptr should be located pretty close to the CD
        assert( far_ptr_offset < 16 );

        offset4_t far_ptr_offset4 = far_ptr_offset / 4;


        child_descriptor_t* root_shadow_cd = svo_get_cd(tree->address_space, block->root_shadow_cd_goffset);
        svo_set_valid_mask(root_shadow_cd, 0);
        svo_set_leaf_mask(root_shadow_cd, 0);

        svo_set_far(root_shadow_cd, true);
        svo_set_child_ptr(root_shadow_cd, far_ptr_offset4);
        
        svo_set_goffset_via_fp(tree->address_space, block->root_shadow_cd_goffset, root_shadow_cd, invalid_goffset);
    }

    auto& children = *slice->children;
    if (children.size() == 0 || children.size() == 1) {
        block->slice = 0;
        
        svo_block_t* new_block = tree->allocate_block(block->size());
        assert(new_block->root_shadow_cd_goffset != invalid_goffset);
        
        //child_descriptor_t* child_root_shadow_cd = svo_get_cd(tree->address_space, new_block->root_shadow_cd_goffset);

        block->child_blocks->push_back(new_block);

        new_block->parent_block = block;
        new_block->side = 1;
        new_block->height = 1;
        new_block->root_level = 0;
        new_block->root_valid_bit = true;
        new_block->root_leaf_bit = true;
        new_block->parent_root_cd_goffset = block->root_shadow_cd_goffset;
        new_block->cd_count = 1;
        new_block->leaf_count = 1;
        assert(new_block->parent_block->is_valid_cd_goffset(new_block->parent_root_cd_goffset));
        assert(new_block->is_valid_cd_goffset(new_block->root_shadow_cd_goffset));
        
        ///make has_root_children_goffset() valid
        {
            auto* root_shadow_cd = svo_get_cd(tree->address_space, new_block->root_shadow_cd_goffset);
            goffset_t dummy_cd_goffset = svo_append_dummy_cd(tree->address_space, new_block);
            assert(dummy_cd_goffset != 0);
            assert(dummy_cd_goffset != invalid_goffset);
            
            offset_t offset = dummy_cd_goffset - new_block->root_shadow_cd_goffset;
            assert(offset % 4 == 0);
            offset4_t offset4 = offset / 4;
            assert(offset4 > 0);
            assert(offset4 < 16);
            
            svo_set_child_ptr(root_shadow_cd, offset4);
            assert(new_block->has_root_children_goffset());
            
            ///set the parent to point to the correct place now
            assert(block->is_valid_cd_goffset(new_block->parent_root_cd_goffset));
            assert(block->root_shadow_cd_goffset == new_block->parent_root_cd_goffset);
            assert(new_block->is_valid_cd_goffset(dummy_cd_goffset));
            auto* root_cd = svo_get_cd(tree->address_space, new_block->parent_root_cd_goffset);
            
            assert(svo_get_far(root_cd));
            svo_set_goffset_via_fp(tree->address_space, new_block->parent_root_cd_goffset, root_cd, dummy_cd_goffset);
            assert(svo_get_child_ptr_goffset(tree->address_space, new_block->root_shadow_cd_goffset, root_shadow_cd) == dummy_cd_goffset);
            assert(svo_get_child_ptr_goffset(tree->address_space, new_block->parent_root_cd_goffset, root_cd) == dummy_cd_goffset);
            
        }
        
        

        if (children.size() == 1)
            new_block->slice = children[0];


        new_leaf_blocks.push_back(new_block);

    } else {
        assert(block->child_blocks);

        assert(false && "TODO");
        /*
        auto& child_blocks = *block->child_blocks;
        block->slice = 0;

        for (svo_slice_t* child_slice : children)
        {
            svo_block_t* new_block = tree->allocate_block(block->size());

            assert(new_block);

            child_blocks.push_back(new_block);
            new_block->slice = child_slice;
            new_block->side = 1;
            new_block->height = 1;
            new_block->parent_block = block;

            goffset_t new_block_shadow_root_goffset = svo_append_cd(tree->address_space, new_block, &root_cd);
            assert(new_block_shadow_root_goffset);
            new_block->root_cd_goffset = new_block_shadow_root_goffset;
        }
        */

    }

    for (svo_block_t* child_block : *block->child_blocks)
    {
        new_leaf_blocks.push_back(child_block);
    }

    DEBUG {
        if (auto error = svo_block_sanity_check(block))
        {
            std::cerr << error << std::endl;
            assert(false && "sanity fail");
        }

        for (svo_block_t* child_block : *block->child_blocks)
        {
            if (auto error = svo_block_sanity_check(child_block))
            {
                std::cerr << error << std::endl;
                assert(false && "sanity fail");
            }

            assert(block->has_child_block(child_block));
        }
    }


    return svo_error_t::OK;
}


svo_error_t svo_load_next_slice(std::vector<svo_block_t*>& new_leaf_blocks, svo_block_t* block)
{
    assert(block);

    slice_inserter_t slice_inserter(block->tree, block);

    auto error = slice_inserter.execute();
    if (error != svo_error_t::OK)
        return error;
    

    new_leaf_blocks = slice_inserter.dst_blocks;


    return svo_error_t::OK;

#if 0
    DEBUG {
        if (auto error = svo_block_sanity_check(block))
        {
            std::cerr << error << std::endl;
            assert(false && "sanity fail");
        }
    }

    assert(block->slice);
    assert(block->tree == tree);
    svo_slice_t* slice = block->slice;

    DEBUG {
        if (auto error = svo_slice_sanity_check_error(slice))
        {
            std::cerr << error << std::endl;
            assert(false && "sanity fail");
        }
    }
    
    assert(slice->data);
    PPK_ASSERT (block->side * 2 == slice->side || (block->side == 0 && slice->side == 1)
                , "block->side * 2: %u, slice->side: %u", block->side*2, slice->side);

    const auto& in_data = *slice->data;
    const auto& children = *slice->children;

    std::size_t in_data_index = 0;

    ///[ (level, vcurve, cd, child offset) ]
    /// level of the cd
    /// vcurve of the cd within the level
    /// the cd, mainly the masks
    /// child offset to the children; 0 is initial invalid offset flag.
    typedef std::vector< std::tuple<std::size_t, vcurve_t, child_descriptor_t, offset_t> > out_data_t;


    ///an @c out_data_t for the unclassified voxels; the ones that can't fit into any children slices.
    out_data_t uc_out_data;

    ///an @c out_data_t for each of the slice's children. we will split the voxels among the the different child-volumes.
    std::vector< out_data_t > out_datas;

    ///for each @c out_data_t in @c out_datas, we keep an offset into @c uc_out_data to the cd that is the parent of the
    /// first node in @c out_data.
    std::vector< offset_t > out_uc_root_offsets;

    if (children.size() == 0 || children.size() == 1) {
        out_datas.push_back(out_data_t());
        ///everything is classified in this case to 0th @c out_data in @c out_datas, so no need for out_uc_root_offsets.
    } else {
        out_datas.resize(children.size());
        out_uc_root_offsets.resize(children.size(), std::size_t(-1));
    }

    auto block_copyinsert_slice_data = [&tree, &block, &slice, &in_data, &in_data_index, &uc_out_data, &out_datas, &out_uc_root_offsets](
                                            goffset_t pcd_goffset
                                          , goffset_t cd_goffset
                                          , ccurve_t voxel_ccurve
                                          , std::tuple<vcurve_t, std::size_t, std::size_t, std::size_t> metadata)
    {
        ///vcurve of the parent cd with its block-level
        vcurve_t pcd_level_vcurve = std::get<0>(metadata);
        std::size_t level = std::get<1>(metadata);
        std::size_t parent_classification = std::get<2>(metadata);
        std::size_t parent_out_data_index = std::get<3>(metadata);



        ///vcurve of this voxel with its block-level
        vcurve_t level_vcurve = pcd_level_vcurve*8 + voxel_ccurve;
        std::size_t classification = parent_classification;
        assert ( level < block->height );

        if (classification == std::size_t(-1))
        {
            assert(slice->children->size() > 1);

            classification = svo_classify_voxel(level, level_vcurve, slice);
        }

        bool is_leaf = cd_goffset == 0;
        bool is_bottom_leaf = level == block->height - 1;

        std::cout << "pcd_level_vcurve: " << pcd_level_vcurve
            << ", level: " << level
            << ", level_vcurve: " << level_vcurve
            << ", classification: " << classification
            << ", block->height: " << block->height
            << std::endl
            << "  cd_goffset: " << cd_goffset
            << ", pcd_goffset: " << pcd_goffset
            << ", voxel_ccurve: " << voxel_ccurve
            << std::endl
            << "  is_leaf: " << (is_leaf ? "true" : "false")
            << ", is_bottom_leaf: " << (is_bottom_leaf ? "true" : "false")
            << std::endl;
        if (cd_goffset)
        {
            const auto* cd = svo_cget_cd(tree->address_space, cd_goffset);
            assert(cd);
            std::cout << *cd << std::endl;
            std::cout << " valid_mask: " << std::bitset<8>(svo_get_valid_mask(cd))
                    << ", leaf_mask: " << std::bitset<8>(svo_get_leaf_mask(cd))
                    << std::endl;
        }
        std::cout
            << std::endl
            << std::endl;

        assert (classification == std::size_t(-1) || classification < out_datas.size());


        std::size_t out_data_index = std::size_t(-1);

        
        ///bottom leaf implies leaf
        assert(!is_bottom_leaf || is_leaf);
        
        ///if we potentially need to add the new stuff
        if (is_bottom_leaf)
        {
            ///this should be a leaf voxel, and thus have no cd in the old block
            assert(is_leaf);

            std::cout << "in_data_index: " << in_data_index
                    << ", in_data.size(): " << in_data.size()
                    << ", level_vcurve: " << level_vcurve;

            if (in_data_index < in_data.size())
                std::cout << ", std::get<0>(in_data[in_data_index])/8: " << std::get<0>(in_data[in_data_index])/8;

            std::cout << std::endl;

            ///if we still have data to use, and the data is a child of the current voxel
            if (in_data_index < in_data.size() && (std::get<0>(in_data[in_data_index])/8) == level_vcurve)
            {
                child_descriptor_t new_cd; svo_init_cd(&new_cd);
                byte_t valid_mask = 0;
                byte_t leaf_mask = 0;

                while (in_data_index < in_data.size() && (std::get<0>(in_data[in_data_index])/8) == level_vcurve)
                {

                    std::cout << "in_data_index: " << in_data_index
                            << ", in_data.size(): " << in_data.size()
                            << ", level_vcurve: " << level_vcurve;

                    if (in_data_index < in_data.size())
                        std::cout << ", std::get<0>(in_data[in_data_index])/8: " << std::get<0>(in_data[in_data_index])/8;

                    std::cout << std::endl;

                    vcurve_t next_new_voxel_vcurve; svo_slice_t::svo_voxel_t voxel;
                    std::tie(next_new_voxel_vcurve, voxel) = in_data[in_data_index++];

                    ccurve_t new_voxel_ccurve = next_new_voxel_vcurve % 8;

                    valid_mask |= (1 << new_voxel_ccurve);
                    leaf_mask |= (1 << new_voxel_ccurve);
                }

                svo_set_valid_mask(&new_cd, valid_mask);
                svo_set_leaf_mask(&new_cd, valid_mask);
                
                ///classification is not invalid flag => classification is sane
                assert(classification == std::size_t(-1) || classification < out_datas.size());
                
                auto& out_data = (classification != std::size_t(-1)) ? out_datas[classification] : uc_out_data;

                out_data_index = out_data.size();
                out_data.push_back( std::make_tuple(level, level_vcurve, new_cd, 0 ) );

                
            }
        }

        if (cd_goffset)
        {
            assert(!is_leaf);
            assert(!is_bottom_leaf);
            
            assert( level < block->height - 1);
            assert(out_data_index == std::size_t(-1));
            assert(block->is_in_block(cd_goffset, sizeof(child_descriptor_t)));
            const auto* cd = svo_cget_cd(tree->address_space, cd_goffset);

            child_descriptor_t new_cd; svo_init_cd(&new_cd);
            
            svo_set_valid_mask(&new_cd, svo_get_valid_mask(cd));
            svo_set_leaf_mask(&new_cd, svo_get_leaf_mask(cd));

                
            ///classification is not invalid flag => classification is sane
            assert(classification == std::size_t(-1) || classification < out_datas.size());
            auto& out_data = (classification != std::size_t(-1)) ? out_datas[classification] : uc_out_data;


            out_data_index = out_data.size();
            out_data.push_back( std::make_tuple(level, level_vcurve, new_cd, 0 /* child offset */) );
        }

        /*
        ///if there was an output, and this is the first classified CD of this class
        if (   out_data_index != std::size_t(-1)
            && parent_classification == std::size_t(-1)
            && classification != std::size_t(-1))
        {
            ///if we do not have a uc-root CD
            if (out_uc_root_offsets[classification] == std::size_t(-1))
            {
                ///duplicate the outputed root-classified into the unclassifieds
            
                ///this should be the second element in the classified array.
                assert(out_data_index >= 1 && out_data_index < (8+1));
                
                std::size_t uc_out_data_index = uc_out_data.size();
                uc_out_data.push_back( out_data.back() );
                out_uc_root_offsets[classification] = uc_out_data_index;

            }
            assert(out_uc_root_offsets[classification] != std::size_t(-1));
        }*/

        /*
        ///if this is a child of the newly classifiied root
        if (classification != std::size_t(-1) && parent_data_index == 0)
        {
            assert( parent_classification == classification );
            
            
        }*/

        ///now update the parent
        ///if there is a parent
        if (parent_out_data_index != std::size_t(-1))
        { 
            ///parent is classified => assert child is classified the same
            assert(parent_classification == size_t(-1) || parent_classification == classification);
            ///parent is classified => classification is valid classification
            assert(parent_classification == size_t(-1) || parent_classification < out_datas.size());


            auto& parent_out_data = (parent_classification != size_t(-1)) ? out_datas[parent_classification]  : uc_out_data;
            ///parent output index must be inside the data
            assert(parent_out_data_index < parent_out_data.size());

            std::size_t& new_pcd_level = std::get<0>(parent_out_data[parent_out_data_index]);
            child_descriptor_t& new_pcd = std::get<2>(parent_out_data[parent_out_data_index]);
            offset_t& new_pcd_child_offset = std::get<3>(parent_out_data[parent_out_data_index]);

            ///if this output a CD, and is the first child of the parent (the parent has an invalid child offset of 0)
            if (out_data_index != std::size_t(-1) && new_pcd_child_offset == 0) {
                ///if the parent is classified (and therefore this CD is classified)
                if (parent_classification != std::size_t(-1)) {
                    ///then set the child offset to the distance between the parent and the first child.
                    new_pcd_child_offset = (out_data_index - parent_out_data_index);
                }
                ///if both parent CD and current voxel are not classified
                else if (classification == std::size_t(-1)){
                    ///then set the child offset to the distance between the parent and the first child.
                    new_pcd_child_offset = (out_data_index - parent_out_data_index);
                } 
                
                ///if the parent is not classified, and the current CD is.
                else {
                    ///make sure this condition branch is true
                    assert(parent_classification == std::size_t(-1) && classification != std::size_t(-1));
                    assert(classification < out_uc_root_offsets.size());
                    
                    if (out_uc_root_offsets[classification] == std::size_t(-1))
                    {
                        out_uc_root_offsets[classification] = parent_out_data_index;
                    }
                    
                }
            }

            bool new_parent_valid_bit = svo_get_valid_bit(&new_pcd, voxel_ccurve);
            bool new_parent_leaf_bit = svo_get_leaf_bit(&new_pcd, voxel_ccurve);

            assert(new_parent_valid_bit);
            assert(new_parent_leaf_bit);
            assert(new_pcd_level == level - 1);

            ///if there was an output CD
            if (out_data_index != std::size_t(-1))
                ///make this voxel a non-leaf in the parent
                svo_set_leaf_bit(&new_pcd, voxel_ccurve, false);

        }

        return std::make_tuple(level_vcurve, level+1, classification, out_data_index);
    };


    auto initial_data = std::make_tuple(  0/*level_vcurve*/
                                        , 0/*level*/
                                        , std::size_t(-1)/*parent classification*/
                                        , std::size_t(-1)/*parent out data index*/);
    
    if (children.size() == 0 || children.size() == 1)
        initial_data = std::make_tuple(  0/*level_vcurve*/
                                        , 0/*level*/
                                        , 0/*parent classification*/
                                        , std::size_t(-1)/*parent out data index*/);

    z_preorder_traverse_block_cds(  tree->address_space
                                    , block
                                    , initial_data, block_copyinsert_slice_data);
    


    std::cout << "size0: " << (block->cd_end - block->cd_start)/8 << std::endl;

    for (auto& out_data : out_datas)
    {
        std::cout << "  out_data.size(): " << out_data.size() << std::endl;
    }
    std::cout << "  uc_out_data.size(): " << uc_out_data.size() << std::endl;

    assert(in_data_index == in_data.size());










    if (children.size() == 0 || children.size() == 1)
    {
        assert(uc_out_data.size() == 0);
        assert(out_datas.size() == 1);

        auto& out_data = out_datas[0];

        block->cd_end = block->cd_start;
        svo_block_t* dst_block = block;


        ///for each CD, we will store an index to the parent CD, or invalid flag if there is none.
        std::vector< std::size_t > cd_parent_indices(out_data.size(), std::size_t(-1));

        ///for each CD we will store the final reverse offset off the end of the block, in bytes.
        ///does not take page-headers into account.
        std::vector< std::size_t > cd_reverse_boffsets(out_data.size(), std::size_t(-1));
        std::vector< bool > cd_requires_far_child_ptr(out_data.size(), false);


        ///find the parent index for each CD
        for (std::size_t out_data_index = 0; out_data_index < out_data.size(); ++out_data_index)
        {

            std::size_t parent_data_index = cd_parent_indices[out_data_index];
            auto& voxel = out_data[out_data_index];

            child_descriptor_t& cd = std::get<2>(voxel);
            offset_t& child_0_offset_in_cds = std::get<3>(voxel);

            ///it is either a non-leaf voxel or a dummy root
            assert(child_0_offset_in_cds || out_data_index == 0);
            assert(out_data_index + child_0_offset_in_cds < out_data.size());


            ///if this node has child CDs (non-leaf children).
            if (child_0_offset_in_cds)
            {
                assert(svo_get_cd_nonleaf_count(&cd) > 0);

                ///fill in cd_parent_indices for all the children of this node.
                for (std::size_t i = 0; i < svo_get_cd_nonleaf_count(&cd); ++i)
                {
                    auto child_i_roffset_in_cds = child_0_offset_in_cds + i;
                    assert(out_data_index + child_i_roffset_in_cds < cd_parent_indices.size());

                    ///make sure no other node claimed to be the parent of this child.
                    assert( cd_parent_indices[out_data_index + child_i_roffset_in_cds] == std::size_t(-1) );


                    cd_parent_indices[out_data_index + child_i_roffset_in_cds] = out_data_index;
                }
            } else {
                assert(svo_get_cd_nonleaf_count(&cd) == 0);
            }
        }

        ///this is a tmp buffer for siblings that must be layed out next to eachother.
        std::vector< std::size_t > sibling_section_indices;
        ///offset off the end off the black
        std::size_t current_reverse_boffset = 0;

        ///now compute the locations and far pointers, by working backwards from the end.
        for (std::size_t out_data_index = out_data.size()-1; out_data_index != std::size_t(-1); --out_data_index)
        {
            sibling_section_indices.push_back(out_data_index);
            assert( sibling_section_indices.size() <= 8 );


            std::size_t parent_data_index = cd_parent_indices[out_data_index];


            bool should_flush_child_section = false;
            if (out_data_index == 0) {
                should_flush_child_section = true;
            } else {
                assert( out_data_index - 1 < out_data.size() );
                auto& next_voxel = out_data[out_data_index - 1];


                assert( out_data_index - 1 < cd_parent_indices.size() );
                std::size_t next_voxel_parent_data_index = cd_parent_indices[out_data_index - 1];

                if (next_voxel_parent_data_index != parent_data_index)
                    should_flush_child_section = true;
                else
                    should_flush_child_section = false;
            }

            if (should_flush_child_section)
            {
                ///calculate the layout of this child section, and the subsequent far pointer section.


                std::size_t cd_section_byte_size = sibling_section_indices.size() * sizeof(child_descriptor_t);

                if (dst_block->trunk)
                    cd_section_byte_size = 8 * sizeof(child_descriptor_t);

                std::size_t section_reverse_boffset0 = current_reverse_boffset + cd_section_byte_size;



                std::size_t far_ptrs = 0;

                ///calculte the number of far pointers in this section
                {

                    ///iterate the children in reverse
                    for (std::size_t sibling_data_index : ireversed(sibling_section_indices) )
                    {
                        assert( sibling_data_index < out_data.size() );

                        auto& sibling_voxel = out_data[ sibling_data_index ];
                        child_descriptor_t& cd = std::get<2>(sibling_voxel);

                        ///relative offset into @c out_data of the children of this voxel.
                        offset_t& child_0_offset_in_cds = std::get<3>(sibling_voxel);

                        if (svo_get_cd_nonleaf_count(&cd) == 0)
                            continue;

                        if (child_0_offset_in_cds == 0)
                        {
                            assert(false && "TODO");
                        }

                        ///calculate the position of child0 in the @c out_data vector
                        std::size_t child0_data_index = sibling_data_index + child_0_offset_in_cds;

                        assert( child0_data_index < cd_reverse_boffsets.size() );
                        std::size_t child_0_reverse_boffset = cd_reverse_boffsets[child0_data_index];

                        ///being further to the right in @c out_data, this child should have a valid final position.
                        assert( child_0_reverse_boffset != std::size_t(-1) );

                        assert( sizeof(child_descriptor_t) % 4 == 0 );
                        offset4_t child_0_offset4 = child_0_offset_in_cds*sizeof(child_descriptor_t) / 4;

                        ///(over)estimate the number of pages this offset crosses
                        std::size_t pages = (child_0_offset4 / SVO_PAGE_SIZE) + 2;

                        std::size_t page_header_wasted_space = pages*sizeof(child_descriptor_t);

                        ///15 bits
                        offset4_t max_child_offset4 = (1 << 16) - 1;


                        if (child_0_offset4 > max_child_offset4 - page_header_wasted_space || page_header_wasted_space > max_child_offset4)
                        {
                            far_ptrs++;
                            cd_requires_far_child_ptr[sibling_data_index] = true;
                        }
                    }
                }

                if (dst_block->trunk)
                    far_ptrs = 8;

                std::size_t far_ptr_section_byte_size = far_ptrs*4;


                cd_section_byte_size += far_ptr_section_byte_size;

                ///round it up to the nearest sizeof(child_descriptor_t) alignment
                //cd_section_byte_size += (cd_section_byte_size % sizeof(child_descriptor_t) == 0) ? 0 : 4;
                cd_section_byte_size = iceil(cd_section_byte_size, sizeof(child_descriptor_t));
                assert(cd_section_byte_size % sizeof(child_descriptor_t) == 0);

                std::size_t cd_section_reverse_boffset = current_reverse_boffset + cd_section_byte_size;

                ///and finally, compute the cd_reverse_boffsets for this section
                std::size_t i = 0;
                for (std::size_t sibling_data_index : ireversed(sibling_section_indices) )
                {
                    std::size_t sibling_reverse_boffset = cd_section_reverse_boffset - i*sizeof(child_descriptor_t);

                    cd_reverse_boffsets[sibling_data_index] = sibling_reverse_boffset;
                    i++;
                }


                sibling_section_indices.clear();
            }

        }

        for (std::size_t out_data_index = 0; out_data_index < cd_reverse_boffsets.size(); ++out_data_index)
        {
            std::cout << " " << cd_reverse_boffsets[out_data_index];
        }
        std::cout << std::endl;

        dst_block->reset_cd_data();
        dst_block->root_valid_bit = false;
        dst_block->root_leaf_bit = false;
        
        ///copy root/shadow root
        if (out_data.size() > 0)
        {
            dst_block->root_valid_bit = true;
            dst_block->root_leaf_bit = true;


            child_descriptor_t* root_shadow_cd = svo_get_cd(tree->address_space, dst_block->root_shadow_cd_goffset);
            auto& voxel = out_data[0];
            child_descriptor_t& cd = std::get<2>(voxel);
            svo_copy_cd(root_shadow_cd, &cd);
        }

        ///root has children
        if (out_data.size() > 1)
            dst_block->root_leaf_bit = false;

        for (std::size_t out_data_index = 1; out_data_index < out_data.size(); ++out_data_index)
        {
            auto& voxel = out_data[out_data_index];
            child_descriptor_t& cd = std::get<2>(voxel);

            svo_append_cd(tree->address_space, dst_block, &cd);
        }

        DEBUG {
            if (auto error = svo_block_sanity_check(dst_block))
            {
                std::cerr << error << std::endl;
                assert(false && "sanity fail");
            }
        }

    }
    assert(false && "TODO");
#endif
}


#if 0
svo_error_t svo_block_append_slice_data(byte_t* address_space, svo_block_t* block, svo_slice_t* slice)
{
    //std::cout << __FILE__ << ":" << __LINE__ << std::endl;

    DEBUG {
        if (auto error = svo_block_sanity_check(block))
        {
            std::cerr << error << std::endl;
            assert(false && "sanity fail");
        }
        if (auto error = svo_slice_sanity_check_error(slice))
        {
            std::cerr << error << std::endl;
            assert(false && "sanity fail");
        }
    }


    assert(block->bottom_fruit);
    assert(slice->data);

    auto& bottom_fruit = *block->bottom_fruit;
    const auto& data = *slice->data;
    //std::cout << __FILE__ << ":" << __LINE__ << std::endl;


    assert (bottom_fruit.size() > 0);
    PPK_ASSERT (block->side * 2 == slice->side, "block->side * 2: %u, slice->side: %u", block->side*2, slice->side);
    //std::cout << "block->side: " << block->side << std::endl;
    //std::cout << "slice->side: " << slice->side << std::endl;


    typedef child_descriptor_t slice_voxel_data_t;
    typedef std::size_t bottom_fruit_index_t;

    std::vector< std::tuple<bottom_fruit_index_t, vcurve_t, corner_t> > new_slice_voxels;

    ///we are going to iterate through all the bottom fruit of the block.
    /// These are the leafs of the last level. And they are the parents of the current slice.
    /// As we iterate through the slice, we also iterate through the bottom fruit simultaniously,
    /// matching them up as we go along.
    /// @c current_pvcurve will point to the vcurve of the current bottom fruit voxel.
    ///     this vcurve is within the bounds of the block, which is half the resolution of the slice
    ///     and thus half the size.
    vcurve_t current_pvcurve;
    std::size_t current_bottom_fruit_index = 0;

    std::tie(current_pvcurve, std::ignore, std::ignore) = bottom_fruit[current_bottom_fruit_index];


    vcurve_t slice_vcurve; svo_slice_t::svo_voxel_t voxel;

    ///find all the locations of the parents of each new voxel
    for (auto& slice_element : data)
    {
        std::tie(slice_vcurve, voxel) = slice_element;

        ///vcurve represents the (x,y,z) coordinates in slice's cube, on the level of the slice.
        ///vcurve = x + (y*side) + (z*side*side); // this is the naive encoding. for this to work
        /// though, the encoding should be in morton encoded (or another encoding with similar
        /// properties), and the slice should be ordered according to this encoding.
        uint32_t x, y, z;
        vcurve2coords(slice_vcurve, slice->side, &x, &y, &z);

        ///parent coords, within the parent level cube; it is half the resolution of the slice level.
        uint32_t px, py, pz;
        ///coords inside previous level: divide each coord by 2, because the parent level
        /// has half the resolution.
        px = x >> 1;
        py = y >> 1;
        pz = z >> 1;

        ///we can compute the vcurve/index of the parent, from its cube-relative coordinates, px,py,pz.
        vcurve_t expected_pvcurve = coords2vcurve(px,py,pz, slice->side / 2);

        ///corner of this voxel within parent
        corner_t corner = get_corner_by_int3( x - px*2, y - py*2, z - pz*2 );
        //child_descriptor_t cd;
        //svo_init_cd(cd);

        ///check if the current bottom fruit voxel pointed to by @c current_bottom_fruit_index
        /// is the parent of this voxel.
        while (current_pvcurve < expected_pvcurve && current_bottom_fruit_index < bottom_fruit.size())
        {
            ///if it is not, iterate along the bottom fruit voxel array until we get to the parent
            /// of this new slice voxel.

            ///Note; if we store the children in morton order, we can be assured that
            /// children of the same parent will be in succession, and thus we will already be at the
            /// correct parent; and that the next parent will be the parent of the next set of successive
            /// children (unless the next parent doesn't have any children, so we can simply iterate over it
            /// and skip until we get the next parent of the next set of successive children).
            current_bottom_fruit_index++;
            std::tie(current_pvcurve, std::ignore, std::ignore) = bottom_fruit[current_bottom_fruit_index];
        }

        if (current_pvcurve == expected_pvcurve)
        {
            ///ok, we found the parent voxel to this slice voxel within the bottom hanging fruit.

            ///now lets edit the future child descriptor of this parent voxel
            child_descriptor_t& pcd = std::get<1>(bottom_fruit[current_bottom_fruit_index]);

            byte_t valid_mask = svo_get_valid_mask(&pcd);
            valid_mask |= (1 << corner2ccurve(corner));
            svo_set_valid_mask(&pcd, valid_mask);

            ///all bottom fruit are leafs, until the next slice is loaded.
            byte_t leaf_mask = svo_get_leaf_mask(&pcd);
            leaf_mask |= (1 << corner2ccurve(corner));
            svo_set_leaf_mask(&pcd, leaf_mask);

            ///now add the voxel to the new voxel list; this will be also be the next bottom hanging fruit list
            /// eventually.
            new_slice_voxels.push_back( std::make_tuple(current_bottom_fruit_index, slice_vcurve, corner) );
            continue;
        } else {
            assert( false && "could not find parent of voxel in slice, at the bottom of the block");
        }


    }

    //std::cout << __FILE__ << ":" << __LINE__ << std::endl;


    std::vector< goffset_t > bottom_fruit_goffsets(bottom_fruit.size(), 0);

    ///put all the bottom fruit directly into the block as child descriptors (if they have one).
    for (std::size_t bottom_fruit_index = 0; bottom_fruit_index < bottom_fruit.size(); ++bottom_fruit_index)
    {
        vcurve_t vcurve = std::get<0>(bottom_fruit[bottom_fruit_index]);
        child_descriptor_t& bottom_fruit_cd = std::get<1>(bottom_fruit[bottom_fruit_index]);
        goffset_t pcd_goffset = std::get<2>(bottom_fruit[bottom_fruit_index]);
        child_descriptor_t* pcd = svo_get_cd(address_space, pcd_goffset);

        //std::cout << "pcd0: " << *pcd << std::endl;
        byte_t valid_mask = svo_get_valid_mask(&bottom_fruit_cd);

        if (valid_mask == 0)
        {
            ///this bottom fruit voxel has no children, and thus does not need a child_descriptor.

            continue;
        }



        goffset_t cd_goffset = svo_append_cd(address_space, block, &bottom_fruit_cd);

        if (cd_goffset == 0)
        {
            return svo_error_t::BLOCK_IS_FULL;
        }

        ///record the child descriptor global offset.
        bottom_fruit_goffsets[bottom_fruit_index] = cd_goffset;

        ///retrieve the coordinates of this bottom fruit, within the block-cube of the bottom level.
        uint32_t x,y,z;
        vcurve2coords(vcurve, block->side, &x, &y, &z);

        ///calculate the coordinates of the parent of this bottom fruit
        uint32_t px = x / 2, py = y / 2, pz = z / 2;

        ///which allows us to calculate the corner of this bottom fruit within its parent.
        corner_t corner = get_corner_by_int3( x - px*2, y - py*2, z - pz*2 );

        ///which allows us to set the leaf_mask in the parent within the block.
        ///first we retreive the leaf_mask.
        byte_t leaf_mask0 = svo_get_leaf_mask(pcd);

        ///then we OR this corner bit in.
        byte_t leaf_mask1 = leaf_mask0 & ~byte_t(1 << corner2ccurve(corner));

        ///now we assert that we changed something.
        assert(leaf_mask0 != leaf_mask1);

        ///and set the leaf_mask back to the child descriptor.
        svo_set_leaf_mask(pcd, leaf_mask1);

        ///then we retreive the child_ptr of the parent's child descriptor
        uint32_t child_ptr_offset4 = svo_get_child_ptr_offset4(pcd);
        /*
        std::cout << "bottom_fruit_cd: " << bottom_fruit_cd << std::endl;
        std::cout << "pcd_goffset: " << pcd_goffset << std::endl;
        std::cout << "corner: " << corner << std::endl;
        std::cout << "corner: " << corner << std::endl;
        std::cout << "leaf_mask0: " << std::bitset<8>(leaf_mask0) << std::endl;
        std::cout << "leaf_mask1: " << std::bitset<8>(leaf_mask1) << std::endl;
        std::cout << "leaf_mask0 != leaf_mask1: " << (leaf_mask0 != leaf_mask1 ? "true" : "false") << std::endl;
        std::cout << "pcd1: " << *pcd << std::endl;

        std::cout << "------------" << std::endl;
        */

        ///if the parent's child_ptr is set, then our child descriptor is not the first child of the parent
        if (child_ptr_offset4)
        {
            ///and we can continue
            continue;
        }

        ///otherwise, we need to set the child_ptr of the parent's child_descriptor to point to the first
        /// child's child descriptor (namely ... this bottom fruit).

        ///calculate the difference.
        goffset_t cd_offset = cd_goffset - pcd_goffset;
        assert ( (cd_offset % sizeof(child_descriptor_t)) == 0 );
        goffset_t cd_offset4 = cd_offset / 4;

        ///15 bits
        if ((cd_offset4 & SVO_CHILD_PTR_MASK) != cd_offset4)
        {
            return svo_error_t::CHILDREN_TOO_FAR;
        }
        svo_set_child_ptr(pcd, cd_offset4);


    }

    //std::cout << __FILE__ << ":" << __LINE__ << std::endl;


    ///prepare the next bottom_fruit array.
    {
        std::vector< std::tuple<vcurve_t, child_descriptor_t, goffset_t> > next_bottom_fruit;
        for (auto& new_slice_voxel : new_slice_voxels)
        {
            std::size_t bottom_fruit_index; vcurve_t vcurve;
            std::tie(bottom_fruit_index, vcurve, std::ignore) = new_slice_voxel;

            goffset_t pcd_goffset = bottom_fruit_goffsets[bottom_fruit_index];
            assert(pcd_goffset != 0);

            child_descriptor_t cd; svo_init_cd(&cd);
            next_bottom_fruit.push_back( std::make_tuple(vcurve, cd, pcd_goffset) );

        }

        //std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        //std::cout << "bottom_fruit.size(): " << bottom_fruit.size() << std::endl;
        //std::cout << "next_bottom_fruit->size(): " << next_bottom_fruit->size() << std::endl;

        bottom_fruit.swap(next_bottom_fruit);
    }

    block->side = slice->side;
    block->height += 1;

    //std::cout << __FILE__ << ":" << __LINE__ << std::endl;
    return svo_error_t::OK;
}

#endif


/**
 * Overview:
 * 1. copy data over the the destination slice.
 * 2. attach the destination slice to the parent. detatch the src slices from the parent.
 * 3. connect the children of the src slices to the destination slice. detatch children of src slices from src slice.
 */
void svo_join_slices(svo_slice_t* dst_slice, const std::vector<svo_slice_t*>& src_slices)
{
    DEBUG {
        
        if(auto error = svo_slice_sanity(dst_slice))
        {
            std::cerr << error << std::endl;
            assert(false && "sanity failed");
        }
        for (const auto* src_slice : src_slices)
        {
            if (!src_slice)
                continue;
            if(auto error = svo_slice_sanity(src_slice))
            {
                std::cerr << error << std::endl;
                assert(false && "sanity failed");
            }
        }
    }

    assert(dst_slice->parent_slice == 0);

    ///assign the properties of the first src slice to the dst slice. Use a loop to find the first valid slice.
    for (const auto* src_slice : src_slices)
    {
        if (!src_slice)
            continue;
        dst_slice->parent_slice = src_slice->parent_slice;
        ///the destination will point to the same place as the first slice.
        ///note however that this might be a little off, as the first octant slice might not be in @c src_slices
        /// and this will be modded down to the lowest corner of the @c dst_slice node.
        dst_slice->parent_vcurve_begin = src_slice->parent_vcurve_begin;
        ///destination will have double the resolution, as it is combining 8 parts into 1.
        dst_slice->side = src_slice->side * 2;

        
        

        ///only want the first slice.
        break;
    }

    assert(dst_slice->parent_slice != 0);

    assert(dst_slice->side < SVO_MAX_VOLUME_SIDE);
    assert(dst_slice->pos_data);
    assert(dst_slice->buffers);

    //vcurvesize_t dst_size = vcurvesize(dst_slice->side);
    ///the virtual size of the full space of the parent.
    /// that is to say, the number of possible voxels in the parent volume.
    vcurvesize_t dst_size_in_parent = vcurvesize(dst_slice->side / 2);

    ///Adjust dst_slice data start in parent to the nearest lower @c dst_size_in_parent (in case the first slice was missing
    /// from src_slices).
    dst_slice->parent_vcurve_begin = dst_slice->parent_vcurve_begin - (dst_slice->parent_vcurve_begin % dst_size_in_parent);




    auto& dst_pos_data = *dst_slice->pos_data;
    auto& dst_buffers = *dst_slice->buffers;

    ///sanity check for the other src slices.
    for (const auto* src_slice : src_slices)
    {
        if (!src_slice)
            continue;

        DEBUG {
            if(auto error = svo_slice_sanity(src_slice))
            {
                std::cerr << error << std::endl;
                assert(false && "sanity failed");
            }
        }

        assert(dst_slice->parent_slice == src_slice->parent_slice);
        assert(dst_slice->parent_vcurve_begin <= src_slice->parent_vcurve_begin && "first slice specified is not the first in the parent");
        assert(dst_slice->side == src_slice->side * 2);

    }

    ///add the channels to the dst_slice
    for (const auto* src_slice : src_slices)
    {
        if (!src_slice)
            continue;
        assert(src_slice->buffers);
        const auto& src_buffers = *src_slice->buffers;
        
        dst_slice->buffers->copy_schema(src_buffers);
        break;
    }
    
    ///the offset of each source slice in the the destination slice; we will compute this momentarily.
    std::vector<vcurvesize_t> src_vcurve_adjustments(src_slices.size(), 0);

    
    ///copy data over to the destination slice
    {
        for (std::size_t src_slice_index = 0; src_slice_index < src_slices.size(); ++src_slice_index)
        {
            const auto* src_slice = src_slices[src_slice_index];

            if (!src_slice)
                continue;

            assert(src_slice);
            assert(src_slice->children);
            assert(src_slice->pos_data);
            assert(src_slice->buffers);

            assert( src_slice->parent_vcurve_begin >= dst_slice->parent_vcurve_begin );

            const auto& src_pos_data = *src_slice->pos_data;
            const auto& src_buffers = *src_slice->buffers;

            ///the offset of this source slice in the the destination slice
            vcurve_t src_vcurve_adjustment = (src_slice->parent_vcurve_begin - dst_slice->parent_vcurve_begin)*8;

            ///cache this for later
            src_vcurve_adjustments[src_slice_index] = src_vcurve_adjustment;


            //std::cout << "----" << std::endl;

            //std::cout << "src_slice_index: " << src_slice_index << std::endl;
            //std::cout << "src_slice->parent_vcurve_begin: " << src_slice->parent_vcurve_begin << std::endl;
            //std::cout << "dst_slice->parent_vcurve_begin: " << dst_slice->parent_vcurve_begin << std::endl;
            //std::cout << "src_vcurve_adjustment: " << src_vcurve_adjustment << std::endl;
            
            
            for (const vcurve_t vcurve : src_pos_data)
            {

                vcurve_t adjusted_vcurve = vcurve + src_vcurve_adjustment;

                //std::cout << "  vcurve: " << vcurve << std::endl;
                //std::cout << "  adjusted_vcurve: " << adjusted_vcurve << std::endl;
                //std::cout << "  dst_data.back().vcurve: " << (dst_data.size() ? std::get<0>(dst_data.back()) : -1) << std::endl;
                //std::cout << "  ---" << std::endl;

                assert(adjusted_vcurve < vcurvesize(dst_slice->side));

                if (dst_pos_data.size() > 0)
                    assert( adjusted_vcurve > dst_pos_data.back());

                dst_pos_data.push_back( adjusted_vcurve );
            }
            
            assert(src_buffers.schema() == dst_buffers.schema());
            
            const auto& src_buffers_list = src_buffers.buffers();
            auto& dst_buffers_list = dst_buffers.buffers();
            
            for (std::size_t buffer_index = 0; buffer_index < src_buffers_list.size(); ++buffer_index)
            {
                const auto& src_buffer = src_buffers_list[ buffer_index ];
                auto& dst_buffer = dst_buffers_list[buffer_index];
                
                assert(src_buffer.entries() == src_pos_data.size());
                assert(dst_buffer.declaration() == src_buffer.declaration());
                
                dst_buffer.append_buffer(src_buffer);
                
                //std::size_t output_byte_index = src_buffer.entries()*dst_channel.element_size;
                //dst_channel.resize(dst_channel.size() + src_channel.size());
                
                ///copy the data over.
                //std::memcpy(dst_channel.rawdata() + output_byte_index, src_channel.rawdata(), src_channel.size()*src_channel.element_size);
            }
        }
    }

    ///attach the destination slice to the parent. detatch the src slices from the parent.
    {
        assert(dst_slice->parent_slice);
        assert(dst_slice->parent_slice->children);

        std::size_t child_index = 0;
        auto& parent_children = *(dst_slice->parent_slice->children);

        //std::cout << "dst_slice->parent_vcurve_begin: " << dst_slice->parent_vcurve_begin << std::endl;
        for (auto* src_slice : src_slices)
        {
            if (!src_slice)
                continue;


            for (; child_index < parent_children.size(); ++child_index)
            {
                if (parent_children[child_index] == src_slice)
                {
                    parent_children[child_index] = 0;
                    break;
                }
            }
            ///
            if (child_index < parent_children.size() && !parent_children[child_index])
                ///the child was removed successfully
                continue;
            assert(false && "source slice was not found as a child of the parent ...");
        }

        ///clean up the null children
        svo_slice_t::children_t parent_children1;
        bool dst_slice_appended = false;
        for (auto* sibling : parent_children){
            ///skip the nullified children
            if (sibling) {
                parent_children1.push_back(sibling);
            } else if (!dst_slice_appended){
                ///instead of the first nullified child, add the new replacement dst_slice
                parent_children1.push_back(dst_slice);
                dst_slice_appended = true;
            }
        }
        parent_children = parent_children1;

        /*
        for (auto* src_slice : src_slices)
            if (src_slice)
                std::cout << "src_slice->parent_vcurve_begin: " << src_slice->parent_vcurve_begin << std::endl;
        for (auto* child : parent_children1)
            std::cout << "sibling->parent_vcurve_begin: " << child->parent_vcurve_begin << std::endl;
        */
    }

    ///detatch children of src slices from src slice; connect the children of the src slices to the destination slice. 
    {
        assert(dst_slice->children);

        auto& dst_children = *dst_slice->children;
        assert(dst_children.size() == 0);

        ///for each src slice
        for (std::size_t src_child_index = 0; src_child_index < src_slices.size(); ++src_child_index)
        {
            svo_slice_t* src_slice = src_slices[src_child_index];

            if (!src_slice)
                continue;





            assert(src_slice->children);

            ///for each child of the src slice, aka gchild slice.
            for (svo_slice_t* gchild : *src_slice->children)
            {
                assert(gchild->parent_slice == src_slice);

                ///reset the parent slice
                gchild->parent_slice = dst_slice;
                ///set the child slice of dst_slice
                dst_children.push_back(gchild);

                ///adjust the @c parent_vcurve_begin
                ///gchild->parent_vcurve_begin points to where in the src slice
                /// gchild begins.
                /// now that src slice has been combined with 7 other slices to form dst slice
                /// gchild points to somewhere in dst slice.
                /// that somewhere depends on which octant gchild was a child of.
                /// we already stored the adjustment for each octant within the grandparent slice in
                /// @c src_vcurve_adjustments; therefore we can use that,
                /// to calculate the adjustment in the grandchild slice from within the dst_slice.
                gchild->parent_vcurve_begin += src_vcurve_adjustments[src_child_index];
            }

            ///clear the children of @c src_slice.
            src_slice->children->clear();
        }
    }

    //std::cout << "---: " << std::endl;
    //std::cout << "parent_slice->side: " << dst_slice->parent_slice->side << std::endl;
    //std::cout << "dst_slice->side: " << dst_slice->side << std::endl;
    //std::cout << "src_slices.size(): " << src_slices.size() << std::endl;
    //::cout << "---: " << std::endl;

    ///check sanity.
    DEBUG {
        if(auto error = svo_slice_sanity(dst_slice))
        {
            std::cerr << error << std::endl;
            assert(false && "sanity failed");
        }

        assert(dst_slice->children);
        for (svo_slice_t* child : *dst_slice->children)
        {
            assert(child->parent_slice == dst_slice);
            if (auto error = svo_slice_sanity(child))
            {
                std::cerr << error << std::endl;
                assert(false && "sanity failed");
            }
        }
        
    }

}

/**
 *
 *
 */
void svo_split_slice(svo_slice_t* src_slice, std::vector<svo_slice_t*>& new_slices, bool copyusedata)
{

    assert(src_slice);
    assert(src_slice->pos_data);
    assert(src_slice->buffers);
    assert(src_slice->side > 1);

    const auto& src_pos_data = *src_slice->pos_data;
    const auto& src_buffers = *src_slice->buffers;
    
    ///the size of the curve.
    vcurvesize_t size = vcurvesize(src_slice->side);

    ///size should be divisible into 8 parts, one for each octant.
    assert(size % 8 == 0);

    vcurvesize_t octant_size = size / 8;

    std::vector< svo_slice_t* > dst_slices;

    ///initialize the destination slices.
    for (ccurve_t ccurve = 0; ccurve < 8; ++ccurve)
    {
        void* userdata = copyusedata ? src_slice->userdata : 0;

        dst_slices.push_back( svo_init_slice(src_slice->level, src_slice->side / 2, userdata) );
        svo_slice_t* dst_slice = dst_slices[ccurve];
        assert(dst_slice);
        dst_slice->side = src_slice->side / 2;
        dst_slice->level = src_slice->level;
        dst_slice->parent_slice = src_slice->parent_slice;
        dst_slice->parent_vcurve_begin = src_slice->parent_vcurve_begin + (octant_size*ccurve);
        
        dst_slice->buffers->copy_schema(src_buffers);
        
    }


    ///then split the actual data into 8 parts.
    {
        ///iterate through the source data and destination data simultaniously.

        ///a pointer to the current source data voxel.
        std::size_t data_index = 0;

        ///while iterating through the destination data, we have to iterate through the destination
        /// slice.
        for (ccurve_t dst_ccurve = 0; dst_ccurve < 8; ++dst_ccurve)
        {
            ///for each destination slice,
            svo_slice_t* dst_slice = dst_slices[dst_ccurve];
            assert(dst_slice);
            assert(dst_slice->pos_data);
            assert(false && "TODO: handle channels");
            auto& dst_pos_data = *dst_slice->pos_data;
            
            ///the first value in the curve for this corner's octant.
            vcurvesize_t octant_parent_vcurve_begin = octant_size*(dst_ccurve);
            assert(octant_parent_vcurve_begin == dst_slice->parent_vcurve_begin);

            ///the last value (+1) in the curve for this corner's octant.
            vcurvesize_t octant_parent_vcurve_end = octant_parent_vcurve_begin + octant_size;
            assert(octant_parent_vcurve_end <= size);

            ///iterate through the data, until done with this corner's octant within the souce data->
            while (data_index < src_pos_data.size())
            {

                vcurve_t vcurve = src_pos_data[data_index];

                assert(vcurve >= octant_parent_vcurve_begin);

                ///if this voxel is outside the the octant, break and move to next destination slice.
                if (!(vcurve < octant_parent_vcurve_end))
                    break;

                ///next source data voxel
                data_index++;

                vcurve_t adjusted_vcurve = vcurve - octant_parent_vcurve_begin;

                dst_pos_data.push_back( adjusted_vcurve );

            }
        }
    }

    ///if there is a parent slice attached, replace the parent's child reference with
    /// the (up to) 8 children.
    {
        assert(false && "TODO");
    }


    ///if there are child slices connected, they must not straddle across octants.
    ///if they do straddle (i.e if there is 1 child slice for example); then it must be split as well.
    ///if they do not straddle the octants, we divy them up to their new parents. 
    {
        assert(false && "TODO");
    }

    ///record the new slices.
    new_slices.insert(new_slices.end(), dst_slices.begin(), dst_slices.end());
}


std::list<svo_slice_child_group_t> svo_find_joinable_groups_of_children(svo_slice_t* slice)
{
    ///pseudocode:
    ///candidate_group = None
    ///for each child
        ///if candidate group is not initialized
            ///initialize candidate group with this child.
        ///else if this child is within the bounds of the candidate group
            ///if this child has the same @c side length as the candidate group
                ///then add this child to the candidate group
            ///else, the candidate group is not valid, throw it out
                ///and put this child into the new candidate group
                ///set the bounds of the candidate group to the first of this child's group's bounds.
        ///else if this child is not in the bounds of this candidate group
            ///add candidate group to the result groups
            ///and put this child into the new candidate group
            ///set the bounds of the candidate group to the first of this child's group's bounds.

    assert(slice);
    assert(slice->children);
    auto& children = *slice->children;


    std::list<svo_slice_child_group_t> groups;

    svo_slice_child_group_t candidate_group;

    auto reset_candidate_group = [&candidate_group]()
    {
        for (ccurve_t ccurve = 0; ccurve < 8; ++ccurve)
            candidate_group.group_children[ccurve] = 0;
        candidate_group.child_side = 0;
        candidate_group.group_side_in_parent = 0;
        candidate_group.group_size_in_parent = 0;
        candidate_group.parent_vcurve_begin = 0;
        candidate_group.count = 0;
    };

    auto add_to_candidate_group = [&candidate_group](svo_slice_t* child)
    {
        assert(child->parent_slice);

        if (child->side * 2 > SVO_MAX_VOLUME_SIDE)
            return;
        ///check if this candidate already takes up the parent's full volume
        if (child->side == child->parent_slice->side * 2)
            ///nothing to combine here
            return;

        ///if the current candidate group is null
        if (candidate_group.group_side_in_parent == 0)
        {
            candidate_group.child_side = child->side;

            vcurvesize_t child_side_in_parent = child->side / 2;
            ///the child is double resolution, so the child's side in the parent resolution is halved,
            /// but the group side is doubled the child's 
            candidate_group.group_side_in_parent = child_side_in_parent * 2;
            candidate_group.group_size_in_parent = vcurvesize(candidate_group.group_side_in_parent);

            ///if we partition the parent's space by group's size, we will know where the beginning
            /// of this group is. the child's position in the group is thus the modulus of the group size.
            vcurvesize_t child_parent_vcurve_in_group = child->parent_vcurve_begin % candidate_group.group_size_in_parent;
            ///and we can thus calculate the beginning of the group.
            candidate_group.parent_vcurve_begin = child->parent_vcurve_begin - child_parent_vcurve_in_group;
        }

        const auto& child_side = candidate_group.child_side;
        const auto& group_size_in_parent = candidate_group.group_size_in_parent;
        auto& group_children = candidate_group.group_children;
        vcurvesize_t group_parent_vcurve_begin = candidate_group.parent_vcurve_begin;
        vcurvesize_t group_parent_vcurve_end = group_parent_vcurve_begin + group_size_in_parent;
        vcurvesize_t child_size_in_parent = vcurvesize(child->side / 2);

        assert(child->side == child_side);

        ///if we partition the parent's space by group's size, we will know where the beginning
        /// of this group is. the child's position in the group is thus the modulus of the group size.
        vcurvesize_t child_parent_vcurve_in_group = child->parent_vcurve_begin % group_size_in_parent;
        //vcurvesize_t child_parent_vcurve_in_group_end = child_parent_vcurve_in_group + child_size_in_parent;

        ///make sure the child does not overflow the group.
        assert( child->parent_vcurve_begin + child_size_in_parent <= group_parent_vcurve_end);
        assert( child->parent_vcurve_begin >= group_parent_vcurve_begin);

        ///the child size should be 1 octant of the group.
        assert(child_size_in_parent == group_size_in_parent / 8);

        assert(child_parent_vcurve_in_group < group_size_in_parent);
        vcurvesize_t octant = child_parent_vcurve_in_group / child_size_in_parent;
        assert(octant < 8);
        ccurve_t child_ccurve = static_cast<ccurve_t>(octant);

        assert(group_children[child_ccurve] == 0 && "adding a child to a candidate group twice, no no no.");

        group_children[child_ccurve] = child;
        candidate_group.count += 1;
    };

    auto is_child_in_candidate_group_bounds = [&candidate_group] (const svo_slice_t* child)->bool{

        const auto& group_size_in_parent = candidate_group.group_size_in_parent;
        vcurvesize_t group_parent_vcurve_begin = candidate_group.parent_vcurve_begin;
        vcurvesize_t group_parent_vcurve_end = group_parent_vcurve_begin + group_size_in_parent;
        vcurvesize_t child_size_in_parent = vcurvesize(child->side / 2);

        if (!( child->parent_vcurve_begin + child_size_in_parent <= group_parent_vcurve_end))
            return false;
        if (!( child->parent_vcurve_begin >= group_parent_vcurve_begin))
            return false;
        return true;
    };


    ///candidate_group = None
    reset_candidate_group();

    ///for each child
    for (std::size_t i = 0; i < children.size(); ++i)
    {
        svo_slice_t* child = children[i];
        assert(child);
        DEBUG_PRINT { std::cerr << "child " << i << std::endl; }


        if (!(child->side*2 <= SVO_MAX_VOLUME_SIDE)) {
            reset_candidate_group();
            continue;
        }

        ///if candidate group is not initialized
        if (candidate_group.count == 0){
            DEBUG_PRINT { std::cerr << "  candidate group is empty; add this child to it" << std::endl; }
            ///initialize candidate group with this child.
            add_to_candidate_group(child);
        }
        ///else if this child is within the bounds of the candidate group
        else if (is_child_in_candidate_group_bounds(child)){
            ///if this child has the same @c side length as the candidate group
            DEBUG_PRINT { std::cerr << "  candidate group is NOT empty" << std::endl; }
            DEBUG_PRINT { std::cerr << "  AND this child is in the candidate group bounds" << std::endl; }
            if (child->side == candidate_group.child_side) {
                ///then add this child to the candidate group
                DEBUG_PRINT { std::cerr << "    this child has the same side length as the candidate group" << std::endl; }
                DEBUG_PRINT { std::cerr << "      adding child to candidate group" << std::endl; }
                add_to_candidate_group(child);
            }
            ///else, the candidate group is not valid,
            else {
                ///throw it out
                DEBUG_PRINT { std::cerr << "    child is not the correct size for the candidate group" << std::endl; }
                DEBUG_PRINT { std::cerr << "      flushing candidate group to the toilet" << std::endl; }
                reset_candidate_group();
                ///and put this child into the new candidate group
                DEBUG_PRINT { std::cerr << "      creating new candidate group for this child" << std::endl; }
                add_to_candidate_group(child);
                ///set the bounds of the candidate group to the beginning of this child's group's bounds.
                ///noop
            }
        }
        ///else if this child is not in the bounds of this candidate group
        else {
            ///add candidate group to the result groups

            DEBUG_PRINT { std::cerr << "  this child is not in the bounds of this candidate group" << std::endl; }
            DEBUG_PRINT { std::cerr << "    add candidate group to the result groups" << std::endl; }
            groups.push_back(candidate_group);

            ///and put this child into the new candidate group
            DEBUG_PRINT { std::cerr << "    reset the candidate group" << std::endl; }
            reset_candidate_group();

            DEBUG_PRINT { std::cerr << "    add this child to the new candidate group" << std::endl; }
            add_to_candidate_group(child);
            ///set the bounds of the candidate group to the first of this child's group's bounds.
            ///noop
        }

    }


    if (candidate_group.count > 1 && candidate_group.group_side_in_parent <= slice->side) {
        groups.push_back(candidate_group);
    }

    return groups;
}


bool svo_is_goffset_in_block(svo_block_t* block, goffset_t goffset)
{
    assert(block);
    if (goffset < block->block_start)
        return false;
    if (goffset >= block->block_end)
        return false;
    return true;
}


bool svo_is_slice_loading(svo_slice_t* slice)
{
    UNUSED(slice);
    return false;
}


bool svo_is_slice_loaded(svo_slice_t* slice)
{
    UNUSED(slice);
    return true;
}




} // namespace
