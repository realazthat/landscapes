
#include "landscapes/mgl2glm.hpp"

#include "format.h"

namespace svo{
    


Frustum junk_frustum(){
    Frustum camera;
    
    // behind origin
    camera.SetPos(float3(0, 0, -10));
    
    float3 front = float3(1,0,1).Normalized();
    float3 up = float3(0,1,0);
    float3::Orthogonalize(front, up);
    up.Normalize();
    
    // facing front
    camera.SetFront(front);
    
    // vertical is up
    camera.SetUp(up);
    
    // simple frustum planes
    camera.SetViewPlaneDistances(20, 250);

    
    int screen_width = 100, screen_height = 100;
    
    float aspect_ratio = float(screen_width) / float(screen_height);
    camera.SetHorizontalFovAndAspectRatio(1, aspect_ratio);

    camera.SetKind(FrustumSpaceGL, FrustumRightHanded);
    return camera;
}

mgl_corners_mapping_t generate_mgl_corners_mapping(){
    mgl_corners_mapping_t mgl_corners_mapping;
    
    Frustum camera = junk_frustum();
    
    float3 corners[8];
    camera.GetCornerPoints(&corners[0]);
    
    //for (int i = 0; i < 8; ++i)
        //std::cout << "corners[" << i << "]: " << corners[i] << std::endl;
    
    
    
    for (int x = -1; x <= +1; x += 2)
    for (int y = -1; y <= +1; y += 2)
    {
        auto nearPlanePos = (camera.NearPlanePos(x,y));
        auto farPlanePos = (camera.FarPlanePos(x,y));
        
        //std::cout << "(x,y): " << "(" << x << "," << y << ")"
        //    << ", nearPlanePos: " << nearPlanePos
        //    << ", farPlanePos: " << farPlanePos
        //    << std::endl;
        
        for (int i = 0; i < 8; ++i)
        {
            //std::cout << "  i: " << i << std::endl;
            if (corners[i].Equals(nearPlanePos))
            {
                //std::cout << "  corners[i].Equals(nearPlanePos)" << std::endl;
                if (mgl_corners_mapping.count(std::make_tuple(x,y,-1)) > 0)
                    throw std::runtime_error("This camera has two frustum corners in the same location");
                mgl_corners_mapping[std::make_tuple(x,y,-1)] = i;
            }
            if (corners[i].Equals(farPlanePos))
            {
                //std::cout << "  corners[i].Equals(farPlanePos)" << std::endl;
                if (mgl_corners_mapping.count(std::make_tuple(x,y,+1)) > 0)
                    throw std::runtime_error("This camera has two frustum corners in the same location");
                mgl_corners_mapping[std::make_tuple(x,y,+1)] = i;
            }
        }
        
        assert( mgl_corners_mapping.count(std::make_tuple(x,y,-1)) > 0 );
        assert( mgl_corners_mapping.count(std::make_tuple(x,y,+1)) > 0 );
    }

#ifndef NDEBUG
    for (int x = -1; x <= +1; x += 2)
    for (int y = -1; y <= +1; y += 2)
    for (int z = -1; z <= +1; z += 2) {
        assert( mgl_corners_mapping.count(std::make_tuple(x,y,z)) > 0 );
        volatile auto a = mgl_corners_mapping.at(std::make_tuple(x,y,z));
    }
#endif
    return mgl_corners_mapping;
}



const mgl_corners_mapping_t mgl_corners_mapping = generate_mgl_corners_mapping();

} // namespace svo
