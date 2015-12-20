
#include "landscapes/debug_macro.h"
#include "landscapes/cpputils.hpp"
#include "landscapes/svo_tree.hpp"
#include "landscapes/svo_tree.block_mgmt.hpp"

#include "landscapes/svo_tree.sanity.hpp"

#include "bprinter/table_printer.h"

#include "pempek_assert.h"
#include <iostream>
#include <deque>
#include <algorithm>

namespace svo{


slice_inserter_t::slice_inserter_t(svo_tree_t* tree, svo_block_t* block)
    : tree(tree), block(block), slice(nullptr), parent_block(nullptr)
{
    assert(block);
    assert(block->slice);
    assert(block->tree == tree);
    assert(!(block->trunk));
    assert(block->parent_block);
    assert(block->parent_block->trunk);
    
    this->slice = block->slice;
    this->parent_block = block->parent_block;
    
    DEBUG {
        if (auto error = svo_block_sanity_check(block))
        {
            std::cerr << error << std::endl;
            assert(false && "sanity fail");
        }
        if (parent_block)
        {
            if (auto error = svo_block_sanity_check(parent_block))
            {
                std::cerr << error << std::endl;
                assert(false && "sanity fail");
            }
        }
        if (auto error = svo_slice_sanity(slice))
        {
            std::cerr << error << std::endl;
            assert(false && "sanity fail");
        }
    }




    assert(slice->pos_data);
    assert(slice->buffers);
    PPK_ASSERT (block->side * 2 == slice->side || (block->side == 0 && slice->side == 1)
                , "block->side * 2: %u, slice->side: %u", block->side*2, slice->side);
    
    assert(slice->children);
    auto& children = *slice->children;
    
    if (children.size() == 0) {
        out_datas.resize(1);
    } else {
        out_datas.resize(children.size());
    }
    
    out_uc_root_offsets.resize(out_datas.size(), offset_t(-1));
    out_data_root_levels.resize(out_datas.size(), std::size_t(-1));
}

svo_error_t slice_inserter_t::execute()
{
    std::size_t total_parent_leafs0 = this->block->parent_block->leaf_count;
    std::size_t total_block_leafs0 = this->block->leaf_count;
    std::size_t total_leafs0 = total_parent_leafs0 + total_block_leafs0;
    //std::size_t duplicate_voxels = 1;
    //total_leafs0 -= duplicate_voxels;
    assert(total_leafs0 > 0);
    
    classify_child_descriptors();
    allocate_dst_blocks();
    
    cd_goffsets_t uc_cd_goffsets(uc_out_data.size(), invalid_goffset);
    insert_unclassified_child_descriptors(uc_cd_goffsets);
    
    ///remove the block from the parent block
    {
        auto& child_blocks = *parent_block->child_blocks;
        
        child_blocks.erase(std::remove(child_blocks.begin(),child_blocks.end(), block), child_blocks.end());
    }
    
    
    for (std::size_t classification = 0; classification < out_datas.size(); classification++){
        assert(classification < dst_blocks.size());
        assert(classification < out_datas.size());
        svo_block_t* dst_block = dst_blocks[classification];
        out_data_t& out_data = out_datas[classification];
    
        
        cd_indices_t cd_parent_indices(out_data.size(), std::size_t(-1));
        calculate_parent_indices(cd_parent_indices, out_data);
        std::vector<std::size_t> cd_reverse_boffsets(out_data.size(), std::size_t(-1));
        std::vector<bool> cd_req_far_ptrs(out_data.size(), false);
        calculate_reverse_offsets(cd_reverse_boffsets, cd_req_far_ptrs, cd_parent_indices, out_data, dst_block->trunk);


        std::vector<goffset_t> cd_goffsets(out_data.size(), invalid_goffset);
        insert_classified_child_descriptors(cd_goffsets, dst_block, classification
                                            , uc_cd_goffsets, cd_req_far_ptrs, cd_parent_indices, out_data);
        
        DEBUG {
            std::vector<std::string> issues;
            if (!dst_block->check_parent_root_cd(issues))
            {
                std::cout << issues[0] << std::endl;
                assert( false );
            }
            
        }
    }

    //update_root_shadows();

    /*
    auto check_sanity = [this, total_voxels0]()
    {
        
        std::size_t expected_total_voxels1 = total_voxels0 + this->slice->data->size();
        
        std::size_t total_voxels1 = 0;
        
        total_voxels1 = this->block->parent_block->voxel_count;
        std::size_t duplicate_voxels1 = 0;
        
        for (svo_block_t* dst_block : dst_blocks){
            total_voxels1 += dst_block->voxel_count;
            
            duplicate_voxels1 += svo_get_cd_valid_count(svo_cget_cd(this->tree->address_space, dst_block->root_shadow_cd_goffset));
        }
        
        assert(total_voxels1 > 0);
        assert(total_voxels1 > duplicate_voxels1);
        total_voxels1 -= duplicate_voxels1;
        
        PPK_ASSERT( total_voxels1 == expected_total_voxels1
                    , "total_voxels0: %i, expected_total_voxels1: %i, total_voxels1: %i"
                    , total_voxels0, expected_total_voxels1, total_voxels1 );
    };
    */
    
    DEBUG {
        
        SCAFFOLDING {
            /*
            pprint_out_data(std::cout, "uc_out_data", uc_out_data);
            pprint_block(std::cout, "parent_block", block->parent_block);
            std::cout << "block->parent_block->root_valid_bit: " << block->parent_block->root_valid_bit
                      << ", block->parent_block->root_leaf_bit" << block->parent_block->root_leaf_bit
                      << std::endl;
            */
        }
        
        //check_sanity();
        
        /*
        if (auto error = svo_block_sanity_check(block))
        {
            std::cerr << error << std::endl;
            assert(false && "sanity fail");
        }
         */
        
        if (block->parent_block)
        {
            if (auto error = svo_block_sanity_check(block->parent_block))
            {
                std::cerr << error << std::endl;
                assert(false && "sanity fail");
            }
        }
        
        //pprint_block(std::cout, "parent_block", parent_block);
        
        for (svo_block_t* dst_block : dst_blocks){
            
            assert(dst_block->parent_block == block->parent_block);
            assert(parent_block->has_child_block(dst_block));
            
            
            assert(parent_block->is_valid_cd_goffset(dst_block->parent_root_cd_goffset));
            
            const auto* parent_root_cd = svo_cget_cd(tree->address_space, dst_block->parent_root_cd_goffset);
            goffset_t parent_root_children_goffset = svo_get_child_ptr_goffset(tree->address_space, dst_block->parent_root_cd_goffset, parent_root_cd);
            
            /*
            std::cout << "dst_block->parent_root_cd_goffset: " << dst_block->parent_root_cd_goffset << std::endl;
            std::cout << "out_uc_root_offsets: " << out_uc_root_offsets << std::endl;
            std::cout << "uc_cd_goffsets: " << uc_cd_goffsets << std::endl;
            std::cout << "dst_block->root_children_goffset(): " << dst_block->root_children_goffset() << std::endl;
            std::cout << "parent_root_children_goffset: " << parent_root_children_goffset << std::endl;
            */
            
            std::vector<std::string> issues;
            if (!dst_block->check_parent_root_cd(issues))
            {
                std::cout << issues[0] << std::endl;
                assert( false );
            }
            
            if (auto error = svo_block_sanity_check(dst_block))
            {
                std::cerr << error << std::endl;
                assert(false && "sanity fail");
            }
        }
        
        
        
        
        
        
        
    }
    

    return svo_error_t::OK;
}



inline std::size_t slice_inserter_t::svo_classify_voxel(std::size_t level, vcurve_t vcurve)
{
    assert(slice);
    assert(slice->children);
    
    assert(level < block->height);
    
    std::size_t bottom_level = block->height;
    
    vside_t projected_voxel_side = 1 << (bottom_level - level);
    vcurve_t projected_voxel_vcurve_begin = vcurve << (3*(bottom_level - level));
    vcurve_t projected_voxel_vcurve_size = vcurvesize(projected_voxel_side);
    vcurvesize_t projected_voxel_vcurve_end = projected_voxel_vcurve_begin + projected_voxel_vcurve_size;
    
    const auto& children = *slice->children;
    
    /*
    SCAFFOLDING {
        std::cout << "svo_classify_voxel: " << ", vcurve: " << vcurve
                  << ", level: " << level
                  << ", bottom_level: " << bottom_level
                  << ", projected_voxel_vcurve_begin: " << projected_voxel_vcurve_begin
                  << ", projected_voxel_side: " << projected_voxel_side
                  << ", projected_voxel_vcurve_size: " << projected_voxel_vcurve_size
                  << ", projected_voxel_vcurve_end: " << projected_voxel_vcurve_end
                  << std::endl;
                  
        const svo_slice_t* child_slice0 = children[0];
        auto child_slice_parent_vcurve_begin = child_slice0->parent_vcurve_begin;
        auto child_slice_parent_vcurve_end = child_slice_parent_vcurve_begin + vcurvesize(child_slice0->side / 2);
        std::cout << "child_slice_parent_vcurve_begin: " << child_slice_parent_vcurve_begin
                  << ", child_slice0->side: " << child_slice0->side
                  << ", child_slice_parent_vcurve_end: " << child_slice_parent_vcurve_end
                  << std::endl;
    }
    */
    
    if (children.size() == 0)
    {
        if (level == 0)
            return std::size_t(-1);
        return 0;
    }
    
    for (std::size_t classification = 0; classification < children.size(); ++classification)
    {
        const svo_slice_t* child_slice = children[classification];
        
        auto child_slice_parent_vcurve_begin = child_slice->parent_vcurve_begin;
        auto child_slice_parent_vcurve_end = child_slice_parent_vcurve_begin + vcurvesize(child_slice->side / 2);
        if (projected_voxel_vcurve_begin >= child_slice->parent_vcurve_begin
            && projected_voxel_vcurve_end <= child_slice_parent_vcurve_end)
        {
            ///if this is the root voxel of this classification, remain unclassified
            if (projected_voxel_vcurve_begin == child_slice->parent_vcurve_begin
                && projected_voxel_vcurve_end == child_slice_parent_vcurve_end)
                return std::size_t(-1);
            
            return classification;
        }
    }
    
    return std::size_t(-1);
}

void slice_inserter_t::classify_child_descriptors()
{

    const auto& in_pos_data = *slice->pos_data;
    const auto& children = *slice->children;
    std::size_t in_data_index = 0;




    auto block_copyinsert_slice_data = [this, &in_pos_data, &in_data_index](
                                            goffset_t pcd_goffset
                                          , goffset_t cd_goffset
                                          , ccurve_t voxel_ccurve
                                          , std::tuple<vcurve_t, std::size_t, std::size_t, std::size_t> metadata)
    {
        assert(pcd_goffset != 0);
        assert(cd_goffset != 0);

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
            classification = svo_classify_voxel(level, level_vcurve);
        }
    
    
        assert(cd_goffset == invalid_goffset || block->is_valid_cd_goffset(cd_goffset));
        
        bool is_cd = cd_goffset != invalid_goffset;
        bool is_bottom_leaf = level == block->height - 1;
        bool is_leaf = cd_goffset == invalid_goffset
                    || svo_get_cd_nonleaf_count(svo_cget_cd(tree->address_space, cd_goffset)) == 0;

        /*
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
            << "  is_cd: " << (is_cd ? "true" : "false")
            << "  is_leaf: " << (is_leaf ? "true" : "false")
            << ", is_bottom_leaf: " << (is_bottom_leaf ? "true" : "false")
            << std::endl;
        */
        /*
        if (cd_goffset != invalid_goffset)
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
        */
        
        assert (classification == std::size_t(-1) || classification < out_datas.size());


        std::size_t out_data_index = std::size_t(-1);

        
        ///bottom leaf implies leaf
        assert(!is_bottom_leaf || is_leaf);
        
        ///no CD, implies leaf
        assert(is_cd || is_leaf);
        
        
        ///we output either to the unclassified out data, or to the classified out data.
        auto& out_data = (classification != std::size_t(-1)) ? out_datas[classification] : uc_out_data;
        //auto& out_data_channels = (classification != std::size_t(-1)) ? out_datas_all_channels[classification] : uc_out_data_channels;
            
        ///if this is a child descriptor already in the block
        ///spoiler: then copy it into the out data.
        if (cd_goffset != invalid_goffset)
        {
            assert(is_cd);
            
            assert( level < block->height);
            assert(out_data_index == std::size_t(-1));
            assert(block->is_in_block(cd_goffset, sizeof(child_descriptor_t)));
            
            ///get the CD
            const auto* cd = svo_cget_cd(tree->address_space, cd_goffset);
    
            ///this is gonna store the valid/leaf masks in @c out_data.
            child_descriptor_t new_cd; svo_init_cd(&new_cd);
            
            ///copy over the masks
            svo_set_valid_mask(&new_cd, svo_get_valid_mask(cd));
            svo_set_leaf_mask(&new_cd, svo_get_leaf_mask(cd));

                
            ///classification is not invalid flag => classification is sane
            assert(classification == std::size_t(-1) || classification < out_datas.size());

            ///store the index that this new out data will have.
            out_data_index = out_data.size();
            
            ///and push it into the data.
            out_data.push_back( std::make_tuple(level, level_vcurve, new_cd, 0 /* child offset */) );


            //for (std::size_t attr_index = 0; attr_index < out_data_channels.size(); ++attr_index)
            //{
            //    auto& out_channel_data = out_data_channels[attr_index];

            //    out_channel_data.write(svo_get_attr_data_base_ptr(tree->address_space, cd_goffset, cd, attr_index), svo_get_cd_valid_count(cd) );
            //}
        }

        ///if we potentially need to add the new stuff
        if (is_bottom_leaf)
        {
            ///this should be a leaf voxel, and thus have no cd in the old block
            assert(is_leaf);
            
            /*
            std::cout << "in_data_index: " << in_data_index
                    << ", in_data.size(): " << in_data.size()
                    << ", level_vcurve: " << level_vcurve;

            if (in_data_index < in_data.size())
                std::cout << ", std::get<0>(in_data[in_data_index])/8: " << std::get<0>(in_data[in_data_index])/8;
            
            std::cout << std::endl;
             */

            ///if we still have data to use, and the data is a child of the current voxel
            if (in_data_index < in_pos_data.size() && (in_pos_data[in_data_index])/8 == level_vcurve)
            {
                if (out_data_index == std::size_t(-1)){
                    child_descriptor_t new_cd; svo_init_cd(&new_cd);
                        
                    out_data_index = out_data.size();
                    out_data.push_back( std::make_tuple(level, level_vcurve, new_cd, 0 ) );
                }
                assert( out_data_index != std::size_t(-1) );
                
                auto* cd = &std::get<2>(out_data[out_data_index]);
                
                child_mask_t valid_mask = svo_get_valid_mask(cd);
                child_mask_t leaf_mask = svo_get_leaf_mask(cd);
                
                assert(valid_mask == 0);
                assert(leaf_mask == 0);

                while (in_data_index < in_pos_data.size() && (in_pos_data[in_data_index])/8 == level_vcurve)
                {
                    /*
                    std::cout << "in_data_index: " << in_data_index
                            << ", in_data.size(): " << in_data.size()
                            << ", level_vcurve: " << level_vcurve;
                    
                    if (in_data_index < in_data.size())
                        std::cout << ", std::get<0>(in_data[in_data_index])/8: " << std::get<0>(in_data[in_data_index])/8;

                    std::cout << std::endl;
                    */
                    
                    vcurve_t next_new_voxel_vcurve = in_pos_data[in_data_index++];

                    ccurve_t new_voxel_ccurve = next_new_voxel_vcurve % 8;

                    valid_mask |= (1 << new_voxel_ccurve);
                    leaf_mask |= (1 << new_voxel_ccurve);
                }

                svo_set_valid_mask(cd, valid_mask);
                svo_set_leaf_mask(cd, valid_mask);
                
                ///classification is not invalid flag => classification is sane
                assert(classification == std::size_t(-1) || classification < out_datas.size());
                


                
            }
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
        
        bool has_output_cd = out_data_index != std::size_t(-1);
        
        ///now update the parent
        ///if there is a parent
        if (parent_out_data_index != std::size_t(-1))
        { 
            ///parent is classified => assert child is classified the same
            assert(parent_classification == std::size_t(-1) || parent_classification == classification);
            ///parent is classified => classification is valid classification
            assert(parent_classification == std::size_t(-1) || parent_classification < out_datas.size());


            auto& parent_out_data = (parent_classification != std::size_t(-1)) ? out_datas[parent_classification]  : uc_out_data;
            ///parent output index must be inside the data
            assert(parent_out_data_index < parent_out_data.size());

            std::size_t& new_pcd_level = std::get<0>(parent_out_data[parent_out_data_index]);
            child_descriptor_t& new_pcd = std::get<2>(parent_out_data[parent_out_data_index]);
            offset_t& new_pcd_child_offset = std::get<3>(parent_out_data[parent_out_data_index]);

            ///if this output a CD, and is the first child of the parent (the parent has an invalid child offset of 0)
            if (out_data_index != std::size_t(-1) && new_pcd_child_offset == 0) {
                
                /*
                std::cout << "out_data_index: " << out_data_index
                    << ", classification: " << classification
                    << ", parent_classification: " << parent_classification
                    << ", out_data_index - parent_out_data_index: " << (out_data_index - parent_out_data_index)
                    << std::endl;
                */
                
                ///if the parent is classified (and therefore this CD is classified)
                if (parent_classification != std::size_t(-1)) {
                    assert( classification != std::size_t(-1) );
                    
                    ///then set the child offset to the distance between the parent and the first child.
                    new_pcd_child_offset = (out_data_index - parent_out_data_index);
                }
                ///if both parent CD and current voxel are not classified
                else if (classification == std::size_t(-1)){
                    assert( parent_classification == std::size_t(-1) );
                    
                    ///then set the child offset to the distance between the parent and the first child.
                    new_pcd_child_offset = (out_data_index - parent_out_data_index);
                } 
                
                ///if the parent is not classified, and the current CD is.
                else {
                    ///make sure this condition branch is true
                    assert(parent_classification == std::size_t(-1) && classification != std::size_t(-1));
                    assert(classification < out_uc_root_offsets.size());
                    
                    if (out_uc_root_offsets[classification] == offset_t(-1))
                    {
                        out_uc_root_offsets[classification] = parent_out_data_index;
                        out_data_root_levels[classification] = level;
                    }
                    
                }
            }

            bool new_parent_valid_bit = svo_get_valid_bit(&new_pcd, voxel_ccurve);
            bool new_parent_leaf_bit = svo_get_leaf_bit(&new_pcd, voxel_ccurve);
            
            
            assert(new_parent_valid_bit);
            assert(new_parent_leaf_bit == !is_cd);
            assert(new_pcd_level == level - 1);

            ///if there was an output CD
            if (has_output_cd)
                ///make this voxel a non-leaf in the parent
                svo_set_leaf_bit(&new_pcd, voxel_ccurve, false);

        }

        return std::make_tuple(level_vcurve, level+1, classification, out_data_index);
    };


    auto initial_data = std::make_tuple(  0/*level_vcurve*/
                                        , 0/*level*/
                                        , std::size_t(-1)/*parent classification*/
                                        , std::size_t(-1)/*parent out data index*/);
    

    z_preorder_traverse_block_cds(  tree->address_space
                                    , block
                                    , initial_data, block_copyinsert_slice_data);
    


    //std::cout << "size0: " << (block->cd_end - block->cd_start)/8 << std::endl;

    for (auto& out_data : out_datas)
    {
        //std::cout << "  out_data.size(): " << out_data.size() << std::endl;
    }
    //std::cout << "  uc_out_data.size(): " << uc_out_data.size() << std::endl;



    SCAFFOLDING {
        
        /*
        pprint_block(std::cout, "block", block);
        
        std::cout << "block->leaf_count: " << block->leaf_count
                  << ", block->cd_count: " << block->cd_count
                  << ", block->root_valid_bit: " << (block->root_valid_bit ? "true" : "false")
                  << ", block->root_leaf_bit: " << (block->root_leaf_bit ? "true" : "false")
                  << std::endl;
        std::cout << "in_data_index: " << in_data_index << std::endl;
        std::cout << "in_data.size(): " << in_data.size() << std::endl;
         */
    }
    
    
    assert(in_data_index == in_pos_data.size());


    for (auto& out_data : out_datas)
    {
        for (std::size_t out_data_index = 0; out_data_index < out_data.size(); ++out_data_index)
        {
            auto& voxel = out_data[out_data_index];
            
            
            const child_descriptor_t& cd = std::get<2>(voxel);
            const offset_t& child_0_offset_in_cds = std::get<3>(voxel);
            
            assert(child_0_offset_in_cds || out_data_index == 0 || svo_get_cd_nonleaf_count(&cd) == 0);
            assert(out_data_index + child_0_offset_in_cds < out_data.size());

        }
        
        
    }
}


void slice_inserter_t::allocate_dst_blocks()
{
    assert(out_datas.size() > 0);
    auto& children = *slice->children;
    
    if (out_datas.size() == 1)
    {
        dst_blocks.push_back(block);

        if (children.size() > 0)
            block->slice = children[0];
        else
            block->slice = 0;

        return;
    }


    assert(out_datas.size() == children.size());
    
    ret_blocks.push_back(block);

    for (std::size_t classification = 0; classification < out_datas.size(); ++classification)
    {
        auto* new_block = tree->allocate_block(block->size());
        assert(new_block);
        dst_blocks.push_back(new_block);

        new_block->slice = children[classification];
        new_block->side = new_block->slice->side / 2;
    }
}

void slice_inserter_t::calculate_parent_indices(cd_indices_t& cd_parent_indices, const out_data_t& out_data, bool allow_non_leafs)
{
    assert(cd_parent_indices.size() == out_data.size());
    ///for each CD, we will store an index to the parent CD, or invalid flag if there is none.



    ///find the parent index for each CD
    for (std::size_t out_data_index = 0; out_data_index < out_data.size(); ++out_data_index)
    {
        assert(out_data_index < cd_parent_indices.size());
        std::size_t parent_data_index = cd_parent_indices[out_data_index];
        const auto& voxel = out_data[out_data_index];

        const child_descriptor_t& cd = std::get<2>(voxel);
        const offset_t& child_0_offset_in_cds = std::get<3>(voxel);

        
        ///it is either a non-leaf voxel or a dummy root
        assert(child_0_offset_in_cds || svo_get_cd_nonleaf_count(&cd) == 0 || allow_non_leafs);
        assert(out_data_index + child_0_offset_in_cds < out_data.size());


        ///if this node has child CDs (non-leaf children).
        if (child_0_offset_in_cds)
        {
            assert(svo_get_cd_nonleaf_count(&cd) > 0);

            ///fill in cd_parent_indices for all the children of this node.
            for (std::size_t i = 0; i < svo_get_cd_nonleaf_count(&cd); ++i)
            {
                auto child_i_offset_in_cds = child_0_offset_in_cds + i;
                auto child_data_index = out_data_index + child_i_offset_in_cds;
                assert(child_data_index < cd_parent_indices.size());
            
                ///get a reference to the parent index so we can set it.
                std::size_t& child_parent_out_data = cd_parent_indices[child_data_index];
                
                
                ///make sure no other node claimed to be the parent of this child.
                assert( child_parent_out_data == std::size_t(-1) );


                child_parent_out_data = out_data_index;
            }
        } else {
            //pprint_out_data(std::cout, "out_data", out_data);
            
            assert(svo_get_cd_nonleaf_count(&cd) == 0 || allow_non_leafs);
        }
    }
}

void slice_inserter_t::calculate_reverse_offsets(
      std::vector<std::size_t>& cd_reverse_boffsets
    , std::vector<bool>& cd_req_far_ptrs
    , const cd_indices_t& cd_parent_indices
    , const out_data_t& out_data
    , bool trunk)
{
    assert(cd_reverse_boffsets.size() == out_data.size());
    assert(cd_req_far_ptrs.size() == out_data.size());
    assert(cd_parent_indices.size() == out_data.size());
    
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

            if (trunk)
                cd_section_byte_size = 8 * sizeof(child_descriptor_t);

            std::size_t section_reverse_boffset0 = current_reverse_boffset + cd_section_byte_size;



            std::size_t far_ptrs = 0;

            ///calculte the number of far pointers in this section
            {

                ///iterate the children in reverse
                for (std::size_t sibling_data_index : ireversed(sibling_section_indices) )
                {
                    assert( sibling_data_index < out_data.size() );

                    const auto& sibling_voxel = out_data[ sibling_data_index ];
                    const child_descriptor_t& cd = std::get<2>(sibling_voxel);

                    ///relative offset into @c out_data of the children of this voxel.
                    const offset_t& child_0_offset_in_cds = std::get<3>(sibling_voxel);

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
                        cd_req_far_ptrs[sibling_data_index] = true;
                    }
                }
            }

