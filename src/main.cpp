#include <WebServer.h>
#include <DNSServer.h>
#include "websocket.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "utils/Multiplexer/Multiplexer.h"
#include <ArduinoOTA.h>
#include "drivers/SensorManager/SensorManager.h"
#include "utils/EEPROM/EEPROM.util.h"
#include <random>
#include "utils/Module/Module.util.h"
#include <Wire.h>
#include "drivers/SensorManager/SensorManager.h"
#include "config.json.h"
#include "utils/SerialManager/SerialManager.h"
#include <ArduinoWebsockets.h>

SerialManager serialManager;
SensorManager sensorManager;

bool isWebSocketConnected = false;

using namespace websockets;

WebServer server(80);
ModuleUtil moduleUtil(32);
String generateSerialNumber(int length);
bool webSocketConnected = false;
unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 5000;

const int EEPROM_SSID_ADDR = 0;
const int EEPROM_PASS_ADDR = 64;
const int EEPROM_MAX_LEN = 32;

const int channelToGPIO[] = {32, 33, 34, 35};

WebsocketsClient client;

String getSSIDs()
{
    String ssids = "";
    WiFi.disconnect();
    int n = WiFi.scanNetworks();
    if (n == 0)
    {
        ssids += "<option>No networks found</option>";
    }
    else
    {
        for (int i = 0; i < n; ++i)
        {
            ssids += "<option value=\"" + WiFi.SSID(i) + "\">" + WiFi.SSID(i) + "</option>";
        }
    }
    return ssids;
}

DynamicJsonDocument jsonDoc(1024);

// HTML content to be served
const char *index_html = R"rawliteral(
<!DOCTYPE html>
<html>
<head><title>ESP32 Wi-Fi Config</title></head>
<body>
<h1>ESP32 Wi-Fi Configuration</h1>
<form action="/save" method="POST">
    <label for="ssid">Wi-Fi SSID:</label>
    <select id="ssid" name="ssid">
        <!--SSIDS_PLACEHOLDER-->
    </select><br><br>
    <label for="password">Wi-Fi Password:</label>
    <input type="password" id="password" name="password" required><br><br>
    <input type="submit" value="Save">
</form>
</body>
</html>
)rawliteral";

void handleRoot()
{
    String html = index_html;
    html.replace("<!--SSIDS_PLACEHOLDER-->", getSSIDs());
    server.send(200, "text/html", html);
}

void connectToWiFi(const String &ssid, const String &password)
{
    WiFi.begin(ssid.c_str(), password.c_str());

    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("Connected to Wi-Fi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void handleSave()
{
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    // Attempt to connect to the selected Wi-Fi network
    connectToWiFi(ssid, password);

    String response = "Trying to connect to " + ssid + "...";
    server.send(200, "text/plain", response);
}

void setupOTA()
{
    ArduinoOTA.onStart([]()
                       {
        String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
        Serial.println("Start updating " + type); });

    ArduinoOTA.onEnd([]()
                     { Serial.println("\nEnd"); });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });

    ArduinoOTA.onError([](ota_error_t error)
                       {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed"); });

    ArduinoOTA.setPort(8266);
    ArduinoOTA.begin(); // Initialize OTA service
}

void setup()
{
    if (!SPIFFS.begin(true))
    { // Pass true to format if failed to mount
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    Serial.begin(115200);
    // Wait for Serial Monitor to open
    while (!Serial)
    {
        ; // Wait for serial port to connect. Needed for native USB
    }
    DeserializationError error = deserializeJson(jsonDoc, configJson);
    if (error)
    {
        Serial.println("Failed to parse JSON");
        return;
    }

    EEPROMUtil eepromUtil(0x50);
    eepromUtil.begin();
    Wire.begin();
    WiFi.mode(WIFI_STA);

    Serial.println("Starting AP mode...");
    WiFi.softAP("ESP32_Config_AP");
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());

    moduleUtil.readModules();

    sensorManager.initializeSensors();

    server.on("/", HTTP_GET, handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.onNotFound(handleRoot);

    server.begin();

    setupOTA();
}

void loop()
{
    ArduinoOTA.handle();
    server.handleClient();
    moduleUtil.readModules();

    if (WiFi.isConnected() && !isWebSocketConnected)
    {
        connectToWebSocket("ws://192.168.0.171:8080/v1/pots/?token=pot_1");
    }

    client.poll();
}

std::mt19937 generator;
String generateSerialNumber(int length)
{
    const String characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    String serialNumber;

    // Use the global random generator (already seeded in setup)
    std::uniform_int_distribution<> distribution(0, characters.length() - 1);

    for (int i = 0; i < length; ++i)
    {
        serialNumber += characters[distribution(generator)];
    }

    return serialNumber;
}