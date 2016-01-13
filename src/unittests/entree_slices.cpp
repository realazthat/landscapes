
#include "landscapes/mcloader.hpp"
#include "landscapes/svo_tree.hpp"
#include "landscapes/svo_tree.slice_mgmt.hpp"
#include "landscapes/svo_tree.sanity.hpp"
#include "gtest/gtest.h"
#include "main.hpp"

#include <vector>
#include <fstream>
#include <tuple>

struct EntreeSlicesTest : public ::testing::Test {
    const svo::volume_of_slices_t* volume_of_slices;
protected:
 
    virtual void SetUp() {
        volume_of_slices = global_env->volume_of_slices;
        
    }

    virtual void TearDown() {
    // Code here will be called immediately after each test
    // (right before the destructor).
        
    }
};





TEST_F(EntreeSlicesTest,entree_slices)
{
    
    {
        ///enough voxels to pack it all into one slice on the bottom level
        std::size_t max_voxels_per_slice = 16*16*16*8;
        auto* root = svo::svo_entree_slices(*volume_of_slices, max_voxels_per_slice/*max_voxels_per_slice*/);
        
        ASSERT_NE(root, nullptr);
        
        
        auto error = svo::svo_slice_sanity(root, svo::svo_sanity_t::enum_t::default_sanity, 1000000);
        
        ASSERT_FALSE(error);
        
        
        
        ASSERT_EQ(root->side, vside_t(1));
        
    }
    
    
    {
        ///least number of voxels possible
        std::size_t max_voxels_per_slice = 8;
        auto* root = svo::svo_entree_slices(*volume_of_slices, max_voxels_per_slice/*max_voxels_per_slice*/);
        
        ASSERT_NE(root, nullptr);
        
        
        auto error = svo::svo_slice_sanity(root, svo::svo_sanity_t::enum_t::default_sanity, 1000000);
        
        ASSERT_FALSE(error);
        
        
        
        ASSERT_EQ(root->side, vside_t(1));
        
    }
}




