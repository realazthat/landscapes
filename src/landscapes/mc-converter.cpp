
#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <deque>
#include <tuple>

#include <tclap/CmdLine.h>

#include "format.h"

#include "landscapes/svo_curves.h"
#include "landscapes/svo_tree.hpp"
#include "landscapes/mcloader.hpp"
#include "landscapes/svo_tree.slice_mgmt.hpp"
#include "landscapes/svo_serialization.v1.hpp"
#include "landscapes/svo_tree.sanity.hpp"


namespace svo{
struct timer_t{
    timer_t(std::ostream& out, const std::string& name)
        : out(out), name(name)
    {
        start = std::chrono::steady_clock::now();
        
        out << fmt::format("{} started at {}", name, start.time_since_epoch().count()) << std::endl;
    }
    
    ~timer_t(){
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> diff = end-start;
        out << fmt::format("{} ended at {}, took {}", name, end.time_since_epoch().count(), diff.count()) << std::endl;
    }
    
    std::ostream& out;
    std::string name;
    std::chrono::time_point<std::chrono::steady_clock> start;
};
} // namespace


svo::svo_slice_t* load_slices(const std::string& slice_path, const std::string& root_node_name){
    auto* root_slice = svo::svo_init_slice(0, 16);
    std::size_t slice_count = 0;
    
    ///load serialized slices
    {

        std::deque< std::tuple< std::string, svo::svo_slice_t* > > queue { std::make_tuple(root_node_name, root_slice) };


        while (queue.size())
        {
            
            std::string node_path;
            svo::svo_slice_t* slice;
            std::tie(node_path, slice) = queue.front();
            queue.pop_front();

            assert(slice);
            assert(slice->children);


            std::string inpath = fmt::format("{}/{}.slice", slice_path, node_path);
            std::ifstream in(inpath, std::ios::binary);

            if(!in)
                throw std::runtime_error(fmt::format("Could not open serialized tree in file {}", inpath));
            
            svo::unserialize_slice(in, slice, true/*load_empty_children*/);

            for (auto* child : *slice->children)
            {
                assert(child);

                std::string child_node_path = node_path + fmt::format(".{}.{}", child->side, child->parent_vcurve_begin);

                queue.push_back( std::make_tuple(child_node_path, child) );
            }
            slice_count++;

        }
    }
    return root_slice;
}
int main(int argc, const char** argv)
{
    
    try {
        
        
        std::string version_str = "NONE";//fmt::format("version: {}, build: {}");
        TCLAP::CmdLine cmd("Converts minecraft mca regions to serialized SVO trees", ' ', version_str);
        
        TCLAP::SwitchArg testLoadArg("test","test","reload the serialized tree, and test it", cmd);
        TCLAP::ValueArg<std::string> regionPathArg("r","region","path to region file",true,"","string", cmd);
        TCLAP::ValueArg<std::string> treePathArg("t","tree","path to tree output directory",true,"","string", cmd);
        TCLAP::ValueArg<uint32_t> threadsArg("j","jobs/threads","number of threads",false,1,"integer", cmd);
        
        cmd.parse( argc, argv );
        
        
        std::ifstream infile(regionPathArg.getValue(), std::ios::binary);
        
        if (!infile)
            throw std::runtime_error("error getting region data");
            
            
        
        svo::volume_of_slices_t wanted_slices(32 /*volume_side*/, 16/*slice side*/);
        for (vcurve_t slice_vcurve = 0; slice_vcurve < vcurvesize(32); ++slice_vcurve)
        {
            vside_t x,y,z;
            vcurve2coords(slice_vcurve, 32, &x, &y, &z);
            
            ///if the slice is higher than 16 slices (of size 16^3)
            if (y > 16)
                ///then no point in making a slice for it; minecraft doesn't support height over 256 atm.
                continue;
            
            ///create a slice, which is a 16^3 section of a chunk.
            auto* slice = svo::svo_init_slice(0, 16);
            wanted_slices.slices.push_back( std::make_tuple( slice_vcurve, slice) );
        }
        
        {
            svo::timer_t load_mca_region_timer(std::cout, "loading MCA region");
            
            svo::load_mca_region(wanted_slices, infile, threadsArg.getValue());
        }
        
        
        svo::volume_of_slices_t& volume_of_slices = wanted_slices;
        
        svo::svo_slice_t* root_slice = nullptr;
        
        {
            svo::timer_t entree_slices_timer(std::cout, "entreeing slices");
            root_slice = svo::svo_entree_slices(volume_of_slices, 1024*100/*max_voxels_per_slice*/);
        }
        
        assert(root_slice);
        
        {
            svo::timer_t serialize_slices_timer(std::cout, "serialize slices");

            auto serialize_a_tree_slice = [&treePathArg](svo::svo_slice_t* current_slice, const std::string& parent_node_path){


                std::string current_node_path = parent_node_path;

                if (current_slice->level > 0) {
                   current_node_path += fmt::format(".{0}.{1}", current_slice->side, current_slice->parent_vcurve_begin);
                }

                std::string outpath = fmt::format("{}/{}.slice", treePathArg.getValue(), current_node_path);

                //std::cout << "outpath: " << outpath << std::endl;
                std::ofstream out(outpath, std::ios::binary);

                if(!out)
                    throw std::runtime_error(fmt::format("could not open path \"{}\" for outputting serialized tree", outpath));
                svo::serialize_slice(out, current_slice);
            
                return current_node_path;
            };

            preorder_traverse_slices(root_slice, std::string("r") /* initial node path */, serialize_a_tree_slice);
        }
        
        
        if (testLoadArg.getValue())
        {
            svo::timer_t unserialize_slices_timer(std::cout, "test: reload serialized slices");
            auto* root_slice1 = load_slices(treePathArg.getValue(), "r");
            
            
            if (auto error = svo::svo_slice_sanity(root_slice1, svo::svo_sanity_t::enum_t::all, 1000000))
            {
                throw std::runtime_error(fmt::format("Error; slice saniy check has failed with: \"{}\"", error.error));
            }
        }
        
        
    } catch (TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
    
    return 0;
}