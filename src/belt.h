#pragma once

#include "building.h"

class Belt : public Building
{
public:
    Belt(sp::P<World> world);

    virtual void onUpdate(float delta) override;
    virtual bool accepts(const ItemType* type, Direction direction) override { return true; }
    virtual bool canBeBridged() override { return true; }

    void save(sp::io::serialization::DataSet& data) const;
    void load(const sp::io::serialization::DataSet& data);
    static sp::AutoPointerObject* create(const sp::io::serialization::DataSet& data);
};
class Splitter : public Building
{
public:
    Splitter(sp::P<World> world);

    virtual void onUpdate(float delta) override;
    virtual bool accepts(const ItemType* type, Direction direction) override { return true; }
    virtual bool canBeBridged() override { return true; }

    void save(sp::io::serialization::DataSet& data) const;
    void load(const sp::io::serialization::DataSet& data);
    static sp::AutoPointerObject* create(const sp::io::serialization::DataSet& data);
private:
    bool flip = false;
};
