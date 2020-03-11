#pragma once

#include "building.h"
#include "recipe.h"

class Storage : public Building
{
public:
    Storage(sp::P<World> world, const ItemType* type);

    virtual void onUpdate(float delta) override;
    virtual bool accepts(const ItemType* type, Direction direction) override;

private:
    const ItemType* inventory_type = nullptr;
    int inventory_count = 0;
};
