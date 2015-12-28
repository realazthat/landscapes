#ifndef UNITTESTS_MAIN_HPP
#define UNITTESTS_MAIN_HPP 1



#include "landscapes/svo_tree.fwd.hpp"

#include <vector>
#include <array>

struct TestEnvironment : ::testing::Environment{
    TestEnvironment()
        : volume_of_slices(nullptr)
        , all_const_slices(nullptr)
        , m_volume_of_slices(nullptr)
        , m_all_const_slices(nullptr)
        , m_all_slices(nullptr)
    {}


    const svo::volume_of_slices_t* volume_of_slices;
    const std::vector<const svo::svo_slice_t*>* all_const_slices;

    std::array<const svo::svo_slice_t*, 8> get_sibling_slices(std::size_t minimum_voxels);
protected:
    svo::volume_of_slices_t* m_volume_of_slices;
    std::vector<const svo::svo_slice_t*>* m_all_const_slices;
    std::vector<svo::svo_slice_t*>* m_all_slices;
    virtual void SetUp();
    
    virtual void TearDown();
};


extern TestEnvironment* global_env;


#endif