            if (trunk) {
                far_ptrs = 8;

                for (std::size_t sibling_data_index : sibling_section_indices)
                    cd_req_far_ptrs[sibling_data_index] = true;
            }

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
                assert(sibling_data_index < cd_reverse_boffsets.size());
                cd_reverse_boffsets[sibling_data_index] = sibling_reverse_boffset;
                i++;
            }


            sibling_section_indices.clear();
        }

    }

}

void slice_inserter_t::insert_classified_child_descriptors (
      cd_goffsets_t& cd_goffsets
    , svo_block_t* dst_block
    , std::size_t classification
    , const cd_goffsets_t& uc_cd_goffsets
    , const std::vector<bool>& cd_req_far_ptrs
    , const cd_indices_t& cd_parent_indices
    , const out_data_t& out_data)
{
    assert(classification < out_datas.size());
    assert(uc_cd_goffsets.size() == uc_out_data.size());
    assert(cd_goffsets.size() == out_data.size());
    assert(cd_req_far_ptrs.size() == out_data.size());
    assert(cd_parent_indices.size() == out_data.size());


    auto* dst_block_slice = dst_block->slice;
    vside_t dst_block_side = ( dst_block_slice ? dst_block_slice->side / 2 : block->side * 2 );
    std::size_t dst_block_height = ilog2(dst_block_side);
    std::size_t original_height = block->height;
    std::size_t expected_height = original_height + 1;
    assert(dst_block_height <= expected_height);
    std::size_t height_delta = expected_height - dst_block_height;
    std::size_t dst_block_root_level = block->root_level + height_delta;

    dst_block->reset_cd_data();
    dst_block->slice = dst_block_slice;
    dst_block->side = dst_block_side;
    dst_block->height = dst_block_height;
    dst_block->root_level = dst_block_root_level;
    
    dst_block->root_valid_bit = true;
    dst_block->root_leaf_bit = true;
    dst_block->leaf_count = 1;
    dst_block->cd_count = 1;
    
    
    assert(!dst_block->has_root_children_goffset());
    assert(dst_block->parent_block == 0);
    assert(dst_block->parent_root_cd_goffset == invalid_goffset);
    
    dst_block->parent_block = parent_block;
    parent_block->child_blocks->push_back(dst_block);
    
    
    
    ///copy root/shadow root
    {
        
        auto* root_shadow_cd = svo_get_cd(tree->address_space, dst_block->root_shadow_cd_goffset);
        
        assert(classification < out_uc_root_offsets.size());
        
        std::size_t root_out_data_index = out_uc_root_offsets[classification];
        ///special case when there is only one voxel
        if (root_out_data_index == std::size_t(-1)) {
            root_out_data_index = 0;
        } else {
            
        }
        
        assert(root_out_data_index < uc_out_data.size());
        
        const auto& root_voxel = uc_out_data[root_out_data_index];
        const auto* root_cd0 = &std::get<2>(root_voxel);
        
        SCAFFOLDING {
            /*
            std::cout << "root_cd0" << *root_cd0 << std::endl;
            pprint_out_data(std::cout, "out_data", out_data);
            pprint_out_data(std::cout, "uc_out_data", uc_out_data);
             */
        }
        
        ///calculate parent_root_cd_goffset
        {
            dst_block->parent_root_cd_goffset = uc_cd_goffsets[root_out_data_index];
            assert(parent_block->is_valid_cd_goffset(dst_block->parent_root_cd_goffset));
        }
        
        ///copy the masks
        {
            dst_block->clear_cd_count(dst_block->root_shadow_cd_goffset);
            svo_set_valid_mask(root_shadow_cd, svo_get_valid_mask(root_cd0));
            svo_set_leaf_mask(root_shadow_cd, svo_get_leaf_mask(root_cd0));
            dst_block->add_cd_count(dst_block->root_shadow_cd_goffset);
        }
        
        
        //cd_goffsets[0] = dst_block->root_shadow_cd_goffset;

        ///root has children
        if (svo_get_cd_valid_count(root_shadow_cd) > 0) {
            dst_block->root_leaf_bit = false;
            dst_block->leaf_count -= 1;
        }
        
    }

    
    std::size_t inserted_cds = 0;
    
    ///append all the non-root CDs
    std::vector< std::size_t > sibling_section_indices;
    for (std::size_t out_data_index = 0; out_data_index < out_data.size(); ++out_data_index)
    {
        sibling_section_indices.push_back(out_data_index);
        
        ///get the parent of the current CD
        std::size_t parent_data_index = cd_parent_indices[out_data_index];
        
        ///just some sanity testing
        if(parent_data_index < out_data.size())
        {
            
            const auto& parent_voxel = out_data[parent_data_index];
            
            ///get the parent data CD
            const child_descriptor_t* pcd0 = &std::get<2>(parent_voxel);
            
            ///get the parent written out cd goffset
            goffset_t parent_cd_goffset = cd_goffsets[parent_data_index];
            
            ///parent should already be assigned a goffset
            assert(dst_block->is_valid_cd_goffset(parent_cd_goffset));
            
            ///get the parent written out cd
            const child_descriptor_t* pcd1 = svo_cget_cd(tree->address_space, parent_cd_goffset);
            assert(pcd1);
            
            ///these things should match.
            assert(svo_get_valid_mask(pcd1) == svo_get_valid_mask(pcd0));
            assert(svo_get_leaf_mask(pcd1) == svo_get_leaf_mask(pcd0));
        }
        
        
        
        
        
        bool should_flush_child_section = false;
        if (out_data_index + 1 == out_data.size() ) {
            should_flush_child_section = true;
        } else {
            assert( out_data_index + 1 < out_data.size() );
            //const auto& next_voxel = out_data[out_data_index + 1];


            assert( out_data_index + 1 < cd_parent_indices.size() );
            std::size_t next_voxel_parent_data_index = cd_parent_indices[out_data_index + 1];

            if (next_voxel_parent_data_index != parent_data_index)
                should_flush_child_section = true;
            else
                should_flush_child_section = false;
        }



        if (should_flush_child_section)
        {
            assert(sibling_section_indices.size() > 0);
            
            ///should have the same number of siblings as parent has children
            //assert( sibling_section_indices.size() == svo_get_cd_nonleaf_count(pcd0));
            
            ///append the child descriptors
            for (std::size_t sibling_data_index : sibling_section_indices)
            {
                ///sanity
                assert(cd_parent_indices[sibling_data_index] == parent_data_index);
                assert(cd_goffsets[sibling_data_index] == invalid_goffset);


                const auto& sibling_voxel = out_data[sibling_data_index];
                const auto* sibling_cd0 = &std::get<2>(sibling_voxel);
                
                ///actually write the CD to the block
                goffset_t cd_goffset = svo_append_cd(tree->address_space, dst_block, sibling_cd0);
                
                if (cd_goffset == invalid_goffset)
                    throw svo_block_full();
                
                dst_block->add_cd_count(cd_goffset);
                ++inserted_cds;
                
                assert(cd_goffset != svo_get_ph_goffset(cd_goffset));
                assert(dst_block->is_valid_cd_goffset(cd_goffset));
                
                
                cd_goffsets[sibling_data_index] = cd_goffset;
            }
            
            ///if this is a trunk, append some more child descriptors
            if (dst_block->trunk)
            {
                std::size_t remaining_dummy_cds = 8 - sibling_section_indices.size();
                for (std::size_t i = 0; i < remaining_dummy_cds; ++i)
                {
                    svo_append_dummy_cd(tree->address_space, dst_block);
                }
            }
            
            
            std::deque< std::size_t > far_ptr_sibling_indices;
            
            ///calculate far ptrs
            for (std::size_t i = 0; i < sibling_section_indices.size(); ++i)
            {
                std::size_t sibling_data_index = sibling_section_indices[i];
                
                if (cd_req_far_ptrs[sibling_data_index])
                {
                    far_ptr_sibling_indices.push_back(sibling_data_index);
                }
            }
            
            std::size_t far_ptrs = dst_block->trunk ? 8 : far_ptr_sibling_indices.size();
            
            
            std::size_t far_ptrs_bytes = far_ptrs*sizeof(goffset_t);
            ///far pointers take up 4 bytes, CDs take up 8 bytes, fit as many as we can but
            /// we might end up with an empty far ptr slot to keep things aligned with CDs.
            far_ptrs_bytes = iceil(far_ptrs_bytes, sizeof(child_descriptor_t));
            
            assert(far_ptrs_bytes >= far_ptrs*sizeof(goffset_t));
            assert(far_ptrs_bytes  % sizeof(child_descriptor_t) == 0);
            
            std::size_t far_ptr_cd_slots = far_ptrs_bytes / sizeof(child_descriptor_t);
            
            ///insert the far pointers in groups of two
            for (std::size_t i = 0; i < far_ptr_cd_slots; ++i)
            {
                ///insert a dummy cd for each group
                goffset_t base_far_ptr_goffset = svo_append_dummy_cd(tree->address_space, dst_block);

                
                
                assert(base_far_ptr_goffset != 0);
                
                if (base_far_ptr_goffset == invalid_goffset)
                    throw svo_block_full();
                assert(base_far_ptr_goffset != svo_get_ph_goffset(base_far_ptr_goffset));

                std::size_t slots = sizeof(child_descriptor_t) / sizeof(goffset_t);
                assert(slots == 2);
                
                ///set the two child ptrs
                for (std::size_t slot = 0; slot < slots && far_ptr_sibling_indices.size() > 0; ++slot)
                {
                    goffset_t far_ptr_goffset = base_far_ptr_goffset + slot*sizeof(goffset_t);
                    
                    std::size_t sibling_data_index = far_ptr_sibling_indices.front();
                    far_ptr_sibling_indices.pop_front();
                    
                    goffset_t sibling_cd_goffset = cd_goffsets[sibling_data_index];
                    assert(sibling_cd_goffset != 0);
                    assert(sibling_cd_goffset != invalid_goffset);
                    assert(sibling_cd_goffset != svo_get_ph_goffset(sibling_cd_goffset));
                    assert(dst_block->is_valid_cd_goffset(sibling_cd_goffset));
                    
                    auto* sibling_cd = svo_get_cd(tree->address_space, sibling_cd_goffset);
                    
                    assert( !svo_get_far(sibling_cd) );
                    assert( svo_get_child_ptr_offset4(sibling_cd) == 0);
                    
                    offset_t offset = far_ptr_goffset - sibling_cd_goffset;
                    assert(offset > 0);
                    ///with this scheme, the far pointer should never be more than 16 CD slots ahead of the CD
                    assert(offset < sizeof(child_descriptor_t)*16);
                    assert(offset % sizeof(goffset_t) == 0);
                    offset4_t offset4 = offset / sizeof(goffset_t);
                    assert(offset4 > 0);

                    svo_set_far(sibling_cd, true);
                    svo_set_child_ptr(sibling_cd, offset4);
                    
                    svo_set_goffset_via_fp(tree->address_space, sibling_cd_goffset, sibling_cd, invalid_goffset);
                    assert(svo_get_child_ptr_goffset(tree->address_space, sibling_cd_goffset, sibling_cd) == invalid_goffset);
                }
            }
            assert(far_ptr_sibling_indices.size() == 0);
            
            
            sibling_section_indices.clear();
        }
    }
    
    assert(sibling_section_indices.size() == 0);
    assert(inserted_cds + 1 == dst_block->cd_count);
    assert(inserted_cds == out_data.size());
    
    ///fill in child_ptr and far_ptr slots
    {

        for (std::size_t out_data_index = 0; out_data_index < out_data.size(); ++out_data_index)
        {
            
            const auto& voxel = out_data[out_data_index];
            vcurve_t voxel_vcurve = std::get<1>(voxel);
            ccurve_t ccurve = voxel_vcurve % 8;
            
            auto parent_out_data_index = cd_parent_indices[out_data_index];
            
            if (parent_out_data_index == std::size_t(-1))
                continue;
            
            if (!(parent_out_data_index < out_data.size()))
            {
                //pprint_out_data(std::cout, "out_data", out_data, &cd_parent_indices);
                //pprint_block(std::cout, "dst_block", dst_block);
            }
            assert(parent_out_data_index < out_data.size());
            
            goffset_t cd_goffset = cd_goffsets[out_data_index];
            goffset_t pcd_goffset = cd_goffsets[parent_out_data_index];
            
            assert(cd_goffset != 0 && cd_goffset != invalid_goffset);
            assert(pcd_goffset != 0 && pcd_goffset != invalid_goffset);
            assert(cd_goffset != svo_get_ph_goffset(cd_goffset));
            assert(pcd_goffset != svo_get_ph_goffset(pcd_goffset));
            assert(dst_block->is_valid_cd_goffset(pcd_goffset));
            assert(dst_block->is_valid_cd_goffset(cd_goffset));
            
            auto* pcd = svo_get_cd(tree->address_space, pcd_goffset);
            
            assert(pcd);
            
            assert( svo_get_valid_bit(pcd, ccurve) );
            assert( !svo_get_leaf_bit(pcd, ccurve) );
            
            assert(cd_req_far_ptrs[parent_out_data_index] == svo_get_far(pcd));
            
            ///if the parent CD has a far ptr, and that far ptr is not yet set to point to anything.
            if (svo_get_far(pcd) && svo_get_child_ptr_goffset(tree->address_space, pcd_goffset, pcd) == invalid_goffset)
            {
                assert(cd_req_far_ptrs[parent_out_data_index] == true);
                offset4_t far_ptr_offset4 = svo_get_child_ptr_offset4(pcd);
                offset_t far_ptr_offset = far_ptr_offset4*4;

                assert(far_ptr_offset < 16*sizeof(child_descriptor_t));
                
                svo_set_goffset_via_fp(tree->address_space, pcd_goffset, pcd, cd_goffset);
                assert(svo_get_child_ptr_goffset(tree->address_space, pcd_goffset, pcd) == cd_goffset);
            }
            ///else if the parent CD is not a far ptr and is unset. 
            else if (svo_get_child_ptr_offset4(pcd) == 0)
            {
                assert(svo_get_far(pcd) == false);
                assert(cd_req_far_ptrs[parent_out_data_index] == false);
                
                offset_t offset = cd_goffset - pcd_goffset;
                assert(offset % 4 == 0);
                offset4_t offset4 = offset / 4;
                assert(offset4 > 0);
                
                assert ((offset4 & SVO_CHILD_PTR_MASK) == offset4);
                
                svo_set_child_ptr(pcd, offset4);
                
                assert(svo_get_child_ptr_goffset(tree->address_space, pcd_goffset, pcd) == cd_goffset);

            } else {
                ///otherwise the pcd already be set
                
                ///it certainly should not point to itself
                assert( svo_get_child_ptr_offset4(pcd) != 0);
                
                goffset_t child_base_goffset = svo_get_child_ptr_goffset(tree->address_space, pcd_goffset, pcd);
                ///and it should not have a ptr to invalid
                assert(child_base_goffset != 0);
                assert(child_base_goffset != invalid_goffset);
                assert(child_base_goffset != svo_get_ph_goffset(child_base_goffset));
                assert(dst_block->is_valid_cd_goffset(child_base_goffset));
                
                
            }
        }
    }
    
    
    ///now fix up the root CD and root shadow CD
    {
        assert(dst_block->is_valid_cd_goffset(dst_block->root_shadow_cd_goffset));
        auto* root_shadow_cd = svo_get_cd(tree->address_space, dst_block->root_shadow_cd_goffset);
        
        assert( svo_get_child_ptr_offset4(root_shadow_cd) == 0 );
        if (svo_get_child_ptr_offset4(root_shadow_cd) == 0)
        {
            
            ///if there are no children CDs, then we need to make a dummy child, so that
            /// the block passes the has_root_children_goffset() test.
            if (svo_get_cd_nonleaf_count(root_shadow_cd) == 0)
            {
                assert(cd_goffsets.size() == 0);
                
                ///add a dummy CD so that dst_block->has_root_children_goffset() returns true.
                goffset_t dummy_children_goffset = svo_append_dummy_cd(tree->address_space, dst_block);
                if (dummy_children_goffset == invalid_goffset)
                    throw svo_block_full();
                
                assert(dummy_children_goffset != 0);
                assert(dummy_children_goffset != svo_get_ph_goffset(dummy_children_goffset));
                assert(dst_block->is_valid_cd_goffset(dummy_children_goffset));
                
                offset_t offset = dummy_children_goffset - dst_block->root_shadow_cd_goffset;
                assert(offset > 0);
                assert(offset % 4 == 0);

                offset4_t offset4 = offset / 4;
                
                svo_set_child_ptr(root_shadow_cd, offset4);
                assert(dst_block->has_root_children_goffset());
            
                assert(dst_block->root_children_goffset() == dummy_children_goffset);

            } else {
                ///this dst block has root children CDs
                
                
                assert(cd_goffsets.size() > 0);
                
                goffset_t root_children_goffset = cd_goffsets[0];
                assert(root_children_goffset != 0);
                assert(root_children_goffset != invalid_goffset);
                assert(root_children_goffset != svo_get_ph_goffset(root_children_goffset));
                assert(dst_block->is_valid_cd_goffset(root_children_goffset));
                
                offset_t offset = root_children_goffset - dst_block->root_shadow_cd_goffset;
                assert(offset < 2*sizeof(child_descriptor_t));
                assert(offset > 0);
                assert(offset % 4 == 0);
                offset4_t offset4 = offset / 4;
                
                
                svo_set_child_ptr(root_shadow_cd, offset4);
                assert(dst_block->has_root_children_goffset());
            
                assert(dst_block->root_children_goffset() == root_children_goffset);
            }
        }
        
        ///now update the parent root cd
        {
            assert(dst_block->parent_block->is_valid_cd_goffset(dst_block->parent_root_cd_goffset));
            auto* parent_root_cd = svo_get_cd(tree->address_space, dst_block->parent_root_cd_goffset);
            assert(svo_get_far(parent_root_cd));
            svo_set_goffset_via_fp(tree->address_space, dst_block->parent_root_cd_goffset, parent_root_cd, dst_block->root_children_goffset());
            
            goffset_t parent_root_children_goffset = svo_get_child_ptr_goffset(tree->address_space, dst_block->parent_root_cd_goffset, parent_root_cd);
            assert(parent_root_children_goffset == dst_block->root_children_goffset());

            //std::cout << "dst_block->root_children_goffset(): " << dst_block->root_children_goffset() << std::endl;
            //std::cout << "parent_root_children_goffset: " << parent_root_children_goffset << std::endl;
        }
        
        DEBUG {
            
            
            std::vector<std::string> issues;
            if (!dst_block->check_parent_root_cd(issues))
            {
                std::cout << issues[0] << std::endl;
                assert( false );
            }
            
        }
    }
    
    
    DEBUG {
        std::vector<std::string> issues;
        if (!dst_block->check_parent_root_cd(issues))
        {
            std::cout << issues[0] << std::endl;
            assert( false );
        }
        
    }

    ///some debug printing
    {
        //pprint_out_data(std::cout, "out_data", out_data);
        //pprint_block(std::cout, "dst_block", dst_block);
    }
    
    /*
     * height:side
     * 1:1
     * 2:2
     * 3:4
     * 4:8
     * 5:16
     * 6:32
     * 7:64
     */
    
    
    assert(dst_block->has_root_children_goffset());
    
    
    /*
    std::size_t voxel_count = 0;
    ///count the voxels
    for (std::size_t out_data_index = 0; out_data_index < out_data.size(); ++out_data_index)
    {
        const auto& voxel = out_data[out_data_index];
        const auto* cd0 = &std::get<2>(voxel);
        
        voxel_count += svo_get_cd_leaf_count(cd0);
        
        assert(dst_block->is_valid_cd_goffset(cd_goffsets[out_data_index]));
    }
    */
    
    DEBUG {
        std::size_t far_ptrs = 0;
        std::set< goffset_t > found_goffsets;
        std::set< goffset_t > found_far_ptrs;
        auto visitor = [&found_goffsets, &found_far_ptrs, &far_ptrs, this](goffset_t pcd_goffset, goffset_t cd_goffset, ccurve_t voxel_ccurve, int metadata)
        {
            if (pcd_goffset != invalid_goffset)
            {
                assert(found_goffsets.count(pcd_goffset) == 1);
            }
            if (cd_goffset != invalid_goffset)
            {
                assert(found_goffsets.count(cd_goffset) == 0);
                found_goffsets.insert(cd_goffset);
                
                const auto* cd = svo_cget_cd(tree->address_space, cd_goffset);
                
                if (svo_get_far(cd))
                {
                    ++far_ptrs;
                    
                    offset4_t offset4 = svo_get_child_ptr_offset4(cd);
                    goffset_t fp_goffset = cd_goffset + offset4*4;
                    
                    assert(found_far_ptrs.count(fp_goffset) == 0);
                    found_far_ptrs.insert(fp_goffset);
                }
                
            }
            
            return 0;
        };
        
        z_preorder_traverse_block_cds(tree->address_space, dst_block, 0, visitor);
        
        
        std::set< goffset_t > expected_goffsets(cd_goffsets.begin(), cd_goffsets.end());
        expected_goffsets.insert( dst_block->root_shadow_cd_goffset );
        
        std::set< goffset_t > missing_goffsets;
        std::set< goffset_t > extra_goffsets;
        
        std::set_difference( expected_goffsets.begin(), expected_goffsets.end()
                           , found_goffsets.begin(), found_goffsets.end()
                           , std::inserter(missing_goffsets, missing_goffsets.begin()));
        std::set_difference( found_goffsets.begin(), found_goffsets.end()
                           , expected_goffsets.begin(), expected_goffsets.end()
                           , std::inserter(extra_goffsets, extra_goffsets.begin()));
        
        /*
        std::cout << "missing_goffsets: " << missing_goffsets << std::endl;
        std::cout << "extra_goffsets: " << extra_goffsets << std::endl;
        std::cout << "far_ptrs: " << far_ptrs << std::endl;
        
        
        SCAFFOLDING {
            pprint_out_data(std::cout, "out_data", out_data, &cd_parent_indices, &cd_goffsets);
            pprint_out_data(std::cout, "uc_out_data", uc_out_data, 0, &uc_cd_goffsets);
            pprint_block(std::cout, "dst_block", dst_block);
            std::cout << "inserted_cds: " << inserted_cds << std::endl;
            std::cout << "out_data.size(): " << out_data.size() << std::endl;
            std::cout << "dst_block->cd_count: " << dst_block->cd_count << std::endl;
        }
        assert(expected_goffsets.size() == cd_goffsets.size() + 1);
        assert(missing_goffsets.size() == 0);
        assert(extra_goffsets.size() == 0);
        
        
        SCAFFOLDING {
            if (auto error = svo_block_sanity_check(dst_block))
            {
                //pprint_out_data(std::cout, "out_data", out_data, &cd_parent_indices, &cd_goffsets);
                //pprint_block(std::cout, "dst_block", dst_block);
                std::cout << "dst_block->slice->side: " << dst_block->slice->side << std::endl;
                std::cout << "dst_block->side: " << dst_block->side << std::endl;
                std::cout << "dst_block->side: " << std::bitset<8>(dst_block->side) << std::endl;
                std::cout << "ilog2(dst_block->side): " << ilog2(dst_block->side) << std::endl;
                std::cout << "dst_block->height: " << dst_block->height << std::endl;
                std::cout << "out_data.size(): " << out_data.size() << std::endl;
                std::cout << "expected_goffsets.size(): " << expected_goffsets.size() << std::endl;
                std::cout << "found_goffsets.size(): " << found_goffsets.size() << std::endl;
                std::cout << "missing_goffsets.size(): " << missing_goffsets.size() << std::endl;
                std::cout << "extra_goffsets.size(): " << extra_goffsets.size() << std::endl;
                std::cout << "dst_block->leaf_count: " << dst_block->leaf_count << std::endl;
                std::cout << "dst_block->cd_count: " << dst_block->cd_count << std::endl;
            }
        }
        */
    }
    
    SCAFFOLDING {
        const auto* parent_root_cd = svo_cget_cd(tree->address_space, dst_block->parent_root_cd_goffset);
        goffset_t parent_root_children_goffset = svo_get_child_ptr_goffset(tree->address_space, dst_block->parent_root_cd_goffset, parent_root_cd);
        
        /*
        std::cout << "------------------" << std::endl;
        std::cout << "dst_block->parent_root_cd_goffset: " << dst_block->parent_root_cd_goffset << std::endl;
        std::cout << "out_uc_root_offsets: " << out_uc_root_offsets << std::endl;
        std::cout << "dst_block->root_children_goffset(): " << dst_block->root_children_goffset() << std::endl;
        std::cout << "parent_root_children_goffset: " << parent_root_children_goffset << std::endl;
        pprint_block(std::cout, "parent_block", parent_block);
        std::cout << "------------------" << std::endl;
         */
    }
    
    DEBUG {
        std::vector<std::string> issues;
        if (!dst_block->check_parent_root_cd(issues))
        {
            std::cout << issues[0] << std::endl;
            assert( false );
        }
    
        if (auto error = svo_block_sanity_check(dst_block))
        {
            std::cerr << error << std::endl;
            assert(false && "sanity fail");
        }
    }
}


