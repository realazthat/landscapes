
#include "landscapes/svo_render.hpp"
#include "landscapes/mgl2glm.hpp"
#include "MathGeoLib.h"
#include "gtest/gtest.h"

#include <vector>
#include <fstream>
#include <tuple>

struct CameraMappingTest : public ::testing::Test {
    
protected:

 
    virtual void SetUp() {
        
        
        
        
    }

    virtual void TearDown() {
        
    }
};


Frustum generate_frustum()
{
    
}

svo::camera_mapping_t extract_mapping(const Frustum& camera)
{
    svo::camera_mapping_t camera_mapping;
    
    float3 corners[8];
    
    camera.GetCornerPoints(&corners[0]);
    
    // see https://github.com/juj/MathGeoLib/blob/b796d8514607bf02012f5856b0a8d8af5d0d6ee6/src/Geometry/Frustum.cpp#L787
    // upper left, near plane, far corner
    camera_mapping.source.A = svo::toglm(corners[0b011]);
    
    // upper right
    camera_mapping.source.B = svo::toglm(corners[0b010]);
    
    // lower right
    camera_mapping.source.C = svo::toglm(corners[0b000]);
    
    // lower left
    camera_mapping.source.D = svo::toglm(corners[0b001]);
    
    
    // upper left, near plane, far corner
    camera_mapping.target.A = svo::toglm(corners[0b111]);
    
    // upper right
    camera_mapping.target.B = svo::toglm(corners[0b110]);
    
    // lower right
    camera_mapping.target.C = svo::toglm(corners[0b100]);
    
    // lower left
    camera_mapping.target.D = svo::toglm(corners[0b101]);
    
    
    return camera_mapping;
}

TEST_F(CameraMappingTest,mgl_to_camera_mapping)
{
    
    auto camera = generate_frustum();
    
    auto mapping = extract_mapping(camera);
    
    
    
}




