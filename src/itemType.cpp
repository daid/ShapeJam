#include "itemType.h"
#include "item.h"
#include "world.h"

#include "miner.h"
#include "belt.h"
#include "factory.h"

#include <sp2/io/keyValueTreeLoader.h>

std::unordered_map<sp::string, std::unique_ptr<ItemType>> ItemType::items;

sp::P<sp::Node> ItemType::placeAt(sp::P<World> world, sp::Vector3d position, sp::Vector3d normal)
{
    sp::P<Building> building;

    switch(building_type)
    {
    case BuildingType::None:
        break;
    case BuildingType::Miner:
        building = new Miner(world, position, normal);
        break;
    case BuildingType::Belt:
        building = new Belt(world, position, normal);
        break;
    case BuildingType::Splitter:
        building = new Splitter(world, position, normal);
        break;
    case BuildingType::Factory:
        building = new Factory(world, position, normal);
        break;
    }

    if (building)
    {
        if (!building->isPlaced())
        {
            building.destroy();
            return nullptr;
        }
        building->placed_from_type = this;
        return building;
    }

    Tile& tile = world->getTileAt(position, normal);
    if (tile.item)
        return nullptr;
    if (tile.building && !tile.building->accepts(*this))
        return nullptr;
    return new Item(&tile, *this);
}

void ItemType::init()
{
    if (!items.empty())
        return;
    
    for(auto info : sp::io::KeyValueTreeLoader::load("items.txt")->getFlattenNodesByIds())
    {
        auto entry = std::unique_ptr<ItemType>(new ItemType());
        entry->name = info.first;
        entry->texture = "item/" + info.second["texture"];
        if (info.second["building"].lower() == "miner")
            entry->building_type = BuildingType::Miner;
        if (info.second["building"].lower() == "belt")
            entry->building_type = BuildingType::Belt;
        if (info.second["building"].lower() == "splitter")
            entry->building_type = BuildingType::Splitter;
        if (info.second["building"].lower() == "factory")
            entry->building_type = BuildingType::Factory;
        items[info.first] = std::move(entry);
    }
}

ItemType& ItemType::get(const sp::string& name)
{
    return *items[name];
}
