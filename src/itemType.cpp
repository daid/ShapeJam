#include "itemType.h"
#include "item.h"
#include "recipe.h"
#include "world.h"

#include "miner.h"
#include "belt.h"
#include "factory.h"
#include "bridge.h"
#include "storage.h"

#include <sp2/io/keyValueTreeLoader.h>
#include <sp2/audio/sound.h>

std::unordered_map<sp::string, std::unique_ptr<ItemType>> ItemType::items;
std::map<sp::string, sp::string> ItemType::translations;


sp::P<sp::Node> ItemType::placeAt(Tile* tile) const
{
    sp::P<Building> building;

    switch(building_type)
    {
    case BuildingType::None:
        break;
    case BuildingType::Miner:
        building = new Miner(&tile->side.world, this);
        break;
    case BuildingType::Belt:
        building = new Belt(&tile->side.world);
        break;
    case BuildingType::Splitter:
        building = new Splitter(&tile->side.world);
        break;
    case BuildingType::Bridge:
        building = new Bridge(&tile->side.world);
        break;
    case BuildingType::Factory:
        building = new Factory(&tile->side.world, this);
        break;
    case BuildingType::Storage:
        building = new Storage(&tile->side.world, this);
        break;
    }

    if (building)
    {
        if (!building->placeAt(tile))
        {
            building.destroy();
            return nullptr;
        }
        building->placed_from_type = this;
        sp::audio::Sound::play("place.wav");
        return building;
    }

    if (tile->item)
        return nullptr;
    if (tile->building && !tile->building->accepts(this, Direction::Forward))
        return nullptr;
    return new Item(tile, this);
}

void ItemType::init()
{
    if (!items.empty())
        return;

    for(auto info : sp::io::KeyValueTreeLoader::load("items.txt")->getFlattenNodesByIds())
    {
        auto entry = std::unique_ptr<ItemType>(new ItemType());
        entry->name = info.first;
        entry->label = info.first;
        entry->size = sp::Vector2i(1, 1);
        if (info.second.find("label") != info.second.end())
            entry->label = info.second["label"];
        if (info.second.find("size") != info.second.end())
            entry->size = sp::stringutil::convert::toVector2i(info.second["size"]);
        entry->size.x = std::max(1, entry->size.x);
        entry->size.y = std::max(1, entry->size.y);
        entry->texture = "item/" + info.second["texture"];
        if (info.second["building"].lower() == "miner")
            entry->building_type = BuildingType::Miner;
        if (info.second["building"].lower() == "belt")
            entry->building_type = BuildingType::Belt;
        if (info.second["building"].lower() == "splitter")
            entry->building_type = BuildingType::Splitter;
        if (info.second["building"].lower() == "factory")
            entry->building_type = BuildingType::Factory;
        if (info.second["building"].lower() == "bridge")
            entry->building_type = BuildingType::Bridge;
        if (info.second["building"].lower() == "storage")
            entry->building_type = BuildingType::Storage;
        translations[info.first] = entry->label;
        items[info.first] = std::move(entry);
    }
}

void ItemType::initRecipes()
{
    for(auto info : sp::io::KeyValueTreeLoader::load("items.txt")->getFlattenNodesByIds())
    {
        if (info.second.find("recipes") != info.second.end())
        {
            items[info.first]->recipes.clear();
            for(auto s : info.second["recipes"].split(" "))
                items[info.first]->recipes.push_back(Recipe::get(s));
        }
        if (Recipe::get(info.first) == nullptr)
            LOG(Warning, "No recipe to create", info.first);
    }
}

const ItemType* ItemType::get(const sp::string& name)
{
    auto it = items.find(name);
    if (it == items.end())
        return nullptr;
    return &*it->second;
}

sp::string ItemType::translate(const sp::string& input)
{
    return input.format(translations);
}