void slice_inserter_t::calculate_trunk_cd_goffsets(
      svo_block_t* parent_block
    , cd_goffsets_t& cd_goffsets
    , const cd_indices_t& cd_parent_indices
    , const out_data_t& out_data)
{
    svo_block_t* block0 = block;
    
    assert(parent_block->root_valid_bit);

    assert(out_data.size() > 0);
    assert(cd_goffsets.size() == out_data.size());
    assert(cd_parent_indices.size() == out_data.size());

    assert( cd_goffsets[0] != 0);
    assert( cd_goffsets[0] != invalid_goffset);
    assert( parent_block->is_valid_cd_goffset(cd_goffsets[0]) );
    
    ///if the parent is a root-leaf
    {
        assert(parent_block->root_valid_bit);
        if (parent_block->root_leaf_bit)
        {
            parent_block->leaf_count -= 1;
            parent_block->root_leaf_bit = 0;
        }
    }
    
    ///handle the old root CD special
    {
        const auto& root_voxel0 = out_data[0];
        const auto* root_cd0 = &std::get<2>(root_voxel0);
        
        goffset_t root_cd_goffset = cd_goffsets[0];
        assert(parent_block->is_valid_cd_goffset(root_cd_goffset));
        
        auto* root_cd = svo_get_cd(parent_block->tree->address_space, root_cd_goffset);
        assert(root_cd);
        assert(svo_get_far(root_cd));

        goffset_t shadow_root_cd_goffset = block0->root_shadow_cd_goffset;
        assert(block0->is_valid_cd_goffset(shadow_root_cd_goffset));
        const auto* shadow_root_cd = svo_get_cd(tree->address_space, shadow_root_cd_goffset);

        //goffset_t shadow_root_cd_children_goffset = svo_get_child_ptr_goffset(tree->address_space, shadow_root_cd_goffset, shadow_root_cd);

        //assert(svo_get_goffset_via_fp(parent_block->tree->address_space, root_cd_goffset, root_cd) == shadow_root_cd_children_goffset);

        
        parent_block->clear_cd_count(root_cd_goffset);
        svo_set_valid_mask(root_cd, svo_get_valid_mask(root_cd0));
        svo_set_leaf_mask(root_cd, svo_get_leaf_mask(root_cd0));
        parent_block->add_cd_count(root_cd_goffset);
        
        svo_set_goffset_via_fp(parent_block->tree->address_space, root_cd_goffset, root_cd, invalid_goffset);
        
        
        //std::cout << "svo_get_valid_mask(root_cd0): " << std::bitset<8>(svo_get_valid_mask(root_cd0)) << std::endl;
        //std::cout << "svo_get_leaf_mask(root_cd0): " << std::bitset<8>(svo_get_leaf_mask(root_cd0)) << std::endl;
    }
    
    //pprint_block(std::cout, "parent_block", parent_block);
    //std::cout << __FILE__ << ":" << __LINE__ << std::endl;
    
    
    ///set the cd_goffsets, put in invalid-valued far ptrs
    {
        std::vector< std::size_t > sibling_section_indices;
        for (std::size_t out_data_index = 1; out_data_index < uc_out_data.size(); ++out_data_index)
        {
            sibling_section_indices.push_back(out_data_index);
            const auto& voxel = uc_out_data[out_data_index];

            std::size_t parent_out_data_index = cd_parent_indices[out_data_index];

            bool should_flush_child_section = false;
            if (out_data_index + 1 == uc_out_data.size())
            {
                should_flush_child_section = true;
            } else {
                std::size_t next_voxel_parent_data_index = cd_parent_indices[ out_data_index + 1];
                if (next_voxel_parent_data_index != parent_out_data_index)
                    should_flush_child_section = true;
                else
                    should_flush_child_section = false;
            }


            if (should_flush_child_section)
            {


                ///CDs
                {
                    for (std::size_t sibling_data_index : sibling_section_indices)
                    {
                        const auto* cd0 = &std::get<2>(uc_out_data[sibling_data_index]);
                        
                        goffset_t& cd_goffset = cd_goffsets[sibling_data_index];
                        assert(cd_goffset != 0);
                        
                        if (cd_goffset == invalid_goffset)
                        {
                            cd_goffset = svo_append_cd(parent_block->tree->address_space, parent_block, cd0);
                        

                            assert(cd_goffset != 0);
                            if(cd_goffset == invalid_goffset)
                                throw svo_block_full();
                        }
                        
                        parent_block->add_cd_count(cd_goffset);
                        
                    }

                    std::size_t dummy_cds = 8 - sibling_section_indices.size();
                    for (std::size_t i = 0; i < dummy_cds; ++i)
                    {
                        goffset_t cd_goffset = svo_append_dummy_cd(parent_block->tree->address_space, parent_block);

                        if(cd_goffset == invalid_goffset)
                            throw svo_block_full();
                    }
                }
                ///far pointers
                {

                    std::vector< goffset_t > sibling_fp_goffsets;
                    
                    while ( sibling_fp_goffsets.size() < sibling_section_indices.size() )
                    {
                        goffset_t cd_goffset = svo_append_dummy_cd(parent_block->tree->address_space, parent_block);

                        if(cd_goffset == invalid_goffset)
                            throw svo_block_full();
                        
                        std::size_t slots = sizeof(child_descriptor_t) / sizeof(goffset_t);
                        assert(slots == 2);
                        
                        for (std::size_t slot = 0; slot < slots && sibling_fp_goffsets.size() < sibling_section_indices.size(); ++slot)
                        {
                            goffset_t fp_goffset = cd_goffset + slot*sizeof(goffset_t);
                            sibling_fp_goffsets.push_back(fp_goffset);
                        }
                    }
                    
                    assert( sibling_fp_goffsets.size() == sibling_section_indices.size());

                    for (std::size_t i = 0; i < sibling_section_indices.size(); ++i)
                    {
                        std::size_t sibling_data_index = sibling_section_indices[i];
                        
                        goffset_t cd_goffset = cd_goffsets[sibling_data_index];
                        assert(parent_block->is_valid_cd_goffset(cd_goffset));
                        
                        goffset_t far_ptr_goffset = sibling_fp_goffsets[i];
                        assert(parent_block->is_in_block(far_ptr_goffset));

                        auto* cd = svo_get_cd(parent_block->tree->address_space, cd_goffset);

                        offset_t offset = far_ptr_goffset - cd_goffset;
                        assert(offset < 16*sizeof(child_descriptor_t));
                        assert(offset % 4 == 0);

                        offset4_t offset4 = offset / 4;

                        svo_set_far(cd,true);
                        svo_set_child_ptr(cd, offset4);
                        ///erase the far ptr
                        svo_set_goffset_via_fp(tree->address_space, cd_goffset, cd, invalid_goffset);
                    }
                }
                
                SCAFFOLDING{

                    //std::cout << "sibling_section_indices.size(): " << sibling_section_indices.size() << std::endl;
                    for (std::size_t sibling_data_index : sibling_section_indices)
                    {
                        goffset_t cd_goffset = cd_goffsets[sibling_data_index];
                        assert(parent_block->is_valid_cd_goffset(cd_goffset));

                        const auto* cd = svo_cget_cd(parent_block->tree->address_space, cd_goffset);
                        assert(svo_get_goffset_via_fp(tree->address_space, cd_goffset, cd) == invalid_goffset);
                    }
                }
                sibling_section_indices.clear();
            }
        }
    }

    //pprint_block(std::cout, "parent_block", parent_block);
    //std::cout << __FILE__ << ":" << __LINE__ << std::endl;
    
    ///sanity
    for (std::size_t out_data_index = 0; out_data_index < out_data.size(); ++out_data_index)
    {
        goffset_t cd_goffset = cd_goffsets[out_data_index];
        assert(cd_goffset != 0);
        assert(cd_goffset != invalid_goffset);
        assert(parent_block->is_valid_cd_goffset(cd_goffset));

        const auto* cd = svo_cget_cd(tree->address_space, cd_goffset);
        offset4_t offset4 = svo_get_child_ptr_offset4(cd);

        assert(offset4 != 0);

        assert(svo_get_far(cd));

        assert(svo_get_valid_mask(cd));

        assert(svo_get_goffset_via_fp(tree->address_space, cd_goffset, cd) == invalid_goffset);
    }
}

