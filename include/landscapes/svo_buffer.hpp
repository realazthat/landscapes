#ifndef SVO_BUFFER_HPP
#define SVO_BUFFER_HPP


#include <string>
#include <vector>
#include <map>
#include <set>
#include <cstddef>
#include <cstdint>
#include <typeinfo>
#include <typeindex>

#include "svo_buffer.fwd.hpp"
#include "svo_tofromstr.hpp"
#include "svo_validenum.hpp"
#include "svo_inttypes.h"


#include "svo_tree.fwd.hpp"
#include "svo_buffer.fwd.hpp"



namespace svo{


static inline std::size_t get_size_bytes(svo_data_type_t data_type);

static inline const char* tostr(svo_data_type_t data_type);
static inline const char* tostr(svo_semantic_t semantic);


template<>
inline svo_data_type_t fromstr<svo_data_type_t>(const std::string& data_type);
template<>
inline svo_semantic_t fromstr<svo_semantic_t>(const std::string& semantic);

template<>
inline bool validenum<svo_data_type_t>(const std::string& data_type);
template<>
inline bool validenum<svo_semantic_t>(const std::string& semantic);


struct svo_element_t{
    svo_element_t(const std::string& name, svo_semantic_t semantic, svo_data_type_t data_type, std::size_t count);

    svo_data_type_t type() const;
    svo_semantic_t semantic() const;
    std::size_t count() const;
    std::size_t type_bytes() const;
    std::size_t bytes() const;
    const std::string& name() const;

    bool operator==(const svo_element_t& other) const;
    bool operator!=(const svo_element_t& other) const;

    void assert_invariants() const;
private:
    std::string m_name;
    svo_semantic_t m_semantic;
    svo_data_type_t m_data_type;
    std::size_t m_count;
    std::size_t m_bytes;
};

struct svo_declaration_t{
    typedef std::vector<svo_element_t> elements_list_t;

    svo_declaration_t();

    std::size_t stride() const;
    void add(const svo_element_t& element);
    std::size_t offset(std::size_t element_index) const;

    const elements_list_t& elements() const;

    bool operator==(const svo_declaration_t& other) const;
    bool operator!=(const svo_declaration_t& other) const;

    void assert_invariants() const;
private:
    elements_list_t m_elements;
    std::vector<std::size_t> m_offsets;
    std::size_t m_stride;
    std::set<std::string> m_names;
};

template<typename svo_buffer_t>
struct svo_base_buffer_t{
    void assert_invariants() const;
    const svo_declaration_t& declaration() const;
    ///number of entries in this buffer.
    std::size_t entries() const;
    ///total size of the buffer in bytes.
    std::size_t bytes() const;
    ///stride of each entry in the buffer.
    std::size_t stride() const;
    ///raw pointer to the beginning of the buffer.
    const uint8_t* rawdata() const;
    ///raw pointer to the beginning of the buffer.
    uint8_t* rawdata();

    void copy_from_buffer(const svo_cpu_buffer_t& src_buffer
                            , std::size_t src_start=0, std::size_t copy_entries = (std::size_t)-1
                            , std::size_t dst_start=0);
    void copy_from_buffer(const svo_gpu_buffer_t& src_buffer
                            , std::size_t src_start=0, std::size_t copy_entries = (std::size_t)-1
                            , std::size_t dst_start=0);

    void copy_from_buffer(    const svo_cpu_buffer_t& src_buffer
                            , std::size_t src_element_index
                            , std::size_t dst_element_index
                            , std::size_t src_start, std::size_t copy_entries = (std::size_t)-1
                            , std::size_t dst_start=0);
    void copy_from_buffer(    const svo_gpu_buffer_t& src_buffer
                            , std::size_t src_element_index
                            , std::size_t dst_element_index
                            , std::size_t src_start=0, std::size_t copy_entries = (std::size_t)-1
                            , std::size_t dst_start=0);



    void append_from_buffer(const svo_cpu_buffer_t& src_buffer
                            , std::size_t src_start=0, std::size_t len=std::size_t(-1));
    void append_from_buffer(const svo_gpu_buffer_t& src_buffer
                            , std::size_t src_start=0, std::size_t len=std::size_t(-1));

    void append_from_buffer(const svo_cpu_buffer_t& src_buffer
                            , std::size_t dst_element_index, std::size_t src_element_index
                            , std::size_t dst_start=0
                            , std::size_t src_start=0
                            , std::size_t len=std::size_t(-1));
    void append_from_buffer(const svo_gpu_buffer_t& src_buffer
                            , std::size_t dst_element_index, std::size_t src_element_index
                            , std::size_t dst_start=0
                            , std::size_t src_start=0
                            , std::size_t len=std::size_t(-1));

    void append_buffer(const svo_cpu_buffer_t& src_buffer);
    void append_buffer(const svo_gpu_buffer_t& src_buffer);

    svo_buffer_t& self();
    const svo_buffer_t& self() const;
protected:

    explicit svo_base_buffer_t(const svo_declaration_t& declaration, std::size_t initial_entries, uint8_t* rawdata);
protected:

