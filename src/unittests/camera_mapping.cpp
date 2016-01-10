
#include "landscapes/svo_camera_mapping.cl.h"
#include "landscapes/svo_formatters.hpp"
#include "landscapes/mgl2glm.hpp"
#include "MathGeoLib.h"
#include "gtest/gtest.h"

#include <vector>
#include <fstream>
#include <tuple>

int screen_width = 100, screen_height = 100;

std::vector<Frustum> generate_test_params();
std::vector<Frustum> handmade_test_params();


struct CameraMappingTest : public ::testing::Test  {
    std::vector<Frustum> tests;
protected:

 
    virtual void SetUp() {
        auto generated_tests = generate_test_params();
        auto handmade_tests = handmade_test_params();
        
        tests.insert(tests.end(), generated_tests.begin(), generated_tests.end());
        
        tests.insert(tests.end(), handmade_tests.begin(), handmade_tests.end());
    }

    virtual void TearDown() {
        
    }
};




struct rng_t
{
    rng_t()
        : gen(0)
    {

    }

    int operator()(std::size_t minimum, std::size_t maximum)
    {
        std::uniform_int_distribution<> dis(minimum, maximum);

        return dis(gen);
    }

    std::random_device rd;
    std::mt19937 gen;
};

static ::std::ostream& operator<<(std::ostream& out, const svo::mgl_corners_mapping_t& mgl_corners_mapping)
{
    out << "{";
    for (auto w = mgl_corners_mapping.begin(); w != mgl_corners_mapping.end(); ++w )
    {
        auto key = w->first;
        out << "(" << std::get<0>(key) << ", " << std::get<1>(key) << ", " << std::get<2>(key)
                  << ") => " << w->second << (std::next(w) == mgl_corners_mapping.end() ? "" : ", ");
    }
    
    out << "}";
    return out;
}