void slice_inserter_t::calculate_trunk_cd_inner_far_ptrs(
      svo_block_t* parent_block
    , const cd_goffsets_t& cd_goffsets
    , const cd_indices_t& cd_parent_indices
    , const out_data_t& out_data)
{
    
    ///set the far ptr values
    for (std::size_t out_data_index = 0; out_data_index < uc_out_data.size(); ++out_data_index)
    {
        const auto& voxel = uc_out_data[out_data_index];

        std::size_t parent_out_data_index = cd_parent_indices[out_data_index];
        
        if (parent_out_data_index == std::size_t(-1))
            continue;
        
        assert(parent_out_data_index < uc_out_data.size());
        //offset_t child_0_offset_in_cds = std::get<3>(voxel);

        goffset_t cd_goffset = cd_goffsets[out_data_index];
        assert(cd_goffset != 0);
        assert(cd_goffset != invalid_goffset);
        assert(parent_block->is_valid_cd_goffset(cd_goffset));

        goffset_t pcd_goffset = cd_goffsets[parent_out_data_index];
        assert(pcd_goffset != 0);
        assert(pcd_goffset != invalid_goffset);
        assert(parent_block->is_valid_cd_goffset(cd_goffset));

        assert(pcd_goffset < cd_goffset);

        auto* cd = svo_get_cd(tree->address_space, cd_goffset);
        auto* pcd = svo_get_cd(tree->address_space, pcd_goffset);

        assert(svo_get_far(pcd));
        
        ///if the far ptr for this parent was already set, just move on.
        if (svo_get_goffset_via_fp(tree->address_space, pcd_goffset, pcd) != invalid_goffset)
            continue;
    
        
        assert(svo_get_goffset_via_fp(tree->address_space, pcd_goffset, pcd) == invalid_goffset);

        svo_set_goffset_via_fp(tree->address_space, pcd_goffset, pcd, cd_goffset);
        
    }

}

