#pragma once

#include "building.h"
#include "recipe.h"

#include <sp2/timer.h>

class Factory : public Building
{
public:
    Factory(sp::P<World> world, sp::Vector3d position, sp::Vector3d normal, const ItemType& type);

    virtual void onUpdate(float delta) override;
    virtual bool accepts(ItemType& type) override;

private:
    void idle();
    void building();
    void ejecting();

    int getInventoryCount(ItemType* type);
    int removeInventory(ItemType* type, int amount);

    std::vector<ItemType*> eject_list;
    class Inventory
    {
    public:
        ItemType* type;
        int amount;
        int maximum;
    };
    std::vector<Inventory> inventory;
    const Recipe* creating = nullptr;
    sp::Timer create_timer;
};
