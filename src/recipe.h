#pragma once

#include "itemType.h"

class Recipe
{
public:
    sp::string name;
    float craft_time;
    std::vector<std::pair<const ItemType*, int>> input;
    std::vector<std::pair<const ItemType*, int>> output;

    static void init();
    static Recipe& get(const sp::string& name);
private:
    static std::unordered_map<sp::string, std::unique_ptr<Recipe>> items;
};
