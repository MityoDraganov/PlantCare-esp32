#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "websocket.h"
#include "utils/driver/driver.h"


using namespace websockets;

WebsocketsClient wsClient; // WebSocket client
DriverUtil driverUtil;

void connectToWebSocket(const char *ws_server_address)
{
    // Attempt to connect to the WebSocket server
    if (wsClient.connect(ws_server_address))
    {
        Serial.println("Connected to WebSocket server!");

        wsClient.onMessage([](WebsocketsMessage message)
                           { Serial.println("Received: " + message.data()); });
    }
    else
    {
        Serial.println("Failed to connect to WebSocket server.");
    }
}

void pollWebSocket()
{
    if (wsClient.available())
    {
        wsClient.poll();
    }
}

void sendWebSocketMessage(const char *event, const JsonObject &data)
{
    if (wsClient.available())
    {
        StaticJsonDocument<256> doc;
        doc["Event"] = event;
        doc["Data"] = data;

        // Serialize JSON document to string
        String message;
        serializeJson(doc, message);

        // Send the JSON string over WebSocket
        wsClient.send(message);
        Serial.println("Sent: " + message);
    }
    else
    {
        Serial.println("WebSocket is not connected.");
    }
}

void receiveWebSocketMessage(const String &message, bool isBinary) {
    if (isBinary) {
        Serial.println("Received firmware binary data.");
        size_t len = message.length();
        uint8_t* firmwareData = new uint8_t[len];
        message.getBytes(firmwareData, len);

        // Initialize the OTA update process
        driverUtil.handleUpdate(firmwareData, len);
        delete[] firmwareData;

    } else {
        Serial.println("Received non-binary message: " + message);
    }
}
