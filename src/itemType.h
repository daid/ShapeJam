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
        Factory
    } building_type = BuildingType::None;
    std::vector<const Recipe*> recipes;

    sp::P<sp::Node> placeAt(sp::P<World> world, sp::Vector3d position, sp::Vector3d normal);

    static void init();
    static void initRecipes();
    static ItemType& get(const sp::string& name);
private:
    static std::unordered_map<sp::string, std::unique_ptr<ItemType>> items;
};
