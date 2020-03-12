#include "recipe.h"

#include <sp2/io/keyValueTreeLoader.h>
#include <sp2/stringutil/convert.h>
#include <sp2/assert.h>

std::unordered_map<sp::string, std::unique_ptr<Recipe>> Recipe::items;

void Recipe::init()
{
    if (!items.empty())
        return;

    auto tree = sp::io::KeyValueTreeLoader::load("recipe.txt");
    for(auto& node : tree->root_nodes)
    {
        LOG(Info, "New recipe:", node.id);

        auto entry = std::unique_ptr<Recipe>(new Recipe());
        entry->name = node.id;
        for(auto& it : node.findId("INPUT")->items)
            entry->input.push_back({ItemType::get(it.first), sp::stringutil::convert::toInt(it.second)});
        if (node.findId("OUTPUT"))
        {
            for(auto& it : node.findId("OUTPUT")->items)
                entry->output.push_back({ItemType::get(it.first), sp::stringutil::convert::toInt(it.second)});
        }
        else
        {
            entry->output.push_back({ItemType::get(node.id), 1});
        }
        entry->craft_time = sp::stringutil::convert::toFloat(node.items["time"]);
        items[node.id] = std::move(entry);
    }
}

const Recipe* Recipe::get(const sp::string& name)
{
    auto it = items.find(name);
    if (it == items.end())
        return nullptr;
    return &*it->second;
}