TEST_F(CameraMappingTest,mgl_corners_mapping)
{

    
    auto mgl_corners_mapping = svo::generate_mgl_corners_mapping();
    
    for (int x = -1; x <= 1; x += 2)
    for (int y = -1; y <= 1; y += 2)
    for (int z = -1; z <= 1; z += 2)
    {
        auto key = std::make_tuple(x,y,z);
        ASSERT_GT(mgl_corners_mapping.count(key), std::size_t(0))
            << "key: " << std::get<0>(key) << ", " << std::get<1>(key) << ", " << std::get<2>(key)
            << ", mgl_corners_mapping: " << mgl_corners_mapping;
    }
    
}
TEST_F(CameraMappingTest,mgl_to_camera_mapping)
{
    
    for (std::size_t test_index = 0; test_index < tests.size(); ++test_index){
        const auto& camera = tests[test_index];
        
        // test NearPlanePos vs CornerPoint
        {
            auto cornerPoint = svo::toglm(camera.CornerPoint(0b000));
            auto nearPlanePos = svo::toglm(camera.NearPlanePos(-1,-1));
            
            for (int i = 0; i < 3; ++i)
                ASSERT_FLOAT_EQ(cornerPoint[i], nearPlanePos[i])
                    << "test_index: " << test_index
                    << ", cornerPoint: " << cornerPoint
                    << ", nearPlanePos: " << nearPlanePos;
        }
        
        // test NearPlanePos vs CornerPoint
        {
            auto cornerPoint = svo::toglm(camera.CornerPoint(0b010));
            auto nearPlanePos = svo::toglm(camera.NearPlanePos(-1,+1));
            
            for (int i = 0; i < 3; ++i)
                ASSERT_FLOAT_EQ(cornerPoint[i], nearPlanePos[i])
                    << "test_index: " << test_index
                    << ", cornerPoint: " << cornerPoint
                    << ", nearPlanePos: " << nearPlanePos;
        }
        
        // test NearPlanePos vs CornerPoint
        {
            auto cornerPoint = svo::toglm(camera.CornerPoint(0b100));
            auto nearPlanePos = svo::toglm(camera.NearPlanePos(+1,-1));
            
            for (int i = 0; i < 3; ++i)
                ASSERT_FLOAT_EQ(cornerPoint[i], nearPlanePos[i])
                    << "test_index: " << test_index
                    << ", cornerPoint: " << cornerPoint
                    << ", nearPlanePos: " << nearPlanePos;
        }
        
        // test NearPlanePos vs CornerPoint
        {
            auto cornerPoint = svo::toglm(camera.CornerPoint(0b110));
            auto nearPlanePos = svo::toglm(camera.NearPlanePos(+1,+1));
            
            for (int i = 0; i < 3; ++i)
                ASSERT_FLOAT_EQ(cornerPoint[i], nearPlanePos[i])
                    << "test_index: " << test_index
                    << ", cornerPoint: " << cornerPoint
                    << ", nearPlanePos: " << nearPlanePos;
        }
        
        // test FarPlanePos vs CornerPoint
        {
            auto cornerPoint = svo::toglm(camera.CornerPoint(0b001));
            auto farPlanePos = svo::toglm(camera.FarPlanePos(-1,-1));
            
            for (int i = 0; i < 3; ++i)
                ASSERT_FLOAT_EQ(cornerPoint[i], farPlanePos[i])
                    << "cornerPoint: " << cornerPoint
                    << ", farPlanePos: " << farPlanePos;
        }
        
        // test FarPlanePos vs CornerPoint
        {
            auto cornerPoint = svo::toglm(camera.CornerPoint(0b011));
            auto farPlanePos = svo::toglm(camera.FarPlanePos(-1,+1));
            
            for (int i = 0; i < 3; ++i)
                ASSERT_FLOAT_EQ(cornerPoint[i], farPlanePos[i])
                    << "cornerPoint: " << cornerPoint
                    << ", farPlanePos: " << farPlanePos;
        }
        
        // test FarPlanePos vs CornerPoint
        {
            auto cornerPoint = svo::toglm(camera.CornerPoint(0b101));
            auto farPlanePos = svo::toglm(camera.FarPlanePos(+1,-1));
            
            for (int i = 0; i < 3; ++i)
                ASSERT_FLOAT_EQ(cornerPoint[i], farPlanePos[i])
                    << "cornerPoint: " << cornerPoint
                    << ", farPlanePos: " << farPlanePos;
        }
        
        // test FarPlanePos vs CornerPoint
        {
            auto cornerPoint = svo::toglm(camera.CornerPoint(0b111));
            auto farPlanePos = svo::toglm(camera.FarPlanePos(+1,+1));
            
            for (int i = 0; i < 3; ++i)
                ASSERT_FLOAT_EQ(cornerPoint[i], farPlanePos[i])
                    << "cornerPoint: " << cornerPoint
                    << ", farPlanePos: " << farPlanePos;
        }
        
        
        
        auto mapping = svo::extract_mapping(camera);
        
        
        
        // test mapping vs CornerPoint
        {
            auto nearPlanePos = svo::toglm(camera.NearPlanePos(-1,-1));
            auto farPlanePos = svo::toglm(camera.FarPlanePos(-1,-1));
            
            for (int i = 0; i < 3; ++i)
                ASSERT_FLOAT_EQ(nearPlanePos[i], mapping.source.bottom_left[i])
                    << "nearPlanePos: " << nearPlanePos
                    << ", mapping.source.bottom_left: " << mapping.source.bottom_left;
            for (int i = 0; i < 3; ++i)
                ASSERT_FLOAT_EQ(farPlanePos[i], mapping.target.bottom_left[i])
                    << "farPlanePos: " << farPlanePos
                    << ", mapping.target.bottom_left: " << mapping.target.bottom_left;
        }
        
        
        // test mapping vs CornerPoint
        {
            auto nearPlanePos = svo::toglm(camera.NearPlanePos(-1,+1));
            auto farPlanePos = svo::toglm(camera.FarPlanePos(-1,+1));
            
            for (int i = 0; i < 3; ++i)
                ASSERT_FLOAT_EQ(nearPlanePos[i], mapping.source.top_left[i])
                    << "nearPlanePos: " << nearPlanePos
                    << ", mapping.source.bottom_left: " << mapping.source.top_left;
            for (int i = 0; i < 3; ++i)
                ASSERT_FLOAT_EQ(farPlanePos[i], mapping.target.top_left[i])
                    << "farPlanePos: " << farPlanePos
                    << ", mapping.target.bottom_left: " << mapping.target.top_left;
        }
        
        
        
        
        // test mapping vs CornerPoint
        {
            auto nearPlanePos = svo::toglm(camera.NearPlanePos(+1,-1));
            auto farPlanePos = svo::toglm(camera.FarPlanePos(+1,-1));
            
            
            
            for (int i = 0; i < 3; ++i)
                ASSERT_FLOAT_EQ(nearPlanePos[i], mapping.source.bottom_right[i])
                    << "nearPlanePos: " << nearPlanePos
                    << ", mapping.source.bottom_left: " << mapping.source.bottom_right;
            for (int i = 0; i < 3; ++i)
                ASSERT_FLOAT_EQ(farPlanePos[i], mapping.target.bottom_right[i])
                    << "farPlanePos: " << farPlanePos
                    << ", mapping.target.bottom_left: " << mapping.target.bottom_right;
        }
        
        /*
        auto uv0ray0 = svo::mglray_to_ray(camera.UnProject(-1,-1), camera);
        
        {
            auto nearPlanePos = svo::toglm(camera.NearPlanePos(-1,-1));
            for (int i = 0; i < 3; ++i)
                ASSERT_FLOAT_EQ(nearPlanePos[i], uv0ray0.source[i])
                    << "camera.NearPlanePos(-1,-1): " << nearPlanePos
                    << ", uv0ray0.source: " << uv0ray0.source
                    << ", NearPlainDistance: " << camera.NearPlaneDistance()
                    << ", d(pos0, uv0ray0.source): " << glm::length(uv0ray0.source - svo::toglm(camera.Pos()))
                    << ", d(pos0, nearPlanePos): " << glm::length(nearPlanePos - svo::toglm(camera.Pos()))
                    << ", d(pos0, camera.UnProject(-1,-1).pos): " << glm::length(svo::toglm(camera.UnProject(-1,-1).pos)
                                                                                - svo::toglm(camera.Pos()))
                    << "\n"
                    << "  dir(pos0, uv0ray0.source): " << glm::normalize(uv0ray0.source - svo::toglm(camera.Pos()))
                    << ", dir(pos0, nearPlanePos): " << glm::normalize(nearPlanePos - svo::toglm(camera.Pos()))
                    << ", camera.UnProject(-1,-1).pos: " << svo::toglm(camera.UnProject(-1,-1).pos)
                    << ", camera.UnProject(-1,-1).dir: " << svo::toglm(camera.UnProject(-1,-1).dir)
                    << ", camera.UnProjectFromNearPlane(-1,-1).pos: " << svo::toglm(camera.UnProjectFromNearPlane(-1,-1).pos)
                    << ", camera.UnProjectFromNearPlane(-1,-1).dir: " << svo::toglm(camera.UnProjectFromNearPlane(-1,-1).dir)

                    << ", camera.UnProjectFromNearPlane(-1,-1).dir: " << svo::toglm(camera.UnProjectFromNearPlane(-1,-1).dir)

                    ;
        }
        
        auto uv0ray1 = uv_to_ray(&mapping, -1,-1);
        for (int i = 0; i < 3; ++i)
            ASSERT_FLOAT_EQ(uv0ray0.source[i], mapping.source.bottom_left[i])
                << "uv0ray0.source: " << uv0ray0.source
                << ", mapping.source.lower_left: " << mapping.source.bottom_left;
        for (int i = 0; i < 3; ++i)
            ASSERT_FLOAT_EQ(uv0ray0.source[i], uv0ray1.source[i])
                << "uv0ray0.source: " << uv0ray0.source
                << ", uv0ray1.source: " << uv0ray1.source;
        */
    }
}


