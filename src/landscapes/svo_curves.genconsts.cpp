

#include "landscapes/svo_curves.h"

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

int main(){

    std::string dim2str[] = {"x", "y", "z"};

    auto K = SVO_VSIDE_LIMIT;
    typedef std::vector<vcurve_t> mortonK_table_t;

    std::vector< mortonK_table_t > tables;


    for (std::size_t primary_dimension = 0; primary_dimension < 3; ++primary_dimension)
    {
        tables.push_back(mortonK_table_t());
        auto& table = tables.back();

        for (vside_t u = 0; u < SVO_VSIDE_LIMIT; ++u)
        {
            vcurve_t xyz[] = {0,0,0};
            xyz[primary_dimension] = u;

            vcurve_t x = xyz[0], y = xyz[1], z = xyz[2];

            vcurve_t vcurve = coords2vcurve_brute(x,y,z,SVO_VSIDE_LIMIT);

            table.push_back(vcurve);
        }
    }

    for (vside_t x = 0; x < SVO_VSIDE_LIMIT; ++x)
    for (vside_t y = 0; y < SVO_VSIDE_LIMIT; ++y)
    for (vside_t z = 0; z < SVO_VSIDE_LIMIT; ++z)
    {
        vcurve_t vcurve0 = coords2vcurve_brute(x,y,z,SVO_VSIDE_LIMIT);
        vcurve_t vcurve1 = tables.at(0).at(x) | tables.at(1).at(y) | tables.at(2).at(z);

        assert(vcurve0 == vcurve1);
    }

    for (std::size_t primary_dimension = 0; primary_dimension < 3; ++primary_dimension)
    {
        const auto& table = tables[primary_dimension];
        auto dimname = dim2str[primary_dimension];

        std::cout << std::endl;
        if (primary_dimension > 0)
            std::cout << "// Pre-shifted table for " << dimname
                      << " (" << primary_dimension << " bits to the left)"
                      << std::endl;

        std::cout << "GLOBAL_STATIC_CONST uint32_t morton" << K << "_" << dimname << "[" << K << "] = {" << std::endl;
        std::cout << "{" << std::endl;

        for (vside_t u = 0; u < SVO_VSIDE_LIMIT; ++u)
        {
            std::size_t vcurve_hex_digits = sizeof(vcurve_t)*2;
            
            std::cout << "    "
                      << "0x" << std::hex << std::setfill('0') << std::setw(vcurve_hex_digits) << table.at(u) << std::dec
                      << (u == SVO_VSIDE_LIMIT-1 ? "" : ",");

            if (u % 8 == 0 || u == SVO_VSIDE_LIMIT-1)
                std::cout << std::endl;
        }

        std::cout << "};" << std::endl;
    }

    
    return 0;
}





