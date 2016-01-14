#ifndef SVO_BUFFER_FWD_HPP
#define SVO_BUFFER_FWD_HPP

#include <set>
#include <vector>

namespace svo{


enum class svo_semantic_t{
      NONE
    , COLOR
    , NORMAL
};

static const std::vector<svo_semantic_t> all_semantics {svo_semantic_t::NONE, svo_semantic_t::COLOR, svo_semantic_t::NORMAL};
static const std::set<svo_semantic_t> all_semantics_set = std::set<svo_semantic_t>(all_semantics.begin(), all_semantics.end());

enum class svo_data_type_t{
    BYTE,
    SHORT,
    INT,
    LONG,
    UNSIGNED_BYTE,
    UNSIGNED_SHORT,
    UNSIGNED_INT,
    UNSIGNED_LONG,
    FLOAT,
    DOUBLE
};

static const std::vector<svo_data_type_t> all_buffer_data_types {
      svo_data_type_t::BYTE
    , svo_data_type_t::SHORT
    , svo_data_type_t::INT
    , svo_data_type_t::LONG
    , svo_data_type_t::UNSIGNED_BYTE
    , svo_data_type_t::UNSIGNED_SHORT
    , svo_data_type_t::UNSIGNED_INT
    , svo_data_type_t::UNSIGNED_LONG
    , svo_data_type_t::FLOAT
    , svo_data_type_t::DOUBLE
};

static const std::set<svo_data_type_t> all_buffer_data_types_set =
    std::set<svo_data_type_t>(all_buffer_data_types.begin(), all_buffer_data_types.end());

struct svo_element_t;
struct svo_declaration_t;

struct svo_cpu_buffer_t;
struct svo_gpu_buffer_t;
struct svo_cpu_buffers_t;
struct svo_gpu_buffers_t;
struct svo_element_info_t;

typedef std::vector<svo_declaration_t> svo_schema_t;

}

#endif
