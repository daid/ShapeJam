#pragma once

#include <sp2/scene/node.h>
#include <sp2/scene/voxelmap.h>
#include <sp2/io/serialization/dataset.h>
#include <array>

enum class Direction
{
    Forward,
    Backward,
    Left,
    Right
};
Direction flipDirection(Direction d);

class Building;
class BridgeNode;
class Item;
class ItemType;
class World;
class WorldSide;
class StatsCollector;
class Tile
{
public:
    Tile(WorldSide& side, sp::Vector3d position)
    : side(side), position(position)
    {
    }

    WorldSide& side;
    sp::Vector3d position;
    int ground_type;
    sp::P<Building> building;
    sp::P<BridgeNode> bridge;
    sp::P<Item> item;

    Tile& getTile(Direction direction);
    const ItemType* getMineType();
};
class WorldSide
{
public:
    WorldSide(World& world, int size, sp::Quaterniond rotation);

    Tile& getTileAt(sp::Vector3d position);

    World& world;
    std::vector<Tile> tiles;
    sp::Vector3d up;
    sp::Vector3d forward;
    sp::Vector3d right;
    sp::Quaterniond rotation;
    int size;
};

class World : public sp::Node
{
public:
    World(sp::P<sp::Node> parent);

    WorldSide& getSideAt(sp::Vector3d normal);

    Tile& getTileAt(sp::Vector3d position, sp::Vector3d normal);

    static constexpr int size=16;
    sp::P<StatsCollector> stats;

    void save(sp::io::serialization::DataSet& data) const;
    void load(const sp::io::serialization::DataSet& data);
    static sp::AutoPointerObject* create(const sp::io::serialization::DataSet& data);
private:
    sp::P<sp::Voxelmap> voxels;
    std::array<WorldSide, 6> sides{{
        {*this, size, sp::Quaterniond::fromAxisAngle(sp::Vector3d(1, 0, 0), 0)},
        {*this, size, sp::Quaterniond::fromAxisAngle(sp::Vector3d(1, 0, 0), 180)},
        {*this, size, sp::Quaterniond::fromAxisAngle(sp::Vector3d(0, 1, 0), 90)},
        {*this, size, sp::Quaterniond::fromAxisAngle(sp::Vector3d(0, 1, 0), 270)},
        {*this, size, sp::Quaterniond::fromAxisAngle(sp::Vector3d(1, 0, 0), 270)},
        {*this, size, sp::Quaterniond::fromAxisAngle(sp::Vector3d(1, 0, 0), 90)},
    }};
};
