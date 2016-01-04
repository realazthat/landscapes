#ifndef SVO_MGL2GLM_HPP
#define SVO_MGL2GLM_HPP


#include "MathGeoLib.h"
#include "landscapes/opencl.shim.h"
#include "landscapes/svo_camera_mapping.cl.h"
#include <map>

namespace svo{
    
    static inline glm::vec3 toglm(float3 v);
    static inline glm::vec4 toglm(float4 v);
    
    
    static inline svo_camera_mapping_t extract_mapping(const Frustum& camera);
    
    
    
    
    
    
    
    
    
    
    
    
    
    static inline glm::vec3 toglm(float3 v)
    {
        return float3_t(v.x,v.y,v.z);
    }

    static inline glm::vec4 toglm(float4 v)
    {
        return glm::vec4(v[0],v[1],v[2],v[3]);
    }

    typedef std::size_t mgl_corner_index_t;
    typedef std::map<std::tuple<int, int, int>, mgl_corner_index_t> mgl_corners_mapping_t;
    mgl_corners_mapping_t generate_mgl_corners_mapping();
    
    extern const mgl_corners_mapping_t mgl_corners_mapping;
    
    
    static inline svo_camera_mapping_t extract_mapping(const Frustum& camera)
    {
        // see https://github.com/juj/MathGeoLib/blob/b796d8514607bf02012f5856b0a8d8af5d0d6ee6/src/Geometry/Frustum.cpp#L787

        svo_camera_mapping_t camera_mapping;
        
        float3 corners[8];
        
        camera.GetCornerPoints(&corners[0]);
        
        
        std::size_t near_top_left_index = 0b010;
        std::size_t near_top_right_index = 0b011;
        std::size_t near_bottom_right_index = 0b001;
        std::size_t near_bottom_left_index = 0b000;
        
        std::size_t far_top_left_index = 0b110;
        std::size_t far_top_right_index = 01011;
        std::size_t far_bottom_right_index = 0b101;
        std::size_t far_bottom_left_index = 0b100;
        
        
        
        
        // top-left, near plane
        camera_mapping.source.top_left = svo::toglm(corners[near_top_left_index].xyz());
        
        // top-right
        camera_mapping.source.top_right = svo::toglm(corners[near_top_right_index].xyz());
        
        // bottom-right
        camera_mapping.source.bottom_right = svo::toglm(corners[near_bottom_right_index].xyz());
        
        // bottom-left
        camera_mapping.source.bottom_left = svo::toglm(corners[near_bottom_left_index].xyz());
        
        // checked
        // top-left, far plane
        camera_mapping.target.top_left = svo::toglm(corners[far_top_left_index].xyz());
        
        // top-right
        camera_mapping.target.top_right = svo::toglm(corners[far_top_right_index].xyz());
        
        // bottom-right
        camera_mapping.target.bottom_right = svo::toglm(corners[far_bottom_right_index].xyz());
        
        // checked
        // bottom-left
        camera_mapping.target.bottom_left = svo::toglm(corners[far_bottom_left_index].xyz());
        
#if NDEBUG
        assert( mgl_corners_mapping.count(std::make_tuple(-1,-1,-1)) > 0 );
        assert( mgl_corners_mapping.count(std::make_tuple(-1,-1,+1)) > 0 );
        assert(mgl_corners_mapping.at(std::make_tuple(-1,-1,-1)) == near_bottom_left_index);
        assert(mgl_corners_mapping.at(std::make_tuple(-1,-1,+1)) == far_bottom_left_index);
        
        assert( mgl_corners_mapping.count(std::make_tuple(+1,-1,-1)) > 0 );
        assert( mgl_corners_mapping.count(std::make_tuple(+1,-1,+1)) > 0 );
        assert(mgl_corners_mapping.at(std::make_tuple(+1,-1,-1)) == near_bottom_right_index);
        assert(mgl_corners_mapping.at(std::make_tuple(+1,-1,+1)) == far_bottom_right_index);
        
        assert( mgl_corners_mapping.count(std::make_tuple(-1,+1,-1)) > 0 );
        assert( mgl_corners_mapping.count(std::make_tuple(-1,+1,+1)) > 0 );
        assert(mgl_corners_mapping.at(std::make_tuple(-1,+1,-1)) == near_top_left_index);
        assert(mgl_corners_mapping.at(std::make_tuple(-1,+1,+1)) == far_top_left_index);
        
        assert( mgl_corners_mapping.count(std::make_tuple(+1,+1,-1)) > 0 );
        assert( mgl_corners_mapping.count(std::make_tuple(+1,+1,+1)) > 0 );
        assert(mgl_corners_mapping.at(std::make_tuple(+1,+1,-1)) == near_top_right_index);
        assert(mgl_corners_mapping.at(std::make_tuple(+1,+1,+1)) == far_top_right_index);
#endif
        
        
        return camera_mapping;
    }
    
    
    
} // namespace svo

#endif

