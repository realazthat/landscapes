#ifndef SVO_SERALIZATION_V1_HPP
#define SVO_SERALIZATION_V1_HPP 1

#include <iosfwd>
#include <vector>
#include <tuple>
#include <string>
#include "svo_curves.h"
#include "svo_tree.fwd.hpp"

namespace svo{
typedef std::vector< std::tuple<vside_t, vcurve_t> > children_params_t;


void serialize_string(std::ostream& out, const std::string& v);
std::string unserialize_string(std::istream& in);

void svo_serialize_slice(std::ostream& out, const svo_slice_t* slice);
children_params_t svo_unserialize_slice(std::istream& in, svo_slice_t* slice, bool load_empty_children);


} //namespace svo

#endif
