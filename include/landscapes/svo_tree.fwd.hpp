#ifndef SVO_TREE_FWD_HPP
#define SVO_TREE_FWD_HPP 1

#include <cstddef>
#include <cstdint>
#include "landscapes/svo_curves.h"

namespace svo{
static const std::size_t SVO_VOLUME_SIDE_LIMIT = SVO_VSIDE_LIMIT;
enum class svo_error_t{
      OK
    , CHILDREN_TOO_FAR
    , BLOCK_IS_FULL
};


struct svo_block_t;
struct svo_slice_t;
struct svo_tree_t;
struct volume_of_slices_t;
struct svo_slice_child_group_t;
struct svo_block_sanity_error_t;
struct svo_slice_sanity_error_t;

} //namespace svo



#endif
