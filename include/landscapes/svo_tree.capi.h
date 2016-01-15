#ifndef SVO_TREE_CAPI_H
#define SVO_TREE_CAPI_H 1

#include "svo_curves.h"
#include "svo_inttypes.h"
#include "opencl.shim.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef struct child_descriptor_t{
    uint64_t data;
} child_descriptor_t;


typedef struct svo_info_section_t{
    
    bool dummy;
} svo_info_section_t;

typedef struct svo_page_header_t{
    offset_t info_offset;
} svo_page_header_t;

#define SVO_PAGE_SIZE ((uint32_t)8192)
#define SVO_PAGE_HEADER_SIZE ((uint32_t)sizeof(child_descriptor_t))

#define SVO_CONTOUR_MASK_CD_POS ((size_t)0)
#define SVO_CONTOUR_PTR_CD_POS ((size_t)(SVO_CONTOUR_MASK_CD_POS + 8))
#define SVO_LEAF_MASK_CD_POS ((size_t)(SVO_CONTOUR_PTR_CD_POS + 24))
#define SVO_VALID_MASK_CD_POS ((size_t)(SVO_LEAF_MASK_CD_POS + 8))
#define SVO_FAR_BIT_CD_POS ((size_t)(SVO_VALID_MASK_CD_POS + 8))
#define SVO_CHILD_PTR_CD_POS ((size_t)(SVO_FAR_BIT_CD_POS + 1))

#define SVO_CONTOUR_MASK_MASK ((uint64_t)(child_mask_mask))
///24 1s
//static const uint64_t SVO_CONTOUR_PTR_MASK = (uint64_t)(0b00000000111111111111111111111111);
#define SVO_CONTOUR_PTR_MASK ((uint64_t)(~((uint64_t)(-1) << 24)))
#define SVO_LEAF_MASK_MASK ((uint64_t)(child_mask_mask))
#define SVO_VALID_MASK_MASK ((uint64_t)(child_mask_mask))
///1 1s
//static const uint64_t SVO_FAR_BIT_MASK = (uint64_t)(0b1);
#define SVO_FAR_BIT_MASK ((uint64_t)(1))
//15 1s
//static const uint64_t SVO_CHILD_PTR_MASK = (uint64_t)(0b0111111111111111);
#define SVO_CHILD_PTR_MASK ((uint64_t)(~((uint64_t)(-1) << 15)))


#define SVO_CONTOUR_MASK_CDMASK (uint64_t)((SVO_CONTOUR_MASK_MASK) << SVO_CONTOUR_MASK_CD_POS)
#define SVO_CONTOUR_PTR_CDMASK (uint64_t)((SVO_CONTOUR_PTR_MASK) << SVO_CONTOUR_MASK_CD_POS)
#define SVO_LEAF_MASK_CDMASK (uint64_t)((SVO_LEAF_MASK_MASK) << SVO_LEAF_MASK_CD_POS)
#define SVO_VALID_MASK_CDMASK (uint64_t)((SVO_VALID_MASK_MASK) << SVO_VALID_MASK_CD_POS)
#define SVO_FAR_BIT_CDMASK (uint64_t)((SVO_FAR_BIT_MASK) << SVO_FAR_BIT_CD_POS)
#define SVO_CHILD_PTR_CDMASK (uint64_t)((SVO_CHILD_PTR_MASK) << SVO_CHILD_PTR_CD_POS)



static inline void svo_set_nth_bit(child_descriptor_t* child_descriptor, size_t n, bool value);
static inline bool svo_get_nth_bit(const child_descriptor_t* child_descriptor, size_t n);
static inline uint64_t svo_nbits_uint64(size_t n);
static inline fast_uint8_t svo_count_bits_uint8(uint8_t value);


static inline void svo_init_cd(child_descriptor_t* child_descriptor);
static inline void svo_copy_cd(child_descriptor_t* dst, const child_descriptor_t* src);



