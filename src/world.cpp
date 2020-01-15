#include "world.h"
#include "itemType.h"

#include <sp2/scene/voxelmap.h>
#include <sp2/collision/3d/box.h>

Tile& Tile::getTile(Direction direction)
{
    sp::Vector3d offset;
    switch(direction)
    {
    case Direction::Forward: offset = side.forward; break;
    case Direction::Backward: offset = -side.forward; break;
    case Direction::Right: offset = side.right; break;
    case Direction::Left: offset = -side.right; break;
    }
    Tile& t = side.getTileAt(position + offset);
    if (&t == this)
        return side.world.getSideAt(offset).getTileAt(position + offset);
    return t;
}

const ItemType* Tile::getMineType()
{
    if (ground_type == 1)
        return ItemType::get("1");
    if (ground_type == 2)
        return ItemType::get("2");
    if (ground_type == 3)
        return ItemType::get("3");
    return nullptr;
}

WorldSide::WorldSide(World& world, int size, sp::Quaterniond rotation)
: world(world), rotation(rotation), size(size)
{
    tiles.reserve(size * size);
    up = rotation * sp::Vector3d(0, 0, 1);
    forward = rotation * sp::Vector3d(0, 1, 0);
    right = rotation * sp::Vector3d(1, 0, 0);
    sp::Vector3d center = up * (size * 0.5);
    for(int y=0; y<size; y++)
    {
        double py = double(y) - double(size) * 0.5 + 0.5;
        for(int x=0; x<size; x++)
        {
            double px = double(x) - double(size) * 0.5 + 0.5;
            tiles.push_back({
                .side = *this,
                .position = center + right * px + forward * py,
            });
        }
    }
}

Tile& WorldSide::getTileAt(sp::Vector3d position)
{
    int x = position.dot(right) + size * 0.5;
    int y = position.dot(forward) + size * 0.5;
    x = std::max(0, std::min(size - 1, x));
    y = std::max(0, std::min(size - 1, y));
    return tiles[x + y * size];
}

World::World(sp::P<sp::Node> parent)
: sp::Node(parent)
{
    voxels = new sp::Voxelmap(this, "ground_tiles.png", 1.0, 4, 4);
    for(int n=0; n<4*4; n++)
        voxels->setVoxelData(n, {n, n, n});
    
    struct AreaDef
    {
        sp::Vector3f position;
        float size;
        int type;
    };
    std::vector<AreaDef> areas;
    areas.push_back({.position = sp::Vector3f(2, 2, 16), .size = 5.0f, .type = 3});
    areas.push_back({.position = sp::Vector3f(10, 14, 16), .size = 3.0f, .type = 1});
    areas.push_back({.position = sp::Vector3f(16, 4, 8), .size = 6.5f, .type = 2});
    areas.push_back({.position = sp::Vector3f(16, 10, 5), .size = 2.5f, .type = 1});
    areas.push_back({.position = sp::Vector3f(3, 15, 0), .size = 5.5f, .type = 3});
    areas.push_back({.position = sp::Vector3f(0, 4, 1), .size = 3.5f, .type = 1});

    for(int x=0; x<size; x++)
    {
        for(int y=0; y<size; y++)
        {
            for(int z=0; z<size; z++)
            {
                sp::Vector3f v(x, y, z);
                int type = 0;
                for(auto& area : areas)
                    if ((v - area.position).length() < area.size)
                        type = area.type;
                voxels->setVoxel(sp::Vector3i(x, y, z), type);
            }
        }
    }
    for(int x=0; x<size; x++)
    {
        for(int y=0; y<size; y++)
        {
            for(size_t n=0; n<sides.size(); n++)
            {
                sp::Vector3d p = sides[n].tiles[x + y * size].position;
                p += sp::Vector3d(size*0.5,size*0.5,size*0.5);
                p -= sides[n].up * 0.5;
                sides[n].tiles[x + y * size].ground_type = voxels->getVoxel(sp::Vector3i(p));
            }
        }
    }

    voxels->setPosition(sp::Vector3d(size*-0.5, size*-0.5, size*-0.5));
    sp::collision::Box3D shape(sp::Vector3d(size, size, size));
    shape.type = sp::collision::Shape::Type::Static;
    setCollisionShape(shape);
}

WorldSide& World::getSideAt(sp::Vector3d normal)
{
    if (std::abs(normal.z) > std::abs(normal.x) && std::abs(normal.z) > std::abs(normal.y))
    {
        if (normal.z > 0)
            return sides[0];
        return sides[1];
    }
    else if (std::abs(normal.x) > std::abs(normal.y))
    {
        if (normal.x > 0)
            return sides[2];
        return sides[3];
    }
    if (normal.y > 0)
        return sides[4];
    return sides[5];
}

Tile& World::getTileAt(sp::Vector3d position, sp::Vector3d normal)
{
    return getSideAt(normal).getTileAt(position);
}

Direction flipDirection(Direction d)
{
    switch(d)
    {
    case Direction::Forward: return Direction::Backward;
    case Direction::Backward: return Direction::Forward;
    case Direction::Right: return Direction::Left;
    case Direction::Left: return Direction::Right;
    }
    return Direction::Forward;
}