TEST_F(CameraMappingTest,svo_uv_to_ray)
{

    for (std::size_t test_index = 0; test_index < tests.size(); ++test_index){
        const auto& camera = tests[test_index];
        
        float3 corners[8];
        camera.GetCornerPoints(&corners[0]);
        auto mgl_corners_mapping = svo::generate_mgl_corners_mapping();
        
        auto camera_mapping = svo::extract_mapping(camera);
        
        /*
        for (int u = -1; u <= 1; u += 2)
        for (int v = -1; v <= 1; v += 2)
        {
            ASSERT_TRUE(mgl_corners_mapping.count(std::make_tuple(u,v,-1)) > 0)
                 << "u: " << u << ", v: " << v;
            ASSERT_LT(mgl_corners_mapping.at(std::make_tuple(u,v,-1)), 8UL)
                 << "u: " << u << ", v: " << v
                 << "mgl_corners_mapping.at(std::make_tuple(u,v,-1)): "
                        << mgl_corners_mapping.at(std::make_tuple(u,v,-1));
            auto nearCornerPos = corners[mgl_corners_mapping.at(std::make_tuple(u,v,-1))];
        }
         */
        
        for (float u = -1; u <= 1; u += 2.0/screen_width)
        for (float v = -1; v <= 1; v += 2.0/screen_height)
        {
            
            auto ray0 = camera.UnProjectFromNearPlane(u,v);
            auto nearPlanePos = svo::toglm(camera.NearPlanePos(u,v));
            
            
            auto ray1 = svo_uv_to_ray(&camera_mapping, u, v);
            
            auto pos0 = svo::toglm(ray0.pos);
            auto dir0 = glm::normalize(svo::toglm(ray0.dir));
            
            auto pos1 = ray1.source;
            auto dir1 = glm::normalize(ray1.target - ray1.source);
            
            for (int i = 0; i < 3; ++i)
                ASSERT_NEAR(pos0[i], pos1[i], .00001)
                        << "u: " << u << ", v: " << v
                        << ", camera.Pos(): " << camera.Pos()
                        << ", camera.HorizontalFov(): " << camera.HorizontalFov()
                        << ", camera.VerticalFov(): " << camera.VerticalFov()
                        << "\n  nearPlanePos: " << nearPlanePos
                        << "\n  pos0: " << pos0
                        << "\n  dir0: " << dir0
                        << "\n  ray1.source: " << ray1.source
                        << "\n  ray1.target: " << ray1.target
                        << "\n  dir(ray1): " << glm::normalize(ray1.target - ray1.source)
                        << "\n  camera_mapping: " << camera_mapping
                        << "\n  test_index: " << test_index;
            /*
            for (int i = 0; i < 3; ++i)
                ASSERT_NEAR(dir0[i], dir1[i], .1)
                        << "u: " << u << ", v: " << v
                        << ", camera.Pos(): " << camera.Pos()
                        << ", pos0: " << pos0 << ", pos1: " << pos1
                        << ", dir0: " << dir0 << ", dir1: " << dir1
                        << ", ray1.target: " << ray1.target;
            */
            
        }
    }
}



