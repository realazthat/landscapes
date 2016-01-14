


#include <cassert>
#include <iostream>
#include <cstring>
#include <exception>

#include "landscapes/svo_formatters.hpp"
#include "format.h"

namespace svo{

static inline std::size_t get_size_bytes(svo_data_type_t data_type)
{
    switch (data_type)
    {
        case(svo_data_type_t::BYTE):
        case(svo_data_type_t::UNSIGNED_BYTE):
            return 1;
        case(svo_data_type_t::SHORT):
        case(svo_data_type_t::UNSIGNED_SHORT):
            return 2;
        case(svo_data_type_t::INT):
        case(svo_data_type_t::UNSIGNED_INT):
        case(svo_data_type_t::FLOAT):
            return 4;
        case(svo_data_type_t::LONG):
        case(svo_data_type_t::UNSIGNED_LONG):
        case(svo_data_type_t::DOUBLE):
            return 8;
        default:{
            assert(false);
            return 0;
        }
    }
}


static inline const char* tostr(svo_data_type_t data_type)
{
    switch (data_type)
    {
        case(svo_data_type_t::BYTE):
            return "BYTE";
        case(svo_data_type_t::UNSIGNED_BYTE):
            return "UNSIGNED_BYTE";
        case(svo_data_type_t::SHORT):
            return "SHORT";
        case(svo_data_type_t::UNSIGNED_SHORT):
            return "UNSIGNED_SHORT";
        case(svo_data_type_t::INT):
            return "INT";
        case(svo_data_type_t::UNSIGNED_INT):
            return "UNSIGNED_INT";
        case(svo_data_type_t::LONG):
            return "LONG";
        case(svo_data_type_t::UNSIGNED_LONG):
            return "UNSIGNED_LONG";
        case(svo_data_type_t::FLOAT):
            return "FLOAT";
        case(svo_data_type_t::DOUBLE):
            return "DOUBLE";
    }
    assert(false);
    return "UNKNOWN";
}


static inline const char* tostr(svo_semantic_t semantic)
{
    switch (semantic)
    {
        case(svo_semantic_t::NONE):
            return "NONE";
        case(svo_semantic_t::COLOR):
            return "COLOR";
        case(svo_semantic_t::NORMAL):
            return "NORMAL";
    }
    assert(false);
    return "UNKNOWN";
}

static const std::map<std::string, svo_data_type_t> svo_data_type_mapping {
      {"BYTE", svo_data_type_t::BYTE}
    , {"UNSIGNED_BYTE", svo_data_type_t::UNSIGNED_BYTE}
    , {"SHORT", svo_data_type_t::SHORT}
    , {"UNSIGNED_SHORT", svo_data_type_t::UNSIGNED_SHORT}
    , {"INT", svo_data_type_t::INT}
    , {"UNSIGNED_INT", svo_data_type_t::UNSIGNED_INT}
    , {"LONG", svo_data_type_t::LONG}
    , {"UNSIGNED_LONG", svo_data_type_t::UNSIGNED_LONG}
    , {"FLOAT", svo_data_type_t::FLOAT}
    , {"DOUBLE", svo_data_type_t::DOUBLE}
};

static const std::map<std::string, svo_semantic_t> svo_semantic_mapping {
      {"NONE", svo_semantic_t::NONE}
    , {"COLOR", svo_semantic_t::COLOR}
    , {"NORMAL", svo_semantic_t::NORMAL}
};

template<>
inline bool validenum<svo_data_type_t>(const std::string& data_type)
{
    return svo_data_type_mapping.count(data_type);
}


template<>
inline bool validenum<svo_semantic_t>(const std::string& semantic)
{
    return svo_semantic_mapping.count(semantic);
}


template<>
inline svo_data_type_t fromstr<svo_data_type_t>(const std::string& data_type)
{
    auto w = svo_data_type_mapping.find(data_type);
    if (w == svo_data_type_mapping.end())
        throw std::runtime_error(fmt::format("Unknown svo_data_type_t {}", quote(data_type)));

    return w->second;
}

template<>
inline svo_semantic_t fromstr<svo_semantic_t>(const std::string& semantic)
{
    auto w = svo_semantic_mapping.find(semantic);
    if (w == svo_semantic_mapping.end())
        throw std::runtime_error(fmt::format("Unknown svo_data_type_t {}", quote(semantic)));

    return w->second;

}



////////////////////////////////////////////////////////////////////////////////


inline svo_element_t::svo_element_t(const std::string& name, svo_semantic_t semantic, svo_data_type_t data_type, std::size_t count)
    : m_name(name), m_semantic(semantic), m_data_type(data_type), m_count(count), m_bytes(count*get_size_bytes(data_type))
{

}

inline svo_data_type_t svo_element_t::type() const
{
    return m_data_type;
}

inline std::size_t svo_element_t::count() const
{
    return m_count;
}

inline std::size_t svo_element_t::bytes() const
{
    return m_bytes;
}

inline std::size_t svo_element_t::type_bytes() const
{
    return get_size_bytes(m_data_type);
}

inline svo_semantic_t svo_element_t::semantic() const
{
    return m_semantic;
}

inline const std::string& svo_element_t::name() const
{
    return m_name;
}

inline bool svo_element_t::operator==(const svo_element_t& other) const
{
    return other.semantic() == semantic()
        && other.count() == count()
        && other.type() == type()
        && other.name() == name();
}

inline bool svo_element_t::operator!=(const svo_element_t& other) const
{
    return !(*this == other);
}



inline void svo_element_t::assert_invariants() const
{
    assert(m_name.size() > 0);
    assert(m_count > 0);
    assert(m_bytes == m_count*get_size_bytes(m_data_type));
}







////////////////////////////////////////////////////////////////////////////////



inline svo_declaration_t::svo_declaration_t()
    : m_stride(0)
{
    assert_invariants();
}

inline void svo_declaration_t::add(const svo_element_t& element)
{
    assert_invariants();
    
    if (m_names.count(element.name()) > 0)
        throw std::runtime_error(fmt::format("element by name {} already contained in declaration"
                                            , element.name()));
    
    std::size_t element_offset = m_offsets.size() > 0    ?
                                         m_offsets.back() + m_elements.back().bytes()
                                    :    0;
    m_elements.push_back(element);
    m_offsets.push_back(element_offset);
    m_names.insert(element.name());
    
    m_stride += element.bytes();
    
    assert_invariants();
}

inline const svo_declaration_t::elements_list_t& svo_declaration_t::elements() const
{
    assert_invariants();
    return m_elements;
}

inline std::size_t svo_declaration_t::offset(std::size_t element_index) const
{
    assert_invariants();
    if (!(element_index < m_elements.size()))
        throw std::runtime_error(fmt::format("element_index is out of bounds, element_index: {}, m_elements.size(): {}"
                                    , element_index, m_elements.size()));
    
    return m_offsets[element_index];
}

inline std::size_t svo_declaration_t::stride() const
{
    assert_invariants();
    return m_stride;
}

inline bool svo_declaration_t::operator==(const svo_declaration_t& other) const
{
    assert_invariants();
    return m_elements == other.m_elements;
}

inline bool svo_declaration_t::operator!=(const svo_declaration_t& other) const
{
    assert_invariants();
    return !(*this == other);
}

inline void svo_declaration_t::assert_invariants() const
{
#ifndef NDEBUG
    ///m_elements
    {
        ///recurse
        for (const auto& element : m_elements)
        {
            element.assert_invariants();
        }
        
        ///check uniqueness of names
        {
            std::set<std::string> names;
            for (const auto& element : m_elements)
            {
                assert(names.count(element.name()) == 0 && "declaration cannot have two elements with the same name");
                names.insert(element.name());
            }
            
            assert(m_names == names);
        }
    }
    ///m_stride
    {
        std::size_t expected_stride = 0;
        for (const auto& element : m_elements)
        {
            expected_stride += element.bytes();
        }
        assert(m_stride == expected_stride);
    }
    
    ///m_offsets
    {
        assert(m_offsets.size() == m_elements.size());
        
        std::size_t expected_offset = 0;
        for (std::size_t element_index = 0; element_index < m_elements.size(); ++element_index)
        {
            const auto& element = m_elements[element_index];
            assert(m_offsets[element_index] == expected_offset);
            expected_offset += element.bytes();
        }
        assert(expected_offset == m_stride);
    }
#endif
}


////////////////////////////////////////////////////////////////////////////////



template<typename svo_buffer_t>
inline
const svo_declaration_t&
svo_base_buffer_t<svo_buffer_t>::
declaration() const
{
    return m_declaration;
}


template<typename svo_buffer_t>
inline
std::size_t
svo_base_buffer_t<svo_buffer_t>::
entries() const
{
    return m_entries;
}

template<typename svo_buffer_t>
inline std::size_t
svo_base_buffer_t<svo_buffer_t>::
bytes() const
{
    return entries() * stride();
}

template<typename svo_buffer_t>
inline std::size_t
svo_base_buffer_t<svo_buffer_t>::
stride() const
{
    return m_declaration.stride();
}

template<typename svo_buffer_t>
const uint8_t*
svo_base_buffer_t<svo_buffer_t>::
rawdata() const
{
    self().assert_invariants();
    return m_rawdata;
}


template<typename svo_buffer_t>
uint8_t*
svo_base_buffer_t<svo_buffer_t>::
rawdata()
{
    self().assert_invariants();
    return m_rawdata;
}

template<typename svo_buffer_t>
const svo_buffer_t&
svo_base_buffer_t<svo_buffer_t>::
self() const
{
    ///note, do not assert_variants() here, because it can result in recursion.
    return *static_cast<const svo_buffer_t*>(this);
}

template<typename svo_buffer_t>
svo_buffer_t&
svo_base_buffer_t<svo_buffer_t>::
self()
{
    ///note, do not assert_variants() here, because it can result in recursion.
    return *static_cast<svo_buffer_t*>(this);
}

template<typename svo_buffer_t>
void
svo_base_buffer_t<svo_buffer_t>::
assert_invariants() const
{
    assert(m_rawdata || (m_entries*m_declaration.stride()) == 0);
    m_declaration.assert_invariants();
}

namespace detail{


/**
 * Convert and copy the data from one element of buffer to an element of the other, given the raw pointers and
 * template data type of *only* the source element.
 *
 * Note that the element ptrs are offset by the element's offset, and therefore misaligned with the buffer's entire
 * data by a small amount.
 */
template<typename src_type>
void convert_and_copy_element_switch(uint8_t* dst_begin, uint8_t* dst_end, const uint8_t* src_begin, const uint8_t* src_end
        , std::size_t copy_entries
        , std::size_t dst_stride, std::size_t src_stride
        , const svo_element_t& dst_element, const svo_element_t& src_element);

/**
 * Convert and copy the data from one element of buffer to an element of the other, given the raw pointers and
 * template data types of the two buffers elements.
 *
 * Note that the element ptrs are offset by the element's offset, and therefore misaligned with the buffer's entire
 * data by a small amount.
 */
template<typename dst_type, typename src_type>
void convert_and_copy_element(uint8_t* dst_begin, uint8_t* dst_end, const uint8_t* src_begin, const uint8_t* src_end
        , std::size_t copy_entries
        , std::size_t dst_stride, std::size_t src_stride
        , std::size_t vector_width);

/**
 * Convert and copy the data from one element of buffer to an element of the other, given the buffers,
 * and specifying which elements, but *without* the compile-time data types of the two buffers elements.
 */
template<typename dst_buffer_t, typename src_buffer_t>
void
copy_element_from_buffer(dst_buffer_t& dst_buffer, const src_buffer_t& src_buffer
                        , std::size_t src_element_index
                        , std::size_t dst_element_index
                        , std::size_t src_start, std::size_t copy_entries
                        , std::size_t dst_start);



template<typename dst_buffer_t, typename src_buffer_t>
void
copy_from_buffer(dst_buffer_t& dst_buffer, const src_buffer_t& src_buffer
                , std::size_t src_start, std::size_t copy_entries
                , std::size_t dst_start)
{
    src_buffer.assert_invariants();
    dst_buffer.assert_invariants();

    if (copy_entries == std::size_t(-1))
        copy_entries = src_buffer.entries();



    if (src_start > src_buffer.entries() || src_start + copy_entries > src_buffer.entries())
        throw std::runtime_error(fmt::format("src data is not a valid range inside the source buffer, src_start: {}, copy_entries: {}, src_buffer.entires: {}"
                                            , src_start, copy_entries, src_buffer.entries()));


    if (dst_start > dst_buffer.entries() || dst_start + copy_entries > dst_buffer.entries())
        throw std::runtime_error(fmt::format("dst data is not a valid range inside the dst buffer, dst_start: {}, copy_entries: {}, dst_buffer.entires: {}"
                                            , dst_start, copy_entries, dst_buffer.entries()));

    const auto& src_elements = dst_buffer.declaration().elements();
    const auto& dst_elements = dst_buffer.declaration().elements();
    
    if (dst_elements.size() != src_elements.size())
        throw std::runtime_error(fmt::format("the number of elements do not match, cannot copy from buffer"
                                             ", dst_buffer.declaration(): {}, src_buffer.declaration(): {}"
                                            , dst_buffer.declaration(), src_buffer.declaration()));



    ///if the buffer is all the same types
    if (dst_buffer.declaration() == src_buffer.declaration())
    {
        ///just do a raw copy.

        const uint8_t* src_data = src_buffer.rawdata();
        uint8_t* dst_data = dst_buffer.rawdata();
        
        


        const uint8_t* src_ptr = src_data + src_start*src_buffer.stride();
        uint8_t* dst_ptr = dst_data + dst_start*dst_buffer.stride();
        std::memcpy(dst_ptr, src_ptr, copy_entries*src_buffer.stride());

        
        src_buffer.assert_invariants();
        dst_buffer.assert_invariants();
        return;
    }

    ///else, the buffer's have disparate types.


    assert(src_elements.size() == dst_elements.size());


    ///copy each element separately.
    for (std::size_t element_index = 0; element_index < src_elements.size(); ++element_index)
    {
        copy_element_from_buffer(dst_buffer, src_buffer, element_index, element_index, src_start, copy_entries, dst_start);
    }


    src_buffer.assert_invariants();
    dst_buffer.assert_invariants();
}







template<typename dst_buffer_t, typename src_buffer_t>
void
copy_element_from_buffer(dst_buffer_t& dst_buffer, const src_buffer_t& src_buffer
                        , std::size_t src_element_index
                        , std::size_t dst_element_index
                        , std::size_t src_start, std::size_t copy_entries
                        , std::size_t dst_start)
{
    src_buffer.assert_invariants();
    dst_buffer.assert_invariants();

    if (copy_entries == std::size_t(-1))
        copy_entries = src_buffer.entries();


    if (src_element_index < src_buffer.declaration().elements().size())
        throw std::runtime_error(fmt::format("invalid element index, src_element_index: {}, elements().size(): {}"
                                            , src_element_index, src_buffer.declaration().elements().size()));
    if (dst_element_index < dst_buffer.declaration().elements().size())
        throw std::runtime_error(fmt::format("invalid element index, dst_element_index: {}, elements().size(): {}"
                                            , dst_element_index, dst_buffer.declaration().elements().size()));


    if (src_start > src_buffer.entries() || src_start + copy_entries > src_buffer.entries())
        throw std::runtime_error(fmt::format("src data is not a valid range inside the source buffer, src_start: {}, copy_entries: {}, src_buffer.entires: {}"
                                            , src_start, copy_entries, src_buffer.entries()));


    if (dst_start > dst_buffer.entries() || dst_start + copy_entries > dst_buffer.entries())
        throw std::runtime_error(fmt::format("dst data is not a valid range inside the dst buffer, dst_start: {}, copy_entries: {}, dst_buffer.entires: {}"
                                            , dst_start, copy_entries, dst_buffer.entries()));


    const auto& src_element = src_buffer.declaration().elements()[src_element_index];
    const auto& dst_element = dst_buffer.declaration().elements()[dst_element_index];

    if (src_element.count() == dst_element.count())
        throw std::runtime_error(fmt::format("elements are not convertible"
                                             "src_element: {}, dst_element: {}"
                                            , src_element, dst_element));


    std::size_t src_element_offset = src_buffer.declaration().offset(src_element_index);
    std::size_t dst_element_offset = dst_buffer.declaration().offset(dst_element_index);
    
    std::size_t src_stride = src_buffer.stride();
    std::size_t dst_stride = dst_buffer.stride();

    std::size_t src_padding = src_stride - src_element_offset;
    std::size_t dst_padding = dst_stride - dst_element_offset;

    const uint8_t* src_data = src_buffer.rawdata();
    uint8_t* dst_data = dst_buffer.rawdata();



    const uint8_t* src_begin = src_data + src_start*src_stride + src_element_offset;
    uint8_t* dst_begin = dst_data + dst_start*dst_stride + dst_element_offset;
    
    const uint8_t* src_end = src_begin + src_stride*copy_entries;
    uint8_t* dst_end = dst_begin + dst_stride*copy_entries;
    
    ///if its the same type, it can get special handling
    if (src_element.type() == dst_element.type())
    {
        assert(src_element.bytes() == dst_element.bytes());

        if (src_padding == 0 && dst_padding == 0)
        {
            ///if there are no holes,
            ///copy it all in one shot.
            std::memcpy(dst_begin, src_begin, copy_entries*src_element.bytes());
            return;
        }

        ///copy it over, raw element by raw element
        uint8_t* dst_ptr = dst_begin;
        const uint8_t* src_ptr = src_begin;
        for (std::size_t i = 0; i < copy_entries; ++i)
        {
            assert(src_ptr >= src_begin);
            assert(src_ptr < src_end);
            assert(dst_ptr >= dst_begin);
            assert(dst_ptr < dst_end);
            
            std::memcpy(dst_begin, src_ptr, src_element.bytes());
            dst_ptr += dst_stride;
            src_ptr += src_stride;
        }
        return;
    }


    ///copy it over, element by element, and casting each vector subelements.
    if (src_element.type() == svo_data_type_t::BYTE || src_element.type() == svo_data_type_t::UNSIGNED_BYTE)
        convert_and_copy_element_switch<uint8_t>(dst_begin, dst_end, src_begin, src_end, copy_entries, dst_stride, src_stride, dst_element, src_element);
    else if (src_element.type() == svo_data_type_t::SHORT || src_element.type() == svo_data_type_t::UNSIGNED_SHORT)
        convert_and_copy_element_switch<uint16_t>(dst_begin, dst_end, src_begin, src_end, copy_entries, dst_stride, src_stride, dst_element, src_element);
    else if (src_element.type() == svo_data_type_t::INT || src_element.type() == svo_data_type_t::UNSIGNED_INT)
        convert_and_copy_element_switch<uint32_t>(dst_begin, dst_end, src_begin, src_end, copy_entries, dst_stride, src_stride, dst_element, src_element);
    else if (src_element.type() == svo_data_type_t::LONG || src_element.type() == svo_data_type_t::UNSIGNED_LONG)
        convert_and_copy_element_switch<uint64_t>(dst_begin, dst_end, src_begin, src_end, copy_entries, dst_stride, src_stride, dst_element, src_element);
    else if (src_element.type() == svo_data_type_t::FLOAT)
        convert_and_copy_element_switch<float>(dst_begin, dst_end, src_begin, src_end, copy_entries, dst_stride, src_stride, dst_element, src_element);
    else if (src_element.type() == svo_data_type_t::DOUBLE)
        convert_and_copy_element_switch<double>(dst_begin, dst_end, src_begin, src_end, copy_entries, dst_stride, src_stride, dst_element, src_element);
    else
        throw std::runtime_error(fmt::format("Unknown conversion, src_element.type(): {}, dst_element.type(): {}"
                                            , src_element.type(), dst_element.type()));


    src_buffer.assert_invariants();
    dst_buffer.assert_invariants();
}




} //namespace detail



inline void
svo_cpu_buffer_t::
assert_invariants() const
{
    super_type::assert_invariants();

    assert(m_data.size() == m_entries*m_declaration.stride());
    assert(m_rawdata == reinterpret_cast<const uint8_t*>(m_data.data()));


}
////////////////////////////////////////////////////////////////////////////////


template<typename svo_buffer_t, typename svo_buffers_t>
typename svo_base_buffers_t<svo_buffer_t, svo_buffers_t>::buffer_list_t&
svo_base_buffers_t<svo_buffer_t, svo_buffers_t>::buffers()
{
    self().assert_invariants();
    return m_buffers;
}

template<typename svo_buffer_t, typename svo_buffers_t>
svo_base_buffers_t<svo_buffer_t, svo_buffers_t>::svo_base_buffers_t()
{
    self().assert_invariants();

}


template<typename svo_buffer_t, typename svo_buffers_t>
const typename svo_base_buffers_t<svo_buffer_t, svo_buffers_t>::buffer_list_t&
svo_base_buffers_t<svo_buffer_t, svo_buffers_t>::
buffers() const
{
    self().assert_invariants();
    return m_buffers;
}

template<typename svo_buffer_t, typename svo_buffers_t>
const svo_schema_t&
svo_base_buffers_t<svo_buffer_t, svo_buffers_t>::
schema() const
{
    self().assert_invariants();
    return m_schema;
}


template<typename svo_buffer_t, typename svo_buffers_t>
bool
svo_base_buffers_t<svo_buffer_t, svo_buffers_t>::
has_schema() const
{
    self().assert_invariants();
    return m_schema.size() > 0;
}


template<typename svo_buffer_t, typename svo_buffers_t>
std::size_t
svo_base_buffers_t<svo_buffer_t, svo_buffers_t>::
entries() const
{
    self().assert_invariants();
    if (m_buffers.size() == 0)
        return 0;
    return m_buffers[0].entries();
}



template<typename svo_buffer_t, typename svo_buffers_t>
bool
svo_base_buffers_t<svo_buffer_t, svo_buffers_t>::
has_named_element(const std::string& element_name) const
{
    return m_buffer_element_mappings.count(element_name) > 0;
}

template<typename svo_buffer_t, typename svo_buffers_t>
const detail::svo_element_view_t<const uint8_t, const svo_buffer_t>
svo_base_buffers_t<svo_buffer_t, svo_buffers_t>::
get_element_view(const std::string& element_name) const
{
    self().assert_invariants();

    auto w = m_buffer_element_mappings.find(element_name);

    if (w == m_buffer_element_mappings.end())
        throw std::runtime_error(fmt::format("buffer does not contain element {}", quote(element_name)));

    std::size_t buffer_index, element_index;
    std::tie(buffer_index, element_index) = w->second;

    assert(buffer_index < m_buffers.size());

    auto& buffer = m_buffers[buffer_index];

    buffer.assert_invariants();
    assert(element_index < buffer.declaration().elements().size());

    typedef detail::svo_element_view_t<const uint8_t, const svo_buffer_t> result_t;
    return result_t( &buffer, element_index, buffer.rawdata(), buffer.stride(), buffer.entries() );
}


template<typename svo_buffer_t, typename svo_buffers_t>
detail::svo_element_view_t<uint8_t, svo_buffer_t>
svo_base_buffers_t<svo_buffer_t, svo_buffers_t>::
get_element_view(const std::string& element_name)
{
    self().assert_invariants();

    auto w = m_buffer_element_mappings.find(element_name);

    if (w == m_buffer_element_mappings.end())
        throw std::runtime_error(fmt::format("buffer does not contain element {}", quote(element_name)));

    std::size_t buffer_index, element_index;
    std::tie(buffer_index, element_index) = w->second;

    assert(buffer_index < m_buffers.size());

    auto& buffer = m_buffers[buffer_index];

    buffer.assert_invariants();
    assert(element_index < buffer.declaration().elements().size());

    typedef detail::svo_element_view_t<uint8_t, svo_buffer_t> result_t;
    return result_t( &buffer, element_index, buffer.rawdata(), buffer.stride(), buffer.entries() );
}


template<typename svo_buffer_t, typename svo_buffers_t>
void
svo_base_buffers_t<svo_buffer_t, svo_buffers_t>::
resize(std::size_t new_entries)
{
    self().assert_invariants();
    for (auto& buffer : m_buffers)
    {
        buffer.resize(new_entries);
    }

    assert(entries() == new_entries);
    self().assert_invariants();
}


template<typename svo_buffer_t, typename svo_buffers_t>
svo_buffers_t&
svo_base_buffers_t<svo_buffer_t, svo_buffers_t>::
self()
{
    ///note, do not assert_variants() here, because it can result in recursion.
    return *static_cast<svo_buffers_t*>(this);
}




template<typename svo_buffer_t, typename svo_buffers_t>
const svo_buffers_t&
svo_base_buffers_t<svo_buffer_t, svo_buffers_t>::
self() const
{
    ///note, do not assert_variants() here, because it can result in recursion.
    return *static_cast<const svo_buffers_t*>(this);
}




template<typename svo_buffer_t, typename svo_buffers_t>
void
svo_base_buffers_t<svo_buffer_t, svo_buffers_t>::
assert_invariants() const
{
#ifndef NDEBUG
    assert(m_schema.size() == m_buffers.size());
    
    
    for (std::size_t buffer_index = 0; buffer_index < m_buffers.size(); ++buffer_index)
        m_buffers[buffer_index].assert_invariants();

    for (std::size_t buffer_index = 0; buffer_index < m_buffers.size(); ++buffer_index)
        assert(m_schema[buffer_index] == m_buffers[buffer_index].declaration());
    for (std::size_t buffer_index = 0; buffer_index < m_buffers.size(); ++buffer_index)
        assert(m_buffers[buffer_index].entries() == m_buffers[0].entries());

    for (const auto& name__buffer_index_pair : m_buffer_element_mappings)
    {
        const auto& name = name__buffer_index_pair.first;
        std::size_t buffer_index; std::size_t element_index;
        std::tie(buffer_index, element_index) = name__buffer_index_pair.second;
        assert(buffer_index < m_buffers.size());

        const auto& buffer = m_buffers[buffer_index];

        const auto& declaration = buffer.declaration();
        declaration.assert_invariants();
        
        const auto& elements = declaration.elements();
        const auto& element = elements[element_index];
        element.assert_invariants();
        
        assert(element.name() == name && "didn't find expected element name in any element in the buffer");

    }

    for (std::size_t buffer_index = 0; buffer_index < m_buffers.size(); ++buffer_index)
    {
        const auto& buffer = m_buffers[buffer_index];
        const auto& declaration = buffer.declaration();
        const auto& elements = declaration.elements();

        for (std::size_t element_index = 0; element_index < elements.size(); ++element_index)
        {
            const auto& element = elements[element_index];

            auto w = m_buffer_element_mappings.find(element.name());

            assert(w != m_buffer_element_mappings.end() && "buffers contains a buffer-element that is not mapped");

            std::size_t expected_buffer_index; std::size_t expected_element_index;
            std::tie(expected_buffer_index, expected_element_index) = w->second;
            
            assert(expected_buffer_index == buffer_index);
            assert(expected_element_index == element_index);
        }
    }
#endif //NDEBUG
}

////////////////////////////////////////////////////////////////////////////////

namespace detail{
    template<typename byte_t, typename svo_buffer_t>
    svo_element_view_t<byte_t, svo_buffer_t>::
    svo_element_view_t(svo_buffer_t* buffer, std::size_t element_index, byte_t* ptr, std::size_t stride, std::size_t entries)
        : buffer(buffer)
        , element_index(element_index)
        , ptr(ptr)
        , stride(stride)
        , entries(entries)
    {
        assert_invariants();
    }

