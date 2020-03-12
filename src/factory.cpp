#include "factory.h"
#include "world.h"
#include "item.h"
#include "stats.h"

#include <sp2/graphics/spriteAnimation.h>
#include <sp2/audio/sound.h>


Factory::Factory(sp::P<World> world, const ItemType* type)
: Building(world, type->size)
{
    setAnimation(sp::SpriteAnimation::load("building/" + type->name.lower() + ".txt"));
    animationPlay("IDLE");

    for(auto recipe : type->recipes)
    {
        for(auto& it : recipe->input)
        {
            bool done = false;
            for(auto& inv : inventory)
            {
                if (inv.type == it.first)
                {
                    inv.maximum = std::max(inv.maximum, it.second);
                    done = true;
                }
            }
            if (!done)
            {
                inventory.push_back({.type = it.first, .amount = 0, .maximum = it.second});
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
                    if (inv.type == tile.item->getType() && inv.amount < inv.maximum)
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
        if (getInventoryCount(input.first) < input.second)
        {
            input_satisfied = false;
            break;
        }
    }
    if (input_satisfied)
    {
        for(auto& input : recipe->input)
        {
            removeInventory(input.first, input.second);
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
            eject_list.resize(eject_list.size() + output.second, output.first);
        creating = nullptr;
    }
}

void Factory::ejecting()
{
    Tile& exit_tile = getTile(sp::Vector2i(0, size.y - 1)).getTile(direction);

    if (!exit_tile.item)
    {
        Item* item = new Item(&exit_tile, eject_list.back());
        corner_tile->side.world.stats->add(eject_list.back(), 1);
        item->fakeMoveFrom(direction);
        sp::audio::Sound::play("craft.wav");
        eject_list.pop_back();
    }
}

bool Factory::accepts(const ItemType* type, Direction direction)
{
    if (creating || eject_list.size() > 0)
        return false;
    for(auto& inv : inventory)
    {
        if (inv.type == type && inv.amount < inv.maximum)
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

void Factory::save(sp::io::serialization::DataSet& data) const
{
    Building::save(data);
    if (eject_list.size())
    {
        auto list = data.createList("eject_list");
        for(auto type : eject_list)
        {
            auto edata = list.next();
            edata.set("type", type->name);
        }
    }
    auto list = data.createList("inventory");
    for(auto& inv : inventory)
    {
        if (inv.amount > 0)
        {
            auto idata = list.next();
            idata.set("amount", inv.amount);
            idata.set("type", inv.type->name);
        }
    }
    if (creating)
    {
        data.set("creating", creating->name);
        data.set("create_timer", create_timer.getProgress());
    }
    if (selected_recipe)
    {
        data.set("selected_recipe", selected_recipe->name);
    }
}

void Factory::load(const sp::io::serialization::DataSet& data)
{
    Building::load(data);
    for(const auto& edata : data.getList("eject_list"))
    {
        eject_list.push_back(ItemType::get(edata.get<sp::string>("type")));
    }
    for(const auto& idata : data.getList("inventory"))
    {
        const ItemType* type = ItemType::get(idata.get<sp::string>("type"));
        for(auto& inv : inventory)
        {
            if (inv.type == type)
                inv.amount = idata.get<int>("amount");
        }
    }
    if (data.has("creating"))
    {
        creating = Recipe::get(data.get<sp::string>("creating"));
        create_timer.start(creating->craft_time);
        create_timer.setProgress(data.get<float>("create_timer"));
    }
    if (data.has("selected_recipe"))
    {
        selected_recipe = Recipe::get(data.get<sp::string>("selected_recipe"));
    }
}

sp::AutoPointerObject* Factory::create(const sp::io::serialization::DataSet& data)
{
    return new Factory(data.getObject("world"), ItemType::get(data.get<sp::string>("type")));
}
