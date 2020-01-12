#pragma once

#include "building.h"
#include <sp2/timer.h>

class Miner : public Building
{
public:
    Miner(sp::P<World> world, sp::Vector3d position, sp::Vector3d normal);

    virtual void onUpdate(float delta) override;

private:
    int mine_spot = 0;
    sp::Timer mine_timer;
};