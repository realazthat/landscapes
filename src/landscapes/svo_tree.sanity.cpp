
#include "landscapes/svo_tree.sanity.hpp"
#include "landscapes/svo_tree.hpp"
#include "landscapes/svo_formatters.hpp"

#include "format.h"
#include <iostream>
#include <tuple>


namespace svo{
    
    
::std::ostream& operator<<(::std::ostream& out, const svo::svo_block_sanity_error_t& error)
{
    if (!error.has_error) {
        out << "no error";
        return out;
    }

    out << error.error;

    return out;
}

::std::ostream& operator<<(::std::ostream& out, const svo::svo_slice_sanity_error_t& error)
{
    if (!error.has_error) {
        out << "no error";
        return out;
    }

    out << error.error;

    return out;
}

svo_slice_sanity_error_t svo_slice_sanity_minimal(const svo_slice_t* slice, svo_sanity_type_t sanity_type);
svo_slice_sanity_error_t svo_slice_sanity_pos_data_to_parent(const svo_slice_t* slice);
svo_slice_sanity_error_t svo_slice_sanity_channel_data_to_parent(const svo_slice_t* slice);


svo_block_sanity_error_t svo_block_sanity_data(const svo_block_t* block)
{
    assert(block);
    assert(block->tree);

    goffset_t root_shadow_cd_goffset = block->root_shadow_cd_goffset;
    goffset_t parent_root_cd_goffset = block->parent_root_cd_goffset;

    if (root_shadow_cd_goffset == 0)
        return svo_block_sanity_error_t("root_shadow_cd_goffset is 0", block);
    if (parent_root_cd_goffset == 0)
        return svo_block_sanity_error_t("parent_root_cd_goffset is 0", block);
    

    if (block->root_shadow_cd_goffset == invalid_goffset)
        return svo_block_sanity_error_t(fmt::format("root_shadow_cd_goffset is invalid")
                                              , block);
    if (!block->is_valid_cd_goffset(block->root_shadow_cd_goffset))
        return svo_block_sanity_error_t(fmt::format("root_shadow_cd_goffset is not within the block")
                                              , block);


    if (!block->parent_block)
    {
        if (block->parent_root_cd_goffset != invalid_goffset)
            return svo_block_sanity_error_t(fmt::format("block->parent_block is blank, but parent_root_cd_goffset is pointing somewhere"
                                                        ", parent_root_cd_goffset: {}"
                                                        , block->parent_root_cd_goffset)
                                                  , block);
    }
    if (block->parent_block)
    {

        if (!block->has_root_children_goffset())
            return svo_block_sanity_error_t(fmt::format("block has parent but has no root_children_goffset ..")
                                                  , block);
        
        if (block->parent_root_cd_goffset == invalid_goffset)
            return svo_block_sanity_error_t(fmt::format("block->parent_block is set, but parent_root_cd_goffset is invalid")
                                                  , block);
        if (!block->parent_block->is_valid_cd_goffset(block->parent_root_cd_goffset))
            return svo_block_sanity_error_t(fmt::format("parent_root_cd_goffset is not within the parent block")
                                                  , block);

        ///gets the root shadow CD, which lies in the current block
        const auto* shadow_root_cd = svo_cget_cd(block->tree->address_space, block->root_shadow_cd_goffset);
        ///gets the root CD, which lies in the parent block
        const auto* parent_root_cd = svo_cget_cd(block->tree->address_space, block->parent_root_cd_goffset);
        
        ///the root CD should always have a far pointer to this block.
        if (!svo_get_far(parent_root_cd))
            return svo_block_sanity_error_t(fmt::format("the root CD that points into this block is not using a far ptr")
                                                  , block);

        
        ///the goffset to the beginning of the root children set.
        goffset_t shadow_root_cd_children_goffset = svo_get_child_ptr_goffset(block->tree->address_space, block->root_shadow_cd_goffset, shadow_root_cd);
        ///the goffset to the beginning of the root children set.
        goffset_t parent_root_cd_children_goffset = svo_get_child_ptr_goffset(block->tree->address_space, block->parent_root_cd_goffset, parent_root_cd);
        
        assert(block->is_valid_cd_goffset(block->root_shadow_cd_goffset));
        assert(block->parent_block->is_valid_cd_goffset(block->parent_root_cd_goffset));
        assert(shadow_root_cd_children_goffset == block->root_children_goffset());



        if (!block->is_valid_cd_goffset(parent_root_cd_children_goffset))
            return svo_block_sanity_error_t(fmt::format("the root CD that points into this block is pointing somewhere not inside this block"
                                                        ", parent_root_cd_children_goffset: {}"
                                                        , parent_root_cd_children_goffset)
                                                  , block);
        if (!block->is_valid_cd_goffset(shadow_root_cd_children_goffset))
            return svo_block_sanity_error_t(fmt::format("the shadow CD of this block is pointing somewhere not inside this block"
                                                        ", shadow_root_cd_children_goffset: {}"
                                                        , shadow_root_cd_children_goffset)
                                                  , block);
        if (shadow_root_cd_children_goffset != parent_root_cd_children_goffset)
            return svo_block_sanity_error_t(fmt::format("the root CD that points into this block is not pointing to the same location as the shadow root CD"
                                                        ", shadow_root_cd_children_goffset: {}, parent_root_cd_children_goffset: {}"
                                                        , shadow_root_cd_children_goffset, parent_root_cd_children_goffset)
                                                  , block);

    }


    if (block->root_valid_bit)
    {
        
    } else {
        //if (root_shadow_cd_goffset != invalid_goffset)
        //    return svo_block_sanity_error_t("root is not root_valid_bit, yet has a root shadow CD", block);

    }
    
    if (block->root_leaf_bit)
    {
        if (!(block->root_valid_bit))
            return svo_block_sanity_error_t("root_valid_bit is not set, yet root_leaf_bit is set ...", block);
        
    } else {
        if (root_shadow_cd_goffset == invalid_goffset)
            return svo_block_sanity_error_t("root is not a leaf, and has no root shadow CD", block);
    }

    std::vector< svo_block_sanity_error_t > issues;
    std::size_t voxel_count = 0;
    std::size_t leaf_count = 0;
    std::size_t cd_count = 0;

    std::set< goffset_t > found_cd_goffsets;
    auto visitor = [&voxel_count, &leaf_count, &cd_count, &block, &issues, &found_cd_goffsets](goffset_t pcd_goffset, goffset_t cd_goffset, ccurve_t voxel_ccurve
                                                            , std::tuple<std::size_t, vcurve_t> metadata)
    {


        ++voxel_count;

        std::size_t level = std::get<0>(metadata);
        vcurve_t parent_vcurve = std::get<1>(metadata);
        vcurve_t voxel_vcurve = parent_vcurve*8 + voxel_ccurve;

        vside_t level_side = 1 << level;
        vcurvesize_t level_size = vcurvesize(level_side);




        bool is_cd = (cd_goffset != invalid_goffset);
        bool is_invalid_cd = is_cd && (!block->is_valid_cd_goffset(cd_goffset));
        const child_descriptor_t* cd = 0;

        if (is_cd && !is_invalid_cd)
            cd = svo_cget_cd(block->tree->address_space, cd_goffset);

        std::size_t cd_nonleaf_count = cd ? svo_get_cd_nonleaf_count(cd) : 0;
        std::size_t cd_valid_count = cd ? svo_get_cd_valid_count(cd) : 0;


        bool is_leaf = !is_cd || (!is_invalid_cd && (cd_valid_count == 0));
        bool is_bottom_leaf = !block->trunk && (level == block->height - 1);

        /*
        std::cout << "is_bottom_leaf: " << (is_bottom_leaf ? "true" : "false")
                  << ", is_leaf: " << (is_leaf ? "true" : "false")
                  << ", is_cd: " << (is_cd ? "true" : "false")
                  << ", cd_nonleaf_count: " << cd_nonleaf_count
                  << std::endl;
        if (cd)
            std::cout << ", cd: " << *cd << std::endl;
        */
        
        if (is_leaf)
            ++leaf_count;
        if (is_cd)
            ++cd_count;


        if(cd_goffset != invalid_goffset)
        {

            if (cd_goffset == svo_get_ph_goffset(cd_goffset))
                issues.push_back(svo_block_sanity_error_t(fmt::format("CD is located on a page header!"
                                                                      ", pcd_goffset: {}, cd_goffset: {}, voxel_ccurve: {}, level: {}, block->height: {}"
                                                                      , pcd_goffset, cd_goffset, voxel_ccurve, level, block->height)
                                                            , block));


            if (found_cd_goffsets.count(cd_goffset) > 0)
                issues.push_back(svo_block_sanity_error_t(fmt::format("Duplicate CD goffset"
                                                                      ", pcd_goffset: {}, cd_goffset: {}, voxel_ccurve: {}, level: {}, block->height: {}"
                                                                      , pcd_goffset, cd_goffset, voxel_ccurve, level, block->height)
                                                            , block));

            if (pcd_goffset != invalid_goffset && found_cd_goffsets.count(pcd_goffset) == 0)
                issues.push_back(svo_block_sanity_error_t(fmt::format("Parent CD is not in found_cd_goffsets"
                                                                      ", pcd_goffset: {}, cd_goffset: {}, voxel_ccurve: {}, level: {}, block->height: {}"
                                                                      , pcd_goffset, cd_goffset, voxel_ccurve, level, block->height)
                                                            , block));
            found_cd_goffsets.insert(cd_goffset);
        }

        if (!block->trunk && !(level < block->height))
        {
            issues.push_back(svo_block_sanity_error_t(fmt::format("Voxel level out of block bounds"
                                                                  ", pcd_goffset: {}, cd_goffset: {}, voxel_ccurve: {}, level: {}, block->height: {}"
                                                                  , pcd_goffset, cd_goffset, voxel_ccurve, level, block->height)
                                                        , block));
        }

        if (!(voxel_vcurve < level_size))
        {
            issues.push_back(svo_block_sanity_error_t(fmt::format("Voxel vcurve out of level bounds"
                                                                  ", pcd_goffset: {}, cd_goffset: {}, block->height: {}"
                                                                  ", level: {}, level_side: {}, level_size:{}, voxel_ccurve: {}"
                                                                  , pcd_goffset, cd_goffset, block->height
                                                                  , level, level_side, level_size, voxel_vcurve)
                                                        , block));
        }

        if (is_invalid_cd)
        {
            issues.push_back(svo_block_sanity_error_t(fmt::format("Voxel is a CD but is not inside block"
                                                                  ", pcd_goffset: {}, cd_goffset: {}, voxel_ccurve: {}, level: {}, block->height: {}"
                                                                  , pcd_goffset, cd_goffset, voxel_ccurve, level, block->height)
                                                        , block));
        }
        if (is_bottom_leaf && !is_leaf)
        {
            issues.push_back(svo_block_sanity_error_t(fmt::format("Voxel is at bottom leaf level, but not a leaf"
                                                                  ", pcd_goffset: {}, cd_goffset: {}, voxel_ccurve: {}, level: {}, block->height: {}"
                                                                  , pcd_goffset, cd_goffset, voxel_ccurve, level, block->height)
                                                        , block));
        }

        /*
        if (is_bottom_leaf && is_cd)
        {
            issues.push_back(svo_block_sanity_error_t(fmt::format("Voxel is at bottom leaf level, but has a CD"
                                                                  ", pcd_goffset: {}, cd_goffset: {}, voxel_ccurve: {}, level: {}, block->height: {}"
                                                                  , pcd_goffset, cd_goffset, voxel_ccurve, level, block->height)
                                                        , block));
        }*/

        if (pcd_goffset == 0)

            issues.push_back(svo_block_sanity_error_t(fmt::format("pcd_goffset somehow is equal to zero, zero is not an defined;"
                                                                " it isn't even an invalid offset, use @c invalid_goffset instead")
                                                    , block));

        if (cd_goffset == 0)

            issues.push_back(svo_block_sanity_error_t(fmt::format("cd_goffset somehow is equal to zero, zero is not an defined;"
                                                                " it isn't even an invalid offset, use @c invalid_goffset instead")
                                                    , block));


        if (!is_leaf && cd_goffset == pcd_goffset)
            issues.push_back(svo_block_sanity_error_t(fmt::format("Child ptr is zero, yet this is a non-leaf voxel .."
                                                                  ", pcd_goffset: {}, cd_goffset: {}, voxel_ccurve: {}"
                                                                  , pcd_goffset, cd_goffset, voxel_ccurve), block));

        if (cd_goffset != invalid_goffset && cd_goffset != 0 && !block->is_valid_cd_goffset(cd_goffset))
            issues.push_back(svo_block_sanity_error_t(fmt::format("cd_goffset is an invalid CD, not inside block bounds .."
                                                                  ", pcd_goffset: {}, cd_goffset: {}, voxel_ccurve: {}"
                                                                  , pcd_goffset, cd_goffset, voxel_ccurve), block));
        if (pcd_goffset != invalid_goffset && pcd_goffset != 0 && !block->is_valid_cd_goffset(pcd_goffset))
            issues.push_back(svo_block_sanity_error_t(fmt::format("pcd_goffset is an invalid CD, not inside block bounds .."
                                                                  ", pcd_goffset: {}, cd_goffset: {}, voxel_ccurve: {}"
                                                                  , pcd_goffset, cd_goffset, voxel_ccurve), block));

        if (pcd_goffset != invalid_goffset && pcd_goffset != 0 && block->is_valid_cd_goffset(pcd_goffset))
        {
            const auto* pcd = svo_cget_cd(block->tree->address_space, pcd_goffset);
            bool parent_valid_bit = svo_get_valid_bit(pcd, voxel_ccurve);
            bool parent_leaf_bit = svo_get_leaf_bit(pcd, voxel_ccurve);

            if (!parent_valid_bit)
                issues.push_back(svo_block_sanity_error_t(fmt::format("Voxel exists, but parent says this is an invalid voxel"
                                                                      ", pcd_goffset: {}, cd_goffset: {}, voxel_ccurve: {}"
                                                                      , pcd_goffset, cd_goffset, voxel_ccurve), block));
            if (parent_leaf_bit && cd_goffset != invalid_goffset && cd_goffset != 0)
                issues.push_back(svo_block_sanity_error_t(fmt::format("Parent says voxel is a leaf, but yet we have a CD for it"
                                                                      ", pcd_goffset: {}, cd_goffset: {}, voxel_ccurve: {}"
                                                                      , pcd_goffset, cd_goffset, voxel_ccurve), block));

        }

        if (cd_goffset != pcd_goffset && cd_goffset != invalid_goffset && cd_goffset != 0)
        {
            const auto* cd = svo_cget_cd(block->tree->address_space, cd_goffset);

            child_mask_t valid_mask = svo_get_valid_mask(cd);
            child_mask_t leaf_mask = svo_get_leaf_mask(cd);

            child_mask_t error_bits = leaf_mask & (~valid_mask);

            if (error_bits)
                issues.push_back(svo_block_sanity_error_t(fmt::format("CD has children that are invalid, but are marked as leafs"
                                                                      ", pcd_goffset: {}, cd_goffset: {}, voxel_ccurve: {}"
                                                                      ", valid_mask: {}, leaf_mask: {}, error_bits: {}"
                                                                      , pcd_goffset, cd_goffset, voxel_ccurve
                                                                      , std::bitset<8>(valid_mask)
                                                                      , std::bitset<8>(leaf_mask)
                                                                      , std::bitset<8>(error_bits)), block));

            bool has_non_leaf_child = cd_nonleaf_count > 0;


            goffset_t child0_base_goffset = svo_get_child_ptr_goffset(block->tree->address_space, cd_goffset, cd);


            if (has_non_leaf_child && child0_base_goffset == invalid_goffset)
                issues.push_back(svo_block_sanity_error_t(fmt::format("CD's child ptr is invalid"
                                                                      ", pcd_goffset: {}, cd_goffset: {}, voxel_ccurve: {}"
                                                                      ", valid_mask: {}, leaf_mask: {}, child0_base_goffset: {}"
                                                                      , pcd_goffset, cd_goffset, voxel_ccurve
                                                                      , std::bitset<8>(valid_mask)
                                                                      , std::bitset<8>(leaf_mask)
                                                                      , child0_base_goffset), block));

            if (has_non_leaf_child && !block->is_valid_cd_goffset(child0_base_goffset))
            {

                if (!block->trunk)
                    issues.push_back(svo_block_sanity_error_t(fmt::format("CD's child ptr points outside of the block's bounds, and this is not a trunk"
                                                                          ", pcd_goffset: {}, cd_goffset: {}, voxel_ccurve: {}"
                                                                          ", valid_mask: {}, leaf_mask: {}, child0_base_goffset: {}"
                                                                          , pcd_goffset, cd_goffset, voxel_ccurve
                                                                          , std::bitset<8>(valid_mask)
                                                                          , std::bitset<8>(leaf_mask)
                                                                          , child0_base_goffset), block));
            }
            

            

        }





        return std::make_tuple(level+1, voxel_vcurve);
    };


    z_preorder_traverse_block_cds(  block->tree->address_space
                        , block
                        , std::make_tuple(std::size_t(0), vcurve_t(0)) ///(level,vcurve) as metadata
                        , visitor);


    if (issues.size() > 0)
        return issues[0];



    if (cd_count != block->cd_count)
        return svo_block_sanity_error_t(fmt::format("CD counts do not match. block->cd_count: {}, actual CD count: {}"
                                                    , block->cd_count, cd_count), block);
    
    if (leaf_count != block->leaf_count)
        return svo_block_sanity_error_t(fmt::format("voxel counts do not match. block->leaf_count: {}, actual leaf_count: {}"
                                                    , block->leaf_count, leaf_count), block);

    return svo_block_sanity_error_t();
}


svo_block_sanity_error_t svo_block_sanity_check(const svo_block_t* block, int recurse)
{

    auto check_is_valid_goffset = [&block](const std::string& name, goffset_t goffset)
    {
        if (goffset == 0 || goffset == invalid_goffset)
            return svo_block_sanity_error_t(fmt::format("{} is is not a valid goffset: {}.", name, goffset), block);
        return svo_block_sanity_error_t();
    };

    auto check_is_range = [&block](const std::string& range_name, goffset_t begin, goffset_t end)
    {
        if (begin > end)
            return svo_block_sanity_error_t(fmt::format("{} is is not a valid range: [{}, {}).", range_name, begin, end), block);
        return svo_block_sanity_error_t();
    };

    auto check_in_range = [&block](   const std::string& name, goffset_t goffset, std::size_t size
                                    , const std::string& range_name, goffset_t begin, goffset_t end)
    {
        if (goffset < begin || goffset + size > end )
            return svo_block_sanity_error_t(fmt::format("{} is out of bounds of {}.", name, range_name), block);
        return svo_block_sanity_error_t();
    };









    if(!block)
        return svo_block_sanity_error_t("block is invalid pointer", block);

    if(!(block->child_blocks))
        return svo_block_sanity_error_t("block->child_blocks is invalid pointer", block);
    if(!(block->tree))
        return svo_block_sanity_error_t("block->tree is invalid pointer", block);

    if (block->trunk)
    {
        if(block->height != 0)
            return svo_block_sanity_error_t("block is trunk, yet has a height", block);
        if(block->side != 0)
            return svo_block_sanity_error_t("block is trunk, yet has a side", block);
    }


    if (block->root_shadow_cd_goffset == 0)
        return svo_block_sanity_error_t(fmt::format("root_shadow_cd_goffset is 0, an undefined value; use @c invalid_goffset somewhere instead"
                                                    ), block);

    if (block->root_valid_bit)
    {
        if (!block->trunk && block->height < 1)
            return svo_block_sanity_error_t(fmt::format("root is valid but block->height is 0"
                                                        ", height: {}, side: {}, root_valid_bit: {}, root_leaf_bit: {}"
                                                        , block->height, block->side, block->root_valid_bit, block->root_leaf_bit), block);

        vside_t expected_side = (1UL << (block->height - 1));
        if (!block->trunk && block->side != expected_side)
            return svo_block_sanity_error_t(fmt::format("side does not match expected side based on height"
                                                        ", height: {}, side: {}, expected side: {}, root_valid_bit: {}, root_leaf_bit: {}"
                                                        , block->height, block->side, expected_side, block->root_valid_bit, block->root_leaf_bit)
                                            , block);
        if (!(block->root_leaf_bit) && block->root_shadow_cd_goffset == invalid_goffset)
            return svo_block_sanity_error_t(fmt::format("nonleaf root, and no shadow root cd specified"), block);
    }

    if (block->root_leaf_bit)
    {
        if (!block->trunk && !(block->root_valid_bit))
            return svo_block_sanity_error_t(fmt::format("root is a leaf but is marked as invalid"
                                                        ", height: {}, side: {}, root_valid_bit: {}, root_leaf_bit: {}"
                                                        , block->height, block->side, block->root_valid_bit, block->root_leaf_bit), block);
        if (!block->trunk && block->height != 1)
            return svo_block_sanity_error_t(fmt::format("root is a leaf but block->height is not 1"
                                                        ", height: {}, side: {}, root_valid_bit: {}, root_leaf_bit: {}"
                                                        , block->height, block->side, block->root_valid_bit, block->root_leaf_bit), block);


    }


    
    if (block->block_start % SVO_PAGE_SIZE != 0 || block->block_end % SVO_PAGE_SIZE != 0)
        return svo_block_sanity_error_t(fmt::format("block is not aligned to page!"
                                                    ", block->block_start: {}, misalignment: {}"
                                                    ", block->block_end: {}, misalignment: {}"
                                                    ", SVO_PAGE_SIZE: {}"
                                                    , block->block_start, block->block_start % SVO_PAGE_SIZE
                                                    , block->block_end, block->block_end % SVO_PAGE_SIZE
                                                    , SVO_PAGE_SIZE)
                                                ,block);

    if (!!(block->slice) && block->child_blocks->size() > 0)
        return svo_block_sanity_error_t(fmt::format("block has a child slice, but also has child blocks ...")
                                        , block);


    if(block->root_shadow_cd_goffset == 0)
        return svo_block_sanity_error_t("block->root_shadow_cd_goffset is not initialized; this should always be pointing to a dummy root CD.", block);


    ///range sanities
    {


        if (auto error = check_is_valid_goffset("block_start", block->block_start))
            return error;
        if (auto error = check_is_valid_goffset("block_end", block->block_end))
            return error;
        if (auto error = check_is_valid_goffset("cd_start", block->cd_start))
            return error;
        if (auto error = check_is_valid_goffset("cd_end", block->cd_end))
            return error;
        if (auto error = check_is_valid_goffset("cdspace_end", block->cdspace_end))
            return error;


        if (auto error = check_is_range("[block_start,block_end)", block->block_start, block->block_end))
            return error;
        if (auto error = check_is_range("[cd_start,cd_end)", block->cd_start, block->cd_end))
            return error;
        if (auto error = check_is_range("[cd_start,cdspace_end)", block->cd_start, block->cdspace_end))
            return error;

        if (auto error = check_in_range("cd_start", block->cd_start, 0
                                        , "the block", block->block_start, block->block_end))
            return error;
        if (auto error = check_in_range("cd_end", block->cd_end, 0
                                        , "the block", block->block_start, block->block_end))
            return error;
        if (auto error = check_in_range("cdspace_end", block->cdspace_end, 0
                                        , "the block", block->block_start, block->block_end))
            return error;
        
        if (auto error = check_in_range("cd_end", block->cd_end, 0
                                        , "cd space", block->cd_start, block->cdspace_end))
            return error;

    }


    if (block->slice)
    {

        vside_t expected_slice_side = block->side == 0 ? 1 : block->side * 2;
        
        if (expected_slice_side != block->slice->side)
            return svo_block_sanity_error_t(fmt::format("slice side does not match the next expected level's side ..."
                                                        " block->side: {}, slice->side: {}, expected_slice_side: {}"
                                                        , block->side, block->slice->side, expected_slice_side)
                                            , block);
    }
    

    auto calculate_cd_level = [](const svo_block_t* block, goffset_t needle)
    {
        assert(block->is_valid_cd_goffset(needle));

        std::size_t needle_level = std::size_t(-1);

        auto visitor = [&needle_level, needle](  goffset_t pcd_goffset, goffset_t cd_goffset, ccurve_t voxel_ccurve
                          , std::tuple<std::size_t, vcurve_t> metadata)
        {
            std::size_t level = std::get<0>(metadata);
            vcurve_t parent_vcurve = std::get<1>(metadata);
            vcurve_t vcurve = parent_vcurve*8 + voxel_ccurve;

            if (cd_goffset == needle)
                needle_level = level;

            return std::make_tuple(level+1, vcurve);
        };

        z_preorder_traverse_block_cds(block->tree->address_space
                        , block
                        , std::make_tuple(std::size_t(0), vcurve_t(0)) ///(level) as metadata
                        , visitor);

        return needle_level;
    };

    if (block->parent_block)
    {
        auto& parent_block = *block->parent_block;
        if (!parent_block.child_blocks)
            return svo_block_sanity_error_t("block->parent_block->child_blocks is invalid pointer", block, block->parent_block);

        if (std::find(parent_block.child_blocks->begin(), parent_block.child_blocks->end(), block) == parent_block.child_blocks->end())
            return svo_block_sanity_error_t("block->parent_block->child_blocks does not contain block ...", block, block->parent_block);

        
        if (!(block->parent_block->is_valid_cd_goffset(block->parent_root_cd_goffset)))
            return svo_block_sanity_error_t("block->parent_root_cd_goffset is pointing to something, but that something is not within the parent block ..."
                                            , block);


        std::size_t parent_root_cd_cd_level = calculate_cd_level(block->parent_block, block->parent_root_cd_goffset);

        std::size_t expected_root_level = block->parent_block->root_level + parent_root_cd_cd_level;

        if (block->root_level != expected_root_level)
            return svo_block_sanity_error_t(fmt::format("block->root_level is not the expected root level"
                                                        " root_level: {}, parent_root_cd_cd_level: {}, expected_root_level: {}"
                                                        , block->root_level, parent_root_cd_cd_level, expected_root_level)
                                                    , block);

    } else {
        if (block->parent_root_cd_goffset != invalid_goffset)
            return svo_block_sanity_error_t("block->parent_root_cd_goffset is pointing to something, but there is no parent block ...", block);

    }

    for (const svo_block_t* child_block : *block->child_blocks)
    {
        if (child_block->parent_block != block)
            return svo_block_sanity_error_t("child block does not point back to the parent ...", block, child_block);
        
    }



    if (auto error = svo_block_sanity_data(block))
    {
        return error;
    }






    if (recurse)
    {
        for (const svo_block_t* child_block : *block->child_blocks)
        {
            auto result = svo_block_sanity_check(child_block, recurse-1);
            if (result)
                return result;
        }
    }

    return svo_block_sanity_error_t();
}


svo_slice_sanity_error_t svo_slice_sanity_channel_data_to_parent(const svo_slice_t* slice)
{

    if (auto error = svo_slice_sanity_minimal(slice, svo_sanity_type_t::minimal))
    {
        return error;
    }

    assert(slice);
    assert(slice->pos_data);
    assert(slice->buffers);

    const auto& pos_data = *(slice->pos_data);
    const auto& buffers = *(slice->buffers);



    for (const auto& buffer : buffers.buffers())
    {
        if (buffer.entries() != pos_data.size())
            return svo_slice_sanity_error_t(fmt::format("different number of channel elements than position elements"
                                                        " buffer.declaration: {}, buffer.entries(): {}, pos_data.size(): {}"
                                                        , buffer.declaration(), buffer.entries(), pos_data.size())
                                            , slice);
    }


    if (slice->parent_slice)
    {
        if (slice->parent_slice->buffers == nullptr)
            return svo_slice_sanity_error_t(fmt::format("slice->parent_slice->buffers is invalid pointer")
                                            , slice);
        
        const auto& parent_buffers = *slice->parent_slice->buffers;
        
        
        if (buffers.schema() != parent_buffers.schema())
            return svo_slice_sanity_error_t(fmt::format("parent_slice->buffers does not match buffers"
                                                        ", buffers: {}, parent_buffers: {}"
                                                        , buffers.schema(), parent_buffers.schema())
                                            , slice);
        


    }


    return svo_slice_sanity_error_t();
}

svo_slice_sanity_error_t svo_slice_sanity_pos_data_to_parent(const svo_slice_t* slice)
{
    if (auto error = svo_slice_sanity_minimal(slice, svo_sanity_type_t::minimal))
    {
        return error;
    }

    assert(slice->pos_data);
    const auto& pos_data = *(slice->pos_data);

    ///check position data sanity.
    {
        vcurvesize_t max_vcurve = vcurvesize(slice->side);

        for (const vcurve_t voxel_vcurve : pos_data)
        {
            if (!(voxel_vcurve < max_vcurve))
                return svo_slice_sanity_error_t(fmt::format("data contains a voxel that is outside of the bounds of the slice ..."
                                                            " voxel_vcurve: {}, max_vcurve: {}"
                                                            , voxel_vcurve, max_vcurve)
                                                , slice); 
        }
    }



    if (!(slice->parent_slice))
        return svo_slice_sanity_error_t();

    assert(slice->parent_slice->pos_data);
    const auto& parent_pos_data = *(slice->parent_slice->pos_data);
    

    ///we will iterate through the parent data in tandem with our data.
    vcurve_t parent_data_index = 0;

    for (const vcurve_t voxel_vcurve : pos_data)
    {
        vcurve_t expected_parent_voxel_vcurve = (voxel_vcurve / 8) + slice->parent_vcurve_begin;


        ///move the cursor in the parent data until we are up to our expected @c parent_voxel_vcurve
        while (parent_data_index < parent_pos_data.size() && parent_pos_data[parent_data_index] < expected_parent_voxel_vcurve)
        {
            ++parent_data_index;
        }

        if (!(parent_data_index < parent_pos_data.size()) || expected_parent_voxel_vcurve != parent_pos_data[parent_data_index])
        {
            return svo_slice_sanity_error_t(fmt::format("data contains a voxel that is not contained in the parent slice ..."
                                                        " voxel_vcurve: {}, expected_parent_voxel_vcurve: {}"
                                                        , voxel_vcurve, expected_parent_voxel_vcurve)
                                            , slice);
        }

        assert(parent_data_index < parent_pos_data.size());
        if (expected_parent_voxel_vcurve == parent_pos_data[parent_data_index])
            ///we found the parent voxel; it exists.
            continue;
    }


    return svo_slice_sanity_error_t();

}

svo_slice_sanity_error_t svo_slice_sanity_minimal(const svo_slice_t* slice, svo_sanity_type_t sanity_type)
{
    if(!slice)
        return svo_slice_sanity_error_t("slice is invalid pointer", slice);


    ///parent <=> slice sanity
    if (slice->parent_slice)
    {
        auto parent_slice = slice->parent_slice;

        if (sanity_type & svo_sanity_type_t::levels)
            if (parent_slice->level + 1 != slice->level)
                return svo_slice_sanity_error_t(
                            fmt::format("parent->level is not one less than slice->level"
                                        ", parent->level: {}, slice->level: {}"
                                        ", parent: {}, slice: {}"
                                        , parent_slice->level, slice->level
                                        , (void*)parent_slice, (void*)slice), slice);

        if (!parent_slice->children)
            return svo_slice_sanity_error_t("parent->children is invalid pointer", slice);

        const auto& siblings = *parent_slice->children;

        auto it = std::find(siblings.begin(), siblings.end(), slice);

        if (it == siblings.end())
            return svo_slice_sanity_error_t("slice not in parent's children", slice);

        
    }
    
    
    if (slice->parent_slice == nullptr)
    {
        if (slice->parent_vcurve_begin != 0)
            return svo_slice_sanity_error_t(fmt::format("slice has no parent, and yet has a parent_vcurve_begin: {}"
                                                        , slice->parent_vcurve_begin), slice);
        
        
        
    }

    if (!slice->pos_data)
        return svo_slice_sanity_error_t("slice->data is an invalid pointer", slice);
    if (!slice->buffers)
        return svo_slice_sanity_error_t("slice->buffers is an invalid pointer", slice);

    const auto& pos_data = *slice->pos_data;

    ///parameter consistency
    {
        if (!slice->parent_slice && slice->parent_vcurve_begin != 0)
            return svo_slice_sanity_error_t(fmt::format("no parent slice specified, but has a non-zero parent_vcurve_begin  ..."
                                                        " parent_vcurve_begin: {}"
                                                        , slice->parent_vcurve_begin)
                                            , slice);

        if (slice->side == 0)
            return svo_slice_sanity_error_t(fmt::format("side is zero ...")
                                            , slice);

        assert(slice->side != 0);
        
        if (slice->side == 1 && slice->parent_slice != nullptr)
            return svo_slice_sanity_error_t(fmt::format("slice has a side of 1, and yet has a parent slice")
                                            , slice->parent_slice, slice);
        
        
        if (slice->side > SVO_VOLUME_SIDE_LIMIT)
            return svo_slice_sanity_error_t(fmt::format("slice has a side of greater than SVO_MAX_VOLUME_SIDE"
                                                        "side: {}, SVO_MAX_VOLUME_SIDE: {}"
                                                        , slice->side, SVO_VOLUME_SIDE_LIMIT)
                                            , slice);
        
        if (slice->side > 1)
        {
            auto size_in_parent = vcurvesize(slice->side / 2);
            assert(size_in_parent != 0);
            if (slice->parent_vcurve_begin % size_in_parent != 0)
                return svo_slice_sanity_error_t(fmt::format("parent_vcurve_begin is not aligned to the beginning of a node of this size ..."
                                                            " parent_vcurve_begin: {}, side: {}, size: {}, size in parent: {}"
                                                            , slice->parent_vcurve_begin, slice->side, vcurvesize(slice->side), vcurvesize(slice->side / 2))
                                                , slice);
        }

    }


    ///data/bounds check
    {
        vcurvesize_t size = vcurvesize(slice->side);
        vcurvesize_t last_vcurve = size;
        for (const auto& voxel_vcurve : pos_data)
        {
            if (!(voxel_vcurve < size))
                return svo_slice_sanity_error_t(fmt::format("data contains voxel that is out of bounds of the slice ..."
                                                            " side: {}, size: {}, vcurve: {}"
                                                            , slice->side, size, voxel_vcurve)
                                                , slice);



            if (last_vcurve == size) {
                ///pass
            } else {
                if (voxel_vcurve == last_vcurve)
                    return svo_slice_sanity_error_t(fmt::format("data contains a duplicate voxel ..."
                                                                " voxel_vcurve: {}"
                                                                , voxel_vcurve)
                                                    , slice);
                if (!(voxel_vcurve > last_vcurve))
                    return svo_slice_sanity_error_t(fmt::format("data contains voxels that are out of order ..."
                                                                " side: {}, size: {}, vcurve: {}, last vcurve: {}"
                                                                , slice->side, size, voxel_vcurve, last_vcurve)
                                                    , slice);
            }

            last_vcurve = voxel_vcurve;
        }

    }




    ///slice => children sanity
    {
        if (!slice->children)
            return svo_slice_sanity_error_t("slice->children is invalid pointer", slice);

        const auto& children = *slice->children;

        ///child space bounds check
        {
            vcurvesize_t size = vcurvesize(slice->side);
            vcurve_t last_parent_vcurve_end = 0;


            for (std::size_t child_index = 0; child_index < slice->children->size(); ++child_index)
            {
                const svo_slice_t* child = (*slice->children)[child_index];

                vcurvesize_t child_size = vcurvesize(child->side);
                vcurvesize_t child_size_in_parent = vcurvesize(child->side / 2);
                vcurvesize_t child_parent_vcurve_end = child->parent_vcurve_begin + child_size_in_parent;

                //std::cout << "child_index: " << child_index << std::endl;
                //std::cout << "child->parent_vcurve_begin: " << child->parent_vcurve_begin << std::endl;
                //std::cout << "child_parent_vcurve_end: " << child_parent_vcurve_end << std::endl;


                if (!child->parent_slice)
                    return svo_slice_sanity_error_t("child->parent_slice is invalid pointer", slice, child);

                if (child->parent_slice != slice)
                    return svo_slice_sanity_error_t("child->parent_slice is not pointing the slice", slice, child, child->parent_slice);




                if (child->side > slice->side * 2)
                    return svo_slice_sanity_error_t(fmt::format("child takes up more space than the parent ..."
                                                                " side: {}, size: {}, child->side: {}, child size (double res): {}"
                                                                ", child size (in parent space): {}"
                                                                , slice->side, size, child->side, child_size, child_size_in_parent)
                                                    , slice, child);

                if (child->side % 2 != 0)
                    return svo_slice_sanity_error_t(fmt::format("child cube side is not divisible by 2; yet it takes up (side/2) space in the parent ..."
                                                                " side: {}, size: {}, child->side: {}, child size: {}"
                                                                ", child size (in parent space): {}"
                                                                , slice->side, size, child->side, child_size, child_size_in_parent)
                                                    , slice, child);

                if (child->parent_vcurve_begin < last_parent_vcurve_end)
                    return svo_slice_sanity_error_t(fmt::format("child overlaps previous child's bounds ..."
                                                                " child->parent_vcurve_begin: {}, child_parent_vcurve_end: {}"
                                                                ", last_parent_vcurve_end: {}, size: {}"
                                                                ", child size (double res): {}, child size (parent res): {}, child_index: {}"
                                                                , child->parent_vcurve_begin, child_parent_vcurve_end
                                                                , last_parent_vcurve_end, size
                                                                , child_size, child_size_in_parent, child_index)
                                                    , slice, child);
                


                if (!(child_parent_vcurve_end <= size))
                    return svo_slice_sanity_error_t(fmt::format("child overflows the bounds of the parent ..."
                                                                " slice size: {}, child_parent_vcurve_end: {}"
                                                                , size, child_parent_vcurve_end)
                                                    , slice, child);


                last_parent_vcurve_end = child_parent_vcurve_end;
            }
        }
    }

    return svo_slice_sanity_error_t();
}

#if 0
svo_slice_sanity_error_t svo_slice_sanity_check_parent_error(const svo_slice_t* slice, int recurse)
{
    if (!slice)
        return svo_slice_sanity_error_t("slice is invalid pointer", slice);
    if (!slice->parent_slice)
        return svo_slice_sanity_error_t();

    return svo_slice_sanity_check_error(slice->parent_slice, recurse);
}

svo_slice_sanity_error_t svo_slice_sanity_check_children_error(const svo_slice_t* slice, int recurse)
{
    if (!slice)
        return svo_slice_sanity_error_t("slice is invalid pointer", slice);
    if (!slice->children)
        return svo_slice_sanity_error_t();

    for (const svo_slice_t* child : *slice->children)
    {
        auto error = svo_slice_sanity_check_error(child, recurse);
        if (error)
            return error;
    }

    return svo_slice_sanity_error_t();
}


svo_slice_sanity_error_t svo_slice_sanity_all(const svo_slice_t* slice)
{
    if (auto error = svo_slice_sanity_minimal(slice))
        return error;
    if (auto error = svo_slice_sanity_check_parent_error(slice))
        return error;
    if (auto error = svo_slice_sanity_check_children_error(slice))
        return error;
    if (auto error = svo_slice_sanity_pos_data(slice))
        return error;
    if (auto error = svo_slice_sanity_channel_data(slice, 1))
        return error;





    return svo_slice_sanity_error_t();
}

#endif



svo_slice_sanity_error_t svo_slice_sanity(
      const svo_slice_t* slice
    , svo_sanity_type_t sanity_type
    , int recurse
    , bool parent_recurse)
{
    if (sanity_type)
        if (auto error = svo_slice_sanity_minimal(slice, sanity_type))
            return error;

    const auto& children = *slice->children;

    if (sanity_type & svo_sanity_type_t::pos_data)
        for (const auto* child_slice : children)
            if (auto error = svo_slice_sanity_pos_data_to_parent(child_slice))
                return error;
    if (sanity_type & svo_sanity_type_t::channel_data)
        for (const auto* child_slice : children)
            if (auto error = svo_slice_sanity_channel_data_to_parent(child_slice))
                return error;

    if (sanity_type & svo_sanity_type_t::parent && slice->parent_slice && parent_recurse)
        if (auto error = svo_slice_sanity(slice->parent_slice, sanity_type, 0/*recurse*/, false/*parent_recurse*/))
            return error;

    if (sanity_type & svo_sanity_type_t::children && recurse > 0)
    {
        for (const auto* child_slice : children)
        {
            if (auto error = svo_slice_sanity(child_slice, sanity_type, recurse - 1))
                return error;
        }
    }
    return svo_slice_sanity_error_t();
}






} // namespace svo
