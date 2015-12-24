
#include "landscapes/mcloader.hpp"
#include "landscapes/svo_tree.hpp"
#include "landscapes/svo_tree.slice_mgmt.hpp"
#include "landscapes/svo_tree.sanity.hpp"
#include "gtest/gtest.h"

#include <vector>
#include <fstream>
#include <tuple>

struct EntreeSlicesTest : public ::testing::Test {
    const svo::volume_of_slices_t* volume_of_slices;
protected:
    svo::volume_of_slices_t* m_volume_of_slices;
 
    virtual void SetUp() {
        
        m_volume_of_slices = new svo::volume_of_slices_t(32,16);
        
        
        ///lets put in 8 slices, and see if we get one
        for (vcurve_t slice_vcurve = 0; slice_vcurve < 32*32*32; ++slice_vcurve)
        {
            auto* slice = svo::svo_init_slice(0, 16);
            m_volume_of_slices->slices.push_back( std::make_tuple( slice_vcurve, slice ) );
        }

        std::ifstream infile("../libs/test-region/r.0.0.mca", std::ios::binary);

        ASSERT_TRUE(infile);

        svo::load_mca_region(*m_volume_of_slices, infile);
        
        volume_of_slices = m_volume_of_slices;

        
        for ( const auto& vcurve_slice_pair : volume_of_slices->slices)
        {
            const auto* slice = std::get<1>(vcurve_slice_pair);
            
            //std::cout << "slice->pos_data->size(): " << slice->pos_data->size() << std::endl;
        }
        
        
    }

    virtual void TearDown() {
    // Code here will be called immediately after each test
    // (right before the destructor).
        
        for (auto& vcurve_slice_pair : m_volume_of_slices->slices)
        {
            vcurve_t slice_vcurve; svo::svo_slice_t* slice;
            std::tie(slice_vcurve, slice) = vcurve_slice_pair;
            
            svo::svo_uninit_slice(slice,true);
        }
    }
};





TEST_F(EntreeSlicesTest,entree_slices)
{
    
    {
        ///enough voxels to pack it all into one slice on the bottom level
        std::size_t max_voxels_per_slice = 16*16*16*8;
        auto* root = svo::svo_entree_slices(*volume_of_slices, max_voxels_per_slice/*max_voxels_per_slice*/);
        
        ASSERT_NE(root, nullptr);
        
        
        auto error = svo::svo_slice_sanity(root, svo::svo_sanity_type_t::default_sanity, 1000000);
        
        ASSERT_FALSE(error);
        
        
        
        ASSERT_EQ(root->side, vside_t(1));
        
    }
    
    
    {
        ///least number of voxels possible
        std::size_t max_voxels_per_slice = 8;
        auto* root = svo::svo_entree_slices(*volume_of_slices, max_voxels_per_slice/*max_voxels_per_slice*/);
        
        ASSERT_NE(root, nullptr);
        
        
        auto error = svo::svo_slice_sanity(root, svo::svo_sanity_type_t::default_sanity, 1000000);
        
        ASSERT_FALSE(error);
        
        
        
        ASSERT_EQ(root->side, vside_t(1));
        
    }
}




