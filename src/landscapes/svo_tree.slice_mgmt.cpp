
#include "landscapes/svo_tree.slice_mgmt.hpp"


#include "landscapes/svo_tree.hpp"
#include "landscapes/svo_tree.sanity.hpp"

namespace svo{

/**
 * This class generalizes the idea of downsampling data by averaging out the
 * 8 child values to get the parent value.
 *
 */
template<typename T, typename element_view_t>
struct siblings_average_downsampler_t
{
    siblings_average_downsampler_t(vcurve_t parent_vcurve_begin, element_view_t element_view, std::size_t out_entry_index);
    ~siblings_average_downsampler_t();

    ///append a voxel of data. will flush if this voxel is not a sibling of the voxels in the
    /// buffer.
    void append(vcurve_t pos, const T& datum);
    ///flushes the current buffer (if any) to the parent, by averaging out the siblings.
    /// call this once at the end, to make sure the last siblings are flushed.
    void flush();


public:

    ///all the positions in this slice start off somewhere in the middle of the parent cube.
    ///this is equivalent to slice->parent_vcurve_begin.
    vcurve_t parent_vcurve_begin;

    ///this is the destination data channel
    element_view_t element_view;
    std::size_t out_entry_index;

    ///this is the parent position of the current buffer.
    vcurve_t sibings_parent_vcurve;
    std::vector< std::tuple<vcurve_t, T> > siblings;