///gets the raw child, which is a 15 bit unsigned integer, and can point: 1. the set of nonleaf children,
/// offset from this CD, in increments of sizeof(far_ptr_t), or to 2. to a far ptr, which is a far_ptr_t,
/// which is a fullsize pointer to the set of nonleaf children, or 3. it can be invalid, and equal to invalid_goffset or maybe even random.
static inline offset4_t svo_get_child_ptr_offset4(const child_descriptor_t* child_descriptor);
///This gets the pointer to the set of children, regardless if this CD uses a far ptr or not.
static inline goffset_t svo_get_child_ptr_goffset(const byte_t* address_space, goffset_t cd_goffset, const child_descriptor_t* cd);
///This gets the pointer to a particular nonleaf child, regardless if this CD uses a far ptr or not.
static inline goffset_t svo_get_child_cd_goffset(const byte_t* address_space, goffset_t pcd_goffset, const child_descriptor_t* pcd, ccurve_t child_ccurve);
///assuming @c pcd has the far bit set, this will return the actual far ptr.
static inline goffset_t svo_get_goffset_via_fp(const byte_t* address_space, goffset_t pcd_goffset, const child_descriptor_t* pcd);

static inline bool svo_get_far(const child_descriptor_t* child_descriptor);
static inline child_mask_t svo_get_valid_mask(const child_descriptor_t* child_descriptor);
static inline child_mask_t svo_get_leaf_mask(const child_descriptor_t* child_descriptor);
static inline child_mask_t svo_get_nonleaf_mask(const child_descriptor_t* child_descriptor);
static inline bool svo_get_valid_bit(const child_descriptor_t* child_descriptor, ccurve_t ccurve);
static inline bool svo_get_leaf_bit(const child_descriptor_t* child_descriptor, ccurve_t ccurve);
static inline bool svo_get_nonleaf_bit(const child_descriptor_t* child_descriptor, ccurve_t ccurve);

///get the index of the child within the parent. so if a parent has 3 children, the last child will have an index = 2.
static inline fast_uint8_t svo_get_cd_child_index(const child_descriptor_t* child_descriptor, ccurve_t child_ccurve);
static inline fast_uint8_t svo_get_cd_valid_count(const child_descriptor_t* child_descriptor);
static inline fast_uint8_t svo_get_cd_leaf_count(const child_descriptor_t* child_descriptor);
static inline fast_uint8_t svo_get_cd_nonleaf_count(const child_descriptor_t* child_descriptor);

static inline void svo_set_child_ptr(child_descriptor_t* child_descriptor, uint16_t child_ptr);
static inline void svo_set_goffset_via_fp(byte_t* address_space, goffset_t pcd_goffset, child_descriptor_t* pcd, goffset_t cd_goffset);
static inline void svo_set_far(child_descriptor_t* child_descriptor, bool far);
static inline void svo_set_valid_mask(child_descriptor_t* child_descriptor, child_mask_t valid_mask);
static inline void svo_set_leaf_mask(child_descriptor_t* child_descriptor, child_mask_t valid_mask);
static inline void svo_set_valid_bit(child_descriptor_t* child_descriptor, ccurve_t ccurve, bool value);
static inline void svo_set_leaf_bit(child_descriptor_t* child_descriptor, ccurve_t ccurve, bool value);



static inline uint32_t svo_get_contour_ptr(const child_descriptor_t* child_descriptor);
static inline child_mask_t svo_get_contour_mask(const child_descriptor_t* child_descriptor);
static inline goffset_t svo_get_ph_goffset(goffset_t cd_goffset);

static inline svo_page_header_t* svo_get_ph(byte_t* address_space, goffset_t cd_goffset);
static inline svo_info_section_t* info_section(byte_t* address_space, goffset_t cd_goffset);
static inline child_descriptor_t* svo_get_cd(byte_t* address_space, goffset_t cd_goffset);
static inline const child_descriptor_t* svo_cget_cd(const byte_t* address_space, goffset_t cd_goffset);








