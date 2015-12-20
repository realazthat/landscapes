
#include "landscapes/svo_buffer.hpp"

#include "landscapes/svo_tree.hpp"

#include <exception>
#include "format.h"
#include "landscapes/unused.h"

namespace svo{

template<typename svo_buffer_t>
svo_base_buffer_t<svo_buffer_t>::
svo_base_buffer_t(const svo_declaration_t& declaration, std::size_t initial_entries, uint8_t* rawdata)
    : m_declaration(declaration)
    , m_entries(initial_entries)
    , m_rawdata(rawdata)
{}

namespace detail{
    template<typename dst_buffer_t, typename src_buffer_t>
    void
    append_buffer(dst_buffer_t& dst_buffer, const src_buffer_t& src_buffer)
    {
        dst_buffer.assert_invariants();
        src_buffer.assert_invariants();
        if (src_buffer.declaration() != dst_buffer.declaration())
            throw std::runtime_error("Cannot append buffer; declarations do not match");

        std::size_t new_size = dst_buffer.entries() + src_buffer.entries();

        std::size_t dst_start = dst_buffer.entries();
        
        dst_buffer.resize(new_size);

        svo::detail::copy_from_buffer(dst_buffer, src_buffer, 0/*src_start*/, src_buffer.entries() /*copy_entries*/, dst_start /*dst_start*/);
    }
}

template<typename svo_buffer_t>
void
svo_base_buffer_t<svo_buffer_t>::
append_buffer(const svo_cpu_buffer_t& src_buffer)
{
    svo::detail::append_buffer(*static_cast<svo_buffer_t*>(this), src_buffer);
}

template<typename svo_buffer_t>
void
svo_base_buffer_t<svo_buffer_t>::
append_buffer(const svo_gpu_buffer_t& src_buffer)
{
    svo::detail::append_buffer(*static_cast<svo_buffer_t*>(this), src_buffer);
}



////////////////////////////////////////////////////////////////////////////////




svo_cpu_buffer_t::svo_cpu_buffer_t(const svo_declaration_t& declaration, std::size_t initial_entries)
    : super_type(declaration, initial_entries, nullptr)
{
    m_data.resize(initial_entries*m_declaration.stride());
    
    if (initial_entries)
        m_rawdata = reinterpret_cast<uint8_t*>(&m_data[0]);
    else
        m_rawdata = nullptr;
    
    self().assert_invariants();
}

void svo_cpu_buffer_t::resize(std::size_t new_size)
{
    self().assert_invariants();
    m_entries = new_size;
    m_data.resize(new_size*m_declaration.stride());
    m_rawdata = reinterpret_cast<uint8_t*>(&m_data[0]);
    self().assert_invariants();
}


////////////////////////////////////////////////////////////////////////////////






svo_gpu_buffer_t::svo_gpu_buffer_t(const svo_declaration_t& declaration, std::size_t initial_entries
                                    , svo_block_t* block, goffset_t start, goffset_t end)
    : super_type(declaration, initial_entries, nullptr)
    , m_block(block)
    , m_start(start)
    , m_end(end)
{
    assert(m_block);
    assert(m_block->tree);
    assert(m_block->tree->address_space);
    m_rawdata = reinterpret_cast<uint8_t*>(m_block->tree->address_space + m_start);
    self().assert_invariants();
}


void svo_gpu_buffer_t::resize(std::size_t new_size)
{
    self().assert_invariants();
    throw std::runtime_error("svo_gpu_buffer_t::resize(): NOT IMPLEMENTED");
    self().assert_invariants();
}


void svo_gpu_buffer_t::assert_invariants() const
{
    super_type::assert_invariants();

    assert(m_block);
    assert(m_block->tree);
    assert(m_block->tree->address_space);
    assert(m_rawdata == reinterpret_cast<uint8_t*>(m_block->tree->address_space + m_start));
    
    assert(m_start <= m_end);
    assert(m_end - m_start == m_entries*m_declaration.stride());
}


////////////////////////////////////////////////////////////////////////////////

template<typename svo_buffer_t, typename svo_buffers_t>
void
svo_base_buffers_t<svo_buffer_t, svo_buffers_t>::
copy_schema(const svo_base_buffers_t& other, std::size_t new_entries)
{
    this->copy_schema(other.schema(), new_entries);
}



template<typename svo_buffer_t, typename svo_buffers_t>
void
svo_base_buffers_t<svo_buffer_t, svo_buffers_t>::
copy_schema(const svo_schema_t& other, std::size_t new_entries)
{
    if (this->has_schema())
        throw std::runtime_error(fmt::format("Cannot copy schema, already has a schema"
                                             ", schema: {}", schema));
    
    for (const auto& declaration : other)
    {
        auto& buffer = static_cast<svo_buffers_t*>(this)->add_buffer(declaration, new_entries);
        UNUSED(buffer);
    }

    assert(entries() == new_entries);
}



////////////////////////////////////////////////////////////////////////////////

svo_cpu_buffers_t::svo_cpu_buffers_t()
{
    self().assert_invariants();
}




svo_cpu_buffer_t&
svo_cpu_buffers_t::
add_buffer(const svo_declaration_t& declaration, std::size_t initial_entries)
{
    self().assert_invariants();
    
    for (const auto& element : declaration.elements())
    {
        if (m_buffer_element_mappings.count(element.name()) > 0)
            throw std::runtime_error(fmt::format("cannot add buffer containing element named {}; buffers already contains this element."
                                                 " buffers.schema(): {}, new buffer.declaration(): {}",
                                                 quote(element.name()), declaration, this->schema()));
    }
    
    ///the index of the new buffer to be
    std::size_t buffer_index = m_buffers.size();
    
    ///we will record each element name mapping like {element-name => (buffer-index, element-index)}
    buffer_element_mappings_t new_buffer_element_mappings;
    const auto& elements = declaration.elements();
    for (std::size_t element_index = 0; element_index < elements.size(); ++element_index)
    {
        const auto& element = elements[element_index];
        
        if (new_buffer_element_mappings.count(element.name()))
            throw std::runtime_error(fmt::format("cannot add buffer containing element named {}; new buffer would contain two or more elements of the same name."
                                                 " buffer.declaration(): {}",
                                                 quote(element.name()), declaration, this->schema()));
            
        new_buffer_element_mappings[element.name()] = std::make_pair(buffer_index, element_index);
    }
    
    
    m_buffer_element_mappings.insert(new_buffer_element_mappings.begin(), new_buffer_element_mappings.end());
    
    
    m_buffers.push_back(svo_cpu_buffer_t(declaration,initial_entries));
    m_schema.push_back(declaration);

    self().assert_invariants();
    
    return m_buffers.back();
}

void
svo_cpu_buffers_t::
reset()
{
    self().assert_invariants();
    m_buffers.clear();
    m_buffer_element_mappings.clear();
    m_schema.clear();
    self().assert_invariants();
}




////////////////////////////////////////////////////////////////////////////////

svo_gpu_buffers_t::svo_gpu_buffers_t(svo_block_t* block)
    : m_block(block)
{
    self().assert_invariants();

}

svo_gpu_buffer_t&
svo_gpu_buffers_t::
add_buffer(const svo_declaration_t& declaration, std::size_t initial_entries)
{
    self().assert_invariants();
    
    goffset_t data_start = m_block->data_end;
    goffset_t data_end = data_start + initial_entries*declaration.stride();
    assert(data_start <= data_end);
    
    if (data_end >= m_block->dataspace_end)
        throw svo_bad_alloc();
    
    
    svo_gpu_buffer_t buffer(declaration, initial_entries, m_block, data_start, data_end);
    m_buffers.push_back(buffer);
    
    m_block->data_end = data_end;
    
    buffer.assert_invariants();
    
    self().assert_invariants();
    
    return m_buffers.back();
}

void
svo_gpu_buffers_t::
assert_invariants() const
{
    super_type::assert_invariants();

    ///TODO check the block pointers match m_block,
    /// and that they are in the dataspace of the block,
    /// and that they do not overlap etc.
}

////////////////////////////////////////////////////////////////////////////////

///explicit instantiations
template struct svo_base_buffer_t<svo_cpu_buffer_t>;
template struct svo_base_buffer_t<svo_gpu_buffer_t>;
template struct svo_base_buffers_t<svo_cpu_buffer_t, svo_cpu_buffers_t>;
template struct svo_base_buffers_t<svo_gpu_buffer_t, svo_gpu_buffers_t>;



} // namespace svo












