#include "belt.h"
#include "world.h"
#include "item.h"

#include <sp2/graphics/spriteAnimation.h>


Belt::Belt(sp::P<World> world, sp::Vector3d position, sp::Vector3d normal)
: Building(world, position, normal, sp::Vector2i(1, 1))
{
    setAnimation(sp::SpriteAnimation::load("building/belt.txt"));
    animationPlay("BELT");
}

void Belt::onUpdate(float delta)
{
    if (corner_tile->item)
    {
        corner_tile->item->requestMove(direction);
    }
}
