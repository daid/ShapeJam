#include "bridge.h"
#include "item.h"

#include <sp2/assert.h>
#include <sp2/graphics/spriteAnimation.h>


Bridge::Bridge(sp::P<World> world)
: Building(world, sp::Vector2i(1, 1))
{
    visual = new sp::Node(this);
    visual->setAnimation(sp::SpriteAnimation::load("building/belt.txt"));
    visual->animationPlay("BRIDGE");
    visual->setPosition(sp::Vector3d(0, 0, 0.5));
    visual->setRotation(sp::Quaterniond::fromAxisAngle(sp::Vector3d(1, 0, 0), 45));
    visual->render_data.scale.y = 1.4;
}

Bridge::~Bridge()
{
    disconnect();
}

void Bridge::userRotate()
{
    disconnect();
    Building::userRotate();
    tryToConnect();
}

bool Bridge::accepts(const ItemType* type, Direction direction)
{
    if (entrance && this->direction == direction)
        return true;
    return false;
}

void Bridge::onUpdate(float delta)
{
    if (init)
    {
        tryToConnect();
        init = false;
    }

    if (other_side && corner_tile->item && !corner_tile->item->isMoving())
    {
        if (entrance)
        {
            corner_tile->item->requestMoveUpBridge(direction);
        }
        else if (corner_tile->getTile(direction).building)
        {
            corner_tile->item->requestMove(direction);
        }
    }
}

void Bridge::tryToConnect()
{
    Tile* t = corner_tile;
    for(int n=0; n<max_length; n++)
    {
        t = &t->getTile(direction);
        if (&t->side != &corner_tile->side || t->bridge)
            break;
        sp::P<Bridge> other = t->building;
        if (other)
        {
            if (other->direction == direction && !other->other_side)
            {
                connect(other);
                return;
            }
            break;
        }
        if (t->building && !t->building->canBeBridged())
            break;
    }
    t = corner_tile;
    for(int n=0; n<max_length; n++)
    {
        t = &t->getTile(flipDirection(direction));
        if (&t->side != &corner_tile->side || t->bridge)
            break;
        sp::P<Bridge> other = t->building;
        if (other)
        {
            if (other->direction == direction && !other->other_side)
                other->connect(this);
            return;
        }
        if (t->building && !t->building->canBeBridged())
            break;
    }
}

void Bridge::connect(sp::P<Bridge> other)
{
    other_side = other;
    other->other_side = this;
    entrance = true;
    other_side->entrance = false;
    other_side->visual->setRotation(sp::Quaterniond::fromAxisAngle(sp::Vector3d(1, 0, 0), -45));

    Tile* t = corner_tile;
    for(int n=0; n<max_length; n++)
    {
        t = &t->getTile(direction);
        if (t->building == other_side)
            break;
        new BridgeNode(this, t);
        t->bridge->setRotation(getRotation3D());
    }
}

void Bridge::disconnect()
{
    if (!other_side)
        return;
    if (!entrance)
    {
        other_side->disconnect();
        return;
    }
    other_side->visual->setRotation(sp::Quaterniond::fromAxisAngle(sp::Vector3d(1, 0, 0), 45));
    other_side->entrance = true;

    Tile* t = corner_tile;
    for(int n=0; n<max_length; n++)
    {
        t = &t->getTile(direction);
        if (t->building == other_side)
            break;
        t->bridge.destroy();
    }
}

BridgeNode::BridgeNode(sp::P<Bridge> owner, Tile* tile)
: sp::Node(owner->getParent()), owner(owner)
{
    sp2assert(!tile->bridge, "Double bridge node...");
    tile->bridge = this;

    setAnimation(sp::SpriteAnimation::load("building/belt.txt"));
    animationPlay("BRIDGE");
    setPosition(tile->position + tile->side.up);

    render_data.type = sp::RenderData::Type::Transparent;
}

BridgeNode::~BridgeNode()
{
    item.destroy();
}

void BridgeNode::onUpdate(float delta)
{
    if (item && !item->isMoving())
    {
        item->requestMoveUpBridge(owner->getDirection());
    }
}

void Bridge::save(sp::io::serialization::DataSet& data) const
{
    Building::save(data);
    data.set("entrance", entrance);
    data.set("other_side", other_side);
}

void Bridge::load(const sp::io::serialization::DataSet& data)
{
    Building::load(data);
    init = false;
    entrance = data.get<int>("entrance");
    other_side = data.getObject("other_side");
    if (!entrance && other_side)
    {
        visual->setRotation(sp::Quaterniond::fromAxisAngle(sp::Vector3d(1, 0, 0), -45));
    }
}

sp::AutoPointerObject* Bridge::create(const sp::io::serialization::DataSet& data)
{
    return new Bridge(data.getObject("world"));
}
