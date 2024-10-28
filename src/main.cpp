#include <WebServer.h>
#include <DNSServer.h>
#include "websocket.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "utils/EEPROM/EEPROM.util.h"
#include "utils/driver/driver.h"
#include "utils/Multiplexer/Multiplexer.h"
#include <ArduinoOTA.h>
#include "drivers/SensorManager/SensorManager.h"
#include <random>

WebServer server(80);
String generateSerialNumber(int length);
EEPROMUtil eepromUtil(0x50);
bool webSocketConnected = false;
unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 5000;

const int EEPROM_SSID_ADDR = 0;
const int EEPROM_PASS_ADDR = 64;
const int EEPROM_MAX_LEN = 32;

const int NUM_MUX_CHANNELS = 4;

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

void handleSave()
{
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    // Attempt to connect to the selected Wi-Fi network
    WiFi.begin(ssid.c_str(), password.c_str());

    // Provide feedback to the user
    String response = "Trying to connect to " + ssid + "...";
    server.send(200, "text/plain", response);

    int timeout = 15000;
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Connected to Wi-Fi!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());

        // WebSocket or other functionality here
        webSocketConnected = true;
    }
    else
    {
        Serial.println("Failed to connect to Wi-Fi.");
    }
}

// void connectToStoredWiFi()
// {
//     String ssid, password;

//     // Load Wi-Fi credentials from EEPROM
//     if (eepromUtil.loadWiFiCredentials(ssid, password))
//     {
//         Serial.println("Attempting to connect to saved Wi-Fi...");
//         WiFi.begin(ssid.c_str(), password.c_str());

//         // Attempt to connect with a timeout
//         unsigned long startTime = millis();
//         int timeout = 10000; // 10 seconds
//         while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout)
//         {
//             delay(500);
//             Serial.print(".");
//         }

//         if (WiFi.status() == WL_CONNECTED)
//         {
//             Serial.println("Connected to Wi-Fi!");
//             Serial.print("IP Address: ");
//             Serial.println(WiFi.localIP());
//             return;
//         }
//         else
//         {
//             Serial.println("Failed to connect to saved Wi-Fi.");
//         }
//     }
// }

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
    Wire.begin();
    Serial.begin(115200);

    eepromUtil.begin();
    WiFi.mode(WIFI_STA);

    Serial.println("Starting AP mode...");
    WiFi.softAP("ESP32_Config_AP");
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());

    int serialNumLenght = 16; // Maximum length
    // for (int i = 0; i < NUM_MUX_CHANNELS; i++)
    // {
    //     String serialNumber = generateSerialNumber(serialNumLenght);
    //     eepromUtil.writeStringExternal(0, serialNumber, serialNumLenght, i);
    //     Serial.print("Written Serial Number for Channel ");
    //     Serial.print(i);
    //     Serial.print(": ");
    //     Serial.println(serialNumber);
    // }

    for (int i = 0; i < NUM_MUX_CHANNELS; i++)
    {
        String serialNumber = eepromUtil.readStringExternal(0, serialNumLenght, i);
        if(serialNumber != ""){
        Serial.print("Read Serial Number for Channel ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(serialNumber);
        }
    }

    for (Sensor *sensor : SensorManager::getAllSensors())
    {
        Serial.println("Found sensor: " + String(sensor->getType()));
        sensor->init();
    }

    if (!SPIFFS.begin(true))
    {
        Serial.println("Failed to mount file system");
        return;
    }
    server.on("/", HTTP_GET, handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.onNotFound(handleRoot);

    server.begin();

    setupOTA();
}

void loop()
{
    server.handleClient();
    // for (Sensor *sensor : SensorManager::getAllSensors())
    // {
    //     Serial.println(String(sensor->getType()) + ": " + String(sensor->readValue()));
    // }

    if (WiFi.status() != WL_CONNECTED)
    {
        return;
    }
    pollWebSocket("ws://192.168.0.171:8080/v1/pots/?token=pot_1");

    ArduinoOTA.handle();
    delay(2000);
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