std::vector<Frustum> handmade_test_params(){
    
    
    std::vector<Frustum> tests;
    
    
        
    // hand-made tests
    {
        Frustum camera;
        

        float3 front = float3(0,0,1).Normalized();
        float3 up = float3(0,1,0);
        float3::Orthogonalize(front, up);
        up.Normalize();
        
        // position eye at origin
        camera.SetPos(float3(0,0,0));
        
        // facing front
        camera.SetFront(front);
        
        // vertical is up
        camera.SetUp(up);
        
        // simple frustum planes
        camera.SetViewPlaneDistances(5, 250);

        //camera.horizontalFov = 3.141592654f/2.f;
        

        float aspect_ratio = float(screen_width) / float(screen_height);
        camera.SetHorizontalFovAndAspectRatio(1, aspect_ratio);

        camera.SetKind(FrustumSpaceGL, FrustumRightHanded);
        tests.push_back(camera);
    }
    
    
    return tests;
}


template<typename rng_t>
Frustum generate_frustum(rng_t& rng)
{
    Frustum camera;
    float k = 10;
    float c = 5;
    
    float near_clip = rng(2,k);
    float far_clip = near_clip + rng(1, k);
    
    
    
    float3 front = float3(rng(-k,k)/c, rng(-k,k)/c, rng(-k,k)/c).Normalized();
    float3 up = float3(rng(-k,k)/c, rng(-k,k)/c, rng(-k,k)/c).Normalized();
    float3::Orthogonalize(front, up);
    up.Normalize();
    
    camera.SetPos(float3(rng(-k,k)/c, rng(-k,k)/c, rng(-k,k)/c));
    // facing front
    camera.SetFront(front);
    
    // vertical is up
    camera.SetUp(up);
    
    camera.SetViewPlaneDistances(near_clip, far_clip);

    


    float aspect_ratio = float(screen_width) / float(screen_height);
    camera.SetHorizontalFovAndAspectRatio(1, aspect_ratio);

    
    
    camera.SetKind(FrustumSpaceGL, FrustumRightHanded);


    assert(camera.Type() == PerspectiveFrustum);
    
    return camera;
}

std::vector<Frustum> generate_test_params(){
    
    rng_t rng;
    
    int random_test_count = 10000;
    
    std::vector<Frustum> tests;
    
    auto is_float_valid = [](const float v)
    {
        if (std::isinf(v) || std::isnan(v))
                return false;
        return true;
    };
    auto is_vec_valid = [is_float_valid](const float3& v)
    {
        for (std::size_t i = 0; i < 3; ++i)
            if (!is_float_valid(v[i]))
                return false;
        return true;
    };
    
    auto is_camera_valid = [is_float_valid,is_vec_valid](const Frustum& camera)
    {
        if (!is_vec_valid(camera.Pos()))
            return false;
        if (!is_vec_valid(camera.Front()))
            return false;
        if (!is_vec_valid(camera.Up()))
            return false;
        if (!is_float_valid(camera.NearPlaneDistance()))
            return false;
        if (!is_float_valid(camera.FarPlaneDistance()))
            return false;
        if (!is_float_valid(camera.HorizontalFov()))
            return false;
        if (!is_float_valid(camera.VerticalFov()))
            return false;
        for (std::size_t i = 0; i < 8; ++i)
            if (!is_vec_valid(camera.CornerPoint(i)))
                return false;
        for (int x = -1; x <= 1; x += 2)
        for (int y = -1; y <= 1; y += 2)
        {
            if (!is_vec_valid(camera.NearPlanePos(x,y)))
                return false;
            if (!is_vec_valid(camera.FarPlanePos(x,y)))
                return false;
            
        
        }
        return true;
    };
    
    for (int i = 0; i < random_test_count; ++i)
    {
        auto camera = generate_frustum(rng);
        
        if (!is_camera_valid(camera))
            continue;
        tests.push_back(camera);
    }
    return tests;
}
