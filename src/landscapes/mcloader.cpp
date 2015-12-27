
#include "landscapes/mcloader.hpp"
#include "nbt.h"
#include "landscapes/debug_macro.h"
#include "format.h"
#include "ThreadPool.h"
#include "landscapes/svo_tree.hpp"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <thread>
#include <exception>


namespace svo{

///just a descriptive typedef
typedef std::size_t chunk_id_t;
static const vside_t base_slice_side = 16;
static const std::size_t region_side_chunks = 32;




static inline unsigned char * swapBytes(unsigned char * addr, unsigned int length) {
    for (unsigned int i = 0; i < length/2; i++) {
        unsigned int j = length-i-1;
        unsigned char swap = addr[i];
        addr[i]  = addr[j];
        addr[j] = swap;
    }
    return addr;
}




struct block_info_t{ int id; const char* desc; uint8_t color[3]; };
static const block_info_t block_infos_array[] = {
    (struct block_info_t){-10, "fill", {110, 86, 44}}
  , (struct block_info_t){1, "stone", {128, 127, 102}}
  , (struct block_info_t){2, "grass", {4, 82, 15}}
  , (struct block_info_t){3, "dirt", {110, 86, 44}}
  , (struct block_info_t){4, "cobblestone", {109, 109, 109}}
  , (struct block_info_t){5, "wooden_plank", {164, 132, 76}}
  , (struct block_info_t){6, "sapling", {60, 28, 4}}
  , (struct block_info_t){7, "adminium", {138, 136, 114}}
  , (struct block_info_t){8, "water", {108, 140, 220}}
  , (struct block_info_t){9, "stationary_water", {76, 84, 252}}
  , (struct block_info_t){10, "lava", {252, 164, 4}}
  , (struct block_info_t){11, "stationary_lava", {252, 164, 4}}
  , (struct block_info_t){12, "sand", {216, 211, 181}}
  , (struct block_info_t){13, "gravel", {86, 80, 50}}
  , (struct block_info_t){14, "gold_ore", {122, 119, 97}}
  , (struct block_info_t){15, "iron_ore", {137, 139, 120}}
  , (struct block_info_t){16, "coal_ore", {35, 35, 24}}
  , (struct block_info_t){17, "wood", {64, 20, 9}}
  , (struct block_info_t){18, "leaves", {4, 70, 4}}
  , (struct block_info_t){19, "sponge", {218, 213, 163}}
  , (struct block_info_t){20, "glass", {94, 116, 121}}
  , (struct block_info_t){21, "lapis_lazuli_ore", {119, 119, 99}}
  , (struct block_info_t){22, "lapis_lazuli_block", {22, 50, 172}}
  , (struct block_info_t){23, "dispenser", {163, 161, 143}}
  , (struct block_info_t){24, "sandstone", {212, 208, 173}}
  , (struct block_info_t){25, "note_block", {99, 54, 4}}
  , (struct block_info_t){26, "colored_wool", {169, 12, 20}}
  , (struct block_info_t){27, "powered_rail", {222, 160, 85}}
  , (struct block_info_t){28, "detector_rail", {221, 116, 66}}
  , (struct block_info_t){29, "sticky_piston", {105, 89, 15}}
  , (struct block_info_t){30, "cobweb", {244, 244, 244}}
  , (struct block_info_t){31, "grass", {4, 82, 15}}
  , (struct block_info_t){32, "dead_bush", {100, 124, 12}}
  , (struct block_info_t){33, "piston", {160, 158, 146}}
  , (struct block_info_t){34, "black_wool", {30, 28, 28}}
  , (struct block_info_t){35, "wool", {232, 232, 232}}
  , (struct block_info_t){36, "wool", {232, 232, 232}}
  , (struct block_info_t){37, "yellow_flower", {244, 220, 84}}
  , (struct block_info_t){38, "red_flower", {244, 220, 84}}
  , (struct block_info_t){39, "brown_mushroom", {191, 170, 136}}
  , (struct block_info_t){40, "red_mushroom", {140, 132, 116}}
  , (struct block_info_t){41, "gold_block", {224, 196, 116}}
  , (struct block_info_t){42, "iron_block", {176, 176, 176}}
  , (struct block_info_t){43, "double_slabs", {95, 94, 73}}
  , (struct block_info_t){44, "slabs", {158, 157, 136}}
  , (struct block_info_t){45, "brick", {132, 66, 20}}
  , (struct block_info_t){46, "tnt", {65, 49, 9}}
  , (struct block_info_t){47, "bookshelf", {100, 34, 8}}
  , (struct block_info_t){48, "moss_stone", {87, 123, 55}}
  , (struct block_info_t){49, "obsidian", {12, 12, 12}}
  , (struct block_info_t){50, "torch", {108, 108, 108}}
  , (struct block_info_t){51, "fire", {252, 172, 4}}
  , (struct block_info_t){52, "monster_spawner", {71, 124, 32}}
  , (struct block_info_t){53, "wooden_stairs", {124, 116, 78}}
  , (struct block_info_t){54, "chest", {94, 50, 5}}
  , (struct block_info_t){55, "redstone_wire", {212, 188, 180}}
  , (struct block_info_t){56, "diamond_ore", {134, 132, 110}}
  , (struct block_info_t){57, "diamond_block", {74, 102, 118}}
  , (struct block_info_t){58, "workbench", {78, 38, 7}}
  , (struct block_info_t){59, "wheat_seeds", {193, 166, 72}}
  , (struct block_info_t){60, "soil", {110, 86, 44}}
  , (struct block_info_t){61, "furnace", {140, 134, 118}}
  , (struct block_info_t){62, "burning_furnace", {140, 134, 118}}
  , (struct block_info_t){63, "signpost", {74, 36, 4}}
  , (struct block_info_t){64, "wooden_door", {94, 50, 5}}
  , (struct block_info_t){65, "ladder", {84, 36, 4}}
  , (struct block_info_t){66, "minecart_track", {129, 129, 129}}
  , (struct block_info_t){67, "cobblestone_stairs", {128, 127, 102}}
  , (struct block_info_t){68, "wall_sign", {74, 36, 4}}
  , (struct block_info_t){69, "lever", {76, 36, 4}}
  , (struct block_info_t){70, "stone_pressure_plate", {128, 127, 102}}
  , (struct block_info_t){71, "iron_door", {40, 40, 47}}
  , (struct block_info_t){72, "wooden_pressure_plate", {124, 116, 78}}
  , (struct block_info_t){73, "redstone_ore", {129, 128, 106}}
  , (struct block_info_t){74, "glowing_redstone_ore", {129, 128, 106}}
  , (struct block_info_t){75, "redstone_torch_off", {108, 108, 108}}
  , (struct block_info_t){76, "redstone_torch_on", {108, 108, 108}}
  , (struct block_info_t){77, "stone_button", {128, 127, 102}}
  , (struct block_info_t){78, "snow", {244, 252, 252}}
  , (struct block_info_t){79, "ice", {113, 155, 251}}
  , (struct block_info_t){80, "snow_block", {244, 252, 252}}
  , (struct block_info_t){81, "cactus", {132, 156, 20}}
  , (struct block_info_t){82, "clay", {144, 73, 30}}
  , (struct block_info_t){83, "sugar_cane", {4, 100, 4}}
  , (struct block_info_t){84, "jukebox", {99, 54, 4}}
  , (struct block_info_t){85, "fence", {156, 156, 164}}
  , (struct block_info_t){86, "pumpkin", {206, 113, 4}}
  , (struct block_info_t){87, "netherrack", {159, 81, 75}}
  , (struct block_info_t){88, "soul_sand", {216, 211, 181}}
  , (struct block_info_t){89, "glowstone", {229, 173, 84}}
  , (struct block_info_t){90, "portal", {159, 81, 75}}
  , (struct block_info_t){91, "jack-o-lantern", {248, 218, 25}}
  , (struct block_info_t){92, "cake", {238, 238, 206}}
  , (struct block_info_t){95, "locked_chest", {94, 50, 5}}
  , (struct block_info_t){96, "trapdoor", {95, 48, 4}}
  , (struct block_info_t){97, "monster_egg", {71, 124, 32}}
  , (struct block_info_t){98, "stone_brick", {82, 77, 55}}
  , (struct block_info_t){99, "huge_brown_mushroom", {164, 124, 92}}
  , (struct block_info_t){100, "huge_red_mushroom", {140, 132, 116}}
  , (struct block_info_t){101, "iron_bars", {156, 156, 164}}
  , (struct block_info_t){102, "glass_pane", {94, 116, 121}}
  , (struct block_info_t){103, "melon", {51, 130, 4}}
  , (struct block_info_t){106, "vines", {4, 58, 4}}
  , (struct block_info_t){107, "fence_gate", {156, 156, 164}}
  , (struct block_info_t){108, "brick_stairs", {132, 66, 20}}
  , (struct block_info_t){109, "stone_brick_stairs", {128, 127, 102}}
  , (struct block_info_t){110, "mycelium", {199, 190, 167}}
  , (struct block_info_t){111, "lily_pad", {44, 100, 4}}
  , (struct block_info_t){112, "nether_brick", {159, 81, 75}}
  , (struct block_info_t){113, "nether_brick_fence", {156, 156, 164}}
  , (struct block_info_t){114, "nether_brick_stairs", {159, 81, 75}}
  , (struct block_info_t){116, "enchantment_table", {44, 44, 44}}
  , (struct block_info_t){121, "end_stone", {204, 204, 162}}
  , (struct block_info_t){122, "dragon_egg", {12, 12, 12}}
  , (struct block_info_t){123, "redstone_lamp", {79, 69, 40}}
  , (struct block_info_t){126, "wooden_slab", {164, 132, 76}}
  , (struct block_info_t){127, "cocoa_plant", {84, 36, 4}}
  , (struct block_info_t){128, "sandstone_stairs", {212, 208, 173}}
  , (struct block_info_t){129, "emerald_ore", {138, 138, 116}}
  , (struct block_info_t){130, "ender_chest", {94, 50, 5}}
  , (struct block_info_t){133, "block_of_emerald", {11, 140, 4}}
  , (struct block_info_t){134, "spruce_wood_stairs", {59, 40, 33}}
  , (struct block_info_t){135, "birch_wood_stairs", {124, 116, 78}}
  , (struct block_info_t){136, "jungle_wood_stairs", {76, 52, 28}}
  , (struct block_info_t){137, "command_block", {190, 138, 109}}
  , (struct block_info_t){138, "beacon", {92, 112, 116}}
  , (struct block_info_t){139, "cobblestone_wall", {109, 109, 109}}
  , (struct block_info_t){143, "wooden_button", {124, 116, 78}}
  , (struct block_info_t){145, "anvil", {36, 36, 36}}
  , (struct block_info_t){146, "trapped_chest", {94, 50, 5}}
  , (struct block_info_t){147, "weighted_pressure_plate_light", {128, 127, 102}}
  , (struct block_info_t){148, "weighted_pressure_plate_heavy", {128, 127, 102}}
  , (struct block_info_t){149, "redstone_comparator_inactive", {167, 164, 158}}
  , (struct block_info_t){150, "redstone_comparator_active", {159, 156, 150}}
  , (struct block_info_t){151, "daylight_sensor", {68, 60, 44}}
  , (struct block_info_t){152, "redstone_block", {152, 4, 4}}
  , (struct block_info_t){153, "nether_quartz_ore", {135, 73, 68}}
  , (struct block_info_t){154, "hopper", {36, 36, 36}}
  , (struct block_info_t){155, "quartz_block", {189, 184, 176}}
  , (struct block_info_t){156, "quartz_stairs", {128, 127, 102}}
  , (struct block_info_t){157, "activator_rail", {130, 130, 130}}
  , (struct block_info_t){158, "dropper", {162, 161, 142}}
  , (struct block_info_t){170, "hay_bale", {193, 166, 72}}
  , (struct block_info_t){171, "carpet", {87, 123, 55}}
  , (struct block_info_t){260, "apple", {100, 68, 17}}
  , (struct block_info_t){262, "arrow", {76, 36, 4}}
  , (struct block_info_t){263, "coal", {35, 35, 24}}
  , (struct block_info_t){264, "diamond", {74, 102, 118}}
  , (struct block_info_t){265, "iron_ingot", {156, 156, 164}}
  , (struct block_info_t){266, "gold_ingot", {224, 196, 116}}
  , (struct block_info_t){280, "stick", {4, 68, 4}}
  , (struct block_info_t){281, "bowl", {44, 100, 4}}
  , (struct block_info_t){282, "mushroom_soup", {44, 100, 4}}
  , (struct block_info_t){287, "string", {232, 232, 232}}
  , (struct block_info_t){288, "feather", {12, 124, 20}}
  , (struct block_info_t){289, "gun_powder", {244, 220, 84}}
  , (struct block_info_t){295, "seeds", {100, 68, 17}}
  , (struct block_info_t){296, "wheat", {193, 166, 72}}
  , (struct block_info_t){297, "bread", {132, 66, 20}}
  , (struct block_info_t){318, "flint", {35, 35, 24}}
  , (struct block_info_t){319, "raw_porkchop", {108, 76, 36}}
  , (struct block_info_t){320, "cooked_porkchop", {108, 76, 36}}
  , (struct block_info_t){321, "paintings", {208, 134, 155}}
  , (struct block_info_t){322, "golden_apple", {224, 196, 116}}
  , (struct block_info_t){323, "sign", {74, 36, 4}}
  , (struct block_info_t){324, "wooden_door", {94, 50, 5}}
  , (struct block_info_t){325, "bucket", {64, 20, 9}}
  , (struct block_info_t){326, "water_bucket", {64, 20, 9}}
  , (struct block_info_t){327, "lava_bucket", {64, 20, 9}}
  , (struct block_info_t){329, "saddle", {216, 211, 181}}
  , (struct block_info_t){330, "iron_door", {40, 40, 47}}
  , (struct block_info_t){331, "redstone_dust", {129, 128, 106}}
  , (struct block_info_t){332, "snowball", {244, 252, 252}}
  , (struct block_info_t){333, "boat", {132, 66, 20}}
  , (struct block_info_t){334, "leather", {64, 20, 9}}
  , (struct block_info_t){335, "milk", {244, 252, 252}}
  , (struct block_info_t){336, "clay_brick", {144, 73, 30}}
  , (struct block_info_t){337, "clay_balls", {144, 73, 30}}
  , (struct block_info_t){338, "sugar_cane", {4, 100, 4}}
  , (struct block_info_t){339, "paper", {208, 134, 155}}
  , (struct block_info_t){340, "book", {100, 34, 8}}
  , (struct block_info_t){341, "slimeball", {49, 65, 28}}
  , (struct block_info_t){344, "egg", {11, 140, 4}}
  , (struct block_info_t){346, "fishing_rod", {156, 156, 164}}
  , (struct block_info_t){348, "glowstone_dust", {229, 173, 84}}
  , (struct block_info_t){349, "raw_fish", {208, 134, 155}}
  , (struct block_info_t){350, "cooked_fish", {208, 134, 155}}
  , (struct block_info_t){351, "dyes", {208, 134, 155}}
  , (struct block_info_t){352, "bone", {244, 252, 252}}
  , (struct block_info_t){353, "sugar", {4, 100, 4}}
  , (struct block_info_t){355, "bed", {156, 156, 148}}
  , (struct block_info_t){356, "redstone_repeater", {212, 188, 185}}
  , (struct block_info_t){357, "cookie", {208, 134, 155}}
  , (struct block_info_t){358, "map", {51, 130, 4}}
  , (struct block_info_t){359, "shears", {208, 134, 155}}
  , (struct block_info_t){360, "melon_slice", {51, 130, 4}}
  , (struct block_info_t){361, "pumpkin_seeds", {192, 108, 4}}
  , (struct block_info_t){362, "melon_seeds", {76, 84, 252}}
  , (struct block_info_t){363, "raw_beef", {208, 134, 155}}
  , (struct block_info_t){364, "steak", {208, 134, 155}}
  , (struct block_info_t){365, "raw_chicken", {208, 134, 155}}
  , (struct block_info_t){366, "cooked_chicken", {208, 134, 155}}
  , (struct block_info_t){367, "rotton_flesh", {208, 134, 155}}
  , (struct block_info_t){368, "ender_pearl", {204, 204, 162}}
  , (struct block_info_t){369, "blaze_rod", {110, 86, 44}}
  , (struct block_info_t){370, "ghast_tear", {204, 204, 162}}
  , (struct block_info_t){374, "glass_bottle", {94, 116, 121}}
  , (struct block_info_t){375, "spider_eye", {49, 65, 28}}
  , (struct block_info_t){376, "fermented_spider_eye", {49, 65, 28}}
  , (struct block_info_t){377, "blaze_powder", {173, 79, 36}}
  , (struct block_info_t){378, "magma_cream", {173, 79, 36}}
  , (struct block_info_t){379, "brewing_stand", {180, 140, 60}}
  , (struct block_info_t){380, "cauldron", {35, 35, 35}}
  , (struct block_info_t){381, "eye_of_ender", {4, 12, 12}}
  , (struct block_info_t){382, "glistering_melon", {51, 130, 4}}
  , (struct block_info_t){383, "spawn_eggs", {11, 140, 4}}
  , (struct block_info_t){384, "bottle_o_enchanting", {44, 44, 44}}
  , (struct block_info_t){385, "fire_charge", {252, 172, 4}}
  , (struct block_info_t){388, "emerald", {11, 140, 4}}
  , (struct block_info_t){390, "flower_pot", {140, 140, 100}}
  , (struct block_info_t){391, "carrot", {4, 76, 4}}
  , (struct block_info_t){392, "unknown", {4, 82, 15}}
  , (struct block_info_t){393, "baked_potato", {92, 112, 116}}
  , (struct block_info_t){394, "poisonous_potato", {92, 112, 116}}
  , (struct block_info_t){395, "map", {51, 130, 4}}
  , (struct block_info_t){396, "golden_carrot", {92, 112, 116}}
  , (struct block_info_t){397, "mob_head", {28, 28, 28}}
  , (struct block_info_t){399, "nether_star", {130, 68, 42}}
  , (struct block_info_t){400, "pumkpin_pie", {192, 108, 4}}
  , (struct block_info_t){401, "firework_rocket", {85, 104, 4}}
  , (struct block_info_t){402, "firework_star", {252, 172, 4}}
};

static const std::size_t block_infos_len = sizeof(block_infos_array) / sizeof(block_info_t);

typedef std::map<int, block_info_t> block_infos_t;

block_infos_t gen_block_infos(){
    std::map<int, block_info_t> result;

    for (std::size_t i = 0; i < block_infos_len; ++i)
    {
        auto block_info = block_infos_array[i];

        assert(result.count(block_info.id) == 0);
        result[block_info.id] = block_info;
    }

    return result;
}

static const block_infos_t block_infos = gen_block_infos();

static inline block_info_t get_block_info(int block_id)
{
  auto w = block_infos.find(block_id);
  if (w != block_infos.end())
    return w->second;

  w = block_infos.find(-10);
  return w->second;
}

/**
 * @brief Callback to iterate minecraft's BlockTag format.
 * @param side the expected side of the chunk, should be 16
 * @param blocks_data the raw pointer to the BlockTag payload
 * @param blocks_data_len the length in bytes of the BlockTag payload
 * @param visitor a callback that will be called for each block.
 */
template<typename visitor_f>
inline void iterate_blocks_and_add_data(
      vside_t side
    , const uint8_t* blocks_data, std::size_t blocks_data_len
    , const uint8_t* add_data, std::size_t add_data_len
    , visitor_f visitor);

/**
 * @brief Callback to iterate minecraft's BlockTag format.
 * @param side the expected side of the chunk, should be 16
 * @param blocks_data the raw pointer to the BlockTag payload
 * @param blocks_data_len the length in bytes of the BlockTag payload
 * @param visitor a callback that will be called for each block.
 */
template<typename visitor_f>
inline void iterate_blocks(
      vside_t side
    , const uint8_t* blocks_data
    , std::size_t blocks_data_len
    , visitor_f visitor);

void extract_chunk(chunk_id_t chunk_id, const uint8_t* compressed_buffer_ptr, std::size_t compressed_buffer_len
                    , std::size_t vertical_slices
                    , std::vector<svo_slice_t*>& all_slices)
{
    assert(all_slices.size() == region_side_chunks*region_side_chunks*region_side_chunks);
    
    ///the schema for storing colors/normals/etc.
    auto buffer_schema = svo_schema_t();
    
    {
        auto color_buffer_decl = svo_declaration_t();
        color_buffer_decl.add(svo_element_t("color", svo_semantic_t::COLOR, svo_data_type_t::FLOAT, 3));
        buffer_schema.push_back(color_buffer_decl);
        
        auto normal_buffer_decl = svo_declaration_t();
        normal_buffer_decl.add(svo_element_t("normal", svo_semantic_t::NORMAL, svo_data_type_t::FLOAT, 3));
        buffer_schema.push_back(normal_buffer_decl);
    }
            
    //std::cout << __FILE__ << ":" << __LINE__ << std::endl;
    //std::cout << __FILE__ << ":" << __LINE__ << std::endl;
    std::size_t chunkX = chunk_id % region_side_chunks;
    std::size_t chunkZ = chunk_id / region_side_chunks;
    assert( ((chunkX % region_side_chunks) + (chunkZ % region_side_chunks) * region_side_chunks) == chunk_id );



    // print bufferCompressed
    /*
    DEBUG {
        printf("lengthCompressed=%i\n", lengthCompressed);
        for (int i = 0; i < lengthCompressed && i < 512; i++) {
            printf("%2x ", (unsigned char) bufferCompressed[i]);
            if (i%8 == 7) printf(" ");
            if (i%32 == 31) printf("\n");
        }
    }
    */

    //std::cout << __FILE__ << ":" << __LINE__ << std::endl;
    nbt_node* node = nbt_parse_compressed(compressed_buffer_ptr, compressed_buffer_len);

    if (!node)
        throw std::runtime_error("Error occured while parsing chunk");
    
    //auto* str = nbt_dump_ascii(node);

    auto* LevelTag = nbt_find_by_name(node, "Level");

    assert(LevelTag);

    auto* xPos = nbt_find_by_name(LevelTag, "xPos");
    auto* zPos = nbt_find_by_name(LevelTag, "zPos");
    assert(xPos);
    assert(zPos);


    auto* Sections = nbt_find_by_name(LevelTag, "Sections");

    if (!Sections)
        return;

    assert(Sections);

    assert (Sections->type == TAG_LIST);
    assert (Sections->payload.tag_list);
    auto* list = Sections->payload.tag_list;
    //const struct list_head* pos;

    struct list_head* current;
    
    

    list_for_each(current, &list->entry)
    {
        struct nbt_list {
            struct nbt_node* data; /* A single node's data. */
            struct list_head entry;
        };
        struct nbt_list* entry = list_entry(current, struct nbt_list, entry);
        assert(entry);

        nbt_node* Section = entry->data;
        assert(Section);

        auto* YTag = nbt_find_by_name(Section, "Y");
        //std::cout << "type(YTag): " << nbt_type_to_string(YTag->type) << std::endl;
        assert(YTag); assert(YTag->type == TAG_BYTE);

        std::size_t Y = YTag->payload.tag_byte;
        assert( Y < vertical_slices );
        
        vcurve_t slice_vcurve = coords2vcurve(chunkX, Y, chunkZ, region_side_chunks);
        
        assert(slice_vcurve < all_slices.size());
        
        svo_slice_t* chunk_cube_slice = all_slices[ slice_vcurve ];
        if (!chunk_cube_slice)
            continue;
        
        assert( chunk_cube_slice->side == base_slice_side );

        auto* Data = nbt_find_by_name(Section, "Data");
        assert(Data);
        assert(Data->type == TAG_BYTE_ARRAY);

        auto* AddTag = nbt_find_by_name(Section, "Add");

        assert(AddTag == nullptr || AddTag->type == TAG_BYTE_ARRAY);

        //std::cout << "Blocks: " << (!Blocks ? std::string("NULL") : Blocks->asString()) << std::endl;
        //std::cout << "Add: " << (!AddTag ? std::string("NULL") : AddTag->asString()) << std::endl;
        //std::cout << "Data: " << (!Data ? std::string("NULL") : Data->asString()) << std::endl;


        
        assert(chunk_cube_slice);
        assert(chunk_cube_slice->pos_data);
        assert(chunk_cube_slice->buffers);
        assert(chunk_cube_slice->buffers->buffers().size() == 0);

        auto& dst_pos_data = *(chunk_cube_slice->pos_data);
        auto& dst_buffers = *chunk_cube_slice->buffers;

        assert(dst_pos_data.size() == 0);


        
        




        const auto* BlocksTag = nbt_find_by_name(Section,"Blocks");
        //std::cout << "type(Blocks): " << nbt_type_to_string(BlocksTag->type) << std::endl;
        assert(BlocksTag->type == TAG_BYTE_ARRAY);

        //std::cout << __FILE__ << ":" << __LINE__ << std::endl;

        if (BlocksTag->payload.tag_byte_array.length == 0)
            continue;

        assert(BlocksTag->payload.tag_byte_array.data);
        
        /*
        SCAFFOLDING{
            char* block_ascii = nbt_dump_ascii(BlocksTag);
            assert(block_ascii);
            std::cout << "BlocksTag: " << std::endl
                << block_ascii << std::endl;
            free(block_ascii);
        }*/
        
        uint32_t blocks_data_len = BlocksTag->payload.tag_byte_array.length;
        const uint8_t* blocks_data = BlocksTag->payload.tag_byte_array.data;

        uint32_t add_data_len = (AddTag) ? AddTag->payload.tag_byte_array.length : 0;
        const uint8_t* add_data = (AddTag) ? AddTag->payload.tag_byte_array.data : nullptr;

        uint32_t data_data_len = Data->payload.tag_byte_array.length;
        const uint8_t* data_data = Data->payload.tag_byte_array.data;


        assert(vcurvesize(chunk_cube_slice->side) == blocks_data_len*1);
        assert(add_data == nullptr || vcurvesize(chunk_cube_slice->side) == add_data_len*2);
        assert(vcurvesize(chunk_cube_slice->side) == data_data_len*2);



        
        auto posdata_push_visitor = [&dst_pos_data](
            vcurve_t vcurve, int x, int y, int z, std::size_t src_data_index, uint16_t block_id)
        {
            if (block_id == 0)
                return;
            
            dst_pos_data.push_back(vcurve);
        };


        ///iterate once, get all the voxel positions
        if (add_data)
            iterate_blocks_and_add_data(chunk_cube_slice->side
                                        , blocks_data, blocks_data_len
                                        , add_data, add_data_len
                                        , posdata_push_visitor);
        else
            iterate_blocks(chunk_cube_slice->side, blocks_data, blocks_data_len, posdata_push_visitor);
        
        ///setup the data buffers
        dst_buffers.copy_schema(buffer_schema, dst_pos_data.size());
        
        auto color_element = dst_buffers.get_element_view("color");
        auto normal_element = dst_buffers.get_element_view("normal");
        
        
        std::size_t out_data_index = 0;
        auto data_push_visitor = [&color_element, &normal_element, &data_data, &out_data_index](
            vcurve_t vcurve, int x, int y, int z, std::size_t src_data_index, uint16_t block_id)
        {
            if (block_id == 0)
                return;
            
            //uint8_t extra_data = (data_data[src_data_index/2] >> ((src_data_index & 1)*4)) & 0b1111;
            
            auto block_info = get_block_info(block_id);

            float3_t color = float3_t(block_info.color[0], block_info.color[1], block_info.color[2]);
            float3_t normal = float3_t(0);

            color_element.template get<float3_t>(out_data_index) = color;
            normal_element.template get<float3_t>(out_data_index) = normal;
            ++out_data_index;
        };
        
        ///iterate again, get all the voxel attribute data
        if (add_data)
            iterate_blocks_and_add_data(chunk_cube_slice->side
                                        , blocks_data, blocks_data_len
                                        , add_data, add_data_len
                                        , data_push_visitor);
        else
            iterate_blocks(chunk_cube_slice->side, blocks_data, blocks_data_len, data_push_visitor);

        assert(out_data_index == dst_pos_data.size());

    }


    nbt_free(node);
}


void load_mca_region(volume_of_slices_t& slices, std::ifstream& region_file, std::size_t num_threads)
{
    
    assert(slices.volume_side == region_side_chunks);
    assert(region_side_chunks*region_side_chunks*region_side_chunks == vcurvesize(region_side_chunks));
    
    std::size_t vertical_slices = 32;

    //slices.clear();
    //slices.resize(32*32*vertical_slices);
    
    

    std::vector<uint8_t> buffer_data(1024*1024);
    std::vector<uint8_t> region_data;

    while(region_file)
    {
        region_file.read((char*)buffer_data.data(), buffer_data.size());
        region_data.insert(region_data.end(), buffer_data.data(), buffer_data.data() + region_file.gcount());
    }


    
    ///vcurve => slice
    std::vector<svo_slice_t*> all_slices;
    
    
    ///fill up all_slices (vcurve => slice) vector-mapping
    for (std::size_t slice_index = 0; slice_index < slices.slices.size(); ++slice_index)
    {
        vcurve_t slice_vcurve; svo_slice_t* slice;
        std::tie(slice_vcurve, slice) = slices.slices[slice_index];
        
        assert(slice_vcurve < vcurvesize(slices.volume_side));
        
        while (all_slices.size() < slice_vcurve)
            all_slices.push_back(nullptr);
        
        assert(all_slices.size() == slice_vcurve);
        
        all_slices.push_back(slice);
    }
    
    while (all_slices.size() < vcurvesize(region_side_chunks))
        all_slices.push_back(nullptr);
    
    
    assert(all_slices.size() == region_side_chunks*region_side_chunks*region_side_chunks);
    
    ///(data pointer, length)
    typedef std::pair<const uint8_t*, std::size_t> data_section_t;
    std::map< chunk_id_t, data_section_t > jobs;
    
    //for (std::size_t chunkID = 0; chunkID < 32*32; ++chunkID)
    ///collect jobs for wanted chunks.
    for (std::size_t slice_index = 0; slice_index < slices.slices.size(); ++slice_index)
    {
        vcurve_t slice_vcurve; svo_slice_t* slice;
        std::tie(slice_vcurve, slice) = slices.slices[slice_index];
        
        ///slice coordinates within the volume of slices, in units of number-of-slices.
        vside_t sx, sy, sz;
        vcurve2coords(slice_vcurve, slices.volume_side, &sx, &sy, &sz);
        
        ///minecraft chunk ID
        chunk_id_t chunk_id = sx + sz*16;
        assert(chunk_id < region_side_chunks * region_side_chunks);
        
        
        if (jobs.count(chunk_id) > 0)
            continue;
        
        assert(chunk_id*4 + 4 <= region_data.size());
        
        uint32_t chunkPos = *reinterpret_cast<uint32_t*>(&region_data[chunk_id*4]);
        
        swapBytes((unsigned char*) &chunkPos, 4);
        chunkPos = chunkPos >> 8;

        //std::cout << "chunkID: " << chunkID << std::endl;
        //std::cout << "chunkPos: " << chunkPos << std::endl;
        if (chunkPos == 0) continue;
        
        assert(chunkPos*4096 + 4 <= region_data.size());
        uint32_t lengthCompressed = *reinterpret_cast<uint32_t*>(&region_data[chunkPos*4096]);
        //region_file.seekg(chunkPos*4096);
        //region_file.read((char*)(&lengthCompressed), 4);
        swapBytes((unsigned char*) &lengthCompressed, 4);


        //std::cout << "lengthCompressed: " << lengthCompressed << std::endl;
        if (lengthCompressed == 0) continue;

        assert(chunkPos*4096 + 4 + 1 <= region_data.size());
        uint8_t compressionType = *reinterpret_cast<uint8_t*>(&region_data[chunkPos*4096 + 4]);
        //region_file.seekg(chunkPos*4096+4);
        //region_file.read(reinterpret_cast<char*>(&compressionType),1);
        assert( compressionType == 2 && "Compression type is not 2/Zlib; compression type is not supported.");

        ///uncompress the data
        //std::vector<uint8_t> bufferCompressed(lengthCompressed);
        //region_file.seekg(chunkPos*4096+5);
        //region_file.read((char*) bufferCompressed.data(), lengthCompressed);

        const uint8_t* compressed_buffer = region_data.data() + chunkPos*4096 + 4 + 1;

        
        auto& job = jobs[chunk_id];
        
        job.first = compressed_buffer;
        job.second = lengthCompressed;
    }
    
    ThreadPool pool(num_threads);
    
    for (const auto& id_job_pair : jobs)
    {
        auto chunk_id = id_job_pair.first;
        const auto& job = id_job_pair.second;
        const uint8_t* compressed_buffer_ptr = job.first;
        std::size_t compressed_buffer_len = job.second;
        pool.enqueue( std::bind(extract_chunk, chunk_id, compressed_buffer_ptr, compressed_buffer_len, vertical_slices, std::ref(all_slices)) );
    }
}













template<typename visitor_f>
inline void iterate_blocks_and_add_data(
      vside_t side
    , const uint8_t* blocks_data, std::size_t blocks_data_len
    , const uint8_t* add_data, std::size_t add_data_len
    , visitor_f visitor)
{
    auto inner_visitor = [&visitor, add_data, add_data_len](
        vcurve_t vcurve, int x, int y, int z, std::size_t src_data_index, uint16_t block_id_a)
    {
        
        std::size_t add_data_index = src_data_index/2;
        
        assert( add_data_index < add_data_len );
        
        ///We want 4 bits here, so get the relevant byte.
        uint8_t block_id_b = add_data[src_data_index/2];
        ///We want 4 bits here, so if index is even, we take the lowest 4, if it is odd, we take the upper 4.
        std::size_t shift_right_len = (src_data_index & 1)*4;
        block_id_b = (block_id_b >> shift_right_len) & 0b1111;

        ///now combine it with the voxel data
        uint16_t block_id = block_id_a | (block_id_b << 8);
        
        
        visitor(vcurve, x,y,z, src_data_index, block_id);
    };
    
    iterate_blocks(side, blocks_data, blocks_data_len, inner_visitor);
}

template<typename visitor_f>
inline void iterate_blocks(
      vside_t side
    , const uint8_t* blocks_data, std::size_t blocks_data_len
    , visitor_f visitor)
{
    
    for (vcurve_t vcurve = 0; vcurve < vcurvesize(side); ++vcurve)
    {
        vside_t x, y, z;
        vcurve2coords(vcurve, side, &x,&y,&z);

        //x = index % 16;
        //y = (index / 16) % 16;
        //z = index / (16*16);

        std::size_t src_data_index = x + y*16 + z*16*16;

        assert( coords2vcurve(x,y,z,side) == vcurve );
        assert( src_data_index < blocks_data_len );
        
        uint8_t block_id_a = blocks_data[src_data_index];
        
        visitor(vcurve, x,y,z, src_data_index, block_id_a);
    }
}


} //namespace svo