static inline fast_uint8_t svo_count_bits_uint8(uint8_t value)
{
    return (
          ((value >> 0) & 1)
        + ((value >> 1) & 1)
        + ((value >> 2) & 1)
        + ((value >> 3) & 1)
        + ((value >> 4) & 1)
        + ((value >> 5) & 1)
        + ((value >> 6) & 1)
        + ((value >> 7) & 1));
}




static inline bool svo_get_nth_bit(const child_descriptor_t* child_descriptor, size_t n)
{
    assert(child_descriptor);
    assert(n < sizeof(child_descriptor_t)*8);

    uint64_t data = child_descriptor->data;

    return (data >> n) & 1;
}



static inline void svo_set_nth_bit(child_descriptor_t* child_descriptor, size_t n, bool value)
{
    ///see http://stackoverflow.com/a/47990/586784
    assert(child_descriptor);
    assert(n < sizeof(child_descriptor_t)*8);

    uint64_t data = child_descriptor->data;
    uint64_t x = value ? 1 : 0;
    
    x = -x;

    data ^= (x ^ data) & (uint64_t)((uint64_t)(1) << n);

    child_descriptor->data = data;

    assert(svo_get_nth_bit(child_descriptor, n) == value);

}



static inline void svo_copy_cd(child_descriptor_t* dst, const child_descriptor_t* src)
{
    assert(src);
    assert(dst);
    dst->data = src->data;
}

static inline void svo_init_cd(child_descriptor_t* child_descriptor)
{
    assert(child_descriptor);
    child_descriptor->data = 0;
}

static inline void svo_set_far(child_descriptor_t* child_descriptor, bool value)
{
    assert(child_descriptor);

    uint64_t farbit = ((uint64_t)(value ? 1 : 0)) << SVO_FAR_BIT_CD_POS;

    ///erase the bits we gonna write.
    child_descriptor->data &= ~SVO_FAR_BIT_CDMASK;
    child_descriptor->data |= farbit & SVO_FAR_BIT_CDMASK;
}

static inline void svo_set_child_ptr(child_descriptor_t* child_descriptor, uint16_t child_ptr)
{
    assert(child_descriptor);

    assert((child_ptr & SVO_CHILD_PTR_MASK) == child_ptr);


    ///erase the bits we gonna write.
    child_descriptor->data &= ~SVO_CHILD_PTR_CDMASK;
    child_descriptor->data |= ((uint64_t)(child_ptr) << SVO_CHILD_PTR_CD_POS) & SVO_CHILD_PTR_CDMASK;
}

static inline void svo_set_goffset_via_fp(byte_t* address_space, goffset_t pcd_goffset, child_descriptor_t* pcd, goffset_t cd_goffset)
{
    assert(address_space);
    assert(pcd_goffset);
    assert(pcd_goffset != invalid_goffset);
    assert( (pcd_goffset & goffset_mask) == pcd_goffset );
    assert(svo_get_cd(address_space, pcd_goffset) == pcd);

    assert(svo_get_far(pcd));

    offset4_t offset4 = svo_get_child_ptr_offset4(pcd);

    assert(offset4 != 0);

    goffset_t far_ptr_goffset = pcd_goffset + offset4*4;

    goffset_t* far_ptr_ptr = (goffset_t*)(address_space + far_ptr_goffset);

    *far_ptr_ptr = cd_goffset;
}


static inline offset4_t svo_get_child_ptr_offset4(const child_descriptor_t* child_descriptor)
{
    assert(child_descriptor);
    return (child_descriptor->data >> SVO_CHILD_PTR_CD_POS) & SVO_CHILD_PTR_MASK;
}