    void assert_invariants() const;
};

template<typename T, typename element_view_t>
siblings_average_downsampler_t<T, element_view_t>
make_avg_downsampler(vcurve_t parent_vcurve_begin, element_view_t element_view, std::size_t out_entry_index)
{
    return siblings_average_downsampler_t<T, element_view_t>(parent_vcurve_begin, element_view, out_entry_index);
}

template<typename T, typename element_view_t>
siblings_average_downsampler_t<T,element_view_t>::
siblings_average_downsampler_t(vcurve_t parent_vcurve_begin, element_view_t element_view, std::size_t out_entry_index)
    :   parent_vcurve_begin(parent_vcurve_begin)
      , element_view(element_view)
      , out_entry_index(out_entry_index)
      , sibings_parent_vcurve(0)
{
    assert_invariants();
}

template<typename T, typename element_view_t>
siblings_average_downsampler_t<T,element_view_t>::
~siblings_average_downsampler_t()
{
    ///make sure everything was flushed.
    assert(siblings.size() == 0);
}

template<typename T, typename element_view_t>
void
siblings_average_downsampler_t<T,element_view_t>::
assert_invariants() const
{
    ///there is never more than 8 siblings.
    assert(siblings.size() <= 8);

    ///sibings_parent_vcurve is either unset or GTE parent_vcurve_begin
    assert(sibings_parent_vcurve == 0 || sibings_parent_vcurve >= parent_vcurve_begin);


}

template<typename T, typename element_view_t>
void
siblings_average_downsampler_t<T,element_view_t>::
append(vcurve_t vcurve, const T& datum)
{
    assert_invariants();

    vcurve_t parent_vcurve = parent_vcurve_begin + (vcurve / 8);

    if (siblings.size() == 0)
    {
        sibings_parent_vcurve = parent_vcurve;
    } else if (parent_vcurve != sibings_parent_vcurve) {
        flush();
        sibings_parent_vcurve = parent_vcurve;
        assert(siblings.size() == 0);
    }
    assert(sibings_parent_vcurve == parent_vcurve);

    siblings.push_back( std::make_tuple(vcurve, datum) );
    
    assert_invariants();
}

template<typename T, typename element_view_t>
void
siblings_average_downsampler_t<T,element_view_t>::
flush()
{
    assert_invariants();
    
    if (siblings.size() == 0)
        return;

    T result = T(0);

    for (const auto& v : siblings)
    {
        T datum;
        std::tie(std::ignore, datum) = v;

        result += datum;
    }

    result = result / float(siblings.size());
    
    T& out_data = element_view.template get<T>(out_entry_index++);
    out_data = result;
    
    siblings.clear();
    
    assert_invariants();
}

void svo_downsample_slice(svo_slice_t* parent_slice, const svo_slice_t* child_slice)
{
    DEBUG {
        
        if (auto error = svo_slice_sanity(child_slice, svo_sanity_type_t::all, 0, false /* parent recurse */))
        {
            std::cerr << "error: " << error << std::endl;
            assert(false && "sanity fail");
        }
        if (auto error = svo_slice_sanity(parent_slice, svo_sanity_type_t(svo_sanity_type_t::all & ~svo_sanity_type_t::all_data), 0, false /* parent recurse */))
        {
            std::cerr << "error: " << error << std::endl;
            assert(false && "sanity fail");
        }
    }
    
    assert(parent_slice);
    assert(parent_slice->pos_data);
    assert(parent_slice->buffers);
    assert(child_slice);
    assert(child_slice->pos_data);
    assert(child_slice->buffers);
    auto& dst_pos_data = *parent_slice->pos_data;
    auto& dst_buffers = *parent_slice->buffers;
    const auto& src_pos_data = *(child_slice->pos_data);
    const auto& src_buffers = *(child_slice->buffers);
    src_buffers.assert_invariants();
    dst_buffers.assert_invariants();

    assert(parent_slice->level + 1 == child_slice->level);

    ///compute the length of the index-space of the parent slice volume.
    vcurvesize_t child_size = vcurvesize(child_slice->side);
    vcurvesize_t parent_size = vcurvesize(parent_slice->side);
    ///it must be divisible by 8, because the parent has 8 children, each with their own section.
    assert(child_size % 8 == 0);

    ///we divide the parent volume into 8 quadrants/corners. Here we calculate the offsets
    /// into the parent that the child begins and ends at. So a child at corner (1,1,1)
    /// will begin 7/8ths through the index-space of the parent.

    ///this is the size of an octant.
    //vcurve_t octant_vcurve_size = (parent_size / 8);

    ///this will be the parent-vcurve at which this child begins. 
    //vcurve_t octant_vcurve_begin = corner2ccurve(child_slice_corner)*octant_vcurve_size;
    //vcurve_t octant_vcurve_end = octant_vcurve_begin + octant_vcurve_size;
    //assert( octant_vcurve_end <= child_size );
    //assert( octant_vcurve_begin == child_slice->parent_vcurve_begin );




    for ( const auto& vcurve : src_pos_data )
    {

        ///translating the voxel's location to the parent's location.
        vcurve_t parent_vcurve = child_slice->parent_vcurve_begin + (vcurve / 8);
        assert( parent_vcurve < parent_size );


        ///if the parent voxel is not yet solid (we can check this by checking if the last voxel on the
        /// list of voxels is the parent, because the children all lie next to eachother in morton-order).
        if (dst_pos_data.size() == 0 || dst_pos_data.back() != parent_vcurve)
            dst_pos_data.push_back( parent_vcurve );
    }

    std::size_t out_data_index = dst_buffers.entries();

    ///if the buffer has no schema
    if (!dst_buffers.has_schema())
        ///copy the schema from the src_buffers
        dst_buffers.copy_schema(src_buffers, dst_pos_data.size());
    else
        dst_buffers.resize(dst_pos_data.size());
    
    assert(dst_buffers.schema() == src_buffers.schema());
    assert(dst_buffers.entries() == dst_pos_data.size());

    std::vector<std::string> avg_element_names { "color", "normal" };
    
    src_buffers.assert_invariants();
    
    for (auto element_name : avg_element_names)
    {
        if (dst_buffers.has_named_element(element_name))
        {
            const auto& src_color_element = src_buffers.get_element_view(element_name);
            
            auto dst_color_element = dst_buffers.get_element_view(element_name);

            
            auto avg_downsampler = make_avg_downsampler<float3_t>(child_slice->parent_vcurve_begin, dst_color_element, out_data_index);

            for ( std::size_t in_data_index = 0; in_data_index < src_pos_data.size(); ++in_data_index )
            {
                vcurve_t vcurve = src_pos_data[in_data_index];
                
                const float3_t& raw_color = src_color_element.get<float3_t>(in_data_index);

                avg_downsampler.append(vcurve, raw_color);

                
            }
            avg_downsampler.flush();
        }
    }
    
    
    dst_buffers.assert_invariants();
    
    DEBUG {
        
        if (auto error = svo_slice_sanity(child_slice, svo_sanity_type_t::all, 0, false /* parent recurse */))
        {
            std::cerr << "error: " << error << std::endl;
            assert(false && "sanity fail");
        }
    }
}





svo_slice_t* svo_entree_slices(const volume_of_slices_t& volume_of_slices, std::size_t max_voxels_per_slice, std::size_t root_level)
{
    assert(volume_of_slices.volume_side % 2 == 0);

    volume_of_slices_t current_level(volume_of_slices.volume_side, volume_of_slices.slice_side);
    volume_of_slices_t next_level(volume_of_slices.volume_side / 2, volume_of_slices.slice_side);

    
    for ( const auto& s : volume_of_slices.slices )
    {
        vcurve_t slice_vcurve; const svo_slice_t* slice0;
        std::tie(slice_vcurve, slice0) = s;
        assert(slice0);
        assert(slice0->buffers);
        
        slice0->buffers->assert_invariants();
        
        DEBUG {
            if (auto error = svo_slice_sanity(slice0))
            {
                std::cerr << "error: " << error << std::endl;
                assert(false && "sanity fail");
            }
        }
    }
    
    ///clone all the slices
    {

        
        for ( const auto& s : volume_of_slices.slices )
        {
            vcurve_t slice_vcurve; const svo_slice_t* slice0;
            std::tie(slice_vcurve, slice0) = s;
            assert(slice0);
            assert(slice0->side == volume_of_slices.slice_side);
            
            slice0->buffers->assert_invariants();

            if (slice0->pos_data->size() == 0)
                continue;



            svo_slice_t* slice = svo_clone_slice(slice0);
            assert(slice->side == volume_of_slices.slice_side);
            
            
            slice0->buffers->assert_invariants();
            slice->buffers->assert_invariants();

            current_level.slices.push_back( std::make_tuple( slice_vcurve, slice ) );
        }
    }
    

    ///while we don't have a root ...
    ///we will build this bottom up.
    while (current_level.slices.size() > 1)
    {
        assert(current_level.volume_side > 1);
        assert(current_level.volume_side == next_level.volume_side * 2);

        ///create parents for every 8 cube of slices, store that in next_level.
        vcurve_t slice_vcurve; svo_slice_t* slice;
        for ( auto& s : current_level.slices )
        {
            std::tie(slice_vcurve, slice) = s;
            
            
            assert(slice);
            assert(slice->buffers);
            slice->buffers->assert_invariants();


            assert( slice->side > 1 );
            assert( slice->side % 2 == 0 );



            ///the parent has the same resolution as the child; because it will have up to 7
            /// other children, but at half resolution.
            vside_t parent_side = slice->side;


            ///the position of the slice within the current volume.
            uint32_t slice_x,slice_y,slice_z;
            vcurve2coords(slice_vcurve, current_level.volume_side, &slice_x,&slice_y,&slice_z);

            ///the position of the *parent slice* within the parent *volume of slices*.
            /// the parent volume, covering the same physical bounds, but having
            /// half the resolution of this, will have the half coordinates of the child.
            uint32_t pslice_x = slice_x / 2;
            uint32_t pslice_y = slice_y / 2;
            uint32_t pslice_z = slice_z / 2;

            ///there are now two ways to calculate the curve of the parent slice within the
            /// next level's volume of slices:
            /// division by 8 of the child curve (slice_curve) or coords2vcurve of px,py,pz.
            /// We do the cheaper version, and double check in an assert.
            vcurve_t parent_slice_vcurve = slice_vcurve / 8;
            assert( coords2vcurve(pslice_x,pslice_y,pslice_z, current_level.volume_side / 2) == parent_slice_vcurve );


            ///corner of this slice within parent slice
            corner_t corner = get_corner_by_int3( slice_x - pslice_x*2
                                                , slice_y - pslice_y*2
                                                , slice_z - pslice_z*2 );

            ///if we don't yet have the parent in the next level,
            if (next_level.slices.size() == 0 || std::get<0>(next_level.slices.back()) != parent_slice_vcurve)
            {
                ///insert the parent.
                svo_slice_t* parent_slice = svo_init_slice(0, parent_side);
                
                
                DEBUG {
                    if (auto error = svo_slice_sanity(parent_slice))
                    {
                        std::cerr << "error: " << error << std::endl;
                        assert(false && "sanity fail");
                    }
                }
                
                assert(parent_slice->buffers);
                parent_slice->buffers->assert_invariants();
                
                next_level.slices.push_back( std::make_tuple(parent_slice_vcurve, parent_slice) );


            }
            
            ///now get the parent slice
            svo_slice_t* parent_slice = std::get<1>(next_level.slices.back());
            parent_slice->buffers->assert_invariants();

            assert(parent_slice);
            assert(parent_slice->children);

            DEBUG {
                if (auto error = svo_slice_sanity(parent_slice, svo_sanity_type_t(svo_sanity_type_t::all & ~svo_sanity_type_t::all_data & ~svo_sanity_type_t::levels)))
                {
                    std::cerr << "error: " << error << std::endl;
                    assert(false && "sanity fail");
                }
            }
            
            ///We are always going to have 8 children slices (except empty ones)
            /// to a parent slice.
            ///So we split up the parent voxel volume into 8 octants.
            vcurvesize_t parent_octant_size = vcurvesize(parent_slice->side) / 8;
            
            ///the voxel position within the parent of this child slice.
            vcurvesize_t parent_octant_vcurve_begin = corner2ccurve(corner) * parent_octant_size;
            //vcurvesize_t parent_octant_vcurve_end = parent_octant_vcurve_begin + parent_octant_size;
            
            /*
            std::cout << "---" << corner << std::endl;
            std::cout << "octant: " << corner2ccurve(corner) << std::endl;
            std::cout << "parent size: " << vcurvesize(parent_slice->side) << std::endl;
            std::cout << "parent_octant_size: " << parent_octant_size << std::endl;
            std::cout << "parent_octant_vcurve_begin: " << parent_octant_vcurve_begin << std::endl;
            std::cout << "parent_octant_vcurve_end: " << parent_octant_vcurve_end << std::endl;
            */

            parent_slice->buffers->assert_invariants();
            slice->buffers->assert_invariants();
            
            ///hack, because we are going to fix the levels last.
            slice->level = 1;
            parent_slice->level = 0;
            svo_slice_attach_child( parent_slice, slice, parent_octant_vcurve_begin );
            
            parent_slice->buffers->assert_invariants();
            slice->buffers->assert_invariants();

            DEBUG {
                
                if (auto error = svo_slice_sanity(slice, svo_sanity_type_t(svo_sanity_type_t::all & ~svo_sanity_type_t::all_data & ~svo_sanity_type_t::levels)))
                {
                    std::cerr << "error: " << error << std::endl;
                    assert(false && "sanity fail");
                }
                if (auto error = svo_slice_sanity(parent_slice, svo_sanity_type_t(svo_sanity_type_t::all & ~svo_sanity_type_t::all_data & ~svo_sanity_type_t::levels)))
                {
                    std::cerr << "error: " << error << std::endl;
                    assert(false && "sanity fail");
                }
            }
        }

        ///all the slices in current_level have parents in next_level. so we can forget the current
        /// level now.

        ///swap out the next level and make it the current level.
        next_level.slices.swap(current_level.slices);
        std::swap(current_level.volume_side, next_level.volume_side);

        next_level.slices.clear();
        next_level.volume_side = current_level.volume_side / 2;
    }

    ///If this volume of slices ends up being entirely empty
    if (current_level.slices.size() == 0)
    {
        ///return an empty root slice.
        svo_slice_t* root_slice = svo_init_slice(root_level, 1);
        return root_slice;
    }

    assert( current_level.slices.size() == 1 );
    svo_slice_t* root_slice;
    std::tie(std::ignore, root_slice) = current_level.slices[0];
    root_slice->level = root_level;
    root_slice->parent_vcurve_begin = 0;
    assert(root_slice->parent_slice == 0);


    ///proceed through the tree, and:
    ///1. reset the levels correctly
    auto reset_levels = [](svo_slice_t* current_slice, std::size_t level)
    {
        current_slice->level = level;

        return level + 1;
    };


    preorder_traverse_slices(root_slice, root_level /*level*/, reset_levels);
    

    DEBUG {
        if (auto err = svo_slice_sanity(root_slice
                            , svo_sanity_type_t(svo_sanity_type_t::all & ~(svo_sanity_type_t::pos_data | svo_sanity_type_t::channel_data))
                            , 1000000))
        {
            std::cerr << err << std::endl;
            assert(false && "sanity fail");
        }
    }
    
    auto assert_buffer_invariants = [](svo_slice_t* current_slice, std::size_t metadata)
    {
        current_slice->buffers->assert_invariants();

        return 0;
    };


    preorder_traverse_slices(root_slice, 0 /*dummy metadata*/, assert_buffer_invariants);
    
    
    
    
    std::set<const svo_slice_t*> slice_set;
    preorder_traverse_slices(root_slice, 0, [&slice_set](svo_slice_t* current_slice, std::size_t dummy){
        slice_set.insert(current_slice);
        return 0;
    });
    
    std::set<const svo_slice_t*> postorder_slice_set;
    
    ///proceed through the tree, and:
    ///1. downsample the data from the bottom up
    auto downsample = [&slice_set, &postorder_slice_set](svo_slice_t* current_slice, std::vector<std::size_t> metadatas)
    {
        assert(current_slice);
        assert(slice_set.count(current_slice) == 1);
        assert(postorder_slice_set.count(current_slice) == 0);
        postorder_slice_set.insert(current_slice);
        
        current_slice->buffers->assert_invariants();
        
        DEBUG {
            if (auto err = svo_slice_sanity(current_slice, svo_sanity_type_t::minimal, 2))
            {
                std::cerr << err << std::endl;
                assert(false && "sanity fail");
            }
        }
        
        const auto& children = *current_slice->children;
        
        for (const auto* child_slice : children)
        {
            assert(slice_set.count(child_slice) == 1);
            assert(postorder_slice_set.count(child_slice) == 1);
            
            DEBUG {
                if (auto err = svo_slice_sanity(child_slice, svo_sanity_type_t::all, 2, false /*parent recurse*/))
                {
                    std::cerr << err << std::endl;
                    assert(false && "sanity fail");
                }
            }
            
            current_slice->buffers->assert_invariants();
            child_slice->buffers->assert_invariants();
            svo_downsample_slice(current_slice, child_slice);
            child_slice->buffers->assert_invariants();
            current_slice->buffers->assert_invariants();
        }
        
        
        current_slice->buffers->assert_invariants();
        
        DEBUG {
            if (auto err = svo_slice_sanity(current_slice, svo_sanity_type_t::all, 2, false /*parent recurse*/))
            {
                std::cerr << err << std::endl;
                assert(false && "sanity fail");
            }
        }
        
        
        return 0;
    };

        
    postorder_traverse_slices<std::size_t>(root_slice, downsample);
    


    assert(slice_set == postorder_slice_set);
    
    
    DEBUG {
        if (auto err = svo_slice_sanity(root_slice, svo_sanity_type_t::all, 1000000))
        {
            std::cerr << err << std::endl;
            assert(false && "sanity fail");
        }
    }



    ///combine slices together with their siblings if they all add up to less than the slice limit.
    auto squeeze_a_tree_slice = [max_voxels_per_slice](svo_slice_t* current_slice){

        assert(current_slice);
        assert(current_slice->children);

        auto& children = *current_slice->children;
        if (children.size() == 0)
            return 0;

        while (true)
        {
            auto groups = svo_find_joinable_groups_of_children(current_slice);

            bool joined_anything = false;
            for (const auto& group : groups)
            {


                assert(group.group_side_in_parent < SVO_VOLUME_SIDE_LIMIT);

                std::size_t total_child_size = 0;

                //std::cout << "group.count: " << group.count << std::endl;

                for (svo_slice_t* child : group.group_children)
                {
                    if (!child)
                        continue;
                    assert(child);
                    assert(child->pos_data);
                    total_child_size += child->pos_data->size();
                }

                if (total_child_size < max_voxels_per_slice)
                {
                    svo_slice_t* new_replacement_slice = svo_init_slice(current_slice->level + 1, current_slice->side*2);


                    for (svo_slice_t* child : group.group_children)
                    {
                        if (!child)
                            continue;
                        assert( child->side*2 < SVO_VOLUME_SIDE_LIMIT );
                    }

                    svo_join_slices(new_replacement_slice, group.group_children);

                    for (svo_slice_t* child : group.group_children)
                    {
                        if (!child)
                            continue;
                        svo_uninit_slice(child);
                    }

                    joined_anything = true;
                }
            }

            if (!joined_anything)
                break;
        }


        return 0;
    };

    ///combine slices that are the same size, are right next to eachother, and are children of the same slice.
    ///this is a top down procedure.
    preorder_traverse_slices(root_slice, squeeze_a_tree_slice);



    DEBUG {
        if (auto err = svo_slice_sanity(root_slice, svo_sanity_type_t::default_sanity, 1000000))
        {
            std::cerr << err << std::endl;
            assert(false && "sanity fail");
        }
    }






    while (root_slice->side > 1)
    {
        assert( root_slice->side % 2 == 0);
        vside_t next_root_slice_side = root_slice->side / 2;

        svo_slice_t* next_root_slice = svo_init_slice(0, next_root_slice_side);

        assert(next_root_slice);
        assert(next_root_slice->children);

        ///TODO: get rid of this annoying level thingy
        root_slice->level = 1;

        svo_slice_attach_child(next_root_slice, root_slice, 0);

        svo_downsample_slice(next_root_slice, root_slice);

        root_slice = next_root_slice;

        DEBUG {
            if (auto err = svo_slice_sanity(root_slice))
            {
                std::cerr << err << std::endl;
                assert(false && "sanity fail");
            }
        }
    }


    assert( root_slice->side == 1);

    preorder_traverse_slices(root_slice, std::size_t(0) /*level*/, reset_levels);




    DEBUG {
        if (auto err = svo_slice_sanity(root_slice))
        {
            std::cerr << err << std::endl;
            assert(false && "sanity fail");
        }
    }


    return root_slice;
}


} //namespace svo
