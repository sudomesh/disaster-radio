#include "Console.h"

#include <Layer1.h>
#include <LoRaLayer2.h>

#include <vector>

#define START_MESSAGE "Type '/join NICKNAME' to join the chat, or '/help' for more commands."

void Console::setup()
{
    DisasterMiddleware::setup();
    if (history)
    {
        history->replay(this);
    }
    client->receive(String(START_MESSAGE) + "\n");
}

void Console::processLine(String message)
{
    if (message.charAt(0) == '/')
    {
        std::vector<String> args;

        int start = 0, end;
        while (start < message.length())
        {
            int index = message.indexOf(' ', start);
            end = index == -1 ? message.length() : index;
            args.push_back(message.substring(start, end));
            start = end + 1;
        }

        String command = args[0].substring(1);
        if (command == "help")
        {
            client->receive("Commands: /help /join /nick /raw /lora /restart\n");
        }
        else if (command == "raw")
        {
            disconnect(client);
            server->disconnect(this);
            server->connect(client);
        }
        else if ((command == "join" || command == "nick") && args.size() > 1)
        {
            String msg = username.length() > 0 ? String("~ ") + username + " is now known as " + args[1] : String("~ ") + args[1] + " joined the channel";
            username = args[1];
            server->transmit(this, String("c|") + msg);
            client->receive(msg);
        }
        else if (command == "restart")
        {
            ESP.restart();
        }
        else if (command == "lora")
        {
            // TODO: print to client instead of serial
            Serial.printf("Address: ");
            LL2.printAddress(Layer1.localAddress());
            LL2.printRoutingTable();
        }
        else
        {
            client->receive(String("Unknown command '") + command + "'\n");
        }
    }
    else if (username.length() > 0)
    {

        server->transmit(this, String("c|") + "<" + username + "> " + message);
        client->receive(String("<") + username + ">" + " " + message + "\n");
    }
    else
    {
        client->receive(String(START_MESSAGE) + "\n");
    }
}

void Console::transmit(DisasterClient *client, String message)
{
    client->receive(message);

    buffer.concat(message);

    int index;
    while ((index = buffer.indexOf('\n')) >= 0)
    {
        // split the string on \n or \r\n
        String line = buffer.charAt(index - 1) == '\r'
                          ? buffer.substring(0, index - 1)
                          : buffer.substring(0, index);
        buffer = buffer.substring(index + 1);
        processLine(line);
    }
};

void Console::receive(String message)
{
    if (message.substring(0, 2) == "c|")
    {
        client->receive(message.substring(2) + "\n");
    }
};
