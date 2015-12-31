

#include "landscapes/svo_formatters.hpp"
#include "landscapes/svo_tree.hpp"
#include "landscapes/svo_tree.capi.h"

#include "bprinter/table_printer.h"

#include <iostream>
#include <string>
#include <iomanip>

//#include "prettyprint.hpp"


std::ostream& operator<<(std::ostream& out, const child_descriptor_t& cd)
{

    uint64_t bytemask = 0b11111111;

    out << std::bitset<8>(cd.data >> (sizeof(uint64_t)*8 - 8));
    for (std::size_t i = 1; i < sizeof(child_descriptor_t); ++i)
    {
        int shift_bits = (sizeof(child_descriptor_t)-i)*8;
        ///size of 1 byte
        shift_bits -= 8;
        out << " " << std::bitset<8>((cd.data >> shift_bits) & bytemask);
    }

    return out;
}


std::ostream& operator<<(std::ostream& out, const svo::svo_element_t& element)
{

    out << "<(svo_element_t name=" << svo::quote(element.name())
        << ", type=" << element.type()
        << ", semantic=" << element.semantic()
        << ", count=" << element.count()
        << ", type_bytes=" << element.type_bytes()
        << ", bytes=" << element.bytes()
        << " )>";
    return out;
}

std::ostream& operator<<(std::ostream& out, const std::vector<svo::svo_element_t>& elements)
{
    out << "[";

    for (std::size_t i = 0; i < elements.size(); ++i)
        out << (i == 0 ?  "" : ", ") << elements[i];

    out << "]";
    return out;
}

std::ostream& operator<<(std::ostream& out, const svo::svo_declaration_t& declaration)
{
    out << "<(svo_declaration_t stride=" << declaration.stride();

    out << ", elements=[";

    const auto& elements = declaration.elements();
    for (std::size_t i = 0; i < elements.size(); ++i)
        out << (i == 0 ? "  " : ", ") << elements[i];

    out << "] )>";
    return out;
}


std::ostream& operator<<(std::ostream& out, const std::vector<svo::svo_declaration_t>& declarations)
{
    out << "[";

    for (std::size_t i = 0; i < declarations.size(); ++i)
        out << (i == 0 ?  "" : ", ") << declarations[i];

    out << "]";
    return out;
}


std::ostream& operator<<(std::ostream& out, const svo::svo_data_type_t& data_type)
{
    return out << tostr(data_type);
}
std::ostream& operator<<(std::ostream& out, const svo::svo_semantic_t& semantic)
{
    return out << tostr(semantic);
}












namespace svo{


void pprint_announce_msg(
    std::ostream& out, const std::string& announce_msg, std::size_t width)
{
    
    std::size_t announce_line_len = width;
    std::size_t numdashes2 = announce_line_len - announce_msg.size();
    std::size_t numdashes = numdashes2 / 2;
    auto dashes = std::string(numdashes, '-');
    auto extra_space = ( (numdashes2 % 2) ? "" : " ");
    out << dashes << announce_msg << extra_space << dashes << std::endl;
}

void pprint_block(
    std::ostream& out, const std::string& announce_msg, const svo_block_t* block)
{
    
    
    std::size_t max_level = block->height;
    
    bprinter::TablePrinter tp(&out);
    tp.AddColumn("vidx", std::max(5, (int)std::ceil(std::log10(block->size()))));
    tp.AddColumn("cdidx", std::max(5, (int)std::ceil(std::log10(block->size())/8)));
    tp.AddColumn("pcdidx", std::max(6, (int)std::ceil(std::log10(block->size())/8)));
    tp.AddColumn("level", std::max(5, (int)std::ceil(std::log10(max_level))));
    tp.AddColumn("ccurve", 6);
    tp.AddColumn("vcurve", std::max(5, (int)std::ceil(std::log10(vcurvesize(SVO_VOLUME_SIDE_LIMIT)))));
    tp.AddColumn("cd", 64+7);
    tp.AddColumn("cd goffset", 10);
    tp.AddColumn("far?", 4);
    tp.AddColumn("child cd goffset", 16);
    
    
    std::size_t voxel_index = 0;
    std::size_t cd_index = 0;
    auto visitor = [&block, &tp, &cd_index ,&voxel_index](
          goffset_t pcd_goffset, goffset_t cd_goffset, ccurve_t voxel_ccurve
        , std::tuple<std::size_t, std::size_t, std::size_t, vcurve_t> metadata)
    {
        std::size_t level = std::get<0>(metadata);
        std::size_t parent_voxel_index = std::get<1>(metadata);
        std::size_t parent_cd_index = std::get<2>(metadata);
        std::size_t parent_vcurve = std::get<3>(metadata);
        
        vcurve_t voxel_vcurve = parent_vcurve*8 + voxel_ccurve;
        
        tp << voxel_index;
        
        if (cd_goffset != invalid_goffset)
        {
            tp << cd_index;
        } else {
            tp << "-";
        }
        
        if (pcd_goffset != invalid_goffset)
        {
            tp << parent_cd_index;
        } else {
            tp << "-";
        }
        
        tp << level;
        
        tp << voxel_ccurve;
        tp << voxel_vcurve;
        
        if (cd_goffset == invalid_goffset)
        {
            tp << "*";
        } else if (block->is_valid_cd_goffset(cd_goffset)) {
            const auto* cd = svo_cget_cd(block->tree->address_space, cd_goffset);
            
            std::ostringstream ostr;
            ostr << *cd;
            tp << ostr.str();
        } else {
            tp << "~";
        }

        if (cd_goffset != invalid_goffset)
            tp << cd_goffset;
        else
            tp << "~";


        if (cd_goffset == invalid_goffset)
        {
            tp << "~";
        } else {
            const auto* cd = svo_cget_cd(block->tree->address_space, cd_goffset);
            tp << (svo_get_far(cd) ? "1" : "0");
        }

        if (cd_goffset == invalid_goffset)
        {
            tp << "~";
        } else {
            const auto* cd = svo_cget_cd(block->tree->address_space, cd_goffset);
            goffset_t child_cd_goffset = svo_get_child_ptr_goffset(block->tree->address_space, cd_goffset, cd);
            tp << child_cd_goffset;
        }
        
        auto result = std::make_tuple(level+1, voxel_index, cd_index, voxel_vcurve);
        
        if (cd_goffset != invalid_goffset)
            ++cd_index;
        ++voxel_index;
        
        return result;
    };
    
    
    pprint_announce_msg(out, announce_msg, tp.get_table_width() + 2);
    
    tp.PrintHeader();
    
    auto initial_data = std::make_tuple(0 /*level*/, 0 /*parent voxel_index*/, 0 /*parent cd index*/, 0 /*parent vcurve*/);
    z_preorder_traverse_block_cds(  block->tree->address_space
                                    , block
                                    , initial_data, visitor);
    tp.PrintFooter();
}






} //namespace svo



