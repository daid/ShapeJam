#include "miner.h"
#include "world.h"
#include "item.h"

#include <sp2/graphics/spriteAnimation.h>

Miner::Miner(sp::P<World> world, sp::Vector3d position, sp::Vector3d normal)
: Building(world, position, normal, sp::Vector2i(2, 2))
{
    mine_timer.repeat(5.0);

    setAnimation(sp::SpriteAnimation::load("building/miner.txt"));
    animationPlay("ACTIVE");
}

void Miner::onUpdate(float delta)
{
    if (mine_timer.isExpired())
    {
        Tile* mine_tile = corner_tile;
        if (mine_spot & 1)
            mine_tile = &mine_tile->getTile(Direction::Forward);
        if (mine_spot & 2)
            mine_tile = &mine_tile->getTile(Direction::Right);
        mine_spot = (mine_spot + 1) % 4;

        if (mine_tile->ground_type)
        {
            Tile& tile = getTile(sp::Vector2i(0, 1));
            Tile& exit_tile = tile.getTile(direction);

            if (!tile.item && !exit_tile.item)
            {
                ItemType& type = ItemType::get(sp::string(mine_tile->ground_type));
                Item* item = new Item(&tile, type);
                item->requestMove(direction);
            }
        }
    }
}
