#pragma once

#include <sp2/scene/node.h>
#include "world.h"
#include "itemType.h"


class Tile;
class World;
class Item;
class Building : public sp::Node
{
public:
    Building(sp::P<World> world, sp::Vector2i size);
    virtual ~Building();

    sp::Vector2i getSize() { return size; }

    bool placeAt(Tile* tile);
    virtual bool accepts(const ItemType* type, Direction direction) { return false; }
    virtual bool canBeBridged() { return true; }

    virtual void userRotate();
    void setDirection(Direction dir);
    Direction getDirection() { return direction; }

    virtual void setRecipe(const Recipe* recipe) {}
    virtual const Recipe* getRecipe() { return nullptr; }

    virtual int getInventoryCount(const ItemType* type) { return 0; }

    const ItemType* placed_from_type;
protected:
    Tile& getTile(sp::Vector2i offset);

    void save(sp::io::serialization::DataSet& data) const;
    void load(const sp::io::serialization::DataSet& data);

    Tile* corner_tile = nullptr;
    Direction direction = Direction::Forward;
    sp::Vector2i size;
};
