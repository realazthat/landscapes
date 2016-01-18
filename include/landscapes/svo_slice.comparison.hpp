#ifndef LANDSCAPES_SVO_TREE_COMPARISONS_HPP
#define LANDSCAPES_SVO_TREE_COMPARISONS_HPP 1

#include <string>
#include <vector>
#include <cassert>

#include "landscapes/svo_tree.fwd.hpp"

namespace svo{
    
    
    
    struct svo_slice_inequality_t{
        svo_slice_inequality_t(
                const svo_slice_t* slice0, const svo_slice_t* slice1
              , const std::string& message)
            : slice0(slice0), slice1(slice1)
            , message(message)
        {
            // message is empty => slice0 is null
            assert( (message.size() != 0 || slice0 == nullptr) && "message is empty => slice0 is null fails");
            // message is empty => slice1 is null
            assert( (message.size() != 0 || slice1 == nullptr) && "message is empty => slice1 is null fails");
            
            
            // slice0 is null => message is empty
            assert( (slice0 != nullptr || message.size() == 0) && "slice0 is null => message is empty fails");
            // slice1 is null => message is empty
            assert( (slice1 != nullptr || message.size() == 0) && "slice1 is null => message is empty fails");
        }
        
        
        const svo_slice_t* slice0;
        const svo_slice_t* slice1;
        std::string message;
    };
    
    
    struct svo_slice_inequalities_t{
        svo_slice_inequalities_t(const svo_slice_t* slice0, const svo_slice_t* slice1)
            : slice0(slice0), slice1(slice1)
        {
            assert(slice0 != nullptr);
            assert(slice1 != nullptr);
        }
            
        operator bool() const{
            return messages.size() > 0;
        }
        
        svo_slice_inequalities_t& operator+=(const svo_slice_inequality_t& slice_inequality)
        {
            assert(slice_inequality.slice0 == slice0);
            assert(slice_inequality.slice1 == slice1);
            
            messages.push_back(slice_inequality.message);
            
            return *this;
        }
        svo_slice_inequalities_t& operator+=(const svo_slice_inequalities_t& slice_inequalities)
        {
            assert(slice_inequalities.slice0 == slice0);
            assert(slice_inequalities.slice1 == slice1);
            
            for (const auto& message : slice_inequalities.messages)
                messages.push_back(message);
            
            return *this;
        }
        
        std::vector<std::string> messages;
        const svo_slice_t* slice0;
        const svo_slice_t* slice1;
    };

    namespace slice_cmp_t{
        enum enum_t{
            
              none =                    0
            , level =                   1 << 1
            , side =                    1 << 2
            , parent_vcurve_begin =     1 << 3
            , pos_data =                1 << 5
            , channel_data =            1 << 6
            , child_count =             1 << 7
            , child_props =             1 << 8
            , has_parent =              1 << 9
            , parent_props =            1 << 10
            , all_data =                pos_data | channel_data
            , all =                     level | side | parent_vcurve_begin | pos_data | channel_data
                                          | child_count| child_props | has_parent
            , default_check = all
        };
    } // namespace svo_slice_cmp_type_t
    
    svo_slice_inequalities_t svo_slice_inequality(
          const svo_slice_t* slice0
        , const svo_slice_t* slice1
        , slice_cmp_t::enum_t svo_slice_cmp_type = slice_cmp_t::default_check
        , int recurse_down = 0
        , int recurse_up = 0);









} // namespace svo


#endif
