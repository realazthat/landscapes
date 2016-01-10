#ifndef SVO_CAMERA_MAPPING_CL_H
#define SVO_CAMERA_MAPPING_CL_H 1

#include "landscapes/svo_camera_mapping.cl.fwd.h"

#include "landscapes/opencl.shim.h"


typedef struct svo_camera_quad_t{
    /* four points of a frustum quad
     */
     float3_t top_left, top_right, bottom_right, bottom_left;
    
} svo_camera_quad_t;

typedef struct svo_camera_mapping_t{
    svo_camera_quad_t source;
    svo_camera_quad_t target;
} svo_camera_mapping_t;

typedef struct svo_ray_t{
    float3_t source;
    float3_t target;
} svo_ray_t;

/*
 * Given a quad, and u,v coordinates in the range of [0,1], starting from the bottom-left of the screen,
 * this will do bilinear interpolation and find the location on the quad
 * represented by (u,v). A,B,C,D are equivalent to the variables in svo_camera_quad_t.
 */
static inline float3_t svo_quad_bilinear(float3_t top_left, float3_t top_right, float3_t bottom_right, float3_t bottom_left, float u, float v);

/*
 * convert u,v coordinates and a camera-mapping to a ray.
 *
 * coordinates each in the range [-1,1], where (-1,-1) is the bottom-left corner and
 * (1,1) is the top-right corner.
 */
static inline svo_ray_t svo_uv_to_ray(const svo_camera_mapping_t* mapping, float u, float v);










static inline float3_t svo_quad_bilinear(float3_t top_left, float3_t top_right, float3_t bottom_right, float3_t bottom_left, float u, float v)
{
    assert(u >= 0);
    assert(v >= 0);
    assert(u <= 1);
    assert(v <= 1);
    
    // See http://www.iquilezles.org/www/articles/ibilinear/ibilinear.htm
    float3_t dAB = (top_right - top_left);
    float3_t dDC = (bottom_right - bottom_left);
    
    float3_t P = top_left + dAB*u;
    float3_t Q = bottom_left + dDC*u;
    
    float3_t dQP = P - Q;
    
    float3_t X = Q + dQP*v;
    
    return X;
}

static inline svo_ray_t svo_uv_to_ray(const svo_camera_mapping_t* mapping, float u, float v)
{
    assert(u >= -1);
    assert(v >= -1);
    assert(u <= +1);
    assert(v <= +1);
    
    // adjust the coords to the range [0,1]
    u = (u + 1) / 2;
    v = (v + 1) / 2;
    
    svo_ray_t ray;
    
    ray.source = svo_quad_bilinear( mapping->source.top_left, mapping->source.top_right
                              , mapping->source.bottom_right, mapping->source.bottom_left
                              , u, v);
    ray.target = svo_quad_bilinear( mapping->target.top_left, mapping->target.top_right
                              , mapping->target.bottom_right, mapping->target.bottom_left
                              , u, v);
    
    return ray;
}


static inline int32_t svo_camera_quad_max_width(const svo_camera_quad_t* quad)
{
    int32_t bottom_width = glm_length(quad->bottom_right - quad->bottom_left);
    int32_t top_width = glm_length(quad->top_right - quad->top_left);
    
    return glm_max(bottom_width, top_width);
}






#endif
