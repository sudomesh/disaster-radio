#include "WebSocketClient.h"
#include "settings/settings.h"

#include <unordered_map>

void WebSocketClient::receive(struct Datagram datagram, size_t len)
{
    unsigned char buf[len-DATAGRAM_HEADER]; //= {'\0'};
    memcpy(buf, &datagram.message, sizeof(buf));

    client->binary(buf, sizeof(buf));
}

void WebSocketClient::handleDisconnect()
{
    server->disconnect(this);
}

void WebSocketClient::handleError(uint16_t code, const char *message)
{
    server->disconnect(this);
}

void WebSocketClient::handleData(void *data, size_t len)
{
    // assume this is a broadcast message for now 
    struct Datagram datagram = {0xff, 0xff, 0xff, 0xff};
    memset(datagram.message, 0, 233);
    datagram.type = 'c';
    memcpy(&datagram.message, data, len);
    len = len+DATAGRAM_HEADER;

    //TODO  work on ACK
    char msg_id_buf[3];
    memcpy(&msg_id_buf, data, 2);
    msg_id_buf[2] = '!';
    client->binary(msg_id_buf, 3);

  if (datagram.message[4] == '~')
  {
    /// \todo Change WS interface to have name directly
    /// \todo Change WS interface to use name from startup
    // Pull out the name from the message
    int idx = 0;
    int startIdx = 0;
    int endIdx = 0;
    while (datagram.message[idx] != 0x20)
    {
      startIdx++;
      idx++;
    }
    idx++;
    endIdx = idx;
    while (datagram.message[idx] != 0x20)
    {
      endIdx++;
      idx++;
    }
    startIdx++;
    endIdx--;

    char tempName[endIdx - startIdx + 2] = {0};
    memcpy(tempName, (char *)&datagram.message[startIdx], endIdx - startIdx + 1);
    username = String(tempName);

    saveUsername(username);
  }

  /// \todo WS command extensions, this should be changed when WS interface is rewritten!
  // Get first two characters after the <NAME> which are used as commands
  int startIdx = 0;
  while (datagram.message[startIdx] != 0x20)
  {
    startIdx++;
  }

  /// \todo Change WS interface to make this with a button
  // Check if user wants to switch UI with command '!!'
  if ((datagram.message[startIdx + 1] == '!') && (datagram.message[startIdx + 2] == '!'))
  {
    // User wants to switch to BLE interface
    username = "";
    saveUsername(username);
  }

  /// \todo Change WS interface to make this with a button
  // Check if user wants to switch UI with command '!~'
  if ((datagram.message[startIdx + 1] == '!') && (datagram.message[startIdx + 2] == '~'))
  {
    // User wants to switch to BLE interface
    saveUI(true);
    delay(500);
    ESP.restart();
  }

    server->transmit(this, datagram, len);
}

std::unordered_map<uint32_t, WebSocketClient *> client_map;

void WebSocketClient::startServer(AsyncWebSocket *ws, void (*callback)(WebSocketClient *))
{
    ws->onEvent([callback](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        if (type == WS_EVT_CONNECT)
        {
            WebSocketClient *ws_client = new WebSocketClient(client);
            client_map[client->id()] = ws_client;
            callback(ws_client);
        }

        WebSocketClient *ws_client = client_map[client->id()];
        if (!ws_client)
        {
            Serial.printf("Missing WebSocketClient!\n");
            return;
        }

        if (type == WS_EVT_DISCONNECT)
        {
            ws_client->handleDisconnect();
            client_map.erase(client->id());
        }
        else if (type == WS_EVT_ERROR)
        {
            ws_client->handleError(*((uint16_t *)arg), (char *)data);
            client_map.erase(client->id());
        }
        else if (type == WS_EVT_PONG)
        {
            // TODO
        }
        else if (type == WS_EVT_DATA)
        {
            AwsFrameInfo *info = (AwsFrameInfo *)arg;
            if (info->final && info->index == 0 && info->len == len)
            {
                ws_client->handleData(data, len);
            }
            else
            {
                // TODO message is comprised of multiple frames or the frame is split into multiple packets
            }
        }
    });
}
