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
    Building(sp::P<World> world, sp::Vector3d position, sp::Vector3d normal, sp::Vector2i size);
    virtual ~Building();

    bool isPlaced() { return placed; }
    virtual bool accepts(ItemType& type) { return false; }

    void userRotate();
    void setDirection(Direction dir);
    Direction getDirection() { return direction; }

    ItemType* placed_from_type;
protected:
    Tile& getTile(sp::Vector2i offset);

    Tile* corner_tile;
    Direction direction = Direction::Forward;
    sp::Vector2i size;
private:
    bool placed = false;
};