static inline goffset_t svo_get_child_ptr_goffset(const byte_t* address_space, goffset_t pcd_goffset, const child_descriptor_t* pcd)
{
    assert(address_space);
    assert(pcd_goffset);
    assert(pcd_goffset != invalid_goffset);
    assert( (pcd_goffset & goffset_mask) == pcd_goffset );
    assert(svo_cget_cd(address_space, pcd_goffset) == pcd);

    offset4_t child_ptr_offset4 = svo_get_child_ptr_offset4(pcd);
    
    if (child_ptr_offset4 == 0)
        return invalid_goffset;
    
    bool farvalue = svo_get_far(pcd);

    goffset_t child_ptr_goffset = pcd_goffset + child_ptr_offset4*4;
    assert( (child_ptr_goffset & goffset_mask) == child_ptr_goffset );

    if (farvalue)
    {
        goffset_t far_ptr_goffset = child_ptr_goffset;
        child_ptr_goffset = *(far_ptr_t*)(address_space + far_ptr_goffset);
    }

    assert( (child_ptr_goffset & goffset_mask) == child_ptr_goffset );
    return child_ptr_goffset;
}

static inline goffset_t svo_get_child_cd_goffset(const byte_t* address_space, goffset_t pcd_goffset, const child_descriptor_t* pcd, ccurve_t child_ccurve)
{
    assert(address_space);
    assert(pcd_goffset);
    assert(pcd_goffset != invalid_goffset);
    assert( (pcd_goffset & goffset_mask) == pcd_goffset );
    assert( child_ccurve < 8 );
    assert(svo_cget_cd(address_space, pcd_goffset) == pcd);

    assert(svo_get_valid_bit(pcd, child_ccurve));
    assert(!svo_get_leaf_bit(pcd, child_ccurve));

    goffset_t cd0_goffset = svo_get_child_ptr_goffset(address_space, pcd_goffset, pcd);
    assert(cd0_goffset != 0 && cd0_goffset != invalid_goffset);

    uint8_t child_index = svo_get_cd_child_index(pcd, child_ccurve);
    assert(child_index < 8);

    goffset_t child_cd_goffset = cd0_goffset + child_index * sizeof(child_descriptor_t);

    ///if we need to skip a page header. 
    if (cd0_goffset < svo_get_ph_goffset(child_cd_goffset))
    {
        child_cd_goffset += sizeof(child_descriptor_t);
    }

    assert(child_cd_goffset != 0 && child_cd_goffset != invalid_goffset);

    return child_cd_goffset;
}

static inline goffset_t svo_get_goffset_via_fp(const byte_t* address_space, goffset_t pcd_goffset, const child_descriptor_t* pcd)
{

    assert(address_space);
    assert(pcd_goffset);
    assert(pcd_goffset != invalid_goffset);
    assert( (pcd_goffset & goffset_mask) == pcd_goffset );
    assert(svo_cget_cd(address_space, pcd_goffset) == pcd);

    assert(svo_get_far(pcd));

    offset4_t offset4 = svo_get_child_ptr_offset4(pcd);

    assert(offset4 != 0);

    goffset_t far_ptr_goffset = pcd_goffset + offset4*4;

    goffset_t far_ptr = *(const goffset_t*)(address_space + far_ptr_goffset);
    return far_ptr;
}

static inline bool svo_get_far(const child_descriptor_t* child_descriptor)
{
    assert(child_descriptor);
    return (child_descriptor->data >> SVO_FAR_BIT_CD_POS) & 1;
}

static inline child_mask_t svo_get_valid_mask(const child_descriptor_t* child_descriptor)
{
    assert(child_descriptor);
    return (child_descriptor->data >> SVO_VALID_MASK_CD_POS) & SVO_VALID_MASK_MASK;
}


static inline bool svo_get_leaf_bit(const child_descriptor_t* child_descriptor, ccurve_t ccurve)
{
    assert(child_descriptor);
    assert(ccurve < 8);

    child_mask_t leaf_mask = svo_get_leaf_mask(child_descriptor);

    return (leaf_mask >> ccurve) & 1;
}

