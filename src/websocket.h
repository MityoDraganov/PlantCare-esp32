#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <ArduinoJson.h>

void connectToWebSocket(const char* ws_server_address);

void sendWebSocketMessage(const char* event, const JsonObject& data);
void sendSensorAttachEvent(String serialNumber);
void sendSensorDetachEvent(String serialNumber);
void receiveWebSocketMessage(const String &message, bool isBinary);
void sendSensorData(); // New function declaration

#endif // WEBSOCKET_H
