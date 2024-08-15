#include <WiFi.h>
#include <WebServer.h>

// Create an instance of the WebServer
WebServer server(80);

// Function to get a list of available SSIDs
String getSSIDs() {
    String ssids = "";
    int n = WiFi.scanNetworks();
    if (n == 0) {
        ssids += "<option>No networks found</option>";
    } else {
        for (int i = 0; i < n; ++i) {
            ssids += "<option value=\"" + WiFi.SSID(i) + "\">" + WiFi.SSID(i) + "</option>";
        }
    }
    return ssids;
}

// HTML content to be served
const char* index_html = R"rawliteral(
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

// Function to handle the root request
void handleRoot() {
    String html = index_html;
    html.replace("<!--SSIDS_PLACEHOLDER-->", getSSIDs());
    server.send(200, "text/html", html);
}

// Function to handle the /save request
void handleSave() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    // Here you can handle the saving of Wi-Fi credentials, e.g., store in EEPROM
    // For demonstration, just print the received data
    Serial.println("Received SSID: " + ssid);
    Serial.println("Received Password: " + password);

    server.send(200, "text/plain", "Configuration saved");
}

void setup() {
    // Start serial communication
    Serial.begin(115200);

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
}

void loop() {
    // Handle client requests
    server.handleClient();
}
