#ifndef OPENCL_SHIM_H
#define OPENCL_SHIM_H 1


////////////////////////////////////////////////////////////////////////////////
////Type/utility/definition abstractions
////////////////////////////////////////////////////////////////////////////////


#ifdef __OPENCL_VERSION__


    #define assert(ignore)((void) 0)
    #define PPK_ASSERT(...)((void) 0)

    typedef uchar uint8_t;
    typedef ushort uint16_t;
    typedef uint uint32_t;
    typedef ulong uint64_t;

    #define GLOBAL_STATIC_CONST __constant const
    #define LOCAL_STATIC_CONST const
    
    
    typedef float4 float4_t;
    typedef float3 float3_t;
    typedef uint3 uint3_t;


    #define make_float4 (float4_t)
    #define make_float3 (float3_t)
    #define make_uint3 (uint3_t)
    
#else

    
    #include <assert.h>
    #include <glm/detail/type_vec3.hpp>
    #include <glm/detail/type_float.hpp>
    #include <glm/gtc/type_ptr.hpp>
    #include <stdint.h>
    
    #define GLOBAL_STATIC_CONST static const
    #define LOCAL_STATIC_CONST static const
    
    typedef glm::vec4 float4_t;
    typedef glm::vec3 float3_t;
    typedef glm::uvec3 uint3_t;

    #define make_float4 float4_t
    #define make_float3 float3_t
    #define make_uint3 uint3_t
#endif



////////////////////////////////////////////////////////////////////////////////
////Math abstractions
////////////////////////////////////////////////////////////////////////////////

#ifdef __OPENCL_VERSION__
#define glm_all all
#define glm_any any
#define glm_equal isequal
#define glm_isnan isnan
#define glm_isinf isinf
#define glm_min min
#define glm_max max
#define glm_normalize normalize
#define glm_lt isless
#define glm_gt isgreater
#define glm_select select
#define glm_dot dot
#define glm_distance distance
#define glm_length length
#define glm_tan tan
#define glm_atan atan
#define glm_sqrt sqrt
#define glm_abs abs
#else

template<typename svec_type>
static inline float3_t select(const float3_t& a, const float3_t& b, const svec_type& c)
{
    //assert(vec_type::components == svec_type::components);
    
    float3_t result;
    
    for (std::size_t i = 0; i < 3; ++i)
    {
        result[i] = c[i] ? b[i] : a[i];
    }
    
    return result;
}

#define glm_all glm::all
#define glm_any glm::any
#define glm_equal glm::equal
#define glm_isnan glm::isnan
#define glm_isinf glm::isinf
#define glm_min glm::min
#define glm_max glm::max
#define glm_normalize glm::normalize
#define glm_lt glm::lessThan
#define glm_gt glm::greaterThan
#define glm_select select
#define glm_dot glm::dot
#define glm_distance glm::distance
#define glm_length glm::length
#define glm_tan glm::tan
#define glm_atan glm::atan
#define glm_sqrt glm::sqrt
#define glm_abs glm::abs
#endif





#endif
