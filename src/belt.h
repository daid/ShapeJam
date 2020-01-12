#pragma once

#include "building.h"

class Belt : public Building
{
public:
    Belt(sp::P<World> world, sp::Vector3d position, sp::Vector3d normal);

    virtual void onUpdate(float delta) override;
    virtual bool accepts(ItemType& type) override { return true; }
};