#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include "websocket.h"
#include "utils/driver/driver.h"
#include "drivers/SensorManager/SensorManager.h"

using namespace websockets;

WebsocketsClient wsClient;
DriverUtil driverUtil;

void connectToWebSocket(const char *ws_server_address)
{
    if (wsClient.connect(ws_server_address))
    {
        Serial.println("Connected to WebSocket server!");

        wsClient.onMessage([](WebsocketsMessage message)
        { 
            receiveWebSocketMessage(message.data(), message.isBinary()); 
        });
    }
    else
    {
        Serial.println("Failed to connect to WebSocket server.");
    }
}

void pollWebSocket(const char *ws_server_address)
{
    if (wsClient.available())
    {
        wsClient.poll();
    } 
    else 
    {
        Serial.println("Attempting to connect to WebSocket server!");
        connectToWebSocket(ws_server_address);
    }
}

void sendWebSocketMessage(const char *event, const JsonObject &data)
{
    if (wsClient.available())
    {
        StaticJsonDocument<256> doc;
        doc["Event"] = event;
        doc["Data"] = data;

        String message;
        serializeJson(doc, message);
        wsClient.send(message);
        Serial.println("Sent: " + message);
    }
    else
    {
        Serial.println("WebSocket is not connected.");
    }
}

// Handles incoming WebSocket messages and processes commands
void receiveWebSocketMessage(const String &message, bool isBinary)
{
    if (isBinary)
    {
        Serial.println("Received binary data.");
        size_t len = message.length();
        uint8_t *firmwareData = new uint8_t[len];
        message.getBytes(firmwareData, len);
        driverUtil.handleUpdate(firmwareData, len);
        delete[] firmwareData;
    }
    else
    {
        Serial.println("Received non-binary message: " + message);
        
        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, message);
        if (error)
        {
            Serial.println("Failed to parse JSON message.");
            return;
        }

        const char *command = doc["command"];
        if (strcmp(command, "readAllSensorData") == 0)
        {
            sendSensorData();
        }
        else
        {
            Serial.println("Unknown command received.");
        }
    }
}

void sendSensorData()
{
    if (wsClient.available())
    {
        // Create a JSON array to hold all sensor data
        StaticJsonDocument<512> doc;
        JsonArray sensorArray = doc.to<JsonArray>();

        // Iterate over all sensors and collect their data
        for (Sensor *sensor : SensorManager::getAllSensors())
        {
            JsonObject sensorData = sensorArray.createNestedObject();
            sensorData["sensor"] = sensor->getType();
            sensorData["data"] = sensor->readValue();
        }

        // Serialize the JSON document to a string
        String jsonData;
        serializeJson(doc, jsonData);

        // Send the JSON string over WebSocket
        wsClient.send(jsonData);
        Serial.println("Sent sensor data: " + jsonData);
    }
    else
    {
        Serial.println("WebSocket is not connected.");
    }
}

void sendSensorAttachEvent(String serialNumber) {
    if (wsClient.available()) {
        StaticJsonDocument<256> doc;
        doc["event"] = "SensorAttached";
        JsonObject data = doc.createNestedObject("data");
        data["serialNumber"] = serialNumber;

        String message;
        serializeJson(doc, message);
        wsClient.send(message);
        Serial.println("Sent sensor attach event: " + message);
    } else {
        Serial.println("WebSocket not connected. Cannot send sensor attach event.");
    }
}