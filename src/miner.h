#pragma once

#include "building.h"
#include <sp2/timer.h>

class ItemType;
class Miner : public Building
{
public:
    Miner(sp::P<World> world, const ItemType* type);

    virtual void onUpdate(float delta) override;

    void save(sp::io::serialization::DataSet& data) const;
    void load(const sp::io::serialization::DataSet& data);
    static sp::AutoPointerObject* create(const sp::io::serialization::DataSet& data);
private:
    int mine_spot = 0;
    sp::Timer mine_timer;
};
