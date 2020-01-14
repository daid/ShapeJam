#pragma once

#include "building.h"

class Belt : public Building
{
public:
    Belt(sp::P<World> world);

    virtual void onUpdate(float delta) override;
    virtual bool accepts(ItemType& type, Direction direction) override { return true; }
    virtual bool canBeBridged() override { return true; }
};
class Splitter : public Building
{
public:
    Splitter(sp::P<World> world);

    virtual void onUpdate(float delta) override;
    virtual bool accepts(ItemType& type, Direction direction) override { return true; }
    virtual bool canBeBridged() override { return true; }
private:
    bool flip = false;
};