    svo_declaration_t m_declaration;
    std::size_t m_entries;
    uint8_t* m_rawdata;
};

struct svo_cpu_buffer_t : svo_base_buffer_t<svo_cpu_buffer_t>{
    typedef svo_base_buffer_t<svo_cpu_buffer_t> super_type;
    explicit svo_cpu_buffer_t(const svo_declaration_t& declaration, std::size_t initial_entries);

    void resize(std::size_t new_size);
    void assert_invariants() const;
private:
    std::vector<uint8_t> m_data;
};

struct svo_gpu_buffer_t : svo_base_buffer_t<svo_gpu_buffer_t>{
    typedef svo_base_buffer_t<svo_gpu_buffer_t> super_type;
    explicit svo_gpu_buffer_t(const svo_declaration_t& declaration, std::size_t initial_entries, svo_block_t* block, goffset_t start, goffset_t end);

    void resize(std::size_t new_size);
    void assert_invariants() const;
protected:


    svo_block_t* m_block;
    goffset_t m_start;
    goffset_t m_end;
};



struct svo_element_info_t{
    void* buffer_ptr;
    void* ptr;
    std::size_t offset;
    std::size_t stride;
    std::size_t padding;
    std::size_t entries;
};

namespace detail{
    template<typename byte_t, typename svo_buffer_t>
    struct svo_element_view_t{
        template<typename T> T& get(std::size_t entry_index);
        template<typename T> const T& get(std::size_t entry_index) const;

        //template<typename T> T& back();
        //template<typename T> const T& back() const;

        void assert_invariants() const;

        svo_element_view_t(svo_buffer_t* buffer, std::size_t element_index, byte_t* ptr, std::size_t stride, std::size_t entries);
    private:
        svo_buffer_t* buffer;
        std::size_t element_index;
        byte_t* ptr;
        std::size_t stride;
        std::size_t entries;
        
    };
}


template<typename svo_buffer_t, typename svo_buffers_t>
struct svo_base_buffers_t{
    
    svo_base_buffers_t(const svo_base_buffers_t&) = delete;
    svo_base_buffers_t& operator=(const svo_base_buffers_t&) = delete;
    
    
    typedef std::vector< svo_buffer_t > buffer_list_t;
    

    buffer_list_t& buffers();
    const buffer_list_t& buffers() const;
    const svo_schema_t& schema() const;
    bool has_schema() const;
    std::size_t entries() const;
    void resize(std::size_t new_entries);
    void reserve(std::size_t new_entries);

    bool has_named_element(const std::string& element_name) const;
    svo_element_info_t get_element_info(const std::string& element_name);
    const detail::svo_element_view_t<const uint8_t, const svo_buffer_t> get_element_view(const std::string& element_name) const;
    detail::svo_element_view_t<uint8_t, svo_buffer_t> get_element_view(const std::string& element_name);

    void assert_invariants() const;

    void copy_schema(const svo_base_buffers_t& other, std::size_t new_entries=0);
    void copy_schema(const svo_schema_t& other, std::size_t new_entries=0);



    svo_buffers_t& self();
    const svo_buffers_t& self() const;

protected:
    explicit svo_base_buffers_t();
protected:

    ///actual storage of buffers
    buffer_list_t m_buffers;

    ///(buffer index, element index).
    typedef std::tuple<std::size_t, std::size_t> buffer_element_t;
    ///{element name => (buffer index, element index)}
    typedef std::map<std::string, buffer_element_t> buffer_element_mappings_t;
    buffer_element_mappings_t m_buffer_element_mappings;

    svo_schema_t m_schema;
};



struct svo_cpu_buffers_t : svo_base_buffers_t<svo_cpu_buffer_t, svo_cpu_buffers_t>{
    typedef svo_base_buffers_t<svo_cpu_buffer_t, svo_cpu_buffers_t> super_type;
    
    svo_cpu_buffers_t(const svo_cpu_buffers_t&) = delete;
    svo_cpu_buffers_t& operator=(const svo_cpu_buffers_t&) = delete;
    explicit svo_cpu_buffers_t();

    ///add buffer.
    svo_cpu_buffer_t& add_buffer(const svo_declaration_t& declaration, std::size_t initial_entries=0);

    void reset();
};

struct svo_gpu_buffers_t : svo_base_buffers_t<svo_gpu_buffer_t, svo_gpu_buffers_t>{
    typedef svo_base_buffers_t<svo_gpu_buffer_t, svo_gpu_buffers_t> super_type;
    
    svo_gpu_buffers_t(const svo_gpu_buffers_t&) = delete;
    svo_gpu_buffers_t& operator=(const svo_gpu_buffers_t&) = delete;
    explicit svo_gpu_buffers_t(svo_block_t* block);

    ///add buffer.
    svo_gpu_buffer_t& add_buffer(const svo_declaration_t& declaration, std::size_t initial_entries=0);

    void assert_invariants() const;
private:
    svo_block_t* m_block;
};



} //namespace svo


#include "svo_buffer.inl.hpp"

#endif
