
#include "landscapes/mcloader.hpp"
#include "landscapes/svo_tree.hpp"
#include "landscapes/svo_serialization.v1.hpp"
#include "landscapes/svo_tree.sanity.hpp"
#include "gtest/gtest.h"
#include <fstream>
#include <sstream>

struct SerializeSliceTest : public ::testing::Test {
    svo::svo_slice_t* m_slice0;
    const svo::svo_slice_t* slice0;
protected:
 
    virtual void SetUp() {
      
        svo::volume_of_slices_t wanted_slices(32,16);
        
        vcurve_t slice_vcurve = 0;
        m_slice0 = svo::svo_init_slice(0, 16);
        wanted_slices.slices.push_back( std::make_tuple( slice_vcurve, m_slice0) );
        

        std::ifstream infile("../libs/test-region/r.0.0.mca", std::ios::binary);

        ASSERT_TRUE(infile);

        svo::load_mca_region(wanted_slices, infile);
        
        slice0 = m_slice0;
    }

    virtual void TearDown() {
    // Code here will be called immediately after each test
    // (right before the destructor).
        svo::svo_uninit_slice(m_slice0, true);
    }
};





TEST_F(SerializeSliceTest,serialize){
    
    std::ostringstream out;
    
    svo::svo_serialize_slice(out, slice0);
    
    
    std::istringstream in(out.str());
    
    svo::svo_slice_t* slice1 = svo::svo_init_slice(0, 16);
    svo::svo_unserialize_slice(in,slice1,true);
    
    ASSERT_TRUE( !svo::svo_slice_sanity(slice0) );
    EXPECT_EQ(slice0->side, slice1->side);
    EXPECT_EQ(slice0->level, slice1->level);
    EXPECT_EQ(slice0->parent_vcurve_begin, slice1->parent_vcurve_begin);
    ASSERT_EQ(slice0->pos_data->size(), slice1->pos_data->size());
    ASSERT_EQ(slice0->buffers->schema(), slice1->buffers->schema());
    ASSERT_EQ(slice0->buffers->entries(), slice1->buffers->entries());
    
    std::size_t entries = slice0->pos_data->size();
    for (std::size_t entry_index = 0; entry_index < entries; ++entry_index)
    {
        ASSERT_LT(entry_index, slice0->pos_data->size());
        ASSERT_LT(entry_index, slice1->pos_data->size());
        
        vcurve_t pos0 = (*slice0->pos_data)[entry_index];
        vcurve_t pos1 = (*slice1->pos_data)[entry_index];
        
        
        EXPECT_EQ(pos0, pos1);
        EXPECT_LT(pos0, vcurvesize(slice0->side));
        
        
        
    }
    
    const auto& buffers0 = *slice0->buffers;
    const auto& buffers1 = *slice1->buffers;
    const auto& buffers0_list = buffers0.buffers();
    const auto& buffers1_list = buffers1.buffers();
    
    buffers0.assert_invariants();
    buffers1.assert_invariants();
    
    for (std::size_t buffer_index = 0; buffer_index < buffers0_list.size(); ++buffer_index)
    {
        ASSERT_LT(buffer_index, buffers0_list.size());
        ASSERT_LT(buffer_index, buffers1_list.size());
        
        const auto& buffer0 = buffers0_list[buffer_index];
        const auto& buffer1 = buffers1_list[buffer_index];
        
        buffer0.assert_invariants();
        buffer1.assert_invariants();
    
        
        ASSERT_LT(buffer0.entries(), buffer1.entries());
        ASSERT_LT(buffer0.stride(), buffer1.stride());
        
        int buffer_cmp = std::memcmp(buffer0.rawdata(), buffer1.rawdata(), buffer0.stride()*buffer0.entries());
        
        EXPECT_EQ(0, buffer_cmp);
    }
}


TEST_F(SerializeSliceTest,unserialize){
    
    std::ostringstream out0;
    svo::svo_serialize_slice(out0, slice0);
    
    
    
    const auto& str0 = out0.str();
    std::vector<uint8_t> serialized_data0(str0.data(), str0.data()+str0.size());
    



    
    std::istringstream in1(str0);
    svo::svo_slice_t* slice1 = svo::svo_init_slice(0, 16);
    svo::svo_unserialize_slice(in1,slice1,true);
    
    
    std::ostringstream out1;
    svo::svo_serialize_slice(out1, slice1);
    
    
    
    
    const auto& str1 = out1.str();
    std::vector<uint8_t> serialized_data1(str1.data(), str1.data() + str1.size());
    
    ASSERT_EQ(serialized_data0.size(), serialized_data1.size());
    
    int serial_cmp = std::memcmp(serialized_data0.data(), serialized_data1.data(), serialized_data0.size());
    
    EXPECT_EQ(0, serial_cmp);
}

