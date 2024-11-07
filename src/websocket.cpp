#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include "websocket.h"
#include "drivers/SensorManager/SensorManager.h"

using namespace websockets;

WebsocketsClient wsClient;

bool isWebSocketConnected = false;

const unsigned long wsReconnectInterval = 2000;

extern SensorManager sensorManager;
void printJson(const JsonVariant &json, int indent = 0)
{
    // Create the indentation string manually
    String indentStr;
    for (int i = 0; i < indent; i++)
    {
        indentStr += "  "; // Adds two spaces per level
    }

    if (json.is<JsonObject>())
    {
        for (JsonPair kv : json.as<JsonObject>())
        {
            Serial.print(indentStr);
            Serial.print(kv.key().c_str());
            Serial.print(": ");
            printJson(kv.value(), indent + 1); // Recursively print nested objects
        }
    }
    else if (json.is<JsonArray>())
    {
        for (JsonVariant v : json.as<JsonArray>())
        {
            printJson(v, indent + 1);
        }
    }
    else if (json.is<const char *>())
    {
        Serial.println(json.as<const char *>());
    }
    else if (json.is<int>())
    {
        Serial.println(json.as<int>());
    }
    else if (json.is<float>())
    {
        Serial.println(json.as<float>());
    }
    else if (json.is<bool>())
    {
        Serial.println(json.as<bool>() ? "true" : "false");
    }
    else
    {
        Serial.println("null");
    }
}

void connectToWebSocket(const char *ws_server_address)
{
    if (!wsClient.available())
    {
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
            wsClient.close();
        }
    }
}

void pollWebSocket(const char *ws_server_address)
{
    if (!wsClient.available())
    {

        connectToWebSocket(ws_server_address);
    }
    else
    {
        wsClient.poll();
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

    Serial.println("Parsed JSON message:");
    printJson(doc);

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
    if (isWebSocketConnected)
    { // Check if the WebSocket is connected before sending
        // Retrieve sensor data as JSON
        DynamicJsonDocument sensorData = sensorManager.readAllSensors();

        // Prepare the WebSocket message with the sensor data
        StaticJsonDocument<512> doc;
        doc["Event"] = "SensorData";
        doc["Data"] = sensorData.as<JsonObject>();

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
    if (isWebSocketConnected) // Check if the WebSocket is connected before sending
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
