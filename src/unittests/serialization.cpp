
#include "landscapes/mcloader.hpp"
#include "landscapes/svo_tree.hpp"
#include "landscapes/svo_serialization.v1.hpp"
#include "landscapes/svo_tree.sanity.hpp"
#include "gtest/gtest.h"

#include <vector>
#include <sstream>

struct SerializeTest : public ::testing::Test {
    std::vector<uint8_t> data0;
protected:
 
    virtual void SetUp() {
        
    
        
        for (std::size_t i = 1; i < 10; ++i)
        {
            data0.push_back(i % 256);
        }
    }

    virtual void TearDown() {
    // Code here will be called immediately after each test
    // (right before the destructor).
    
    }
};





TEST_F(SerializeTest,serialize_string){
    
    
    
    auto data0str = std::string((const char*)data0.data(), data0.size());
    EXPECT_EQ(data0str.size(), data0.size());
        
    std::ostringstream out0;
    svo::serialize_string(out0, data0str);
    
    
    auto serializeddatastr = out0.str();
    std::istringstream in2(serializeddatastr);
    
    
    auto data1str = svo::unserialize_string(in2);
    EXPECT_EQ(data1str, data0str);
    std::vector<uint8_t> data1(data1str.data(), data1str.data() + data1str.size());
    
    EXPECT_EQ(data0.size(), data1.size());
    EXPECT_EQ(data0, data1);
    
}


