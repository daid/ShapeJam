#include "item.h"
#include "building.h"

#include <sp2/assert.h>
#include <sp2/tween.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>

Item::Item(Tile* tile, ItemType& type)
: sp::Node(&tile->side.world), type(type), tile(tile)
{
    sp2assert(!tile->item, "Cannot spawn 2 items on same tile.");
    tile->item = this;
    render_data.type = sp::RenderData::Type::Normal;
    render_data.shader = sp::Shader::get("shader/billboard.shader");
    render_data.mesh = sp::MeshData::createQuad(sp::Vector2f(1, 1));
    render_data.texture = sp::texture_manager.get(type.texture);

    setPosition(tile->position + tile->side.up * 0.5);
}

bool Item::isMoving()
{
    return move_timer.isRunning();
}

void Item::fakeMoveFrom(Direction direction)
{
    Tile* t = nullptr;
    switch(direction)
    {
    case Direction::Forward: t = &tile->getTile(Direction::Backward); break;
    case Direction::Backward: t = &tile->getTile(Direction::Forward); break;
    case Direction::Right: t = &tile->getTile(Direction::Left); break;
    case Direction::Left: t = &tile->getTile(Direction::Right); break;
    }
    move_timer.start(0.25);
    old_position = t->position + t->side.up * 0.5;
    new_position = tile->position + tile->side.up * 0.5;
}

bool Item::requestMove(Direction direction)
{
    if (move_timer.isRunning())
        return false;
    Tile& t = tile->getTile(direction);

    if (t.item)
        return false;
    if (t.building && !t.building->accepts(type))
        return false;

    tile->item = nullptr;
    tile = &t;
    tile->item = this;

    move_timer.start(0.25);
    old_position = getPosition3D();
    new_position = tile->position + tile->side.up * 0.5;
    return true;
}

void Item::onUpdate(float delta)
{
    if (move_timer.isRunning())
    {
        if (move_timer.isExpired())
            setPosition(new_position);
        else
            setPosition(sp::Tween<sp::Vector3d>::linear(move_timer.getProgress(), 0, 1, old_position, new_position));
    }
}
