
#include "landscapes/svo_serialization.v1.hpp"
#include "landscapes/svo_tree.hpp"
#include "landscapes/svo_tree.sanity.hpp"

#include <iostream>

namespace svo{


static const int FORMAT_VERSION = 1;

template<typename T>
static inline void serialize_uint(std::ostream& out, T v);
void serialize_string(std::ostream& out, const std::string& v);

template<typename T>
static inline T unserialize_uint(std::istream& in);
std::string unserialize_string(std::istream& in);










void serialize_string(std::ostream& out, const std::string& v)
{
    serialize_uint<uint32_t>(out, v.size());
    out.write(v.data(), v.size());
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
static inline void serialize_uint(std::ostream& out, T v)
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

void svo_serialize_slice_child_info(std::ostream& out, const svo_slice_t* slice)
{
    assert(slice);
    assert(slice->children);

    
    
    uint32_t children_count = slice->children->size();

    serialize_uint<uint32_t>(out, uint32_t(slice->side));
    serialize_uint<uint32_t>(out, uint32_t(children_count));

    for (uint32_t i = 0; i < children_count; ++i)
    {
        svo_slice_t* child = (*slice->children)[i];
        assert(child);
        serialize_uint<uint32_t>(out, uint32_t(child->side));
        serialize_uint<uint32_t>(out, uint32_t(child->parent_vcurve_begin));
    }
}



void svo_serialize_slice(std::ostream& out, const svo_slice_t* slice)
{
    assert(slice);
    assert(slice->children);
    assert(slice->pos_data);
    assert(slice->buffers);

    serialize_uint<uint32_t>(out, uint32_t(FORMAT_VERSION));
    
    svo_serialize_slice_child_info(out, slice);

    const auto& pos_data = *slice->pos_data;
    const auto& buffers = *slice->buffers;

    serialize_uint<uint32_t>(out, uint32_t(pos_data.size()));

    ///output voxel coordinates
    for (std::size_t i = 0; i < pos_data.size(); ++i)
    {
        vcurve_t vcurve = pos_data[i];
        serialize_uint<uint32_t>(out, uint32_t(vcurve));
    }

    
    svo_serialize_buffers(out, buffers, pos_data.size());
    
}

void svo_serialize_buffers_schema_declaration(std::ostream& out, const svo_declaration_t& declaration)
{
    serialize_uint<uint32_t>(out, declaration.elements().size());
    
    for (const auto& element : declaration.elements())
    {
        serialize_string(out, element.name());
        serialize_string(out, tostr(element.semantic()));
        serialize_string(out, tostr(element.type()));
        serialize_uint<uint32_t>(out, element.count());
    }
}

svo_declaration_t svo_unserialize_buffers_schema_declaration(std::istream& in)
{
    std::size_t elements_size =unserialize_uint<uint32_t>(in);
    
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

void svo_serialize_buffers_schema(std::ostream& out, const svo_schema_t& schema)
{
    
    serialize_uint<uint32_t>(out, schema.size());
    for (const auto& declaration : schema)
    {
        svo_serialize_buffers_schema_declaration(out, declaration);
    }
}

svo_schema_t svo_unserialize_buffers_schema(std::istream& in)
{
    svo_schema_t schema;
    
    auto schema_size = unserialize_uint<uint32_t>(in);
    
    for (std::size_t declaration_index = 0; declaration_index < schema_size; ++declaration_index)
    {
        auto declaration = svo_unserialize_buffers_schema_declaration(in);
        schema.push_back(declaration);
    }
    return schema;
}

void svo_serialize_buffers(std::ostream& out, const svo_cpu_buffers_t& buffers, std::size_t expected_entries)
{
    
    svo_serialize_buffers_schema(out, buffers.schema());
    
    for (const auto& buffer : buffers.buffers())
    {
        assert(buffer.entries() == expected_entries);
        
        serialize_uint<uint32_t>(out, buffer.entries());
        out.write(reinterpret_cast<const char*>(buffer.rawdata()), buffer.bytes());
        
    }
    
}


void svo_unserialize_buffers(std::istream& in, svo_cpu_buffers_t& buffers, std::size_t data_size)
{
    assert(buffers.buffers().size() == 0);
    
    auto schema = svo_unserialize_buffers_schema(in);
    
    assert(buffers.entries() == 0);
    
    for (const auto& declaration : schema)
    {
        auto& buffer = buffers.add_buffer(declaration, data_size);
        assert(buffers.entries() == data_size);
        assert(buffer.entries() == data_size);
    
    }
        
    for (auto& buffer : buffers.buffers())
    {
        in.read(reinterpret_cast<char*>(buffer.rawdata()), buffer.bytes());
        assert(in.gcount() == buffer.bytes());
    }
    
}

children_params_t svo_unserialize_slice_child_info(std::istream& in, svo_slice_t* slice)
{
    assert(slice);
    assert(slice->children);

    auto slice_side = unserialize_uint<uint32_t>(in);
    auto children_count = unserialize_uint<uint32_t>(in);
    
    slice->side = slice_side;

    children_params_t children_params;
    for (uint32_t i = 0; i < children_count; ++i)
    {
        auto child_side = unserialize_uint<uint32_t>(in);
        uint32_t child_parent_vcurve_begin = unserialize_uint<uint32_t>(in);

        children_params.push_back(std::make_tuple(child_side, child_parent_vcurve_begin));
    }
    
    return children_params;
}


void svo_unserialize_slice_load_empty_children(svo_slice_t* slice, const children_params_t& children_params)
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

        slice->children->push_back(child_slice);


        ///todo: make this an exception or return error code.
        if(auto error = svo_slice_sanity(child_slice))
        {
            std::cerr << error << std::endl;
            assert(false && "sanity fail");
        }
    }
}

children_params_t svo_unserialize_slice(std::istream& in, svo_slice_t* slice, bool load_empty_children)
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
    
    ///todo: make this a runtime error
    assert(format_version == FORMAT_VERSION);


    auto children_params = svo_unserialize_slice_child_info(in, slice);

    if (load_empty_children)
    {
        svo_unserialize_slice_load_empty_children(slice, children_params);
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
    svo_unserialize_buffers(in, buffers, data_size);
    

    ///todo: make this an exception or return error code.
    if(auto error = svo_slice_sanity(slice))
    {
        std::cerr << error << std::endl;
        assert(false && "sanity fail");
    }

    return children_params;
}




} //namespace svo
