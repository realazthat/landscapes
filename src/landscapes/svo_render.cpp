





#include "landscapes/svo_render.hpp"
#include "landscapes/svo_tree.hpp"
#include "landscapes/svo_camera_mapping.cl.h"
#include "landscapes/svo_serialization.v1.hpp"
#include "landscapes/common.math.cl.h"
#include "landscapes/svo_formatters.hpp"

#include <set>
#include <deque>
#include <tuple>
#include <fstream>

#define RAYMARCH_COMPUTE_LEVELS 1
#define RAYMARCH_COMPUTE_ITERATIONS 1
#include "landscapes/svo_tree.raymarch.h"


namespace svo{
    
    struct svo_render_pimpl_t{
        std::size_t max_levels;
        std::size_t block_count;
        std::size_t slice_count;
    };
    
    svo_render_t::svo_render_t(float3_t light, const std::string& slice_path, const std::string& root_node_name)
        : root_slice(nullptr)
        , light(light)
        , slice_path(slice_path)
        , root_node_name(root_node_name)
        , pimpl(new svo_render_pimpl_t())
    {
        tree.reset( new svo_tree_t(std::size_t(1024*1024*1024)*1, 1024*1024*1) );
        pimpl->max_levels = 0;
        pimpl->block_count = 0;
        pimpl->slice_count = 0;
    }
    
    svo_render_t::~svo_render_t(){
        
    }
    
    void svo_render_t::load_slices()
    {
        
        root_slice = svo_init_slice(0, 16);
        ///load serialized slices
        {

            std::deque< std::tuple< std::string, svo_slice_t* > > queue { std::make_tuple(root_node_name, root_slice) };


            while (queue.size())
            {
                
                std::string node_path;
                svo_slice_t* slice;
                std::tie(node_path, slice) = queue.front();
                queue.pop_front();

                assert(slice);
                assert(slice->children);


                std::string inpath = fmt::format("{}/{}.slice", slice_path, node_path);
                std::ifstream in(inpath, std::ios::binary);

                if(!in)
                    throw std::runtime_error(fmt::format("Could not open serialized tree in file {}", inpath));
                
                unserialize_slice(in, slice, true/*load_empty_children*/);

                for (svo_slice_t* child : *slice->children)
                {
                    assert(child);

                    std::string child_node_path = node_path + fmt::format(".{}.{}", child->side, child->parent_vcurve_begin);

                    queue.push_back( std::make_tuple(child_node_path, child) );
                }
                pimpl->slice_count++;

            }
        }

        assert( root_slice->side == 1);
    }
    
    
    
    void svo_render_t::load_blocks()
    {
        assert(root_slice);
        
        {
            std::vector<svo_block_t*> new_leaf_blocks;
            auto success = svo_block_initialize_slice_data(new_leaf_blocks, tree.get(), tree->root_block, root_slice);

            leaf_blocks.insert(new_leaf_blocks.begin(), new_leaf_blocks.end());

            if (success != svo_error_t::OK)
                throw std::runtime_error(fmt::format(":(: {}", success));
        }

        assert(leaf_blocks.size() > 0);




        std::set<svo_block_t*> loadable_blocks(leaf_blocks);

        while (loadable_blocks.size() > 0)
        {
            for (svo_block_t* leaf_block : std::set<svo_block_t*>(loadable_blocks))
            {
                leaf_blocks.erase(leaf_block);
                loadable_blocks.erase(leaf_block);


                if (!(leaf_block->slice))
                {
                    leaf_blocks.insert(leaf_block);
                    continue;
                }
                
                
                std::vector<svo_block_t*> new_leaf_blocks;
                svo_load_next_slice(new_leaf_blocks, leaf_block);


                leaf_blocks.insert(new_leaf_blocks.begin(), new_leaf_blocks.end());
                loadable_blocks.insert(new_leaf_blocks.begin(), new_leaf_blocks.end());
            }
        }
        
        std::size_t& max_levels = pimpl->max_levels;
        std::size_t& block_count = pimpl->block_count;
        
        ///count the blocks
        {
            auto visitor = [&block_count, &max_levels](svo_block_t* parent_block, svo_block_t* block, std::tuple<std::size_t> metadata)
            {
                std::size_t level = std::get<0>(metadata);

                std::size_t block_leaf_voxel_level = block->root_level + block->height;

                max_levels = std::max<std::size_t>(max_levels, block_leaf_voxel_level);

                ++block_count;

                return std::make_tuple(level+1);
            };

            preorder_traverse_blocks(tree->root_block, std::make_tuple(0), visitor);


        }

        pprint_block(std::cout, "root_block", tree->root_block);
    }

    uint32_t svo_render_t::render_tile(float* buffer
                    , const svo_camera_mapping_t& camera_mapping
                    , std::size_t screen_width, std::size_t screen_height
                    , glm::uvec2 nuv0, glm::uvec2 nuv1
                    , glm::vec3 lod_source)
    {
        
        auto compute_ray_scale_2 = [&camera_mapping, &screen_width](){
            int32_t source_max_width = svo_camera_quad_max_width(&(camera_mapping.source));
            int32_t target_max_width = svo_camera_quad_max_width(&(camera_mapping.target));
            
            int32_t triangle_height = target_max_width - source_max_width;
            
            svo_ray_t center_ray = svo_uv_to_ray(&camera_mapping, 0, 0);
            int32_t triangle_base = glm_length(center_ray.target - center_ray.source);
            
            ///calculating this based on camera, even though LOD is independent of camera via
            /// `lod_source`.
            float horizontal_fov = glm_atan(triangle_height / triangle_base);
            
            ///See voxelpixelerror() function in raymarch.hpp
            //float aA = camera.HorizontalFov();
            float aA = horizontal_fov;
            ///FOV for a single ray
            float aa = aA / float(screen_width);
            float rayScale = (1.0/(2.0*glm_tan(aa/2.0)));
            float rayScale2 = rayScale*rayScale;
            return rayScale2;
        };
        
        float rayScale2 = compute_ray_scale_2();
        
        
        std::size_t hit_count = 0;
        
        float pixel_width = float(1) / float(screen_width);
        
        for (auto nu = nuv0.x; nu < nuv1.x; ++nu)
        for (auto nv = nuv0.y; nv < nuv1.y; ++nv)
        {
            float nuf = (float(nu)+.5) / float(screen_width);
            float nvf = (float(nv)+.5) / float(screen_height);
            
            float uf = (nuf * 2) - 1;
            float vf = (nvf * 2) - 1;
            svo_ray_t ray = svo_uv_to_ray(&camera_mapping, uf, vf);
            
            float3_t raydir = glm_normalize(ray.target - ray.source);
            
            
            float3_t normal;
            float t;
            uint32_t levels;
            uint32_t iterations;
            
            bool hit = svo_tree_raymarch( tree->address_space
                                        , tree->root_block->root_shadow_cd_goffset
                                        , ray.source
                                        , raydir
                                        , rayScale2, &normal, &t, &levels, &iterations);
                                        
            if (hit)
                hit_count++;
            
            float4_t pixel = make_float4(0,0,0,.9);
            
            ///view iterations
            pixel.x = float(iterations) / 30;
            
            
            if (hit) {
                //pixel.xyz = make_float3(glm_sqrt(float(iterations) / 30));
            }
            
            float* buffer_pixel_ptr = buffer + (nu + nv*screen_width)*4;
            
            for (size_t i = 0; i < 4; ++i)
                buffer_pixel_ptr[i] = getcomponentf4(pixel,i);
            
        }
        
        return hit_count;
    }

} // namespace svo





