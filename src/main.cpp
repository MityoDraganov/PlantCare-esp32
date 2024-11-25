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

#include <Preferences.h>
using namespace websockets;

Preferences preferences;
Ticker keepAliveTicker;
DynamicJsonDocument jsonDoc(1024);
String generateSerialNumber(int length);

DNSServer dnsServer;
WebServer server(80);
WebsocketsClient client;
bool isWebSocketConnected = false;

EEPROMUtil eepromUtil(0x50);
ModuleUtil moduleUtil(32);

const unsigned long reconnectInterval = 5000;
const int channelToGPIO[] = {32, 33, 34, 35};
const char *html_path = "/index.html";

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

void connectToWiFi(const String &ssid, const String &password)
{
    WiFi.begin(ssid.c_str(), password.c_str());

    Serial.print("Connecting to Wi-Fi");

    unsigned long startAttemptTime = millis();
    const unsigned long connectionTimeout = 15000; // Timeout in milliseconds (15 seconds)

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < connectionTimeout)
    {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        // Save SSID and password only if connected
        preferences.begin("wifi", false);
        preferences.putString("ssid", ssid);
        preferences.putString("password", password);
        preferences.end();

        Serial.println("\nConnected to Wi-Fi!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        // Connection attempt timed out
        Serial.println("\nFailed to connect to Wi-Fi. Please check SSID and password.");
        WiFi.disconnect();
    }
}


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

void handleSave()
{
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    // Attempt to connect to the selected Wi-Fi network
    connectToWiFi(ssid, password);

    String response = "Trying to connect to " + ssid + "...";
    server.send(200, "text/plain", response);
}

void handleNotFound()
{
    server.sendHeader("Location", "/portal");
    server.send(302, "text/plain", "redirect to captive portal");
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
    Wire.begin(25, 26);
    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    client.close();
    Serial.begin(115200);
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
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32_Config_AP");

    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.setTTL(300);
    if ( dnsServer.start(53, "*", WiFi.softAPIP()))
    {
        Serial.println("Started DNS server in captive portal-mode");
    }
    else
    {
        Serial.println("Err: Can't start DNS server!");
    }

    preferences.begin("wifi", true);
    String ssid = preferences.getString("ssid", "");
    String password = preferences.getString("password", "");
    preferences.end();
    if (ssid != "" && password != "")
    {
        connectToWiFi(ssid, password);
    }

    moduleUtil.readModules();
    SensorManager::initializeSensors();

    server.on("/", HTTP_GET, handleRoot);
    server.on("/favicon.ico", HTTP_GET, []()
              { server.send(200, "image/x-icon", ""); }); // Sends an empty favicon
    server.on("/save", HTTP_POST, handleSave);
    server.on("/portal", handleRoot);
    server.onNotFound(handleNotFound);
    server.begin();

    setupOTA();
}

void loop()
{
    ArduinoOTA.handle();
    server.handleClient();
    ;
    moduleUtil.readModules();
    dnsServer.processNextRequest();

    if (WiFi.isConnected() && !isWebSocketConnected)
    {
        connectToWebSocket("ws://188.34.162.248:8000/api/v1/pots/?token=pot1");
    } 
    else if (WiFi.isConnected())
    {
        sendPendingSerials();
        client.ping();
        delay(1000);
        client.poll();
    }
    else
    {
        delay(5); // give CPU some idle time
    }
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