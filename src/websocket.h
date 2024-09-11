#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <ArduinoJson.h>
#include "utils/driver/driver.h"

void connectToWebSocket(const char* ws_server_address);
void pollWebSocket();

void sendWebSocketMessage(const char* event, const JsonObject& data);
void receiveWebSocketMessage(const String &message, bool isBinary);

#endif // WEBSOCKET_H
