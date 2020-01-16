#pragma once

#include "itemType.h"

#include <sp2/scene/node.h>
#include <sp2/graphics/gui/widget/widget.h>


class StatsCollector : public sp::Node
{
public:
    using sp::Node::Node;

    void add(const ItemType* type, int amount);
    void show(sp::P<sp::gui::Widget> panel);

    virtual void onFixedUpdate() override;

private:
    class Group
    {
    public:
        void add(const ItemType* type, int amount);
        bool shift();

        using Record = std::unordered_map<const ItemType*, int>;
        Record totals;
    private:
        std::array<Record, 60> records;
        int index = 0;
    };
    Group seconds_group;
    Group minutes_group;
    Group hours_group;
};
