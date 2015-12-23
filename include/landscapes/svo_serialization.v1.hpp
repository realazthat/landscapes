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



void svo_serialize_slice(std::ostream& out, const svo_slice_t* slice);
children_params_t svo_unserialize_slice(std::istream& in, svo_slice_t* slice, bool load_empty_children);







void serialize_string(std::ostream& out, const std::string& v);
void svo_serialize_buffers(std::ostream& out, const svo_cpu_buffers_t& buffers, std::size_t expected_entries);
void svo_serialize_buffers_schema_declaration(std::ostream& out, const svo_declaration_t& declaration);
void svo_serialize_buffers_schema(std::ostream& out, const svo_schema_t& schema);
void svo_serialize_slice(std::ostream& out, const svo_slice_t* slice);
void svo_serialize_slice_child_info(std::ostream& out, const svo_slice_t* slice);

std::string unserialize_string(std::istream& in);
void svo_unserialize_buffers(std::istream& in, svo_cpu_buffers_t& buffers, std::size_t data_size);
svo_declaration_t svo_unserialize_buffers_schema_declaration(std::istream& in);
svo_schema_t svo_unserialize_buffers_schema(std::istream& in);
children_params_t svo_unserialize_slice_child_info(std::istream& in, svo_slice_t* slice);







} //namespace svo

#endif