void slice_inserter_t::calculate_trunk_cd_terminal_far_ptrs(
      svo_block_t* parent_block
    , const cd_goffsets_t& cd_goffsets
    , const cd_indices_t& cd_parent_indices
    , const out_data_t& out_data)
{
    
    ///in the special case where there is no split of child slices.
    if (uc_out_data.size() == 1)
    {
        ///the blocks didn't put their roots into unclassified, so there should be no split, and one destination block.
        assert(dst_blocks.size() == 1);
        
        svo_block_t* child_block = dst_blocks[0];
        
        ///gets the root shadow CD, which lies in the current child_block
        const auto* shadow_root_cd = svo_cget_cd(tree->address_space, child_block->root_shadow_cd_goffset);
        ///gets the root CD, which lies in the parent block
        auto* parent_root_cd = svo_get_cd(tree->address_space, child_block->parent_root_cd_goffset);
                
        assert(svo_get_far(parent_root_cd));
        
        ///this is a terminal far ptr root cd, it should not have a far ptr set.
        assert(svo_get_goffset_via_fp(tree->address_space, child_block->parent_root_cd_goffset, parent_root_cd) == invalid_goffset);

        assert(child_block->has_root_children_goffset());
        svo_set_goffset_via_fp(tree->address_space, child_block->parent_root_cd_goffset, parent_root_cd
                                , child_block->root_children_goffset());

        assert(svo_get_child_ptr_goffset(tree->address_space, child_block->parent_root_cd_goffset, parent_root_cd)
                == child_block->root_children_goffset());
        
        return;
    }
    
    ///set the terminal nonleaf CDs to point to the new child blocks.
    assert(out_datas.size() == dst_blocks.size());
    for (std::size_t classification = 0; classification < out_datas.size(); ++classification)
    {
        svo_block_t* child_block = dst_blocks[classification];

        assert(classification < out_uc_root_offsets.size());
        std::size_t root_uc_data_index = out_uc_root_offsets[classification];
        
        if (root_uc_data_index == std::size_t(-1))
            continue;
        assert(root_uc_data_index != std::size_t(-1));
        assert(root_uc_data_index < cd_goffsets.size());
        child_block->parent_root_cd_goffset = cd_goffsets[root_uc_data_index];

        assert(child_block->parent_root_cd_goffset != 0);
        assert(child_block->parent_root_cd_goffset != invalid_goffset);

        ///gets the root shadow CD, which lies in the current child_block
        const auto* shadow_root_cd = svo_cget_cd(tree->address_space, child_block->root_shadow_cd_goffset);
        ///gets the root CD, which lies in the parent block
        auto* parent_root_cd = svo_get_cd(tree->address_space, child_block->parent_root_cd_goffset);

        ///the goffset to the beginning of the root children set.
        //goffset_t shadow_root_cd_children_goffset = svo_get_child_ptr_goffset(tree->address_space, child_block->root_shadow_cd_goffset, shadow_root_cd);


        SCAFFOLDING{
            /*
            if (!(svo_get_goffset_via_fp(tree->address_space, child_block->parent_root_cd_goffset, parent_root_cd) == invalid_goffset))
            {
                pprint_out_data(std::cout, "out_datas[classification]", out_datas[classification]);
                pprint_out_data(std::cout, "uc_out_data", uc_out_data, &cd_parent_indices, &cd_goffsets);
                pprint_block(std::cout, "parent_block", parent_block);
                pprint_block(std::cout, "child_block", child_block);
                assert(parent_block->is_valid_cd_goffset(child_block->parent_root_cd_goffset));
                
                goffset_t goffset = svo_get_goffset_via_fp(tree->address_space, child_block->parent_root_cd_goffset, parent_root_cd);
                std::cout << "child_block->parent_root_cd_goffset: " << child_block->parent_root_cd_goffset << std::endl;
                std::cout << "svo_get_goffset_via_fp(tree->address_space, child_block->parent_root_cd_goffset, parent_root_cd): " << goffset << std::endl;
            }
             */
        }

        assert(svo_get_far(parent_root_cd));
        ///this is a terminal far ptr root cd, it should not have a far ptr set.
        //assert(svo_get_goffset_via_fp(tree->address_space, child_block->parent_root_cd_goffset, parent_root_cd) == invalid_goffset);
        
        assert(child_block->has_root_children_goffset());
        svo_set_goffset_via_fp(tree->address_space, child_block->parent_root_cd_goffset, parent_root_cd, child_block->root_children_goffset());
    
    }
}


