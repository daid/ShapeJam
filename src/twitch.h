#pragma once

#include <sp2/io/network/tcpSocket.h>
#include <sp2/updatable.h>
#include <sp2/timer.h>

class TwitchTest : public sp::Updatable
{
public:
    TwitchTest(sp::string channel);

    virtual void onUpdate(float delta) override;

    void joinChannel(const sp::string& channel);

    void sendMessage(const sp::string& channel, const sp::string& message);
private:
    void handleLine(sp::string line);
    void onMessage(const sp::string& channel, const sp::string& user, const sp::string& message);
    void onWhisper(const sp::string& user, const sp::string& message);

    void startVotingRound();
    void processVoteResult();

    void give(sp::string itemName, int amount);

    sp::string received_data;
    sp::string channel;
    sp::io::network::TcpSocket socket;

    sp::Timer timer;

    bool voting_started = false;
    bool initial_done = false;
    std::unordered_map<sp::string, std::function<void()>> options;
    std::unordered_map<sp::string, sp::string> votes;
};
