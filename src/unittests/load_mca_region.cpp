
#include "landscapes/mcloader.hpp"
#include "landscapes/svo_tree.hpp"
#include "gtest/gtest.h"
#include <fstream>

class LoadMCARegionTest : public ::testing::Test {
protected:
    virtual void SetUp() {
      
    }

    virtual void TearDown() {
    // Code here will be called immediately after each test
    // (right before the destructor).
    }
    };





TEST_F(LoadMCARegionTest,load_mca_region){
    
    
    svo::volume_of_slices_t wanted_slices(32,16);
    
    for (vcurve_t slice_vcurve = 0; slice_vcurve < vcurvesize(32); ++slice_vcurve)
    {
        vside_t sx, sy, sz;
        vcurve2coords(slice_vcurve, 32, &sx, &sy, &sz);
        
        if (sy > 16)
            continue;
        
        auto* slice = svo::svo_init_slice(0, 16);
        assert(slice->side == 16);
        wanted_slices.slices.push_back( std::make_tuple( slice_vcurve, slice) );
    }
    
    std::ifstream infile("../libs/test-region/r.0.0.mca", std::ios::binary);
    
    ASSERT_TRUE(infile);
    
    svo::load_mca_region(wanted_slices, infile);
    
    
    
}

