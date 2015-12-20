#ifndef SVO_TREE_BLOCK_MGMT_HPP
#define SVO_TREE_BLOCK_MGMT_HPP 1

#include "svo_curves.h"
#include "svo_inttypes.h"
#include "svo_tree.capi.h"
#include "svo_tree.fwd.hpp"
#include "svo_buffer.fwd.hpp"

#include <tuple>
#include <vector>
#include <iosfwd>

namespace svo{
typedef std::vector< std::size_t > cd_indices_t;
typedef std::vector< goffset_t > cd_goffsets_t;

struct slice_inserter_t{
    ///[ (level, vcurve, cd, child offset) ]
    /// level of the cd
    /// vcurve of the cd within the level
    /// the cd, mainly the masks
    /// child offset to the children; 0 is initial invalid offset flag.
    typedef std::vector< std::tuple<std::size_t, vcurve_t, child_descriptor_t, offset_t> > out_data_t;

    //struct out_data_t{
    //    out_pos_data_t pos_data;
    //    std::vector<svo_cpu_buffer_t> buffers;
    //};

    
    
    slice_inserter_t(svo_tree_t* tree, svo_block_t* block);

    ///this is what you call.
    svo_error_t execute();

private:
    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///when splitting a block, as per the paper, when there is more than 1 child slice, this function will
    /// classify a particular voxel into one or none of the child slices. A return value of
    /// `std::size_t(-1)` indiciates the voxel is not classifiable.
    std::size_t svo_classify_voxel(std::size_t level, vcurve_t vcurve);
    ///////////////////////////////////////////////////////////////////////////////////////////////
    void classify_child_descriptors();
    ///////////////////////////////////////////////////////////////////////////////////////////////
    void calculate_parent_indices(cd_indices_t& cd_parent_indices, const out_data_t& out_data, bool allow_non_leafs=false);
    ///////////////////////////////////////////////////////////////////////////////////////////////
    void calculate_reverse_offsets(
          std::vector<std::size_t>& cd_reverse_boffsets
        , std::vector<bool>& cd_req_far_ptrs
        , const cd_indices_t& cd_parent_indices
        , const out_data_t& out_data
        , bool trunk);
    ///////////////////////////////////////////////////////////////////////////////////////////////
    void allocate_dst_blocks();
    ///////////////////////////////////////////////////////////////////////////////////////////////
    void insert_classified_child_descriptors(
          cd_goffsets_t& cd_goffsets
        , svo_block_t* dst_block
        , std::size_t classification
        , const cd_goffsets_t& uc_cd_goffsets
        , const std::vector<bool>& cd_req_far_ptrs
        , const cd_indices_t& cd_parent_indices
        , const out_data_t& out_data);
    ///////////////////////////////////////////////////////////////////////////////////////////////
    void insert_unclassified_child_descriptors(cd_goffsets_t& uc_cd_goffsets);
    ///////////////////////////////////////////////////////////////////////////////////////////////
    void calculate_trunk_cd_goffsets(
          svo_block_t* parent_block
        , cd_goffsets_t& cd_goffsets
        , const cd_indices_t& cd_parent_indices
        , const out_data_t& out_data);
    void calculate_trunk_cd_inner_far_ptrs(
          svo_block_t* parent_block
        , const cd_goffsets_t& cd_goffsets
        , const cd_indices_t& cd_parent_indices
        , const out_data_t& out_data);
    void calculate_trunk_cd_terminal_far_ptrs(
          svo_block_t* parent_block
        , const cd_goffsets_t& cd_goffsets
        , const cd_indices_t& cd_parent_indices
        , const out_data_t& out_data);
    ///////////////////////////////////////////////////////////////////////////////////////////////
    //void update_root_shadows();
    ///////////////////////////////////////////////////////////////////////////////////////////////

    void pprint_out_data( std::ostream& out, const std::string& announce_msg, const out_data_t& out_data
                        , const cd_indices_t* parents=0, const cd_goffsets_t* cd_goffsets=0) const;


    ///////////////////////////////////////////////////////////////////////////////////////////////

    svo_tree_t* tree;
    svo_block_t* block;
    svo_slice_t* slice;
    svo_block_t* parent_block;



    ///an @c out_data_t for the unclassified voxels; the ones that can't fit into any children slices.
    out_data_t uc_out_data;

    ///an @c out_data_t for each of the slice's children. we will split the voxels among the the different child-volumes.
    std::vector< out_data_t > out_datas;
    std::vector< std::size_t > out_data_root_levels;



    ///for each @c out_data_t in @c out_datas, we keep an offset into @c uc_out_data to the cd that is the parent of the
    /// first node in @c out_data.
    std::vector< offset_t > out_uc_root_offsets;


public:
    std::vector<svo_block_t*> ret_blocks;
    std::vector<svo_block_t*> dst_blocks;


};


} //namespace svo

#endif
