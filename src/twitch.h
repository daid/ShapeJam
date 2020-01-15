#include <sp2/io/network/tcpSocket.h>
#include <sp2/scene/node.h>

class TwitchTest : public sp::Node
{
public:
    TwitchTest(sp::P<sp::Node> parent)
    : sp::Node(parent)
    {
        socket.connect(sp::io::network::Address("irc.chat.twitch.tv"), 6667);
        socket.setBlocking(false);
        sp::string connect = sp::io::ResourceProvider::get("twitch.key")->readAll();
        socket.send(connect.data(), connect.length());
        //sp::string c = "CAP REQ :twitch.tv/tags twitch.tv/commands\n";
        sp::string c = "CAP REQ :twitch.tv/commands\n";
        socket.send(c.data(), c.length());

        joinChannel("daid303");
        //joinChannel("mrjoshuamclean");
        sendMessage("daid303", "Hello everyone!");
    }

    virtual void onUpdate(float delta) override
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
    }

    void joinChannel(const sp::string& channel)
    {
        sp::string connect = "JOIN #" + channel + "\n";
        socket.send(connect.data(), connect.length());
    }

    void sendMessage(const sp::string& channel, const sp::string& message)
    {
        sp::string command = "PRIVMSG #" + channel + " :" + message + "\n";
        socket.send(command.data(), command.length());
    }
private:
    void handleLine(sp::string line)
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

        LOG(Debug, "<", prefix, "#", command, "#", line, "#", trailing);
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

    void onMessage(const sp::string& channel, const sp::string& user, const sp::string& message)
    {
        LOG(Debug, "MSG", channel, user, message);
    }

    void onWhisper(const sp::string& user, const sp::string& message)
    {
        LOG(Debug, "WHISPER", user, message);
    }

    sp::string received_data;
    sp::io::network::TcpSocket socket;
};
