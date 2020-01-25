#include "item.h"
#include "building.h"
#include "bridge.h"

#include <sp2/assert.h>
#include <sp2/tween.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>

Item::Item(Tile* tile, const ItemType* type)
: sp::Node(&tile->side.world), type(type), tile(tile)
{
    sp2assert(!tile->item, "Cannot spawn 2 items on same tile.");
    tile->item = this;
    render_data.type = sp::RenderData::Type::Normal;
    render_data.shader = sp::Shader::get("shader/billboard.shader");
    render_data.mesh = sp::MeshData::createQuad(sp::Vector2f(1, 1));
    render_data.texture = sp::texture_manager.get(type->texture);

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
    if (t.building && !t.building->accepts(type, direction))
        return false;

    if (on_bridge)
        tile->bridge->item = nullptr;
    else
        tile->item = nullptr;
    tile = &t;
    tile->item = this;

    move_timer.start(0.25);
    old_position = getPosition3D();
    new_position = tile->position + tile->side.up * 0.5;
    return true;
}

bool Item::requestMoveUpBridge(Direction direction)
{
    if (move_timer.isRunning())
        return false;
    Tile& t = tile->getTile(direction);

    if (t.bridge && t.bridge->item)
        return false;
    if (!t.bridge && t.item)
        return false;

    if (on_bridge)
        tile->bridge->item = nullptr;
    else
        tile->item = nullptr;
    tile = &t;
    if (tile->bridge)
    {
        tile->bridge->item = this;
        on_bridge = true;
        new_position = tile->position + tile->side.up * 1.5;
    }
    else
    {
        tile->item = this;
        on_bridge = false;
        new_position = tile->position + tile->side.up * 0.5;
    }

    move_timer.start(0.25);
    old_position = getPosition3D();
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

void Item::save(sp::io::serialization::DataSet& data) const
{
    data.set("type", type->name);
    data.set("world", &tile->side.world);
    data.set("tile_normal", tile->side.up);
    data.set("tile_position", tile->position);

    if (move_timer.isRunning())
    {
        data.set("move_timer", move_timer.getProgress());
        data.set("old_position", old_position);
        data.set("new_position", new_position);
    }
}

void Item::load(const sp::io::serialization::DataSet& data)
{
    if (data.has("move_timer"))
    {
        move_timer.start(0.25);
        //TODO: set timer progress.
        old_position = data.get<sp::Vector3d>("old_position");
        new_position = data.get<sp::Vector3d>("new_position");
    }
}

sp::AutoPointerObject* Item::create(const sp::io::serialization::DataSet& data)
{
    sp::P<World> world = data.getObject("world");
    Tile& tile = world->getTileAt(data.get<sp::Vector3d>("tile_position"), data.get<sp::Vector3d>("tile_normal"));
    return new Item(&tile, ItemType::get(data.get<sp::string>("type")));
}