static inline bool svo_get_nonleaf_bit(const child_descriptor_t* child_descriptor, ccurve_t ccurve)
{
    assert(child_descriptor);
    assert(ccurve < 8);

    child_mask_t valid_mask = svo_get_valid_mask(child_descriptor);
    child_mask_t leaf_mask = svo_get_leaf_mask(child_descriptor);

    bool valid_bit = (valid_mask >> ccurve) & 1;
    bool leaf_bit = (leaf_mask >> ccurve) & 1;
    return valid_bit && !leaf_bit;
}


static inline void svo_set_valid_mask(child_descriptor_t* child_descriptor, child_mask_t valid_mask)
{
    assert(child_descriptor);

    ///should be 8 bits.
    assert( (valid_mask & SVO_VALID_MASK_MASK) == valid_mask );


    ///erase the bits we gonna write.
    child_descriptor->data &= ~SVO_VALID_MASK_CDMASK;

    ///write the bits
    child_descriptor->data |= ((uint64_t)(valid_mask) << SVO_VALID_MASK_CD_POS) & SVO_VALID_MASK_CDMASK;

    assert( svo_get_valid_mask(child_descriptor) == valid_mask );
}


static inline void svo_set_valid_bit(child_descriptor_t* child_descriptor, ccurve_t ccurve, bool value)
{
    assert(child_descriptor);
    assert(ccurve < 8);



#ifndef NDEBUG
    uint64_t readonly_data0 = (child_descriptor->data & ~SVO_VALID_MASK_CDMASK);
#endif
    svo_set_nth_bit(child_descriptor, (SVO_VALID_MASK_CD_POS + ccurve), value);
#ifndef NDEBUG
    assert( bool((svo_get_valid_mask(child_descriptor) >> ccurve) & 1) == value );
    uint64_t readonly_data1 = (child_descriptor->data & ~SVO_VALID_MASK_CDMASK);
    assert( readonly_data0 == readonly_data1 );

#endif
}


static inline child_mask_t svo_get_leaf_mask(const child_descriptor_t* child_descriptor)
{
    assert(child_descriptor);
    return (child_descriptor->data >> SVO_LEAF_MASK_CD_POS) & SVO_LEAF_MASK_MASK;
}


static inline child_mask_t svo_get_nonleaf_mask(const child_descriptor_t* child_descriptor)
{
    assert(child_descriptor);
    child_mask_t valid_mask = svo_get_valid_mask(child_descriptor);
    child_mask_t leaf_mask = svo_get_leaf_mask(child_descriptor);

    child_mask_t result = valid_mask & (~leaf_mask);
    assert( (result & child_mask_mask) == result );
    return result;
}




static inline bool svo_get_valid_bit(const child_descriptor_t* child_descriptor, ccurve_t ccurve)
{
    assert(child_descriptor);
    assert(ccurve < 8);

    child_mask_t valid_mask = svo_get_valid_mask(child_descriptor);

    return (valid_mask >> ccurve) & 1;
}


static inline void svo_set_leaf_mask(child_descriptor_t* child_descriptor, child_mask_t leaf_mask)
{
    assert(child_descriptor);

    ///should be 8 bits.
    assert( (leaf_mask & SVO_LEAF_MASK_MASK) == leaf_mask );

    ///erase the bits we gonna write.
    child_descriptor->data &= ~SVO_LEAF_MASK_CDMASK;

    ///write the bits
    child_descriptor->data |= ((uint64_t)(leaf_mask) << SVO_LEAF_MASK_CD_POS) & SVO_LEAF_MASK_CDMASK;

    assert(svo_get_leaf_mask(child_descriptor) == leaf_mask);
}



