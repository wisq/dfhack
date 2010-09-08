#include <map>
#include <iostream>
#include <algorithm>
#include <tinyxml.h>
#include <tinystr.h>
#include <set>
#include <map>
#include <vector>
#include <sstream>
#include <dfhack/DFIntegers.h>
#include <stddef.h>
#include <dfhack/BaseTypes.h>
using namespace DFHack;
using namespace std;

class DF2010_offsets
{
    public:
    Address WORLD;
    struct grp_vector
    {
        Offset start;
        HexValue size_of;
    } vector;
    struct grp_string
    {
        HexValue size_of;
    } string;
    struct grp_name
    {
        Offset first;
        Offset nick;
        Offset second_words;
    } name;
    struct grp_Position
    {
        Address cursor_xyz;
        Address mouse_pos;
        Address screen_tiles_pointer;
        Address window_dims;
        Address window_x;
        Address window_y;
        Address window_z;
    } Position;
    struct grp_GUI
    {
        Address current_cursor_creature;
        Address current_menu_state;
        Address pause_state;
        Address view_screen;
    } GUI;
    struct grp_Maps
    {
        Address map_data;
        Address region_x;
        Address region_y;
        Address region_z;
        Address world_size_x;
        Address world_size_y;
        Address x_count;
        Address x_count_block;
        Address y_count;
        Address y_count_block;
        Address z_count;
        Address z_count_block;
        struct grp_block
        {
            Offset biome_stuffs;
            Offset designation;
            Offset feature_global;
            Offset feature_local;
            Offset occupancy;
            Offset pathfinding;
            Offset temperature1;
            Offset temperature2;
            Offset type;
            Offset vein_vector;
        } block;
        struct grp_features
        {
            struct grp_global
            {
                Address vector;
                Offset funcptr;
                Offset material;
                Offset submaterial;
            } global;
            struct grp_local
            {
                Address start_ptr;
                Offset material;
                Offset submaterial;
            } local;
        } features;
        struct grp_geology
        {
            Address geoblock_vector;
            Address ptr2_region_array;
            Offset geolayer_geoblock_offset;
            Offset region_geo_index_off;
            Offset type_inside_geolayer;
            HexValue region_size;
        } geology;
    } Maps;
    struct grp_Creatures
    {
        Address current_civ;
        Address current_race;
        Address vector;
        struct grp_creature
        {
            Offset caste;
            Offset civ;
            Offset custom_profession;
            Offset flags1;
            Offset flags2;
            Offset id;
            Offset name;
            Offset position;
            Offset profession;
            Offset race;
            Offset sex;
            struct grp_advanced
            {
                Offset appearance_vector;
                Offset artifact_name;
                Offset birth_time;
                Offset birth_year;
                Offset current_job;
                Offset current_job_skill;
                Offset current_soul;
                Offset happiness;
                Offset inventory_vector;
                Offset labors;
                Offset mood;
                Offset physical;
                Offset pickup_equipment_bit;
                Offset pregnancy;
                Offset pregnancy_ptr;
                Offset soul_vector;
            } advanced;
        } creature;
        struct grp_soul
        {
            Offset mental;
            Offset name;
            Offset skills_vector;
            Offset traits;
        } soul;
        struct grp_job
        {
            Offset id;
            Offset materials_vector;
            Offset type;
            struct grp_material
            {
                Offset flags;
                Offset maintype;
                Offset sectype1;
                Offset sectype2;
                Offset sectype3;
            } material;
        } job;
    } Creatures;
    struct grp_Materials
    {
        Address creature_type_vector;
        Address inorganics;
        Address organics_all;
        Address organics_plants;
        Address organics_trees;
        Address other;
        struct grp_creature
        {
            Offset caste_vector;
            Offset extract_vector;
            Offset tile;
            Offset tile_color;
            struct grp_caste
            {
                Offset attributes;
                Offset bodypart_vector;
                Offset color_modifiers;
            } caste;
            struct grp_caste_color_mods
            {
                Offset enddate;
                Offset part;
                Offset startdate;
            } caste_color_mods;
            struct grp_caste_bodyparts
            {
                Offset category;
                Offset id;
                Offset layers_vector;
                Offset plural_vector;
                Offset singular_vector;
            } caste_bodyparts;
        } creature;
        struct grp_descriptors
        {
            Address all_colors_vector;
            Address colors_vector;
            Address vectors_start;
            Offset color_b;
            Offset color_r;
            Offset color_v;
            Offset name;
            Offset rawname;
        } descriptors;
    } Materials;
    struct grp_Constructions
    {
        Address vector;
        Offset size_of;
    } Constructions;
    struct grp_Translations
    {
        Address language_vector;
        Address translation_vector;
        Offset word_table;
    } Translations;
    struct grp_Vegetation
    {
        Address vector;
        Offset tree_desc_offset;
    } Vegetation;
    struct grp_Buildings
    {
        Address buildings_vector;
        Address custom_workshop_vector;
        Offset building_custom_workshop_type;
        Offset custom_workshop_name;
        Offset custom_workshop_type;
    } Buildings;
    struct grp_Hotkeys
    {
        Address start;
        Offset coords;
        Offset mode;
        HexValue size_of;
    } Hotkeys;
    struct grp_Items
    {
        Address items_vector;
        Offset item_improvement_index;
        Offset item_improvement_quality;
        Offset item_improvement_subindex;
        Offset item_improvement_vector;
        Offset item_index_accessor;
        Offset item_quality_accessor;
        Offset item_subindex_accessor;
        Offset item_subtype_accessor;
        Offset item_type_accessor;
    } Items;
    struct grp_World
    {
        Address current_tick;
        Address current_weather;
        Address current_year;
    } World;

    bool Load(TiXmlElement* version);
};


class _Reader
{
    std::map <std::string, uint32_t> Offsets;
    std::map <std::string, uint32_t> Addresses;
    std::map <std::string, uint32_t> HexValues;
    std::map <std::string, uint32_t> Strings;
public:
    bool Read(const TiXmlElement * root, DF2010_offsets & target);
    _Reader()
    {
        Addresses["WORLD"] = offsetof(class DF2010_offsets, WORLD);
        Addresses["World.current_tick"] = offsetof(class DF2010_offsets, World.current_tick);
    }
};

bool _Reader::Read(const TiXmlElement * root, DF2010_offsets & target)
{
    
}

int main ( int argc, char** argv )
{
    if(argc < 2)
    {
        cout << "A file is required!" << ::std::endl;
        return EXIT_FAILURE;
    }
    DF2010_offsets offs;
    _Reader zlo;
    //zlo.Read();
    return EXIT_SUCCESS;
}