#include "miner.h"
#include "world.h"
#include "item.h"

#include <sp2/graphics/spriteAnimation.h>

Miner::Miner(sp::P<World> world, sp::Vector3d position, sp::Vector3d normal, const ItemType& type)
: Building(world, position, normal, type.size)
{
    mine_timer.repeat(5.0);

    setAnimation(sp::SpriteAnimation::load("building/" + type.name.lower() + ".txt"));
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

        ItemType* mine_type = mine_tile->getMineType();
        if (mine_type)
        {
            Tile& tile = getTile(sp::Vector2i(0, 1));
            Tile& exit_tile = tile.getTile(direction);

            if (!tile.item && !exit_tile.item && (!exit_tile.building || exit_tile.building->accepts(*mine_type)))
            {
                sp::P<Item> item = new Item(&tile, *mine_type);
                if (!item->requestMove(direction))
                    item.destroy();
            }
        }
    }
}