void slice_inserter_t::insert_unclassified_child_descriptors(cd_goffsets_t& uc_cd_goffsets)
{
    if (uc_out_data.size() == 0)
        return;
    assert(uc_cd_goffsets.size() == uc_out_data.size());
    
    svo_block_t* block0 = block;
    
    assert(parent_block);
    assert(parent_block->trunk);
    
    //pprint_out_data(std::cout, "uc_out_data", uc_out_data);

    cd_indices_t cd_parent_indices(uc_out_data.size(), std::size_t(-1));
    calculate_parent_indices(cd_parent_indices, uc_out_data, true/*allow_non_leafs*/);
    






    uc_cd_goffsets[0] = block0->parent_root_cd_goffset;
    
    assert(parent_block->is_valid_cd_goffset(uc_cd_goffsets[0]));
    
    ///note: erases the far ptr of the root CD
    calculate_trunk_cd_goffsets(parent_block, uc_cd_goffsets, cd_parent_indices, uc_out_data);
    
    ///corrects all the far pts of the new CDs, except the terminal CDs that point into the new child blocks.
    calculate_trunk_cd_inner_far_ptrs(parent_block, uc_cd_goffsets, cd_parent_indices, uc_out_data);

    ///corrects the terminal CDs that they point into the new child blocks.
    calculate_trunk_cd_terminal_far_ptrs(parent_block, uc_cd_goffsets, cd_parent_indices, uc_out_data);
    
    /*
    pprint_out_data(std::cout, "uc_out_data", uc_out_data);
    pprint_block(std::cout, "parent_block", parent_block);
    //sanity
    {
        assert(block0->parent_root_cd_goffset != 0);
        assert(block0->parent_root_cd_goffset != invalid_goffset);
        assert(parent_block->is_valid_cd_goffset(block0->parent_root_cd_goffset));

        
        ///gets the root shadow CD, which lies in the current block
        const auto* shadow_root_cd = svo_cget_cd(tree->address_space, block0->root_shadow_cd_goffset);
        ///gets the root CD, which lies in the parent block
        const auto* parent_root_cd = svo_cget_cd(tree->address_space, block0->parent_root_cd_goffset);
        
        ///the root CD should always have a far pointer to this block.
        assert(svo_get_far(parent_root_cd));
        
        ///the goffset to the beginning of the root children set.
        goffset_t shadow_root_cd_children_goffset = svo_get_child_ptr_goffset(tree->address_space, block0->root_shadow_cd_goffset, shadow_root_cd);
        ///the goffset to the beginning of the root children set.
        goffset_t parent_root_cd_children_goffset = svo_get_child_ptr_goffset(tree->address_space, block0->parent_root_cd_goffset, parent_root_cd);
        
        assert(shadow_root_cd_children_goffset == parent_root_cd_children_goffset);
    }
    */
    
    
    SCAFFOLDING {
        /*
        for (std::size_t classified = 0; classified < out_datas.size(); ++classified)
        {
            std::cout << "out_uc_root_offsets[" << classified << "]: " << out_uc_root_offsets[classified] << std::endl;
        }
        pprint_block(std::cout, "parent_block", parent_block);
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        */
    }
    
    
    DEBUG {
        
        if (auto error = svo_block_sanity_check(parent_block))
        {
            std::cerr << error << std::endl;
            assert(false && "sanity fail");
        }
    
    }
    
}

