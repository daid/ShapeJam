#pragma once

#include "building.h"

class Bridge : public Building
{
public:
    Bridge(sp::P<World> world);
    virtual ~Bridge();

    virtual void onUpdate(float delta) override;
    virtual bool accepts(const ItemType* type, Direction direction) override;
    virtual void userRotate() override;
private:
    void tryToConnect();
    void disconnect();

    void connect(sp::P<Bridge> other);

    sp::P<sp::Node> visual;
    sp::P<Bridge> other_side;
    bool entrance = true;

    bool init = true;

    static constexpr int max_length = 4;
};

class BridgeNode : public sp::Node
{
public:
    BridgeNode(sp::P<Bridge> owner, Tile* tile);
    virtual ~BridgeNode();

    virtual void onUpdate(float delta) override;

    sp::P<Item> item;
    sp::P<Bridge> owner;
};
