#include "miner.h"
#include "world.h"
#include "item.h"
#include "stats.h"

#include <sp2/graphics/spriteAnimation.h>
#include <sp2/audio/sound.h>

Miner::Miner(sp::P<World> world, const ItemType* type)
: Building(world, type->size)
{
    mine_timer.repeat(5.0);

    setAnimation(sp::SpriteAnimation::load("building/" + type->name.lower() + ".txt"));
    animationPlay("ACTIVE");
}

void Miner::onUpdate(float delta)
{
    if (mine_timer.isExpired())
    {
        Tile* mine_tile = corner_tile;
        if (mine_spot & 1)
            mine_tile = &mine_tile->getTile(Direction::Forward);
        if (mine_spot & 2)
            mine_tile = &mine_tile->getTile(Direction::Right);
        mine_spot = (mine_spot + 1) % 4;

        const ItemType* mine_type = mine_tile->getMineType();
        if (mine_type)
        {
            Tile& exit_tile = getTile(sp::Vector2i(0, 1)).getTile(direction);

            if (!exit_tile.item && (!exit_tile.building || exit_tile.building->accepts(mine_type, direction)))
            {
                sp::P<Item> item = new Item(&exit_tile, mine_type);
                item->fakeMoveFrom(direction);
                sp::audio::Sound::play("craft.wav");
                corner_tile->side.world.stats->add(mine_type, 1);
            }
        }
    }
}

void Miner::save(sp::io::serialization::DataSet& data) const
{
    Building::save(data);
    data.set("mine_spot", mine_spot);
    data.set("mine_timer", mine_timer.getProgress());
}

void Miner::load(const sp::io::serialization::DataSet& data)
{
    Building::load(data);
    mine_spot = data.get<int>("mine_spot");
    mine_timer.setProgress(data.get<float>("mine_timer"));
}

sp::AutoPointerObject* Miner::create(const sp::io::serialization::DataSet& data)
{
    return new Miner(data.getObject("world"), ItemType::get(data.get<sp::string>("type")));
}
