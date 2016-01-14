
#include "landscapes/svo_inttypes.h"
#include "landscapes/svo_curves.h"
#include "landscapes/svo_buffer.fwd.hpp"
#include "gtest/gtest.h"
#include <fstream>
#include <bitset>

class ConstantsTest : public ::testing::Test {
protected:
    virtual void SetUp() {
      
    }

    virtual void TearDown() {
    // Code here will be called immediately after each test
    // (right before the destructor).
    }
    };



template<typename T>
T n_ones(std::size_t n)
{
    static const T all_ones = ~T(0);
    
    if (n == 0)
        return T(0);
    if (n >= sizeof(T)*8)
        return all_ones;
    
    T result = all_ones >> (sizeof(T)*8 - n);
    
    
    return result;
}



TEST_F(ConstantsTest,constants){


    //std::cout << std::bitset<32>(offset4_uint16_mask) << std::endl;
    //std::cout << std::bitset<32>(n_ones<offset4_uint16_t>(16)) << std::endl;
    EXPECT_EQ(byte_mask, 255);
    ASSERT_EQ(goffset_mask, n_ones<goffset_t>(32));
    EXPECT_EQ(offset_mask, n_ones<offset_t>(32));
    EXPECT_EQ(offset4_mask, n_ones<offset4_t>(32));
    EXPECT_EQ(far_ptr_mask, n_ones<far_ptr_t>(32));
    EXPECT_EQ(offset4_uint16_mask, n_ones<offset4_uint16_t>(16));
    EXPECT_EQ(child_mask_mask, 255);


    EXPECT_EQ(std::size_t(1) << SVO_VSIDE_BITS, SVO_VSIDE_LIMIT) << "SVO_VSIDE_BITS: " << SVO_VSIDE_BITS;

    // vcurve_t has to be able to encode 3 vside_t values
    // the largest vside_t value we can accomodate has to
    // fit in to 1/3 of the value-space of a vcurve_t.
    EXPECT_EQ(SVO_VSIDE_LIMIT*SVO_VSIDE_LIMIT*SVO_VSIDE_LIMIT, SVO_VCURVE_LIMIT);
    EXPECT_LE(SVO_VSIDE_BITS, 8*sizeof(vcurve_t)/3);
    
    
    /// svo::svo_semantic_t enum sanity
    {
        auto all_semantics_set1 = std::set<svo::svo_semantic_t>(svo::all_semantics.begin(), svo::all_semantics.end());
        EXPECT_EQ(svo::all_semantics_set, all_semantics_set1);
        
        for (svo::svo_semantic_t semantic : svo::all_semantics_set)
        {
            // the switch should generate a warning if we are missing a semantic
            switch (semantic)
            {
                case(svo::svo_semantic_t::NONE):
                case(svo::svo_semantic_t::COLOR):
                case(svo::svo_semantic_t::NORMAL):
                    continue;
            }
        }
        
        
        EXPECT_TRUE(svo::all_semantics_set.count(svo::svo_semantic_t::NONE));
        EXPECT_TRUE(svo::all_semantics_set.count(svo::svo_semantic_t::COLOR));
        EXPECT_TRUE(svo::all_semantics_set.count(svo::svo_semantic_t::NORMAL));
    }
    
    /// svo::svo_data_type_t enum sanity
    {
        auto all_buffer_data_types_set1 = std::set<svo::svo_data_type_t>(svo::all_buffer_data_types.begin(), svo::all_buffer_data_types.end());
        EXPECT_EQ(svo::all_buffer_data_types_set, all_buffer_data_types_set1);
        
        for (svo::svo_data_type_t data_type : svo::all_buffer_data_types_set)
        {
            // the switch should generate a warning if we are missing a semantic
            switch (data_type)
            {
                case(svo::svo_data_type_t::BYTE):
                case(svo::svo_data_type_t::UNSIGNED_BYTE):
                case(svo::svo_data_type_t::SHORT):
                case(svo::svo_data_type_t::UNSIGNED_SHORT):
                case(svo::svo_data_type_t::INT):
                case(svo::svo_data_type_t::UNSIGNED_INT):
                case(svo::svo_data_type_t::LONG):
                case(svo::svo_data_type_t::UNSIGNED_LONG):
                case(svo::svo_data_type_t::FLOAT):
                case(svo::svo_data_type_t::DOUBLE):
                    continue;
            }
        }
        
        
        EXPECT_TRUE(svo::all_buffer_data_types_set.count(svo::svo_data_type_t::BYTE));
        EXPECT_TRUE(svo::all_buffer_data_types_set.count(svo::svo_data_type_t::UNSIGNED_BYTE));
        EXPECT_TRUE(svo::all_buffer_data_types_set.count(svo::svo_data_type_t::SHORT));
        EXPECT_TRUE(svo::all_buffer_data_types_set.count(svo::svo_data_type_t::UNSIGNED_SHORT));
        EXPECT_TRUE(svo::all_buffer_data_types_set.count(svo::svo_data_type_t::INT));
        EXPECT_TRUE(svo::all_buffer_data_types_set.count(svo::svo_data_type_t::UNSIGNED_INT));
        EXPECT_TRUE(svo::all_buffer_data_types_set.count(svo::svo_data_type_t::LONG));
        EXPECT_TRUE(svo::all_buffer_data_types_set.count(svo::svo_data_type_t::UNSIGNED_LONG));
        EXPECT_TRUE(svo::all_buffer_data_types_set.count(svo::svo_data_type_t::FLOAT));
        EXPECT_TRUE(svo::all_buffer_data_types_set.count(svo::svo_data_type_t::DOUBLE));
    }
}

