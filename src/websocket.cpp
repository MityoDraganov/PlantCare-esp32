#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#include "websocket.h"
#include "drivers/SensorManager/SensorManager.h"

using namespace websockets;

extern WebsocketsClient client;

extern bool isWebSocketConnected;

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

void onEventsCallback(WebsocketsEvent event, String data)
{
    if (event == WebsocketsEvent::ConnectionOpened)
    {
        Serial.println("Connnection Opened");
    }
    else if (event == WebsocketsEvent::ConnectionClosed)
    {
        Serial.println("Connnection Closed");
    }
    else if (event == WebsocketsEvent::GotPing)
    {
        client.pong();
        Serial.println("Got a Ping!");
    }
    else if (event == WebsocketsEvent::GotPong)
    {
        //Serial.println("Got a Pong!");
    }
}

// Update this size based on your JSON structure (you might need to increase it further if the data is more complex)
StaticJsonDocument<2048> doc;

void onMessageCallback(WebsocketsMessage message)
{
    Serial.print("Got Message: ");
    Serial.println(message.data());

    // Increase buffer size for larger messages
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, message.data());

    if (error)
    {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
        return;
    }

    // Extract message type/event
    const char *event = doc["Event"] | "unknown";

    // Handle different event types
    if (strcmp(event, "Connected") == 0)
    {
        isWebSocketConnected = true;
        Serial.println("Connection confirmed by server");
    }
    else if (strcmp(event, "KeepAlive") == 0)
    {
        // Respond to keep-alive
        DynamicJsonDocument response(128);
        response["Event"] = "KeepAliveResponse";
        String jsonResponse;
        serializeJson(response, jsonResponse);
        client.send(jsonResponse);
    }
    else if (strcmp(event, "ReadSensors") == 0)
    {
        // Handle sensor read request
        sendSensorData();
    }
    else
    {
        // Log unknown event type
        Serial.print("Unknown event type: ");
        Serial.println(event);
    }

    // Print parsed message for debugging
    Serial.println("Parsed JSON message:");
    serializeJsonPretty(doc, Serial);
}

void connectToWebSocket(const char *ws_server_address)
{
    if (client.connect(ws_server_address))
    {
        Serial.println("Connected to WebSocket server!");
        isWebSocketConnected = true;

        client.onMessage(onMessageCallback);
        client.onEvent(onEventsCallback);
    }
    else
    {
        Serial.println("Failed to connect to WebSocket server. Error code: " + String(client.getCloseReason()));
        isWebSocketConnected = false;
        client.close();
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
        client.send(message);
        Serial.println("Sent: " + message);
    }
    else
    {
        Serial.println("WebSocket is not connected.");
    }
}

void sendSensorData()
{
    if (isWebSocketConnected)
    {
        // Retrieve sensor data as JSON
        DynamicJsonDocument sensorData = sensorManager.readAllSensors();
        Serial.println("sensorData");
        printJson(sensorData);
        // Prepare the WebSocket message with the sensor data
        StaticJsonDocument<512> doc;
        doc["Event"] = "HandleMeasurements";
        doc["Data"] = sensorData;

        // Serialize the JSON document to a string
        String jsonData;
        serializeJson(doc, jsonData);

        // Send the JSON string over WebSocket
        client.send(jsonData);
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
        client.send(jsonData);
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
    client.send(jsonData);
    Serial.println("Sent: " + jsonData);
}

void sendKeepAlive()
{
    if (isWebSocketConnected)
    {
        client.ping();
        Serial.println("Sent keep-alive ping");
    }
}