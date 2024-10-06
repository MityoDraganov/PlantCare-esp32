#include <WebServer.h>
#include "websocket.h"
#include <SPIFFS.h>

#include <ArduinoJson.h>

#include "utils/EEPROM/EEPROM.util.h"
#include "utils/driver/driver.h"
#include <ArduinoOTA.h>

WebServer server(80);

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

bool hasDownloadedZip = false;

void listFilesInSPIFFS(const char *dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\n", dirname);

    File root = SPIFFS.open(dirname);
    if (!root)
    {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels)
            {
                listFilesInSPIFFS(file.name(), levels - 1);
            }
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void setupOTA()
{
    ArduinoOTA.onStart([]()
                       {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
        else // U_SPIFFS
            type = "filesystem";
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

// Function to handle the root request
void handleRoot()
{
    String html = index_html;
    html.replace("<!--SSIDS_PLACEHOLDER-->", getSSIDs());
    server.send(200, "text/html", html);
}

// Function to handle the /save request
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

    // Check if connected
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Connected to Wi-Fi!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());

        connectToWebSocket("ws://192.168.0.171:8080/v1/pots/?token=pot_1");
    }
    else
    {
        Serial.println("Failed to connect to Wi-Fi.");
    }
}

void testEEPROM()
{
    EEPROMUtil eepromUtil(64);
    eepromUtil.begin();

    String readString = eepromUtil.readString(0, 32);
    Serial.println("Read String: " + readString);
}

void setup()
{
    // Start serial communication
    Serial.begin(115200);

    if (!SPIFFS.begin(true))
    {
        Serial.println("Failed to mount file system");
        return;
    }

    // testEEPROM();

    // Set up ESP32 as an access point
    WiFi.softAP("ESP32_Config_AP");

    // Print the IP address of the access point
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());

    // Define routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/save", HTTP_POST, handleSave);

    // Start the server

    server.begin();

    setupOTA();
}

void sendSensorData()
{
    // Read the actual moisture sensor value
    // int moistureLevel = readMoistureLevel();

    // Create a JSON document to hold the data
    StaticJsonDocument<256> dataDoc;
    dataDoc["sensorID"] = 0;
    // dataDoc["value"] = moistureLevel;

    // Send the data using the helper function
    sendWebSocketMessage("HandleMeasurements", dataDoc.as<JsonObject>());
}

void loop()
{
    server.handleClient();
    if (WiFi.status() != WL_CONNECTED)
    {
        return;
    }

    pollWebSocket();
    ArduinoOTA.handle();
    // listFilesInSPIFFS("/", 0);

    // sendSensorData();

    delay(1000);
}
