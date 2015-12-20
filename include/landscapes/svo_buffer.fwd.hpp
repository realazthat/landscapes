#ifndef SVO_BUFFER_FWD_HPP
#define SVO_BUFFER_FWD_HPP

namespace svo{


enum class svo_semantic_t{
      NONE
    , COLOR
    , NORMAL
};

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

struct svo_element_t;
struct svo_declaration_t;

struct svo_cpu_buffer_t;
struct svo_gpu_buffer_t;
struct svo_cpu_buffers_t;
struct svo_gpu_buffers_t;
struct svo_element_info_t;

}

#endif