static inline void svo_set_leaf_bit(child_descriptor_t* child_descriptor, ccurve_t ccurve, bool value)
{
    assert(child_descriptor);
    assert(ccurve < 8);

#ifndef NDEBUG
    uint64_t readonly_data0 = (child_descriptor->data & ~SVO_LEAF_MASK_CDMASK);
#endif
    svo_set_nth_bit(child_descriptor, SVO_LEAF_MASK_CD_POS + ccurve, value);
#ifndef NDEBUG
    assert( bool((svo_get_leaf_mask(child_descriptor) >> ccurve) & 1) == value );
    uint64_t readonly_data1 = (child_descriptor->data & ~SVO_LEAF_MASK_CDMASK);
    assert( readonly_data0 == readonly_data1 );
#endif
}


static inline uint32_t svo_get_contour_ptr(const child_descriptor_t* child_descriptor)
{
    assert(child_descriptor);
    return (child_descriptor->data >> SVO_CONTOUR_PTR_CD_POS) & SVO_CONTOUR_PTR_MASK;
}

static inline child_mask_t svo_get_contour_mask(const child_descriptor_t* child_descriptor)
{
    assert(child_descriptor);
    return (child_descriptor->data >> SVO_CONTOUR_MASK_CD_POS) & SVO_CONTOUR_MASK_MASK;
}


static inline goffset_t svo_get_ph_goffset(goffset_t goffset)
{
    assert(goffset != invalid_goffset);
    assert( (goffset & goffset_mask) == goffset );

    goffset_t result = (goffset_t)(goffset) & ~(goffset_t)(SVO_PAGE_SIZE-1);
    assert((result & goffset_mask) == result);
    assert(result != invalid_goffset);

    return result;
}



static inline svo_page_header_t* svo_get_ph(byte_t* address_space, goffset_t cd_goffset)
{
    assert(address_space);
    assert(cd_goffset);
    assert(cd_goffset != invalid_goffset);
    assert( (cd_goffset & goffset_mask) == cd_goffset );

    goffset_t ph_goffset = svo_get_ph_goffset(cd_goffset);
    svo_page_header_t* page_header = (svo_page_header_t*)(address_space + ph_goffset);
    return page_header;
}

static inline svo_info_section_t* info_section(byte_t* address_space, goffset_t cd_goffset)
{
    assert(address_space);
    assert(cd_goffset);
    assert(cd_goffset != invalid_goffset);
    assert( (cd_goffset & goffset_mask) == cd_goffset );

    const svo_page_header_t* page_header = svo_get_ph(address_space, cd_goffset);
    goffset_t ph_goffset = svo_get_ph_goffset(cd_goffset);


    byte_t* info_ptr = address_space + ph_goffset + page_header->info_offset;

    return (svo_info_section_t*)(info_ptr);
}

static inline child_descriptor_t* svo_get_cd(byte_t* address_space, goffset_t cd_goffset)
{
    assert(address_space);
    assert(cd_goffset);
    assert(cd_goffset != invalid_goffset);
    assert( (cd_goffset & goffset_mask) == cd_goffset );
    child_descriptor_t* cd = (child_descriptor_t*)(address_space + cd_goffset);

    return cd;
}


static inline const child_descriptor_t* svo_cget_cd(const byte_t* address_space, goffset_t cd_goffset)
{
    assert(address_space);
    assert(cd_goffset);
    assert(cd_goffset != invalid_goffset);
    assert( (cd_goffset & goffset_mask) == cd_goffset );
    const child_descriptor_t* cd = (const child_descriptor_t*)(address_space + cd_goffset);

    return cd;
}


