
#include "landscapes/svo_curves.h"
#include "gtest/gtest.h"
#include <fstream>
#include <vector>
#include <algorithm>

class ZOrderTest : public ::testing::Test {
protected:
    virtual void SetUp() {
      
    }

    virtual void TearDown() {
    // Code here will be called immediately after each test
    // (right before the destructor).
    }
    };





TEST_F(ZOrderTest,coords2vcurve){

    std::vector<bool> vcurves_seen(SVO_MAX_VCURVE, false);

    for (vside_t x = 0; x < SVO_MAX_VSIDE; ++x)
    for (vside_t y = 0; y < SVO_MAX_VSIDE; ++y)
    for (vside_t z = 0; z < SVO_MAX_VSIDE; ++z)
    {
        auto vcurve0 = coords2vcurve_brute(x,y,z,SVO_MAX_VSIDE);
        auto vcurve = coords2vcurve(x,y,z,SVO_MAX_VSIDE);
    
        
        
        ASSERT_LT(vcurve, SVO_MAX_VCURVE);
        ASSERT_EQ(vcurve0, vcurve);
        ASSERT_LT(vcurve, vcurves_seen.size());
        ASSERT_FALSE(vcurves_seen[vcurve]);

        vcurves_seen[vcurve] = true;
        ASSERT_TRUE(vcurves_seen[vcurve]);

        vside_t x1, y1, z1;
        vcurve2coords(vcurve, SVO_MAX_VSIDE, &x1, &y1, &z1);

        ASSERT_EQ(x, x1);
        ASSERT_EQ(y, y1);
        ASSERT_EQ(z, z1);
        
    }

    ASSERT_TRUE(std::all_of(vcurves_seen.begin(), vcurves_seen.end(), [](bool value){ return value; }) );

    
}

