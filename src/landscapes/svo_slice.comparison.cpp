

#include "landscapes/svo_slice.comparison.hpp"
#include "landscapes/svo_tree.hpp"

#include "format.h"


namespace svo{
    
    
    
    static svo_slice_inequalities_t svo_slice_inequality_internal(
          const svo_slice_t* slice0, const std::string& name0
        , const svo_slice_t* slice1, const std::string& name1
        , slice_cmp_t::enum_t svo_slice_cmp_type
        , int recurse_down
        , int recurse_up)
    {
        assert(slice0);
        assert(slice1);
        
        
        svo_slice_inequalities_t results(slice0, slice1);
        
        if (    (svo_slice_cmp_type & slice_cmp_t::enum_t::level)
                && slice0->level != slice1->level)
            results += svo_slice_inequality_t(slice0, slice1,
                            fmt::format("{0}->level != {1}->level"
                                        ", {0}->level: {2}, {1}->level: {3}"
                                        , name0, name1
                                        , slice0->level, slice1->level));
        
        if (   (svo_slice_cmp_type & slice_cmp_t::enum_t::side)
               && slice0->side != slice1->side)
            results += svo_slice_inequality_t(slice0, slice1,
                            fmt::format("{0}->side != {1}->side"
                                        ", {0}->side: {2}, {1}->side: {3}"
                                        , name0, name1
                                        , slice0->side, slice1->side));
        
        if ( (svo_slice_cmp_type & slice_cmp_t::enum_t::parent_vcurve_begin) 
             && slice0->parent_vcurve_begin != slice1->parent_vcurve_begin)
            results += svo_slice_inequality_t(slice0, slice1,
                            fmt::format("{0}->parent_vcurve_begin != {1}->parent_vcurve_begin"
                                        ", {0}->parent_vcurve_begin: {2}, {1}->parent_vcurve_begin: {3}"
                                        , name0, name1
                                        , slice0->parent_vcurve_begin, slice1->parent_vcurve_begin));
        
        
        
        
        if (svo_slice_cmp_type & slice_cmp_t::enum_t::pos_data)
        {
            if (slice0->pos_data->size() != slice1->pos_data->size()) {
                results += svo_slice_inequality_t(slice0, slice1,
                                fmt::format("{0}->pos_data->size() != {1}->pos_data->size()"
                                            ", {0}->pos_data->size(): {2}, {1}->pos_data->size(): {3}"
                                            , name0, name1
                                            , slice0->pos_data->size(), slice1->pos_data->size()));
            } else {
                std::size_t elements = slice0->pos_data->size();
                
                const auto& pos_data0 = *slice0->pos_data;
                const auto& pos_data1 = *slice1->pos_data;
                for (std::size_t element_index = 0; element_index < elements; ++element_index)
                {
                    vcurve_t vcurve0 = pos_data0[element_index];
                    vcurve_t vcurve1 = pos_data1[element_index];
                    
                    if (vcurve0 != vcurve1){
                        results += svo_slice_inequality_t(slice0, slice1,
                                fmt::format("{0}->pos_data[{4}] != {1}->pos_data[{4}]"
                                            ", {0}->pos_data[{4}]: {2}, {1}->pos_data[{4}]: {3}"
                                            , name0, name1
                                            , vcurve0, vcurve1
                                            , element_index));
                        break;
                    }
                }
                
            }
        }
        
        
        
        if (svo_slice_cmp_type & slice_cmp_t::enum_t::channel_data)
        {
            if (slice0->buffers->entries() != slice1->buffers->entries()) {
                results += svo_slice_inequality_t(slice0, slice1,
                                fmt::format("{0}->buffers->entries() != {1}->buffers->entries()"
                                            ", {0}->buffers->entries(): {2}, {1}->buffers->entries(): {3}"
                                            , name0, name1
                                            , slice0->buffers->entries(), slice1->buffers->entries()));
            } else {
                std::size_t elements = slice0->buffers->entries();
                
                const auto& buffers0 = *slice0->buffers;
                const auto& buffers1 = *slice1->buffers;
                
                if (buffers0.schema() != buffers1.schema()) {
                    results += svo_slice_inequality_t(slice0, slice1,
                                    fmt::format("{0}->buffers->schema() != {1}->buffers->schema()"
                                                ", {0}->buffers->schema(): {2}, {1}->buffers->schema(): {3}"
                                                , name0, name1
                                                , slice0->buffers->schema(), slice1->buffers->schema()));
                                                
                } else {
                    
                    const auto& buffer0_list = buffers0.buffers();
                    const auto& buffer1_list = buffers1.buffers();
                    std::size_t buffer_count = buffer0_list.size();
                    
                    for (std::size_t buffer_index = 0; buffer_index < buffer_count; ++buffer_index)
                    {
                        const auto& buffer0 = buffer0_list[buffer_index];
                        const auto& buffer1 = buffer1_list[buffer_index];
                        
                        auto vertex_bytes = buffer0.stride();
                        
                        const auto* rawdata0 = buffer0.rawdata();
                        const auto* rawdata1 = buffer1.rawdata();
                        
                        const auto* vertex_data0_ptr = rawdata0;
                        const auto* vertex_data1_ptr = rawdata1;
                        for (std::size_t element_index = 0; element_index < elements; ++element_index)
                        {
                            auto notequal = std::memcmp(vertex_data0_ptr, vertex_data1_ptr, vertex_bytes);
                            
                            if (notequal)
                            {
                                
                                results += svo_slice_inequality_t(slice0, slice1,
                                        fmt::format("{name0}->buffers[{buffer_index}][{element_index}] != {name1}->buffers[{buffer_index}][{element_index}]"
                                                    ", {name0}->buffers[{buffer_index}][{element_index}]: {value0}, {name1}->buffers[{buffer_index}][{element_index}]: {value1}"
                                                    , fmt::arg("name0",name0), fmt::arg("name1",name1)
                                                    , fmt::arg("value0", buffer0.tostr(element_index))
                                                    , fmt::arg("value1", buffer1.tostr(element_index))));
                                break;
                            }
                            
                            vertex_data0_ptr += vertex_bytes;
                            vertex_data1_ptr += vertex_bytes;
                        }
                    }
                }
                
            }
        }
        
        
        
        auto slice0_has_parent = slice0->parent_slice != nullptr;
        auto slice1_has_parent = slice1->parent_slice != nullptr;
        bool should_care_about_parent =  (svo_slice_cmp_type & slice_cmp_t::enum_t::has_parent)
                                    ||  (svo_slice_cmp_type & slice_cmp_t::enum_t::parent_props); 
        if ( should_care_about_parent && (slice0_has_parent != slice1_has_parent))
            results += svo_slice_inequality_t(slice0, slice1,
                            fmt::format("{0}->parent is {2} and {1}->parent is {3}"
                                        , name0, name1
                                        , (slice0_has_parent ? "is null" : "is not null")
                                        , (slice1_has_parent ? "is null" : "is not null")));
        
        if ((svo_slice_cmp_type & slice_cmp_t::enum_t::parent_props) && slice0_has_parent && slice1_has_parent)
        {
            assert(false && "TODO");
        }
        
        
        assert(slice0->children);
        assert(slice1->children);
        
        const auto& children0 = *slice0->children;
        const auto& children1 = *slice1->children;
        
        
        bool should_care_about_child_count = (svo_slice_cmp_type & slice_cmp_t::enum_t::child_count)  || (svo_slice_cmp_type & slice_cmp_t::enum_t::child_props);
        if (should_care_about_child_count && children0.size() != children1.size())
            results += svo_slice_inequality_t(slice0, slice1,
                            fmt::format("{0}->children->size() != {1}->children->size()"
                                        ", {0}->children->size(): {2}, {1}->children->size(): {3}"
                                        , name0, name1
                                        , children0.size(), children1.size()));
        
        
        if ((svo_slice_cmp_type & slice_cmp_t::enum_t::child_props) && children0.size() == children1.size())
        {
            std::size_t child_count = slice0->children->size();
            
            for (std::size_t child_index = 0; child_index < child_count; ++child_index)
            {
                const auto* child0 = children0[child_index];
                const auto* child1 = children1[child_index];
                assert(child0);
                assert(child1);
                
                std::string child0_name = fmt::format("{0}->children[{1}]", name0, child_index);
                std::string child1_name = fmt::format("{0}->children[{1}]", name1, child_index);
                
                slice_cmp_t::enum_t child_cmp_mask = slice_cmp_t::enum_t(
                                          slice_cmp_t::enum_t::level | slice_cmp_t::enum_t::side
                                        | slice_cmp_t::enum_t::parent_vcurve_begin
                                        | slice_cmp_t::enum_t::has_parent | slice_cmp_t::enum_t::child_count);
                child_cmp_mask = slice_cmp_t::enum_t(child_cmp_mask & svo_slice_cmp_type);
                results += svo_slice_inequality_internal(slice0, child0_name, slice1, child1_name
                                    , child_cmp_mask, 0 /*recurse_down*/, 0 /*recurse_up*/);
                
                
                if (svo_slice_cmp_type & slice_cmp_t::enum_t::has_parent)
                {
                    bool child0_points_back_to_parent = child0->parent_slice == slice0;
                    bool child1_points_back_to_parent = child1->parent_slice == slice1;
                    
                    if (child0_points_back_to_parent != child1_points_back_to_parent)
                        results += svo_slice_inequality_t(slice0, slice1,
                            fmt::format("{0} {2}, but {1} {3}"
                                        , child0_name, child1_name
                                        , (child0_points_back_to_parent ? "points back to parent" : "does not point back to parent")
                                        , (child1_points_back_to_parent ? "points back to parent" : "does not point back to parent")
                                        ));
                        
                }
            }
        }
        
        
        
        
        
        
        if ((recurse_down > 0) && children0.size() == children1.size())
        {
            std::size_t child_count = slice0->children->size();
            
            for (std::size_t child_index = 0; child_index < child_count; ++child_index)
            {
            
                std::string child0_name = fmt::format("{0}->children[{1}]", name0, child_index);
                std::string child1_name = fmt::format("{0}->children[{1}]", name1, child_index);
                results += svo_slice_inequality_internal(slice0, child0_name, slice1, child1_name
                                    , svo_slice_cmp_type, recurse_down-1 /*recurse_down*/, 0 /*recurse_up*/);
                                
            }
        }
        
        if ((recurse_up > 0) && (slice0_has_parent && slice1_has_parent))
        {
            
            std::string child0_name = fmt::format("{0}->parent", name0);
            std::string child1_name = fmt::format("{0}->parent", name1);
            results += svo_slice_inequality_internal(slice0, child0_name, slice1, child1_name
                                , svo_slice_cmp_type, 1 /*recurse_down*/, recurse_up - 1/*recurse_up*/);
            
        }
        return results;
    }





    svo_slice_inequalities_t svo_slice_inequality(
          const svo_slice_t* slice0
        , const svo_slice_t* slice1
        , slice_cmp_t::enum_t svo_slice_cmp_type
        , int recurse_down
        , int recurse_up)
    {
        return svo_slice_inequality_internal(slice0, "slice0", slice1, "slice1", svo_slice_cmp_type, recurse_down, recurse_up);
    }




} // namespace

