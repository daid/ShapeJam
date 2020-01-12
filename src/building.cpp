#include "building.h"
#include "world.h"

#include <sp2/graphics/meshdata.h>
#include <sp2/assert.h>

Building::Building(sp::P<World> world, sp::Vector3d position, sp::Vector3d normal, sp::Vector2i size)
: sp::Node(world), size(size)
{
    WorldSide& side = world->getSideAt(normal);
    position -= side.right * ((size.x - 1) * 0.5);
    position -= side.forward * ((size.y - 1) * 0.5);
    Tile& tile = side.getTileAt(position);
    corner_tile = &tile;

    for(int x=0; x<size.x; x++)
    {
        for(int y=0; y<size.y; y++)
        {
            sp::Vector3d p = tile.position + side.right * double(x) + side.forward * double(y);
            Tile& t = side.getTileAt(p);
            if (t.building || (p - t.position).length() > 0.7)
                return;
            //TODO: Placing a building on an item is generally not proper, but could be acceptable for belts
            if (t.item)
                return;
        }
    }
    for(int x=0; x<size.x; x++)
    {
        for(int y=0; y<size.y; y++)
        {
            Tile& t = side.getTileAt(tile.position + side.right * double(x) + side.forward * double(y));
            t.building = this;
        }
    }

    setPosition(tile.position + side.up * 0.1 + side.right * ((size.x - 1) * 0.5) + side.forward * ((size.y - 1) * 0.5));
    setRotation(side.rotation);

    render_data.type = sp::RenderData::Type::Normal;
    render_data.shader = sp::Shader::get("internal:basic_shaded.shader");
    render_data.mesh = sp::MeshData::createQuad(sp::Vector2f(size));
    placed = true;
}

Building::~Building()
{
}

Tile& Building::getTile(sp::Vector2i offset)
{
    switch(direction)
    {
    case Direction::Forward: break;
    case Direction::Backward: offset = sp::Vector2i(size.x - 1 - offset.x, size.y - 1 - offset.y); break;
    case Direction::Right: offset = sp::Vector2i(offset.y, size.y - 1 - offset.x); break;
    case Direction::Left: offset = sp::Vector2i(size.x - 1 - offset.y, offset.x); break;
    }
    sp2assert(offset.x >= 0 && offset.x < size.x, "Building::getTile needs so stay within building size");
    sp2assert(offset.y >= 0 && offset.y < size.y, "Building::getTile needs so stay within building size");
    Tile* t = corner_tile;
    while(offset.x > 0)
    {
        t = &t->getTile(Direction::Right);
        offset.x--;
    }
    while(offset.y > 0)
    {
        t = &t->getTile(Direction::Forward);
        offset.y--;
    }
    return *t;
}

void Building::userRotate()
{
    switch(direction)
    {
    case Direction::Forward: setDirection(Direction::Right); break;
    case Direction::Backward: setDirection(Direction::Left); break;
    case Direction::Right: setDirection(Direction::Backward); break;
    case Direction::Left: setDirection(Direction::Forward); break;
    }
}

void Building::setDirection(Direction dir)
{
    direction = dir;
    switch(direction)
    {
    case Direction::Forward: setRotation(corner_tile->side.rotation); break;
    case Direction::Backward: setRotation(sp::Quaterniond::fromAngle(180) * corner_tile->side.rotation); break;
    case Direction::Right: setRotation(sp::Quaterniond::fromAngle(270) * corner_tile->side.rotation); break;
    case Direction::Left: setRotation(sp::Quaterniond::fromAngle(90) * corner_tile->side.rotation); break;
    }
}