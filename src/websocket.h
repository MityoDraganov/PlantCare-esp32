#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <ArduinoJson.h>

void connectToWebSocket(const char* ws_server_address);
void pollWebSocket();

// The 'JsonObject' type should be used with a const reference to avoid issues with temporary objects.
void sendWebSocketMessage(const char* event, const JsonObject& data);

#endif // WEBSOCKET_H
