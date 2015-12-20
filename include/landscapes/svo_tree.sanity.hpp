#ifndef SVO_TREE_SANITY_HPP
#define SVO_TREE_SANITY_HPP 1


#include "svo_tree.fwd.hpp"
#include <iosfwd>
#include <string>

std::ostream& operator<<(std::ostream& out, const svo::svo_block_sanity_error_t& error);
std::ostream& operator<<(std::ostream& out, const svo::svo_slice_sanity_error_t& error);

namespace svo{
    


struct svo_slice_sanity_error_t
{
    svo_slice_sanity_error_t()
        : has_error(false)
        , slice0(0), slice1(0), slice2(0)
    {}

    svo_slice_sanity_error_t(const std::string& error, const svo_slice_t* slice0=0, const svo_slice_t* slice1=0, const svo_slice_t* slice2=0)
        : has_error(true)
        , error(error)
        , slice0(slice0)
        , slice1(slice1)
        , slice2(slice2)
    {}

    operator bool() const { return has_error; }

    bool has_error;
    std::string error;
    const svo_slice_t* slice0;
    const svo_slice_t* slice1;
    const svo_slice_t* slice2;
};
struct svo_block_sanity_error_t
{
    svo_block_sanity_error_t()
        : has_error(false)
        , block0(0), block1(0), block2(0)
    {}

    svo_block_sanity_error_t(const std::string& error, const svo_block_t* block0=0, const svo_block_t* block1=0, const svo_block_t* block2=0)
        : has_error(true)
        , error(error)
        , block0(block0)
        , block1(block1)
        , block2(block2)
    {}

    operator bool() const { return has_error; }

    bool has_error;
    std::string error;
    const svo_block_t* block0;
    const svo_block_t* block1;
    const svo_block_t* block2;
};



enum svo_sanity_type_t{
      none =            0
    , minimal =         1 << 1
    , levels =          1 << 2
    , pos_data =        1 << 3
    , channel_data =    1 << 4
    , children =        1 << 5
    , parent =          1 << 6
    , all_data =        pos_data | channel_data
    , all =             minimal | levels | pos_data | channel_data | children | parent
    , default_sanity = all
};



svo_slice_sanity_error_t svo_slice_sanity(
      const svo_slice_t* slice
    , svo_sanity_type_t sanity_type = svo_sanity_type_t::default_sanity
    , int recurse = 1
    , bool parent_recurse = true);


//svo_slice_sanity_error_t svo_slice_sanity_check_error(const svo_slice_t* slice, int recurse=0);
//svo_slice_sanity_error_t svo_slice_sanity_check_parent_error(const svo_slice_t* slice, int recurse=0);
//svo_slice_sanity_error_t svo_slice_sanity_check_children_error(const svo_slice_t* slice, int recurse=0);
//svo_slice_sanity_error_t svo_slice_pos_data_sanity_check(const svo_slice_t* slice);
//svo_slice_sanity_error_t svo_slice_channels_sanity_check(const svo_slice_t* slice);
//svo_slice_sanity_error_t svo_slice_all_sanity_check(const svo_slice_t* slice);


svo_block_sanity_error_t svo_block_sanity_check(const svo_block_t* block, int recurse=0);

} //namespace svo




#endif
