#ifndef LANDSCAPES_SVO_MCLOADER_HPP
#define LANDSCAPES_SVO_MCLOADER_HPP 1

#include <iosfwd>
#include "svo_tree.fwd.hpp"

namespace svo{


void load_mca_region(volume_of_slices_t& slices, std::ifstream& region_file, std::size_t num_threads = 1);

} //namespace


#endif
