#ifndef SVO_TREE_SLICE_MGMT_HPP
#define SVO_TREE_SLICE_MGMT_HPP




#include <cstddef>
#include <cstdint>

namespace svo{

struct svo_slice_t;
struct volume_of_slices_t;


svo_slice_t* svo_entree_slices(const volume_of_slices_t& volume_of_slices, std::size_t max_voxels_per_slice, std::size_t root_level=0);


void svo_downsample_slice(svo_slice_t* parent_slice, const svo_slice_t* child_slice);


/**
 * When a group of leaf voxels downsamples to a parent voxel, and do not add any significant detail,
 * we call the group "shadowed" voxels.
 *
 * In such a case, we can remove the group.
 *
 * This is analogous to decimation.
 */
void svo_clear_shadowed_voxels_from_slice(const svo_slice_t* parent_slice, svo_slice_t* slice, const svo_slice_t* child_slice);



void svo_clear_enclosed_voxels_from_slice(svo_slice_t* slice, const svo_slice_t* child_slice);




} // namespace svo








#endif
