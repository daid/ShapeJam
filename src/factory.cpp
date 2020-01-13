#include "factory.h"
#include "world.h"
#include "item.h"

#include <sp2/graphics/spriteAnimation.h>


Factory::Factory(sp::P<World> world, sp::Vector3d position, sp::Vector3d normal, const ItemType& type)
: Building(world, position, normal, type.size)
{
    setAnimation(sp::SpriteAnimation::load("building/" + type.name.lower() + ".txt"));
    animationPlay("IDLE");

    for(auto recipe : type.recipes)
    {
        for(auto& it : recipe->input)
        {
            bool done = false;
            for(auto& inv : inventory)
            {
                if (inv.type == &it.first)
                {
                    inv.maximum = std::max(inv.maximum, it.second);
                    done = true;
                }
            }
            if (!done)
            {
                inventory.push_back({.type = &it.first, .amount = 0, .maximum = it.second});
            }
        }
    }
}

void Factory::onUpdate(float delta)
{
    if (creating)
        building();
    else if (!eject_list.empty())
        ejecting();
    else
        idle();
}

void Factory::idle()
{
    for(int x=0; x<size.x; x++)
    {
        for(int y=0; y<size.y; y++)
        {
            Tile& tile = getTile(sp::Vector2i(x, y));
            if (tile.item && !tile.item->isMoving())
            {
                for(auto& inv : inventory)
                {
                    if (inv.type == &tile.item->getType() && inv.amount < inv.maximum)
                    {
                        inv.amount += 1;
                        tile.item.destroy();
                        break;
                    }
                }
            }
        }
    }

    if (selected_recipe != nullptr)
    {
        tryToMake(selected_recipe);
    }
    else
    {
        for(auto recipe : placed_from_type->recipes)
        {
            if (tryToMake(recipe))
                break;
        }
    }
}

bool Factory::tryToMake(const Recipe* recipe)
{
    bool input_satisfied = true;
    for(auto& input : recipe->input)
    {
        if (getInventoryCount(&input.first) < input.second)
        {
            input_satisfied = false;
            break;
        }
    }
    if (input_satisfied)
    {
        for(auto& input : recipe->input)
        {
            removeInventory(&input.first, input.second);
        }
        creating = recipe;
        animationPlay("ACTIVE");
        create_timer.start(recipe->craft_time);
        return true;
    }
    return false;
}

void Factory::building()
{
    if (create_timer.isExpired())
    {
        animationPlay("IDLE");
        for(auto& output : creating->output)
            eject_list.resize(eject_list.size() + output.second, &output.first);
        creating = nullptr;
    }
}

void Factory::ejecting()
{
    Tile& exit_tile = getTile(sp::Vector2i(0, size.y - 1)).getTile(direction);

    if (!exit_tile.item)
    {
        Item* item = new Item(&exit_tile, *eject_list.back());
        item->fakeMoveFrom(direction);
        eject_list.pop_back();
    }
}

bool Factory::accepts(ItemType& type)
{
    if (creating || eject_list.size() > 0)
        return false;
    for(auto& inv : inventory)
    {
        if (inv.type == &type && inv.amount < inv.maximum)
            return true;
    }
    return false;
}

int Factory::getInventoryCount(const ItemType* type)
{
    for(auto& inv : inventory)
        if (inv.type == type)
            return inv.amount;
    return 0;
}

int Factory::removeInventory(const ItemType* type, int amount)
{
    for(auto& inv : inventory)
    {
        if (inv.type == type)
        {
            amount = std::min(inv.amount, amount);
            inv.amount -= amount;
            return amount;
        }
    }
    return 0;
}

void Factory::setRecipe(const Recipe* recipe)
{
    selected_recipe = nullptr;
    for(auto r : placed_from_type->recipes)
    {
        if (r == recipe)
            selected_recipe = r;
    }
}
