#ifndef SVO_INTTYPES_H
#define SVO_INTTYPES_H 1




#include "opencl.shim.h"

typedef unsigned int fast_uint8_t;
typedef uint8_t byte_t;
typedef uint32_t goffset_t;
typedef uint32_t offset_t;
typedef uint32_t offset4_t;
typedef uint32_t far_ptr_t;
typedef uint16_t offset4_uint16_t;
typedef uint8_t child_mask_t;

#define byte_mask (byte_t)(255)
#define goffset_mask (goffset_t)(((uint64_t)(-1) << 32) >> 32)
#define offset_mask (offset_t)(((uint64_t)(-1) << 32) >> 32)
#define offset4_mask (offset4_t)(((uint64_t)(-1) << 32) >> 32)
#define far_ptr_mask (far_ptr_t)(((uint64_t)(-1) << 32) >> 32)
#define offset4_uint16_mask (offset4_uint16_t)( ((uint64_t)(-1) << 48) >> 48 )
#define child_mask_mask (child_mask_t)(255)



static const goffset_t invalid_goffset = (goffset_t)(-1);

#endif