#if 0
void slice_inserter_t::update_root_shadows()
{
    assert(block->parent_block->trunk);
    
    SCAFFOLDING {
        //pprint_block(std::cout, "parent_block", block->parent_block);
        //std::cout << __FILE__ << ":" << __LINE__ << std::endl;
    }
    for (svo_block_t* dst_block : dst_blocks)
    {
        if (!(dst_block->parent_block))
            continue;
        
        
        svo_block_t* pblock = dst_block->parent_block;
        
        assert(dst_block->parent_root_cd_goffset != invalid_goffset);
        assert(dst_block->parent_root_cd_goffset != 0);
        assert(pblock->is_valid_cd_goffset(dst_block->parent_root_cd_goffset));
        
        auto* root_cd = svo_get_cd(tree->address_space, dst_block->parent_root_cd_goffset);

        assert(svo_get_far(root_cd));

        
        auto* shadow_root_cd = svo_get_cd(tree->address_space, dst_block->root_shadow_cd_goffset);
        
        ///copy the root_cd to shadow_root_cd
        svo_set_valid_mask(shadow_root_cd, svo_get_valid_mask(root_cd));
        svo_set_leaf_mask(shadow_root_cd, svo_get_leaf_mask(root_cd));
        
        
        ///set the root_cd to properly point to the block.
        {
            
            goffset_t root_cd_far_ptr_goffset = dst_block->parent_root_cd_goffset + svo_get_child_ptr_offset4(root_cd)*4;
            goffset_t base_child_goffset = svo_get_child_ptr_goffset(tree->address_space, dst_block->root_shadow_cd_goffset, shadow_root_cd);
            
            svo_set_goffset_via_fp(tree->address_space, dst_block->parent_root_cd_goffset, root_cd, base_child_goffset);
            
            assert(svo_get_child_ptr_goffset(tree->address_space, dst_block->parent_root_cd_goffset, root_cd) == base_child_goffset);
            assert(svo_get_child_ptr_goffset(tree->address_space, dst_block->root_shadow_cd_goffset, shadow_root_cd) == base_child_goffset);
        }
    
        DEBUG {
            if (auto error = svo_block_sanity_check(dst_block))
            {
                SCAFFOLDING {
                    //pprint_block(std::cout, "dst_block", dst_block);
                }
                std::cerr << error << std::endl;
                assert(false && "sanity fail");
            }
        
        }
    
    }

    DEBUG {
        if (auto error = svo_block_sanity_check(block->parent_block))
        {
            SCAFFOLDING {
                //pprint_block(std::cout, "parent_block", block->parent_block);
            }
            std::cerr << error << std::endl;
            assert(false && "sanity fail");
        }
    
        for (svo_block_t* dst_block : dst_blocks)
        {
            
            if (auto error = svo_block_sanity_check(dst_block->parent_block))
            {
                std::cerr << error << std::endl;
                assert(false && "sanity fail");
            }
        }
    }
    
}
#endif