static inline fast_uint8_t svo_get_cd_child_index(const child_descriptor_t* child_descriptor, ccurve_t child_ccurve)
{
    ///sanity checks
    assert(child_ccurve < 8);
    assert(child_descriptor);

    fast_uint8_t result = 0;
    child_mask_t valid_mask = svo_get_valid_mask(child_descriptor);
    child_mask_t leaf_mask = svo_get_leaf_mask(child_descriptor);

    ///for each bit that is set as valid but a non-leaf, we increment the result index.
    for (ccurve_t ccurve = 0; ccurve < child_ccurve; ++ccurve)
    {
        bool valid_bit = (valid_mask >> ccurve) & 1;
        bool leaf_bit = (leaf_mask >> ccurve) & 1;

        if (valid_bit && !leaf_bit)
        {
            ++result;
            continue;
        }
    }

    ///also double check that the child requested actually exists.
    assert( (valid_mask >> child_ccurve) & 1);
    assert( !((leaf_mask >> child_ccurve) & 1));

    return result;
}


static inline fast_uint8_t svo_get_cd_nonleaf_count_check(const child_descriptor_t* child_descriptor)
{
    assert(child_descriptor);

    fast_uint8_t result = 0;
    child_mask_t valid_mask = svo_get_valid_mask(child_descriptor);
    child_mask_t leaf_mask = svo_get_leaf_mask(child_descriptor);


    ///for each bit that is set as valid but a non-leaf, we increment the result index.
    for (ccurve_t ccurve = 0; ccurve < 8; ++ccurve)
    {
        bool valid_bit = (valid_mask >> ccurve) & 1;
        bool leaf_bit = (leaf_mask >> ccurve) & 1;

        if (valid_bit && !leaf_bit)
        {
            ++result;
            continue;
        }
    }

    return result;
}


static inline fast_uint8_t svo_get_cd_valid_count_check(const child_descriptor_t* child_descriptor)
{
    assert(child_descriptor);

    fast_uint8_t result = 0;
    child_mask_t valid_mask = svo_get_valid_mask(child_descriptor);


    ///for each bit that is set as valid but a non-leaf, we increment the result index.
    for (ccurve_t ccurve = 0; ccurve < 8; ++ccurve)
    {
        bool valid_bit = (valid_mask >> ccurve) & 1;

        if (valid_bit)
        {
            ++result;
            continue;
        }
    }

    return result;
}
static inline fast_uint8_t svo_get_cd_leaf_count_check(const child_descriptor_t* child_descriptor)
{
    assert(child_descriptor);

    fast_uint8_t result = 0;
    child_mask_t leaf_mask = svo_get_leaf_mask(child_descriptor);


    ///for each bit that is set as valid but a non-leaf, we increment the result index.
    for (ccurve_t ccurve = 0; ccurve < 8; ++ccurve)
    {
        bool leaf_bit = (leaf_mask >> ccurve) & 1;

        if (leaf_bit)
        {
            ++result;
            continue;
        }
    }

    return result;
}

static inline fast_uint8_t svo_get_cd_nonleaf_count(const child_descriptor_t* child_descriptor)
{
    assert(child_descriptor);
    child_mask_t nonleaf_mask = svo_get_nonleaf_mask(child_descriptor);

    fast_uint8_t result = svo_count_bits_uint8(nonleaf_mask);
    assert( result == svo_get_cd_nonleaf_count_check(child_descriptor) );
    return result;
}
static inline fast_uint8_t svo_get_cd_valid_count(const child_descriptor_t* child_descriptor)
{
    assert(child_descriptor);
    child_mask_t valid_mask = svo_get_valid_mask(child_descriptor);

    fast_uint8_t result = svo_count_bits_uint8(valid_mask);
    assert( result == svo_get_cd_valid_count_check(child_descriptor) );
    return result;
}

static inline fast_uint8_t svo_get_cd_leaf_count(const child_descriptor_t* child_descriptor)
{
    assert(child_descriptor);
    child_mask_t leaf_mask = svo_get_leaf_mask(child_descriptor);

    fast_uint8_t result = svo_count_bits_uint8(leaf_mask);
    assert( result == svo_get_cd_leaf_count_check(child_descriptor) );
    return result;
}

#ifdef __cplusplus
}
#endif

#endif