    template<typename byte_t, typename svo_buffer_t>
    template<typename T>
    inline T&
    svo_element_view_t<byte_t, svo_buffer_t>::
    get(std::size_t entry_index)
    {
        assert_invariants();
        assert(entry_index < entries);

        byte_t* element_ptr = ptr + entry_index*stride;

        return *reinterpret_cast<T*>(element_ptr);
    }

    template<typename byte_t, typename svo_buffer_t>
    template<typename T>
    inline const T&
    svo_element_view_t<byte_t, svo_buffer_t>::
    get(std::size_t entry_index) const
    {
        assert_invariants();
        assert(entry_index < entries);

        byte_t* element_ptr = ptr + entry_index*stride;

        return *reinterpret_cast<const T*>(element_ptr);
    }

    template<typename byte_t, typename svo_buffer_t>
    void
    svo_element_view_t<byte_t, svo_buffer_t>::assert_invariants() const
    {
        assert(buffer);
        buffer->assert_invariants();
        assert(ptr || entries == 0);
        
        const auto& declaration = buffer->declaration();
        const auto& elements = declaration.elements();
        assert(element_index < elements.size());
        
        assert(ptr == buffer->rawdata() + declaration.offset(element_index));
        assert(stride == buffer->stride());
        assert(entries == buffer->entries());


    }





} //namespace detail






























































////////////////////////////////////////////////////////////////////////////////

namespace detail{

template<typename src_type>
void convert_and_copy_element_switch(uint8_t* dst_begin, uint8_t* dst_end, const uint8_t* src_begin, const uint8_t* src_end
        , std::size_t copy_entries
        , std::size_t dst_stride, std::size_t src_stride
        , const svo_element_t& dst_element, const svo_element_t& src_element)
{
    assert(copy_entries != std::size_t(-1));
    assert(dst_element.count() == src_element.count());
    
    std::size_t vector_width = src_element.count();
    
    switch(dst_element.type())
    {
        case(svo_data_type_t::BYTE):
        case(svo_data_type_t::UNSIGNED_BYTE):
            convert_and_copy_element<uint8_t, src_type>(dst_begin, dst_end, src_begin, src_end, copy_entries, dst_stride, src_stride, vector_width);
            break;
        case(svo_data_type_t::SHORT):
        case(svo_data_type_t::UNSIGNED_SHORT):
            convert_and_copy_element<uint16_t, src_type>(dst_begin, dst_end, src_begin, src_end, copy_entries, dst_stride, src_stride, vector_width);
            break;
        case(svo_data_type_t::INT):
        case(svo_data_type_t::UNSIGNED_INT):
            convert_and_copy_element<uint32_t, src_type>(dst_begin, dst_end, src_begin, src_end, copy_entries, dst_stride, src_stride, vector_width);
            break;
        case(svo_data_type_t::LONG):
        case(svo_data_type_t::UNSIGNED_LONG):
            convert_and_copy_element<uint64_t, src_type>(dst_begin, dst_end, src_begin, src_end, copy_entries, dst_stride, src_stride, vector_width);
            break;
        case(svo_data_type_t::FLOAT):
            convert_and_copy_element<float, src_type>(dst_begin, dst_end, src_begin, src_end, copy_entries, dst_stride, src_stride, vector_width);
            break;
        case(svo_data_type_t::DOUBLE):
            convert_and_copy_element<double, src_type>(dst_begin, dst_end, src_begin, src_end, copy_entries, dst_stride, src_stride, vector_width);
            break;
        default:
            throw std::runtime_error(fmt::format("Unknown conversion, src_element.type(): {}, dst_element.type(): {}"
                                                , src_element.type(), dst_element.type()));
    }
}

////////////////////////////////////////////////////////////////////////////////
template<typename dst_type, typename src_type>
void convert_and_copy_element(uint8_t* dst_begin, uint8_t* dst_end, const uint8_t* src_begin, const uint8_t* src_end
        , std::size_t copy_entries
        , std::size_t dst_stride, std::size_t src_stride
        , std::size_t vector_width)
{
    assert(copy_entries != std::size_t(-1));
    assert(dst_begin);
    assert(dst_end);
    assert(src_begin);
    assert(src_end);
    assert(dst_end >= dst_begin);
    assert(src_end >= src_begin);
    assert(sizeof(dst_type)*vector_width <= dst_stride);
    assert(sizeof(src_type)*vector_width <= src_stride);


    uint8_t* dst_ptr = dst_begin;
    const uint8_t* src_ptr = src_begin;

    std::size_t src_element_bytes = sizeof(src_type);
    std::size_t dst_element_bytes = sizeof(dst_type);
    std::size_t src_padding = src_stride - src_element_bytes;
    std::size_t dst_padding = dst_stride - dst_element_bytes;



    ///copy it over, element by element, and casting each vector subelements.
    for (std::size_t i = 0; i < copy_entries; ++i)
    {

        for (std::size_t vector_subelement = 0; vector_subelement < vector_width; ++vector_subelement)
        {
            assert(dst_ptr >= dst_begin);
            assert(src_ptr >= src_begin);
            assert(dst_ptr < dst_end);
            assert(src_ptr < src_end);
            
            src_type src_value = *reinterpret_cast<const src_type*>(src_ptr);
            dst_type& dst_value = *reinterpret_cast<dst_type*>(dst_ptr);

            
            ///conversion
            dst_value = src_value;

            src_ptr += sizeof(src_type);
            dst_ptr += sizeof(dst_type);
        }

        dst_ptr += src_padding;
        src_ptr += dst_padding;
    }
}

} //namespace detail


} //namespace svo

