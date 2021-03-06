#pragma once

#include <sp2/string.h>
#include <sp2/math/vector3.h>
#include <unordered_map>
#include <memory>

#include "world.h"

class Recipe;
class ItemType
{
public:
    sp::string name;
    sp::string label;
    sp::string texture;
    sp::Vector2i size;
    enum class BuildingType
    {
        None,
        Miner,
        Belt,
        Splitter,
        Bridge,
        Factory,
        Storage
    } building_type = BuildingType::None;
    std::vector<const Recipe*> recipes;

    sp::P<sp::Node> placeAt(Tile* tile) const;

    static void init();
    static void initRecipes();
    static const ItemType* get(const sp::string& name);
    static sp::string translate(const sp::string& input);
private:
    static std::unordered_map<sp::string, std::unique_ptr<ItemType>> items;
    static std::map<sp::string, sp::string> translations;
};
