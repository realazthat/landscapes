
#include "landscapes/mcloader.hpp"
#include "landscapes/svo_tree.hpp"
#include "landscapes/svo_serialization.v1.hpp"
#include "landscapes/svo_tree.sanity.hpp"
#include "landscapes/svo_formatters.hpp"
#include "gtest/gtest.h"

#include <vector>
#include <sstream>
#include <fstream>
#include <random>
#include <memory>

struct SerializeTest : public ::testing::Test {
    std::vector<uint8_t> data0;
    const svo::svo_slice_t* slice0;
protected:
 
    svo::svo_slice_t* m_slice0;

    virtual void SetUp() {
        
    
        ///random data
        {
            for (std::size_t i = 0; i < 1000; ++i)
            {
                data0.push_back(i % 256);
            }
        }


        ///slice
        {
            svo::volume_of_slices_t wanted_slices(32,16);
            
            vcurve_t slice_vcurve = 0;
            m_slice0 = svo::svo_init_slice(0, 16);
            wanted_slices.slices.push_back( std::make_tuple( slice_vcurve, m_slice0) );
            

            std::ifstream infile("../libs/test-region/r.0.0.mca", std::ios::binary);

            ASSERT_TRUE(infile);

            svo::load_mca_region(wanted_slices, infile);
            
            slice0 = m_slice0;
        }


    }

    virtual void TearDown() {
    // Code here will be called immediately after each test
    // (right before the destructor).
        svo::svo_uninit_slice(m_slice0, true);

    }
};