void slice_inserter_t::pprint_out_data(
      std::ostream& out, const std::string& announce_msg, const out_data_t& out_data
    , const cd_indices_t* cd_parent_indices, const cd_goffsets_t* cd_goffsets) const
{
    assert(cd_parent_indices == 0 || cd_parent_indices->size() == out_data.size());
    
    std::size_t max_level = block->height + 1;
    
    
    bprinter::TablePrinter tp(&out);
    tp.AddColumn("idx", 6);
    tp.AddColumn("level", std::max(5, (int)std::ceil(std::log10(max_level))));
    tp.AddColumn("vcurve", std::max(5, (int)std::ceil(std::log10(vcurvesize(SVO_MAX_VOLUME_SIDE)))));
    tp.AddColumn("cd", 64+7);
    tp.AddColumn("child offset", std::max(12, (int)std::ceil(std::log10(offset_t(-1)))));
    
    if (cd_parent_indices)
        tp.AddColumn("pidx", std::max(12, (int)std::ceil(std::log10(out_data.size()))));
    if (cd_goffsets)
        tp.AddColumn("cd goffset", std::max(8, (int)std::ceil(std::log10(goffset_t(-1)))));
    
    pprint_announce_msg(out, announce_msg, tp.get_table_width() + 2);
    
    tp.PrintHeader();
    for (std::size_t out_data_index = 0; out_data_index < out_data.size(); out_data_index++)
    {
        const auto& voxel = out_data[out_data_index];
        const std::size_t& level = std::get<0>(voxel);
        vcurve_t vcurve = std::get<1>(voxel);
        const child_descriptor_t& cd = std::get<2>(voxel);
        const offset_t& offset = std::get<3>(voxel);
        
        
        std::ostringstream ostr;
        ostr << cd;
        
        tp << out_data_index << level << vcurve << ostr.str() << offset;
        
        if (cd_parent_indices)
            tp << (*cd_parent_indices)[out_data_index];
        if (cd_goffsets)
            tp << (*cd_goffsets)[out_data_index];
    }
    
    tp.PrintFooter();
    
}


} //namespace svo
