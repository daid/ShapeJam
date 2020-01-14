#include "belt.h"
#include "world.h"
#include "item.h"

#include <sp2/graphics/spriteAnimation.h>


Belt::Belt(sp::P<World> world)
: Building(world, sp::Vector2i(1, 1))
{
    setAnimation(sp::SpriteAnimation::load("building/belt.txt"));
    animationPlay("BELT");
}

void Belt::onUpdate(float delta)
{
    if (corner_tile->item && !corner_tile->item->isMoving() && corner_tile->getTile(direction).building)
    {
        corner_tile->item->requestMove(direction);
    }
}

Splitter::Splitter(sp::P<World> world)
: Building(world, sp::Vector2i(1, 1))
{
    setAnimation(sp::SpriteAnimation::load("building/belt.txt"));
    animationPlay("SPLITTER");
}

void Splitter::onUpdate(float delta)
{
    if (corner_tile->item && !corner_tile->item->isMoving())
    {
        Direction d = direction;
        if (flip)
        {
            switch(d)
            {
            case Direction::Forward: d = Direction::Backward; break;
            case Direction::Backward: d = Direction::Forward; break;
            case Direction::Right: d = Direction::Left; break;
            case Direction::Left: d = Direction::Right; break;
            }
        }
        if (corner_tile->getTile(d).building)
            corner_tile->item->requestMove(d);
        flip = !flip;
    }
}
