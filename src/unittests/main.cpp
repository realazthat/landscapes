#include <limits.h>
#include "gtest/gtest.h"

#include "main.hpp"

#include "landscapes/svo_tree.hpp"
#include "landscapes/mcloader.hpp"

#include <fstream>
#include <vector>

void TestEnvironment::SetUp()
{
    m_volume_of_slices = new svo::volume_of_slices_t(32,16);
    m_all_const_slices = new std::vector<const svo::svo_slice_t*>;
    m_all_slices = new std::vector<svo::svo_slice_t*>;
    all_const_slices = m_all_const_slices;
    volume_of_slices = m_volume_of_slices;
    
    
    for (vcurve_t slice_vcurve = 0; slice_vcurve < vcurvesize(32); ++slice_vcurve)
    {
        auto* slice = svo::svo_init_slice(0, 16);
        m_all_slices->push_back(slice);
        m_all_const_slices->push_back(slice);
        m_volume_of_slices->slices.push_back( std::make_tuple( slice_vcurve, slice ) );
    }

    std::ifstream infile("../libs/test-region/r.0.0.mca", std::ios::binary);

    ASSERT_TRUE(infile);

    svo::load_mca_region(*m_volume_of_slices, infile, 2/*num_threads*/);
    
}

void TestEnvironment::TearDown()
{
    assert(m_all_slices);
    for (auto* slice : *m_all_slices)
    {
        svo::svo_uninit_slice(slice,true);
    }
}

TestEnvironment* global_env = nullptr;

int main(int argc, char **argv){
    ::testing::InitGoogleTest(&argc, argv);

    global_env = new TestEnvironment();


    
    ::testing::AddGlobalTestEnvironment(global_env);
    
    return RUN_ALL_TESTS();
}

