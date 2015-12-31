
#include "landscapes/svo_inttypes.h"
#include "landscapes/svo_curves.h"
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
}

