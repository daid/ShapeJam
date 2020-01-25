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
    data.set("world", getParent());
    data.set("corner_tile_position", corner_tile->position);
    data.set("corner_tile_normal", corner_tile->side.up);
    data.set("direction", direction);
    data.set("type", placed_from_type->name);
}

void Belt::load(const sp::io::serialization::DataSet& data)
{
    sp::P<World> world = getParent();
    placeAt(&world->getTileAt(data.get<sp::Vector3d>("corner_tile_position"), data.get<sp::Vector3d>("corner_tile_normal")));
    setDirection(data.get<Direction>("direction"));
    placed_from_type = ItemType::get(data.get<sp::string>("type"));
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
    data.set("world", getParent());
    data.set("corner_tile_position", corner_tile->position);
    data.set("corner_tile_normal", corner_tile->side.up);
    data.set("direction", direction);
    data.set("type", placed_from_type->name);
    data.set("flip", flip);
}

void Splitter::load(const sp::io::serialization::DataSet& data)
{
    sp::P<World> world = getParent();
    placeAt(&world->getTileAt(data.get<sp::Vector3d>("corner_tile_position"), data.get<sp::Vector3d>("corner_tile_normal")));
    setDirection(data.get<Direction>("direction"));
    placed_from_type = ItemType::get(data.get<sp::string>("type"));
    flip = data.get<int>("flip");
}

sp::AutoPointerObject* Splitter::create(const sp::io::serialization::DataSet& data)
{
    return new Splitter(data.getObject("world"));
}
