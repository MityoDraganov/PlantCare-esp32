#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include "websocket.h"
#include "drivers/SensorManager/SensorManager.h"

using namespace websockets;

WebsocketsClient wsClient;

bool isWebSocketConnected = false;

const unsigned long wsReconnectInterval = 5000;

void connectToWebSocket(const char *ws_server_address)
{
    if (!wsClient.available()) // Check if already connected
    {
        wsClient.close(); // Ensure any previous connection is closed

        if (wsClient.connect(ws_server_address))
        {
            Serial.println("Connected to WebSocket server!");
            isWebSocketConnected = true;
            wsClient.onMessage([](WebsocketsMessage message)
                               { receiveWebSocketMessage(message.data(), message.isBinary()); });
        }
        else
        {
            Serial.println("Failed to connect to WebSocket server. Error code: " + String(wsClient.getCloseReason()));
            isWebSocketConnected = false;
        }
    }
}

void pollWebSocket(const char *ws_server_address)
{
    if (!wsClient.available())
    {
        static unsigned long lastReconnectAttempt = 0;
        if (millis() - lastReconnectAttempt > wsReconnectInterval)
        {
            lastReconnectAttempt = millis();
            connectToWebSocket(ws_server_address);
        }
    }
}

void sendWebSocketMessage(const char *event, const JsonObject &data)
{
    if (isWebSocketConnected) // Check if the WebSocket is connected before sending
    {
        StaticJsonDocument<512> doc;
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

void receiveWebSocketMessage(const String &message, bool isBinary)
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

void sendSensorData()
{
    if (wsClient.available()) // Check if the WebSocket is connected before sending
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

void sendSensorAttachEvent(String serialNumber)
{
    StaticJsonDocument<512> doc;
    doc["Event"] = "HandleAttachSensor";
    
    // Create a nested "Data" object inside the main JSON document
    JsonObject data = doc.createNestedObject("Data");
    data["SerialNumber"] = serialNumber;

    // Serialize the JSON document to a string
    String jsonData;
    serializeJson(doc, jsonData);

    // Send the JSON string over WebSocket
    wsClient.send(jsonData);
    Serial.println("Sent: " + jsonData);
}

void sendSensorDetachEvent(String serialNumber)
{
    StaticJsonDocument<512> doc;
    doc["Event"] = "HandleDetachSensor";
    
    // Create a nested "Data" object inside the main JSON document
    JsonObject data = doc.createNestedObject("Data");
    data["SerialNumber"] = serialNumber;

    // Serialize the JSON document to a string
    String jsonData;
    serializeJson(doc, jsonData);

    // Send the JSON string over WebSocket
    wsClient.send(jsonData);
    Serial.println("Sent: " + jsonData);
}
