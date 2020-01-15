#pragma once

#include <sp2/scene/node.h>
#include <sp2/timer.h>
#include "itemType.h"
#include "world.h"

class Item : public sp::Node
{
public:
    Item(Tile* tile, const ItemType* type);

    void fakeMoveFrom(Direction direction);
    bool requestMove(Direction direction);
    bool requestMoveUpBridge(Direction direction);
    bool isMoving();

    const ItemType* getType() { return type; }

    virtual void onUpdate(float delta) override;
private:
    const ItemType* type;
    Tile* tile;
    sp::Timer move_timer;
    sp::Vector3d old_position;
    sp::Vector3d new_position;
    bool on_bridge = false;
};
