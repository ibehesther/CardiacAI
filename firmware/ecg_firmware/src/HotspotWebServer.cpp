// HotspotWebServer.cpp
// This file implements the methods defined in the HotspotWebServer class.

#include "HotspotWebServer.h" // Include the corresponding header file

// Constructor definition
HotspotWebServer::HotspotWebServer(WirelessCommunication& comm)
    : _server(80), _wirelessComm(comm), _wifiSwitchRequested(false) {
    // The server is initialized on port 80 (standard HTTP port).
    // The reference to WirelessCommunication is stored for later use.
    // The wifi switch request flag is initialized to false.
}

// Starts the web server and configures its routes.
void HotspotWebServer::begin() {
    // Define the root route ("/") to serve the WiFi configuration HTML page.
    // This HTML is a simple form to allow users to input SSID and Password.
    _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", R"rawliteral(
            <!DOCTYPE html>
            <html>
            <head>
                <title>ECG Hotspot Setup</title>
                <meta name="viewport" content="width=device-width, initial-scale=1">
                <style>
                    body { font-family: Arial, sans-serif; background-color: #f0f0f0; margin: 0; padding: 20px; }
                    .container { background-color: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); max-width: 400px; margin: 20px auto; }
                    h2 { color: #333; text-align: center; }
                    label { display: block; margin-bottom: 5px; color: #555; }
                    input[type="text"], input[type="password"] { width: calc(100% - 22px); padding: 10px; margin-bottom: 15px; border: 1px solid #ddd; border-radius: 4px; }
                    button { background-color: #007bff; color: white; padding: 10px 15px; border: none; border-radius: 4px; cursor: pointer; width: 100%; font-size: 16px; }
                    button:hover { background-color: #0056b3; }
                    p { text-align: center; color: #666; font-size: 0.9em; }
                    .status { text-align: center; margin-top: 15px; font-weight: bold; }
                </style>
            </head>
            <body>
                <div class="container">
                    <h2>ECG Device WiFi Setup</h2>
                    <form id="wifiForm">
                        <label for="ssid">WiFi SSID:</label>
                        <input type="text" id="ssid" name="ssid" required><br>
                        <label for="password">WiFi Password:</label>
                        <input type="password" id="password" name="password" required><br>
                        <button type="submit">Save & Connect</button>
                    </form>
                    <p class="status" id="responseStatus"></p>
                </div>
                <script>
                    document.getElementById('wifiForm').addEventListener('submit', async function(event) {
                        event.preventDefault(); // Prevent default form submission
                        const ssid = document.getElementById('ssid').value;
                        const password = document.getElementById('password').value;
                        const responseStatus = document.getElementById('responseStatus');

                        responseStatus.textContent = 'Saving...';
                        responseStatus.style.color = 'orange';

                        try {
                            // Send credentials as JSON to the /setupWifi endpoint
                            const response = await fetch('/setupWifi', {
                                method: 'POST',
                                headers: {
                                    'Content-Type': 'application/json'
                                },
                                body: JSON.stringify({ ssid: ssid, password: password })
                            });
                            const data = await response.json(); // Parse the JSON response from the server

                            if (response.ok) { // Check if the HTTP status code is 2xx (success)
                                responseStatus.textContent = data.message || 'Credentials saved! Connecting to WiFi...';
                                responseStatus.style.color = 'green';
                                // You might want to automatically redirect the user or provide further instructions here
                            } else {
                                responseStatus.textContent = data.error || 'Failed to save credentials.';
                                responseStatus.style.color = 'red';
                            }
                        } catch (error) {
                            console.error('Error:', error); // Log any network or fetch errors
                            responseStatus.textContent = 'Network error. Try again.';
                            responseStatus.style.color = 'red';
                        }
                    });
                </script>
            </body>
            </html>
        )rawliteral"); // R"rawliteral(...)rawliteral" allows multi-line string without escaping
    });

    // Define the /setupWifi POST route to receive new WiFi credentials.
    _server.on("/setupWifi", HTTP_POST,
        // This handler processes the incoming request body (data), not files.
        // The first lambda is for file uploads, which we don't need here, so it's empty.
        [](AsyncWebServerRequest *request) {}, // No file upload handler
        NULL, // The second NULL is for the upload handler if there were file uploads
        // This lambda processes the received data (JSON payload).
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            // Parse the incoming JSON request body using ArduinoJson.
            JsonDocument doc; // Allocate a buffer for the JSON document (adjust size as needed)
            DeserializationError error = deserializeJson(doc, data, len); // Deserialize the JSON data

            if (error) {
                // If JSON parsing fails, send a 400 Bad Request response.
                Serial.printf("[HotspotWebServer] JSON parsing failed: %s\n", error.c_str());
                request->send(400, "application/json", "{\"error\":\"Invalid JSON format\"}");
                return;
            }

            // Extract SSID and password from the parsed JSON document.
            const char* ssid = doc["ssid"];
            const char* password = doc["password"];

            if (!ssid || !password) {
                // If SSID or password are missing, send a 400 Bad Request response.
                Serial.println("[HotspotWebServer] Missing SSID or password in JSON payload.");
                request->send(400, "application/json", "{\"error\":\"Missing SSID or password\"}");
                return;
            }

            Serial.printf("[HotspotWebServer] Received new credentials: SSID='%s'\n", ssid);

            // Save the new WiFi credentials using the WirelessCommunication instance.
            _wirelessComm.saveWiFiCredentials(ssid, password);

            // Send success response FIRST, then set flag for main loop to switch WiFi mode.
            request->send(200, "application/json", "{\"message\":\"WiFi credentials saved! Attempting to connect to WiFi in a moment...\"}");

            // Set the flag to indicate that a WiFi switch is requested.
            // The actual mode change will be handled in the main loop.
            _wifiSwitchRequested = true;
        }
    );

    _server.begin(); // Start the server, making it listen for incoming connections.
    Serial.println("[HotspotWebServer] Web server started on Hotspot mode (port 80).");
}

// Stops the web server.
void HotspotWebServer::end() {
    _server.end(); // Stops the server instance.
    Serial.println("[HotspotWebServer] Web server stopped.");
}

bool HotspotWebServer::isWifiSwitchRequested() {
    return _wifiSwitchRequested;
}

void HotspotWebServer::resetWifiSwitchRequest() {
    _wifiSwitchRequested = false;
}
