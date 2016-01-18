
#include "landscapes/svo_serialization.v1.hpp"
#include "landscapes/svo_tree.hpp"
#include "landscapes/svo_tree.sanity.hpp"
#include "landscapes/svo_slice.comparison.hpp"

#include <iostream>

namespace svo{


static const int FORMAT_VERSION = 1;

template<typename T>
static inline void serialize_uint(std::ostream& out, T v, bool debug=false);

template<typename T>
static inline T unserialize_uint(std::istream& in);
std::string unserialize_string(std::istream& in);










void serialize_string(std::ostream& out, const std::string& v, bool debug)
{
    serialize_uint<uint32_t>(out, v.size(), debug);
    out.write(v.data(), v.size());
    
    if (debug)
    {
        std::ostringstream debugout;
        serialize_string(debugout,v);
        
        
        std::istringstream debugin(debugout.str());
        auto result = unserialize_string(debugin);
        
        
        if (result != v)
            throw std::runtime_error(fmt::format("serialize_string({}) failed, unserialized result: {}", svo::quote(v), svo::quote(result)));
    }
}

std::string unserialize_string(std::istream& in)
{
    auto size = unserialize_uint<uint32_t>(in);
    
    std::string v;
    v.resize(size);
    in.read(&v[0], v.size());
    assert(in.gcount() == size);
    
    return v;
}

template<typename T>
static inline void serialize_uint(std::ostream& out, T v, bool debug)
{
    T bmask = 255;

    uint8_t buffer[sizeof(T)];


    for (std::size_t i = 0; i < sizeof(T); ++i)
    {
        uint8_t byte = (v >> (i*8)) & bmask;

        //buffer[ i ] = byte;
        buffer[ sizeof(T) - 1 - i ] = byte;
    }

    out.write(reinterpret_cast<char*>(&buffer[0]),sizeof(T));
    
    
    if (debug)
    {
        std::ostringstream debugout;
        serialize_uint<T>(debugout,v);
        
        
        std::istringstream debugin(debugout.str());
        auto result = unserialize_uint<T>(debugin);
        
        
        if (result != v)
            throw std::runtime_error(fmt::format("serialize_uint({}) failed, unserialized result: {}", v, result));
    }
}


template<typename T>
static inline T unserialize_uint(std::istream& in)
{
    uint8_t buffer[sizeof(T)];
    in.read(reinterpret_cast<char*>(&buffer[0]), sizeof(T));
    assert(in.gcount() == sizeof(T));

    T v = 0;
    for (std::size_t i = 0; i < sizeof(T); ++i)
    {
        //uint8_t byte = buffer[i];
        uint8_t byte = buffer[sizeof(T) - 1 - i];

        v |= T(byte) << (i*8);
    }
    return v;
}

void serialize_slice_child_info(std::ostream& out, const svo_slice_t* slice, bool debug)
{
    assert(slice);
    assert(slice->children);

    
    
    uint32_t children_count = slice->children->size();

    serialize_uint<uint32_t>(out, uint32_t(children_count), debug);

    for (uint32_t i = 0; i < children_count; ++i)
    {
        svo_slice_t* child = (*slice->children)[i];
        assert(child);
        serialize_uint<uint32_t>(out, uint32_t(child->side), debug);
        serialize_uint<uint32_t>(out, uint32_t(child->parent_vcurve_begin), debug);
    }
    
    if (debug)
    {
        ///@TODO
    }
}



void serialize_slice(std::ostream& out, const svo_slice_t* slice, bool debug)
{
    assert(slice);
    assert(slice->children);
    assert(slice->pos_data);
    assert(slice->buffers);

    serialize_uint<uint32_t>(out, uint32_t(FORMAT_VERSION), debug);
    serialize_uint<uint32_t>(out, uint32_t(slice->level), debug);
    serialize_uint<uint32_t>(out, uint32_t(slice->side), debug );
    serialize_uint<uint32_t>(out, uint32_t(slice->parent_vcurve_begin), debug);
    
    serialize_slice_child_info(out, slice, debug);

    const auto& pos_data = *slice->pos_data;
    const auto& buffers = *slice->buffers;

    serialize_uint<uint32_t>(out, uint32_t(pos_data.size()), debug);

    ///output voxel coordinates
    for (std::size_t i = 0; i < pos_data.size(); ++i)
    {
        vcurve_t vcurve = pos_data[i];
        serialize_uint<uint32_t>(out, uint32_t(vcurve), debug);
    }

    
    serialize_buffers(out, buffers, pos_data.size(), debug);
    
    
    if (debug)
    {
        std::ostringstream debugout;
        serialize_slice(debugout, slice);
        
        auto* debug_slice = svo_init_slice(0, 16);
        
        std::istringstream debugin(debugout.str());
        unserialize_slice(debugin, debug_slice, true/*load_empty_children*/);
        
        
        auto mask = slice_cmp_t::enum_t(slice_cmp_t::all & ~(slice_cmp_t::has_parent | slice_cmp_t::parent_props) );
        if (auto inequality = svo_slice_inequality(slice,debug_slice, mask))
            throw std::runtime_error(fmt::format("{}", inequality));
        svo_uninit_slice(debug_slice,true);
    }
}

void serialize_declaration(std::ostream& out, const svo_declaration_t& declaration, bool debug)
{
    serialize_uint<uint32_t>(out, declaration.elements().size());
    
    for (const auto& element : declaration.elements())
    {
        serialize_string(out, element.name(), debug);
        serialize_string(out, tostr(element.semantic()), debug);
        serialize_string(out, tostr(element.type()), debug);
        serialize_uint<uint32_t>(out, element.count(), debug);
    }
    
    if (debug)
    {
        std::ostringstream debugout;
        serialize_declaration(debugout,declaration);
        
        
        std::istringstream debugin(debugout.str());
        auto result = unserialize_declaration(debugin);
        
        
        if (result != declaration)
            throw std::runtime_error(fmt::format("serialize_declaration({}) failed, unserialized result: {}", declaration, result));
    }
}

svo_declaration_t unserialize_declaration(std::istream& in)
{
    std::size_t elements_size = unserialize_uint<uint32_t>(in);
    
    svo_declaration_t declaration;
    
    for (std::size_t element_index = 0; element_index < elements_size; ++element_index)
    {
        
        auto element_name = unserialize_string(in);
        auto element_semantic_str = unserialize_string(in);
        auto element_type_str = unserialize_string(in);
        auto element_vector_width = unserialize_uint<uint32_t>(in);
        
        if (!validenum<svo_semantic_t>(element_semantic_str))
            throw std::runtime_error(fmt::format("Invalid semantic: {}", element_semantic_str));
        if (!validenum<svo_data_type_t>(element_type_str))
            throw std::runtime_error(fmt::format("Invalid data type: {}", element_type_str));
        
        svo_element_t element(element_name
            , fromstr<svo_semantic_t>(element_semantic_str)
            , fromstr<svo_data_type_t>(element_type_str)
            , element_vector_width);
        declaration.add(element);
    }
    return declaration;
}

void serialize_schema(std::ostream& out, const svo_schema_t& schema, bool debug)
{
    
    serialize_uint<uint32_t>(out, schema.size(), debug);
    for (const auto& declaration : schema)
    {
        serialize_declaration(out, declaration, debug);
    }
    
    
    if (debug)
    {
        std::ostringstream debugout;
        serialize_schema(debugout,schema);
        
        
        std::istringstream debugin(debugout.str());
        auto result = unserialize_schema(debugin);
        
        
        if (result != schema)
            throw std::runtime_error(fmt::format("serialize_declaration({}) failed, unserialized result: {}", schema, result));
    }
}

svo_schema_t unserialize_schema(std::istream& in)
{
    svo_schema_t schema;
    
    auto schema_size = unserialize_uint<uint32_t>(in);
    
    for (std::size_t declaration_index = 0; declaration_index < schema_size; ++declaration_index)
    {
        auto declaration = unserialize_declaration(in);
        schema.push_back(declaration);
    }
    return schema;
}

void serialize_buffer(std::ostream& out, const svo_cpu_buffer_t& buffer, bool debug)
{
    serialize_declaration(out, buffer.declaration(), debug);
    serialize_uint<uint32_t>(out, buffer.entries(), debug);
    serialize_buffer_data(out, buffer, debug);
    
    if (debug)
    {
        std::ostringstream debugout;
        serialize_buffer(debugout,buffer);
        
        
        std::istringstream debugin(debugout.str());
        auto result = unserialize_buffer(debugin);
        
        
        if (result->declaration() != buffer.declaration())
            throw std::runtime_error(fmt::format("serialize_buffer() failed, declaration: {} unserialized declaration: {}"
                                                    , buffer.declaration()
                                                    , result->declaration()));
        ///@TODO: compare buffer data
    }
}

std::unique_ptr<svo_cpu_buffer_t> unserialize_buffer(std::istream& in)
{
    auto declaration = unserialize_declaration(in);
    std::size_t entries = unserialize_uint<uint32_t>(in);
    
    std::unique_ptr<svo_cpu_buffer_t> buffer(new svo_cpu_buffer_t(declaration, entries));
    
    unserialize_buffer_data(in, *buffer, entries);
    
    return buffer;
}

void serialize_buffer_data(std::ostream& out, const svo_cpu_buffer_t& buffer, bool debug)
{
    out.write(reinterpret_cast<const char*>(buffer.rawdata()), buffer.bytes());
    
    
    if (debug)
    {
        
        ///todo
    }
}

void unserialize_buffer_data(std::istream& in, svo_cpu_buffer_t& buffer, std::size_t expected_entries)
{
    assert(buffer.entries() == expected_entries);
    
    in.read(reinterpret_cast<char*>(buffer.rawdata()), buffer.bytes());
    assert(in.gcount() == buffer.bytes());
}

void serialize_buffers(std::ostream& out, const svo_cpu_buffers_t& buffers, std::size_t expected_entries, bool debug)
{
    
    serialize_schema(out, buffers.schema(), debug);
    
    for (const auto& buffer : buffers.buffers())
    {
        assert(buffer.entries() == expected_entries);
        
        serialize_uint<uint32_t>(out, buffer.entries(), debug);
        serialize_buffer_data(out, buffer, debug);
        
    }
    
    if (debug)
    {
        ///TODO
    }
    
}



void unserialize_buffers(std::istream& in, svo_cpu_buffers_t& buffers, std::size_t expected_entries)
{
    assert(buffers.buffers().size() == 0);
    assert(buffers.schema().size() == 0);
    
    auto schema = unserialize_schema(in);
    
    assert(buffers.entries() == 0);
    
    for (const auto& declaration : schema)
    {
        auto& buffer = buffers.add_buffer(declaration, expected_entries);
        assert(buffers.entries() == expected_entries);
        assert(buffer.entries() == expected_entries);
    
    }
        
    for (auto& buffer : buffers.buffers())
    {
        std::size_t buffer_entries = unserialize_uint<uint32_t>(in);
        if (buffer_entries != expected_entries)
            throw std::runtime_error("buffer's entries does not match expected");
        unserialize_buffer_data(in, buffer, expected_entries);
    }
    
}

children_params_t unserialize_slice_child_info(std::istream& in, svo_slice_t* slice)
{
    assert(slice);
    assert(slice->children);

    auto children_count = unserialize_uint<uint32_t>(in);
    
    children_params_t children_params;
    for (uint32_t i = 0; i < children_count; ++i)
    {
        auto child_side = unserialize_uint<uint32_t>(in);
        uint32_t child_parent_vcurve_begin = unserialize_uint<uint32_t>(in);

        children_params.push_back(std::make_tuple(child_side, child_parent_vcurve_begin));
    }
    
    return children_params;
}


void slice_load_empty_children(svo_slice_t* slice, const children_params_t& children_params)
{
    for (auto params : children_params)
    {
        vside_t child_side;
        vcurve_t child_parent_vcurve_begin;
        std::tie(child_side, child_parent_vcurve_begin) = params;

        svo_slice_t* child_slice = svo_init_slice(slice->level + 1, child_side);
        child_slice->parent_slice = slice;
        child_slice->side = child_side;
        child_slice->parent_vcurve_begin = child_parent_vcurve_begin;
        
        child_slice->buffers->copy_schema(*slice->buffers);

        assert(child_slice->buffers->schema() == slice->buffers->schema());

        slice->children->push_back(child_slice);
        
        


        //if(auto error = svo_slice_sanity(child_slice, svo_sanity_t::enum_t(svo_sanity_t::minimal), 0 /*recurse*/, false))
        //{
        //    throw std::runtime_error(fmt::format("Cannot load slice, the following sanity error occured: {}", error));
        //}
    }
}

children_params_t unserialize_slice(std::istream& in, svo_slice_t* slice, bool load_empty_children)
{
    assert(slice);
    assert(slice->children);
    assert(slice->pos_data);
    assert(slice->buffers);
    
    assert(slice->children->size() == 0);
    assert(slice->pos_data->size() == 0);
    assert(slice->buffers->buffers().size() == 0);

    auto& pos_data = *slice->pos_data;
    auto& buffers = *slice->buffers;

    
    uint32_t format_version = unserialize_uint<uint32_t>(in);
    
    if(format_version != FORMAT_VERSION)
    {
        throw std::runtime_error(fmt::format("format_version != FORMAT_VERSION, {} != {}", format_version, FORMAT_VERSION));
    }


    slice->level = unserialize_uint<uint32_t>(in);
    slice->side = unserialize_uint<uint32_t>(in);
    slice->parent_vcurve_begin = unserialize_uint<uint32_t>(in);
    

    auto children_params = unserialize_slice_child_info(in, slice);

    if (load_empty_children)
    {
        slice_load_empty_children(slice, children_params);
    }

    auto data_size = unserialize_uint<uint32_t>(in);
    pos_data.reserve(data_size);
    
    ///unseralized position data
    for (uint32_t data_index = 0; data_index < data_size; ++data_index)
    {
        uint32_t vcurve = unserialize_uint<uint32_t>(in);
        pos_data.push_back( vcurve );
    }


    ///unserialize buffer data
    unserialize_buffers(in, buffers, data_size);
    

    //if(auto error = svo_slice_sanity(slice, svo_sanity_t::enum_t(svo_sanity_t::all & ~(svo_sanity_t::children)), 0 /*recurse*/))
    //{
    //    throw std::runtime_error(fmt::format("Cannot load slice, the following sanity error occured: {}", error));
    //}
    
    
    return children_params;
}




} //namespace svo
