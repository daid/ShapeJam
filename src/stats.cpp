#include "stats.h"

void StatsCollector::add(const ItemType* type, int amount)
{
    seconds_group.add(type, amount);
    minutes_group.add(type, amount);
    hours_group.add(type, amount);
}

void StatsCollector::show(sp::P<sp::gui::Widget> panel)
{
    while(!panel->getChildren().empty())
        (*panel->getChildren().begin()).destroy();

    struct Entry
    {
        const ItemType* type;
        int per_second;
        int per_minute;
        int per_hour;
    };
    std::vector<Entry> entries;
    for(auto it : hours_group.totals)
    {
        entries.push_back({it.first, seconds_group.totals[it.first], minutes_group.totals[it.first], it.second});
    }
    std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b)
    {
        return a.per_second > b.per_second;
    });
}

void StatsCollector::onFixedUpdate()
{
    if (seconds_group.shift())
        if (minutes_group.shift())
            hours_group.shift();
}

void StatsCollector::Group::add(const ItemType* type, int amount)
{
    totals[type] += amount;
    records[index][type] += amount;
}

bool StatsCollector::Group::shift()
{
    index = (index + 1) % 60;
    for(auto it : records[index])
        totals[it.first] -= it.second;
    records[index].clear();
    return index == 0;
}
