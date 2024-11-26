#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#include "websocket.h"
#include "drivers/SensorManager/SensorManager.h"
#include <queue>
#include "utils/Json/Json.util.h"
#include <HTTPClient.h>
#include <Update.h>

using namespace websockets;

extern WebsocketsClient client;
extern bool isWebSocketConnected;

const unsigned long wsReconnectInterval = 2000;

std::queue<String> pendingSerials;

extern SensorManager sensorManager;
bool performFirmwareUpdate(const char *firmwareUrl);

void onEventsCallback(WebsocketsEvent event, String data)
{
    if (event == WebsocketsEvent::ConnectionOpened)
    {
        Serial.println("Connnection Opened");
    }
    else if (event == WebsocketsEvent::ConnectionClosed)
    {
        Serial.println("Connnection Closed");
        isWebSocketConnected = false;
    }
    else if (event == WebsocketsEvent::GotPing)
    {
        client.pong();
        Serial.println("Got a Ping!");
    }
    else if (event == WebsocketsEvent::GotPong)
    {
        Serial.println("Got a Pong!");
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
    const char *event = doc["event"] | "unknown";

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
    else if (strcmp(event, "HandleSensorDataRequest") == 0)
    {
        sendSensorData();
    }
    else if (strcmp(event, "FirmwareUpdate") == 0)
    {
        const char *downloadUrl = doc["data"]["downloadUrl"];
        Serial.println(downloadUrl);
        Serial.println("Firmware update request received.");
        performFirmwareUpdate(downloadUrl);
    }
    else
    {
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
        delay(250);
    }
    else
    {
        Serial.println("Failed to connect to WebSocket server. Error code: " + String(client.getCloseReason()));
        isWebSocketConnected = false;
        client.close();
    }
}

void sendSensorData()
{
    if (isWebSocketConnected)
    {
        DynamicJsonDocument sensorData = sensorManager.readAllSensors();
        Serial.println("sensorData");
        JsonUtil jsonUtil;
        jsonUtil.printJson(sensorData, 2);
        StaticJsonDocument<512> doc;
        doc["Event"] = "HandleMeasurements";
        doc["Data"] = sensorData;

        String jsonData;
        serializeJson(doc, jsonData);

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
        // Serial.println("Sent keep-alive ping");
    }
}

void addPendingSerial(const String &serialNumber)
{
    pendingSerials.push(serialNumber);
}

void sendPendingSerials()
{
    while (!pendingSerials.empty())
    {
        sendSensorAttachEvent(pendingSerials.front());
        pendingSerials.pop();
    }
}

bool performFirmwareUpdate(const char *firmwareUrl)
{
    HTTPClient http;
    http.begin(firmwareUrl); // Initialize connection to firmware URL

    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
        int contentLength = http.getSize();

        if (contentLength > 0)
        {
            // Start OTA update
            bool canBegin = Update.begin(contentLength);
            if (canBegin)
            {
                WiFiClient *stream = http.getStreamPtr();
                size_t written = Update.writeStream(*stream);

                if (written == contentLength)
                {
                    if (Update.end())
                    {
                        Serial.println("Firmware updated successfully!");
                        ESP.deepSleep(0)
                    }
                    else
                    {
                        Serial.println("Firmware update failed!");
                        return false;
                    }
                }
                else
                {
                    Serial.println("Firmware update error: Incorrect content length");
                    return false;
                }
            }
            else
            {
                Serial.println("Not enough space for OTA update");
                return false;
            }
        }
    }
    else
    {
        Serial.printf("HTTP GET failed, error: %d\n", httpCode);
        return false;
    }

    http.end();
    return false;
}