
#include "landscapes/mcloader.hpp"
#include "landscapes/svo_tree.hpp"
#include "gtest/gtest.h"
#include <fstream>

class SliceToBlocksTest : public ::testing::Test {
    const svo::volume_of_slices_t* volume_of_slices;
    svo::svo_slice_t root;
    
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


        {
            ///enough voxels to pack it all into one slice on the bottom level
            std::size_t max_voxels_per_slice = 16*16*16*8;
            root = svo::svo_entree_slices(*volume_of_slices, max_voxels_per_slice/*max_voxels_per_slice*/);
            
            
            
            auto error = svo::svo_slice_sanity(root, svo::svo_sanity_type_t::default_sanity, 1000000);

            assert(!error);
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





TEST_F(SliceToBlocksTest,initialize_slice_data){
    
    svo::svo_block_t* svo_block = svo::
    
    std::size_t blocks = 1000;
    std::size_t block_size = 1024*1024/2;
    std::size_t tree_size = blocks*block_size;
    
    std::unique_ptr<svo_tree_t> tree( new svo_tree_t(tree_size, block_size) );
    
    
}



TEST_F(SliceToBlocksTest,load_next_slice){
    
    
    
}












