#include <WebServer.h>
#include <DNSServer.h>
#include "websocket.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "utils/EEPROM/EEPROM.util.h"
#include "utils/driver/driver.h"
#include <ArduinoOTA.h>
#include "drivers/SensorManager/SensorManager.h"

WebServer server(80);
DNSServer dnsServer;

const byte DNS_PORT = 53;                     // DNS port
bool webSocketConnected = false;              // WebSocket connection status
unsigned long lastReconnectAttempt = 0;       // Time of last WebSocket reconnect attempt
const unsigned long reconnectInterval = 5000; // Attempt reconnection every 5 seconds
EEPROMUtil eepromUtil(0x50); 

// Function to get a list of available SSIDs
String getSSIDs()
{
    String ssids = "";
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

    // Attempt to connect for a limited time (e.g., 10 seconds)
    int timeout = 10000; // 10 seconds
    int startTime = millis();
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

        connectToWebSocket("ws://192.168.0.171:8080/v1/pots/?token=pot_1");
        webSocketConnected = true; // Mark WebSocket as connected
    }
    else
    {
        Serial.println("Failed to connect to Wi-Fi.");
    }
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
    eepromUtil.begin();

    Serial.begin(115200);

    // Now the moisture sensor should be in the sensor manager
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

    WiFi.softAP("ESP32_Config_AP");
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());

    // Define routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.onNotFound(handleRoot); // Redirect all unknown URLs to the captive portal

    server.begin();

    setupOTA();
}

void loop()
{
    for (Sensor *sensor : SensorManager::getAllSensors())
    {
        Serial.println(String(sensor->getType()) + ": " + String(sensor->readValue()));
    }


    server.handleClient();

    if (WiFi.status() != WL_CONNECTED)
    {
        return;
    }
    pollWebSocket("ws://192.168.0.171:8080/v1/pots/?token=pot_1");

    ArduinoOTA.handle();
    delay(2000);
}
