
#include "landscapes/cpputils.hpp"
#include "gtest/gtest.h"
#include <fstream>

class OverlapAlgoTest : public ::testing::Test {
protected:
    virtual void SetUp() {
      
    }

    virtual void TearDown() {
    // Code here will be called immediately after each test
    // (right before the destructor).
    }
    };





TEST_F(OverlapAlgoTest,overlap_open_close_range){
    
    EXPECT_TRUE(overlap_open_close_range(1,2,1,3));
    EXPECT_TRUE(overlap_open_close_range(1,2,1,2));
    
    EXPECT_FALSE(overlap_open_close_range(1,2,2,3));
    EXPECT_FALSE(overlap_open_close_range(1,2,2,2));
    EXPECT_FALSE(overlap_open_close_range(2,2,2,2));
    EXPECT_FALSE(overlap_open_close_range(3,2,3,2));
    
    
}

