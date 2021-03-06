#pragma once

#include "building.h"
#include "recipe.h"

#include <sp2/timer.h>

class Factory : public Building
{
public:
    Factory(sp::P<World> world, const ItemType* type);

    virtual void onUpdate(float delta) override;
    virtual bool accepts(const ItemType* type, Direction direction) override;

    virtual void setRecipe(const Recipe* recipe) override;
    virtual const Recipe* getRecipe() override { return selected_recipe; }

    void save(sp::io::serialization::DataSet& data) const;
    void load(const sp::io::serialization::DataSet& data);
    static sp::AutoPointerObject* create(const sp::io::serialization::DataSet& data);
private:
    void idle();
    void building();
    void ejecting();

    bool tryToMake(const Recipe* recipe);

    int getInventoryCount(const ItemType* type) override;
    int removeInventory(const ItemType* type, int amount);

    std::vector<const ItemType*> eject_list;
    class Inventory
    {
    public:
        const ItemType* type;
        int amount;
        int maximum;
    };
    std::vector<Inventory> inventory;
    const Recipe* creating = nullptr;
    const Recipe* selected_recipe = nullptr;
    sp::Timer create_timer;
};
