#include <WebServer.h>
#include <DNSServer.h>
#include "websocket.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "utils/Multiplexer/Multiplexer.h"
#include <ArduinoOTA.h>
#include "utils/EEPROM/EEPROM.util.h"
#include <random>
#include "utils/Module/Module.util.h"
#include <Wire.h>
#include "drivers/SensorManager/SensorManager.h"
#include "config.json.h"
#include <ArduinoWebsockets.h>
#include <Ticker.h>
#include "Globals.h"

EEPROMUtil eepromUtil(0x50);
bool isWebSocketConnected = false;

using namespace websockets;

WebServer server(80);
ModuleUtil moduleUtil(32);
String generateSerialNumber(int length);
unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 5000;

const int EEPROM_SSID_ADDR = 0;
const int EEPROM_PASS_ADDR = 64;
const int EEPROM_MAX_LEN = 32;

int channelToGPIO[] = {32, 33, 34, 35};

WebsocketsClient client;

Ticker keepAliveTicker;

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

const char *html_path = "/index.html";

void handleRoot()
{
    if (SPIFFS.exists(html_path))
    {
        String html = SPIFFS.open("/index.html", "r").readString();
        html.replace("<!--SSIDS_PLACEHOLDER-->", getSSIDs());
        server.send(200, "text/html", html);
    }
    else
    {
        server.send(500, "text/plain", "Internal Server Error");
    }
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
    isWebSocketConnected = false;
    client.close();
    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    Serial.begin(115200);
    // connectToWiFi("Welikowi", "password here");
    // connectToWebSocket("ws://192.168.0.171:8080/v1/pots/?token=pot_1");
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

    eepromUtil.begin();
    Wire.begin();
    WiFi.mode(WIFI_STA);

    for (int channel = 0; channel < 4; ++channel)
    {
        String serialNumber = generateSerialNumber(16);
        eepromUtil.writeStringExternal(0, serialNumber, serialNumber.length(), channel);

        Serial.print("Written serial number for channel ");
        Serial.print(channel);
        Serial.print(": ");
        Serial.println(serialNumber);
    }

    Serial.println("Starting AP mode...");
    WiFi.softAP("ESP32_Config_AP");
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());

    moduleUtil.readModules();
    SensorManager::initializeSensors();

    server.on("/", HTTP_GET, handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.onNotFound(handleRoot);

    server.begin();

    setupOTA();
    keepAliveTicker.attach(5, sendKeepAlive);
}

void loop()
{
    ArduinoOTA.handle();
    server.handleClient();
    client.ping();

    if (WiFi.isConnected() && !isWebSocketConnected)
    {
        // connectToWebSocket("ws://188.34.162.248:8000/api/v1/pots/?token=pot1");
        connectToWebSocket("ws://192.168.0.171:8080/v1/pots/?token=pot_1");
    }
    else if (WiFi.isConnected())
    {
        sendPendingSerials();
        client.ping();
        delay(1000);
        client.poll();
    }
    if (!isWebSocketConnected)
    {
        return;
    }
    moduleUtil.readModules();
    sendPendingSerials();

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

    String control = "type=control";
    for (int i = 0; i < control.length(); ++i)
    {
        serialNumber += control[i];
    }

    return serialNumber;
}