#include "twitch.h"
#include "mainScene.h"

#include <sp2/io/resourceProvider.h>
#include <sp2/random.h>
#include <sp2/stringutil/convert.h>


TwitchTest::TwitchTest(sp::string channel)
: channel(channel)
{
    socket.connect(sp::io::network::Address("irc.chat.twitch.tv"), 6667);
    socket.setBlocking(false);
    sp::string connect = sp::io::ResourceProvider::get("twitch.key")->readAll();
    socket.send(connect.data(), connect.length());
    //sp::string c = "CAP REQ :twitch.tv/tags twitch.tv/commands\n";
    sp::string c = "CAP REQ :twitch.tv/commands\n";
    socket.send(c.data(), c.length());

    joinChannel(channel.lower());
    //joinChannel("mrjoshuamclean");
    sendMessage(channel.lower(), "Hello everyone! You will be able to vote here on different ways to assist " + channel);
    timer.start(30);
}

void TwitchTest::onUpdate(float delta)
{
    char buffer[128];
    size_t received_size = socket.receive(buffer, sizeof(buffer));
    if (received_size > 0)
    {
        received_data += sp::string(buffer, received_size);
        while(received_data.find_first_of("\r\n") != std::string::npos)
        {
            size_t eol = received_data.find_first_of("\r\n");
            if (eol > 0)
            {
                handleLine(received_data.substr(0, eol));
            }
            received_data = received_data.substr(eol + 1);
        }
    }

    if (timer.isExpired())
    {
        if (!voting_started)
        {
            voting_started = true;
            timer.start(30);
            votes.clear();
            startVotingRound();
        }
        else
        {
            processVoteResult();
            voting_started = false;
            timer.start(60 * 4);
        }
    }
}

void TwitchTest::joinChannel(const sp::string& channel)
{
    sp::string connect = "JOIN #" + channel + "\n";
    socket.send(connect.data(), connect.length());
}

void TwitchTest::sendMessage(const sp::string& channel, const sp::string& message)
{
    sp::string command = "PRIVMSG #" + channel + " :" + message + "\n";
    socket.send(command.data(), command.length());
}

void TwitchTest::handleLine(sp::string line)
{
    sp::string prefix;
    int idx;
    if (line[0] == ':')
    {
        idx = line.find(" ");
        prefix = line.substr(1, idx);
        line = line.substr(idx + 1);
    }
    idx = line.find(" ");
    sp::string command = line.substr(0, idx);
    line = line.substr(idx + 1);
    idx = line.find(":");
    sp::string trailing;
    if (idx > -1)
    {
        trailing = line.substr(idx + 1);
        line = line.substr(0, idx).strip();
    }

    //LOG(Debug, "<", prefix, "#", command, "#", line, "#", trailing);
    if (command == "001")
    {
        //Succesfully connected
    }
    else if (command == "PING")
    {
        sp::string pong = "PONG :" + trailing + "\n";
        socket.send(pong.data(), pong.length());
    }
    else if (command == "PRIVMSG")
    {
        if (prefix.find("!") != -1)
            prefix = prefix.substr(0, prefix.find("!"));
        onMessage(line, prefix, trailing);
    }
    else if (command == "WHISPER")
    {
        if (prefix.find("!") != -1)
            prefix = prefix.substr(0, prefix.find("!"));
        onWhisper(prefix, trailing);
    }
}

void TwitchTest::onMessage(const sp::string& channel, const sp::string& user, const sp::string& message)
{
    LOG(Debug, "MSG", channel, user, message);
    if (message.startswith("!give "))
    {
        if (user == "daid303")
        {
            auto parts = message.split(" ");
            int amount = 1;
            if (parts.size() > 2)
                amount = sp::stringutil::convert::toInt(parts[2]);
            give(parts[1], amount);
        }
        return;
    }
    if (voting_started)
    {
        //if (user == channel.lower())
        //    return;
        if (message.startswith("!"))
        {
            int end = message.find_first_of(" :");
            if (end < 0)
                end = message.length();
            sp::string vote = message.substr(1, end).lower();
            LOG(Debug, "vote", vote, user, options.find(vote) != options.end(), votes.find(user) == votes.end());
            if (options.find(vote) != options.end() && votes.find(user) == votes.end())
                votes[user] = vote;
        }
    }
}

void TwitchTest::onWhisper(const sp::string& user, const sp::string& message)
{
    LOG(Debug, "WHISPER", user, message);
}

void TwitchTest::startVotingRound()
{
    if (!initial_done)
    {
        sendMessage(channel.lower(), "Let us start by testing the voting system. Everyone vote. You can vote with chatting '!option'. !cool: Voting is cool. !great: Voting is great. !stop: Voting should be stopped. You have 30 seconds to vote.");
        options = {
            {"cool", [=](){}},
            {"great", [=](){}},
            {"stop", [=](){}},
        };
        initial_done = true;
        return;
    }
    sendMessage(channel.lower(), "Time to vote. Give lets give more buildings. Vote for: !miner !shaper !cutter !factory !none");
    options = {
        {"miner", [=](){give("MINER", 3);}},
        {"shaper", [=](){give("MINER", 3);}},
        {"cutter", [=](){give("CUT_FACTORY", 2);}},
        {"factory", [=](){give("FACTORY", 1);}},
        {"none", [=](){}},
    };
}

void TwitchTest::processVoteResult()
{
    std::unordered_map<sp::string, int> results;
    for(auto it : options)
        results[it.first] = 0;
    for(auto it : votes)
        results[it.second] += 1;
    sp::string msg = "Vote results are in: ";
    int max = 0;
    for(auto it : results)
    {
        max = std::max(max, it.second);
    }
    if (max == 0)
    {
        msg += "No votes.";
    }
    else
    {
        std::vector<sp::string> top_votes;
        for(auto it : results)
            if (it.second == max)
                top_votes.push_back(it.first);
        sp::string result = *sp::randomSelect(top_votes);
        msg += result + " won!";
        options[result]();
        options.clear();
    }
    sendMessage(channel.lower(), msg);
}

void TwitchTest::give(sp::string itemName, int amount)
{
    sp::P<Scene> scene = sp::Scene::get("MAIN");
    if (!scene)
        return;
    const ItemType* type = ItemType::get(itemName);
    if (!type)
        type = ItemType::get(itemName.upper());
    if (type)
        scene->addInventory(type, amount);
}
