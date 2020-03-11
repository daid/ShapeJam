#include "storage.h"
#include "world.h"
#include "item.h"

#include <sp2/graphics/spriteAnimation.h>
#include <sp2/audio/sound.h>

Storage::Storage(sp::P<World> world, const ItemType* type)
: Building(world, type->size)
{
    setAnimation(sp::SpriteAnimation::load("building/" + type->name.lower() + ".txt"));
    animationPlay("IDLE");
}

void Storage::onUpdate(float delta)
{
    if (inventory_count < 1000)
    {
        for(int x=0; x<size.x; x++)
        {
            for(int y=0; y<size.y; y++)
            {
                Tile& tile = getTile(sp::Vector2i(x, y));
                if (tile.item && !tile.item->isMoving())
                {
                    if (tile.item->getType() == inventory_type || inventory_type == nullptr)
                    {
                        inventory_count += 1;
                        tile.item.destroy();
                    }
                }
            }
        }
    }
}

bool Storage::accepts(const ItemType* type, Direction direction)
{
    if ((type == inventory_type || inventory_type == nullptr) && inventory_count < 1000)
    {
        return true;
    }
    return false;
}
