#ifndef SVO_SERALIZATION_V1_HPP
#define SVO_SERALIZATION_V1_HPP 1

#include <iosfwd>
#include <vector>
#include <tuple>
#include <string>
#include <memory>
#include "svo_curves.h"
#include "svo_tree.fwd.hpp"
#include "svo_buffer.fwd.hpp"

namespace svo{
    

typedef std::vector< std::tuple<vside_t, vcurve_t> > children_params_t;



void serialize_slice(std::ostream& out, const svo_slice_t* slice);
children_params_t unserialize_slice(std::istream& in, svo_slice_t* slice, bool load_empty_children);







void serialize_string(std::ostream& out, const std::string& v);
void serialize_buffer(std::ostream& out, const svo_cpu_buffer_t& buffer);
void serialize_buffer_data(std::ostream& out, const svo_cpu_buffer_t& buffer);
void serialize_buffers(std::ostream& out, const svo_cpu_buffers_t& buffers, std::size_t expected_entries);
void serialize_declaration(std::ostream& out, const svo_declaration_t& declaration);
void serialize_schema(std::ostream& out, const svo_schema_t& schema);
void serialize_slice_child_info(std::ostream& out, const svo_slice_t* slice);

std::string unserialize_string(std::istream& in);
std::unique_ptr<svo_cpu_buffer_t> unserialize_buffer(std::istream& in);
void unserialize_buffer_data(std::istream& in, svo_cpu_buffer_t& buffers, std::size_t expected_entries);
void unserialize_buffers(std::istream& in, svo_cpu_buffers_t& buffers, std::size_t expected_entries);
svo_declaration_t unserialize_declaration(std::istream& in);
svo_schema_t unserialize_schema(std::istream& in);
children_params_t unserialize_slice_child_info(std::istream& in, svo_slice_t* slice);







} //namespace svo

#endif