TEST_F(SerializeTest,serialize_string)
{
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

template<typename rng_t>
svo::svo_element_t generate_element(rng_t& rng, const std::string& name)
{
    auto semantic = svo::all_semantics.at(rng(0, svo::all_semantics.size()-1));
    auto data_type = svo::all_data_types.at(rng(0, svo::all_data_types.size()-1));
    auto count = rng(1, 15);

    return svo::svo_element_t(name, semantic, data_type, count);
}

template<typename rng_t>
svo::svo_declaration_t generate_declaration(rng_t& rng, const std::string& base_name)
{
    svo::svo_declaration_t declaration;

    std::size_t elements = rng(0,6);

    for (std::size_t element_index = 0; element_index < elements; ++element_index)
    {
        std::string element_name = base_name + "." + svo::tostr(element_index);

        declaration.add( generate_element(rng, element_name) );
    }

    return declaration;
}


template<typename rng_t>
svo::svo_schema_t generate_schema(rng_t& rng, const std::string& base_name)
{
    std::size_t declaration_count = rng(0,16);
    
    svo::svo_schema_t schema;
    for (std::size_t declaration_index = 0; declaration_index < declaration_count; ++declaration_index)
    {
        
        auto declaration = generate_declaration(rng, base_name + "." + svo::tostr(declaration_index));

        declaration.assert_invariants();
        schema.push_back(declaration);
    }
    
    return schema;
}


template<typename rng_t>
std::unique_ptr<svo::svo_cpu_buffer_t> generate_buffer(rng_t& rng, const std::string& base_name, std::size_t entries)
{
    
    auto declaration = generate_declaration(rng, base_name);
    
    std::unique_ptr<svo::svo_cpu_buffer_t> buffer( new svo::svo_cpu_buffer_t(declaration, entries) );
    
    for (std::size_t byte_index = 0; byte_index < buffer->bytes(); ++byte_index)
    {
        uint8_t* ptr = buffer->rawdata() + byte_index;
        *ptr = rng(0,255);
    }
    return buffer;
}
template<typename rng_t>
std::unique_ptr<svo::svo_cpu_buffers_t> generate_buffers(rng_t& rng, const std::string& base_name, std::size_t entries)
{
    
    
    std::unique_ptr<svo::svo_cpu_buffers_t> buffers(new svo::svo_cpu_buffers_t());
    
    
    auto schema = generate_schema(rng, base_name);
    
    for (auto declaration : schema)
    {
        buffers->add_buffer(declaration, entries);
    }
    
    return buffers;
}

struct rng_t
{
    rng_t()
        : gen(0)
    {

    }

    int operator()(std::size_t minimum, std::size_t maximum)
    {
        std::uniform_int_distribution<> dis(minimum, maximum);

        return dis(gen);
    }

    std::random_device rd;
    std::mt19937 gen;
};

TEST_F(SerializeTest,serialize_declaration)
{
    rng_t rng;

    for (std::size_t i = 0; i < 10000; ++i)
    {
        auto declaration0 = generate_declaration(rng, "noname");

        declaration0.assert_invariants();
        
        std::ostringstream out;
        svo::serialize_declaration(out, declaration0);

        std::istringstream in(out.str());

        auto declaration1 = svo::unserialize_declaration(in);

        EXPECT_EQ(declaration0, declaration1);
    }
}


TEST_F(SerializeTest,serialize_schema)
{
    rng_t rng;

    for (std::size_t i = 0; i < 10000; ++i)
    {
        auto schema0 = generate_schema(rng, "noname");
        
        std::ostringstream out;
        svo::serialize_schema(out, schema0);

        std::istringstream in(out.str());

        auto schema1 = svo::unserialize_schema(in);

        EXPECT_EQ(schema0, schema1);
    }
}



TEST_F(SerializeTest,serialize_buffer)
{
    
    rng_t rng;
    
    
    for (std::size_t i = 0; i < 50; ++i)
    {
        std::size_t entries = 10000;
        auto buffer0_ptr = generate_buffer(rng, "noname", entries);
        auto& buffer0 = *buffer0_ptr;
        
        
        std::ostringstream out;
        svo::serialize_buffer(out, buffer0);
        
        std::istringstream in(out.str());
        auto buffer1_ptr = svo::unserialize_buffer(in);
        auto& buffer1 = *buffer1_ptr;
        
        EXPECT_EQ(buffer0.declaration(), buffer1.declaration());
        EXPECT_EQ(buffer0.entries(), buffer1.entries());
        EXPECT_EQ(buffer0.bytes(), buffer1.bytes());
        
        int buffer_cmp = std::memcmp(buffer0.rawdata(), buffer1.rawdata(), buffer0.bytes());
        
        EXPECT_EQ(0, buffer_cmp);
    }
}


TEST_F(SerializeTest,serialize_buffers)
{
    
    rng_t rng;
    
    
    for (std::size_t i = 0; i < 50; ++i)
    {
        std::size_t entries = 10000;
        auto buffers0_ptr = generate_buffers(rng, "noname", entries);
        auto& buffers0 = *buffers0_ptr;
        
        
        std::ostringstream out;
        svo::serialize_buffers(out, buffers0, entries);
        
        std::istringstream in(out.str());
        
        svo::svo_cpu_buffers_t buffers1;
        svo::unserialize_buffers(in, buffers1, entries);
        
        EXPECT_EQ(buffers0.buffers().size(), buffers1.buffers().size());
        EXPECT_EQ(buffers0.schema(), buffers1.schema());
        EXPECT_EQ(buffers0.entries(), buffers1.entries());
        
        
        for (std::size_t buffer_index = 0; buffer_index < buffers1.buffers().size(); ++buffer_index)
        {
            ASSERT_LT(buffer_index, buffers0.buffers().size());
            ASSERT_LT(buffer_index, buffers1.buffers().size());
            
            const auto& buffer0 = buffers0.buffers()[buffer_index];
            const auto& buffer1 = buffers1.buffers()[buffer_index];
            
            int buffer_cmp = std::memcmp(buffer0.rawdata(), buffer1.rawdata(), buffer0.bytes());
            
            EXPECT_EQ(0, buffer_cmp);
            
        }
    }
}


TEST_F(SerializeTest,serialize_slice)
{
    
    std::ostringstream out;
    
    svo::serialize_slice(out, slice0);
    
    
    std::istringstream in(out.str());
    
    svo::svo_slice_t* slice1 = svo::svo_init_slice(0, 16);
    svo::unserialize_slice(in,slice1,true);
    
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
    
    ASSERT_EQ(buffers0.buffers().size(), buffers1.buffers().size());
    
    for (std::size_t buffer_index = 0; buffer_index < buffers0_list.size(); ++buffer_index)
    {
        ASSERT_LT(buffer_index, buffers0_list.size());
        ASSERT_LT(buffer_index, buffers1_list.size());
        
        const auto& buffer0 = buffers0_list[buffer_index];
        const auto& buffer1 = buffers1_list[buffer_index];
        
        buffer0.assert_invariants();
        buffer1.assert_invariants();
    
        
        EXPECT_EQ(buffer0.entries(), buffer1.entries());
        EXPECT_EQ(buffer0.stride(), buffer1.stride());
        ASSERT_EQ(buffer0.bytes(), buffer1.bytes());
        
        int buffer_cmp = std::memcmp(buffer0.rawdata(), buffer1.rawdata(), buffer0.bytes());
        
        EXPECT_EQ(0, buffer_cmp);
    }
}


TEST_F(SerializeTest,unserialize_slice)
{
    
    std::ostringstream out0;
    svo::serialize_slice(out0, slice0);
    
    
    
    const auto& str0 = out0.str();
    std::vector<uint8_t> serialized_data0(str0.data(), str0.data()+str0.size());
    



    
    std::istringstream in1(str0);
    svo::svo_slice_t* slice1 = svo::svo_init_slice(0, 16);
    svo::unserialize_slice(in1,slice1,true);
    
    
    std::ostringstream out1;
    svo::serialize_slice(out1, slice1);
    
    
    
    
    const auto& str1 = out1.str();
    std::vector<uint8_t> serialized_data1(str1.data(), str1.data() + str1.size());
    
    EXPECT_EQ(serialized_data0, serialized_data1);
    
    
    ASSERT_EQ(serialized_data0.size(), serialized_data1.size());
    
    int serial_cmp = std::memcmp(serialized_data0.data(), serialized_data1.data(), serialized_data0.size());
    
    EXPECT_EQ(0, serial_cmp);
}










