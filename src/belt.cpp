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

void Belt::save(sp::io::serialization::DataSet& data) const
{
    Building::save(data);
}

void Belt::load(const sp::io::serialization::DataSet& data)
{
    Building::load(data);
}

sp::AutoPointerObject* Belt::create(const sp::io::serialization::DataSet& data)
{
    return new Belt(data.getObject("world"));
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
            case Direction::Forward: d = Direction::Right; break;
            case Direction::Right: d = Direction::Backward; break;
            case Direction::Backward: d = Direction::Left; break;
            case Direction::Left: d = Direction::Forward; break;
            }
        }
        if (corner_tile->getTile(d).building)
            corner_tile->item->requestMove(d);
        flip = !flip;
    }
}

void Splitter::save(sp::io::serialization::DataSet& data) const
{
    Building::save(data);
    data.set("flip", flip);
}

void Splitter::load(const sp::io::serialization::DataSet& data)
{
    Building::load(data);
    flip = data.get<int>("flip");
}

sp::AutoPointerObject* Splitter::create(const sp::io::serialization::DataSet& data)
{
    return new Splitter(data.getObject("world"));
}
