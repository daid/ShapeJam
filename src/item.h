#pragma once

#include <sp2/scene/node.h>
#include <sp2/timer.h>
#include "itemType.h"
#include "world.h"

class Item : public sp::Node
{
public:
    Item(Tile* tile, ItemType& type);

    void fakeMoveFrom(Direction direction);
    bool requestMove(Direction direction);
    bool isMoving();

    ItemType& getType() { return type; }

    virtual void onUpdate(float delta) override;
private:
    ItemType& type;
    Tile* tile;
    sp::Timer move_timer;
    sp::Vector3d old_position;
    sp::Vector3d new_position;
};