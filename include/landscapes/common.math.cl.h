#ifndef SVO_COMMON_MATH_CL_H
#define SVO_COMMON_MATH_CL_H 1

#include "opencl.shim.h"

#include "cubelib/cubelib.h"

GLOBAL_STATIC_CONST float fposinf = 1.0f/0.0f;
GLOBAL_STATIC_CONST float fneginf = -1.0f/0.0f;
GLOBAL_STATIC_CONST float fnan = 0.0f/0.0f;


static inline
bool containsf3(float3_t bounds_lower, float3_t bounds_upper, float3_t pos)
{
    bool result = !glm_any(glm_isnan(pos));

    result &= !glm_any( glm_lt(pos, bounds_lower) );
    result &= !glm_any( glm_gt(pos, bounds_upper) );

    return result;
}


static inline
bool contains_strict_f3(float3_t bounds_lower, float3_t bounds_upper, float3_t pos)
{
    bool result = !glm_any(glm_isnan(pos));

    result &= glm_all( glm_gt(pos, bounds_lower) );
    result &= glm_all( glm_lt(pos, bounds_upper) );

    return result;
}

static inline bool fequalsf1(float v0, float v1, float epsilon=.0001)
{
    return glm_abs(v0-v1) < epsilon;
}
static inline bool fequalsf3(float3_t v0, float3_t v1, float epsilon=.0001)
{
    return glm_length(v0 - v1) < epsilon;
}

static inline bool fiszerof1(float v, float epsilon=.0001)
{
    return fequalsf1(v,0, epsilon);
}

static inline float3_t coalescenanf3(float3_t v, float3_t default_value)
{
    return glm_select(v, default_value, glm_isnan(v));
}

static inline float3_t coalesceinff3(float3_t v, float3_t default_value)
{
    return glm_select(v, default_value, glm_isinf(v));
}


static inline float getcomponentf3(float3_t v, size_t i)
{
    assert(i < 3);
    /*
    union{
        float3_t v;
        float s[3];
    } elements;

    elements.v = v;

    return elements.s[i];
    */

    return ((float*)&v)[i];
}


static inline float getcomponentf4(float4_t v, size_t i)
{
    assert(i < 4);

    return ((float*)&v)[i];
}

static inline void setcomponentf3(float3_t* v, size_t i, float value)
{
    assert(i < 3);

    ((float*)v)[i] = value;
}


static inline float mincomponentf3(float3_t v)
{
    return glm_min(v.x,glm_min(v.y, v.z));
}

static inline size_t mincomponentf3index(float3_t v)
{
    /*
    return
            !(v.x > v.y)
        ?       (!(v.x > v.z)
            ?   0
            :       (!(v.y > v.z)
                ?   1
                :   2))
        :       (!(v.y > v.z)
            ?   1
            :   2)
        ;
    */

    size_t index =
            !(v.x > v.y)
        ?       (!(v.x > v.z)
            ?   0
            :       (!(v.y > v.z)
                ?   1
                :   2))
        :       (!(v.y > v.z)
            ?   1
            :   2)
        ;
    assert(mincomponentf3(v) == getcomponentf3(v,index));

    return index;
}


static inline float maxcomponentf3(float3_t v)
{
    return glm_max(v.x,glm_max(v.y, v.z));
}

static inline size_t maxcomponentf3index(float3_t v)
{
    size_t index =
            (v.x > v.y)
        ?       ((v.x > v.z)
            ?   0
            :       ((v.y > v.z)
                ?   1
                :   2))
        :       ((v.y > v.z)
            ?   1
            :   2)
        ;
    assert(maxcomponentf3(v) == getcomponentf3(v,index));

    return index;
}



/**
 * http://tavianator.com/fast-branchless-raybounding-box-intersections/
 */
static inline bool svo_fast_forward_intersects_f3(const float3_t lower, const float3_t upper, const float3_t raypos, const float3_t raydirinv)
{
    
    float3_t t1 = (lower - raypos) * raydirinv;
    float3_t t2 = (upper - raypos) * raydirinv;

    float tmin = maxcomponentf3(glm_min(t1, t2));
    float tmax = mincomponentf3(glm_max(t1, t2));

    return tmax >= 0 && tmax >= tmin;
}




static inline
float3_t squaref3(float3_t v)
{
    return v*v;
}


static inline
float sdistancef3(float3_t start, float3_t end)
{
    /**
    float3_t s = squaref3(end - start);
    return s.x + s.y + s.z;
    */

    ///v  ·  v   =   |v|^2
    {
        float3_t v = end - start;
        return glm_dot(v,v);
    }
}


/**
 * Tests if point is on the surface of (part of) a cube.
 * 
 * @param the point to be tested.
 * @param dir_upper a corner of the cube; the three adjacent faces of this cube will be
 *      tested against the point.
 */
static inline
bool is_on_surface(float3_t point, float3_t dir_upper,float epsilon=.0001)
{
    return glm_any(glm_lt(glm_abs(point - dir_upper), make_float3(epsilon)));
}






#endif
