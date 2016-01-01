#ifndef SVO_RENDER_HPP
#define SVO_RENDER_HPP 1

#include "landscapes/opencl.shim.h"
#include "landscapes/svo_tree.hpp"
#include "landscapes/svo_camera_mapping.cl.h"

#include <memory>
#include <set>
#include <string>


namespace svo{
    
    
    /* A utility class that actually essentially does this:
     * `svo tree + empty texture ... + raymarching = rendered texture`
     *
     */
    struct svo_render_t{
        
        // this will be the light position, change this to alter the lighting
        float3_t light;
    
        
        // the root slice of the tree, which will be loaded from the serialized slices
        svo_slice_t* root_slice;
        
        // this will be the generated block-tree, and this will be what is raymarched
        std::unique_ptr<svo_tree_t> tree;
        
        /* Construct the svo_render_t object, using a directory of serialized slices as a voxel source.
         * 
         * @light the initial position of the light
         * @slice_path the path to the directory containing the serialized slices
         * @root_node_name the name of the root slice, without the `.slice` extension; defaults to `"r"`
         */
        svo_render_t(float3_t light, const std::string& slice_path, const std::string& root_node_name="r");
        
        // Loads the slices from the serialized directory all at once.
        void load_slices();
        
        // Once the slices are loaded, this will load the blocks into the <svo_tree_t>
        void load_blocks();
        
        
        /* Renders a section of the screen to a texture.
         *
         * @buffer the raw float buffer for the output texture.
         * @camera_mapping two quads representing the near and far planes; note that the far plane
         *          will not limit the length of the rays, it is strictly for computing the rays.
         * @screen_width the pixel width of the camera screen.
         * @screen_height the pixel height of the camera screen.
         * @xy0 a 2D vector representing the lower bound of the section of the screen to render;
         *          the vector values are measured in pixels, and should be in the range
         *          `xy.x \in [0,screen_width]` and `xy.y \in [0,screen_height]`
         *          where `xy=(0,0)` is the upper left corner of the screen, and `xy=(screen_width,screen_height)`
         *          is the lower right corner of the screen. An example rectanlge would be
         *          `xy0=(screen_width/2,screen_height/2), xy1=(screen_width, screen_height)`. This allows
         *          one to render sections of the screen in separate calls.
         * @xy1 a 2D vector representing the upper bound of the section of the screen to render; see @xy0
         * @lod_source 
         */
        void render_tile( float* buffer
                    , const camera_mapping_t& camera_mapping
                    , std::size_t screen_width, std::size_t screen_height
                    , glm::uvec2 xy0, glm::uvec2 xy1
                    , glm::vec3 lod_source );
    protected:
        // this stores the path to the directory containing the serialized slices
        std::string slice_path;
        
        // the name of the slice of the root node, without the extension
        std::string root_node_name;
        
        // keeps track of the blocks at the bottom of the tree, where more voxel data might be added to;
        // this is useful as the blocks are being loaded to know which blocks need to be appended to.
        std::set<svo_block_t*> leaf_blocks;
        
    };
    
    
} //namespace svo


#endif
