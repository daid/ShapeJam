#include "miner.h"
#include "world.h"
#include "item.h"

#include <sp2/graphics/spriteAnimation.h>

Miner::Miner(sp::P<World> world, const ItemType* type)
: Building(world, type->size)
{
    mine_timer.repeat(5.0);

    setAnimation(sp::SpriteAnimation::load("building/" + type->name.lower() + ".txt"));
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

        const ItemType* mine_type = mine_tile->getMineType();
        if (mine_type)
        {
            Tile& tile = getTile(sp::Vector2i(0, 1));
            Tile& exit_tile = tile.getTile(direction);

            if (!tile.item && !exit_tile.item && (!exit_tile.building || exit_tile.building->accepts(mine_type, direction)))
            {
                sp::P<Item> item = new Item(&tile, mine_type);
                if (!item->requestMove(direction))
                    item.destroy();
            }
        }
    }
}
