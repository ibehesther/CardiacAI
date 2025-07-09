// HotspotWebServer.cpp
// This file implements the methods defined in the HotspotWebServer class.

#include "HotspotWebServer.h" // Include the corresponding header file

// Constructor definition
HotspotWebServer::HotspotWebServer(WirelessCommunication &comm)
    : _server(80), _wirelessComm(comm), _wifiSwitchRequested(false)
{
    // The server is initialized on port 80 (standard HTTP port).
    // The reference to WirelessCommunication is stored for later use.
    // The wifi switch request flag is initialized to false.
}

// Starts the web server and configures its routes.
void HotspotWebServer::begin()
{
    // Define the root route ("/") to serve the WiFi configuration HTML page.
    // This HTML is a simple form to allow users to input SSID and Password.
    _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
               {
                   request->send(200, "text/html", R"rawliteral(
        <!DOCTYPE html>
        <html>
            <head>
                <title>CardiacAI Hotspot Setup</title>
                <meta name="viewport" content="width=device-width, initial-scale=1" />
                <style>
                    body {
                        font-family: Arial, sans-serif;
                        background-color: #f0f0f0;
                        margin: 0;
                        padding: 20px;
                    }
                    .container {
                        background-color: #fff;
                        padding: 20px;
                        border-radius: 8px;
                        box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
                        max-width: 400px;
                        margin: 20px auto;
                    }
                    h2 {
                        color: #333;
                        text-align: center;
                    }
                    label {
                        display: block;
                        margin-bottom: 5px;
                        color: #555;
                    }
                    .ssid,
                    .password-container {
                        width: calc(100% - 22px);
                        padding: 10px;
                        margin-bottom: 15px;
                        border: 1px solid #ddd;
                        border-radius: 4px;
                    }

                    .password-container {
                        padding: 0 0;
                        width: 100%;
                    }
                    .password-focus {
                        border: 2px solid #007bff;
                    }

                    .password-text {
                        display: inline-block;
                    }
                    .hidden {
                        display: none;
                    }
                    #password {
                        border: 0;
                        padding: 10px;
                        width: 80%;
                        height: 100%;
                    }
                    #password:focus-visible {
                        outline: 0;
                    }
                    button {
                        background-color: #007bff;
                        color: white;
                        padding: 10px 15px;
                        border: none;
                        border-radius: 4px;
                        cursor: pointer;
                        width: 100%;
                        font-size: 16px;
                    }
                    button:hover {
                        background-color: #0056b3;
                    }
                    p {
                        text-align: center;
                        color: #666;
                        font-size: 0.9em;
                    }
                    .status {
                        text-align: center;
                        margin-top: 15px;
                        font-weight: bold;
                    }
                    #networksList {
                        list-style: none;
                        padding: 0;
                        margin-top: 15px;
                        border-top: 1px solid #eee;
                    }
                    #networksList li {
                        padding: 8px 0;
                        border-bottom: 1px solid #eee;
                        cursor: pointer;
                    }
                    #networksList li:hover {
                        background-color: #f9f9f9;
                    }
                </style>
            </head>
            <body>
                <div class="container">
                    <h2>CardiacAI WiFi Setup</h2>
                    <form id="wifiForm">
                        <label for="ssid">WiFi SSID:</label>
                        <input
                            type="text"
                            class="ssid"
                            id="ssid"
                            name="ssid"
                            tabindex="0"
                            required
                        /><br />
                        <label for="password">WiFi Password:</label>
                        <div class="password-container" tabindex="0">
                            <input type="password" id="password" name="password" required />
                            <p id="show-password" class="password-text">Show</p>
                            <p id="hide-password" class="password-text hidden">Hide</p>
                        </div>
                        <br />
                        <button type="submit">Save & Connect</button>
                    </form>
                    <p class="status" id="responseStatus"></p>

                    <hr />
                    <h3>Available WiFi Networks</h3>
                    <button id="scanButton">Scan Networks</button>
                    <p id="scanStatus"></p>
                    <ul id="networksList"></ul>
                </div>
                <script>
                    const showPassword = document.getElementById("show-password");
                    const hidePassword = document.getElementById("hide-password");
                    const passwordContainer = document.querySelector(".password-container");
                    document
                        .getElementById("wifiForm")
                        .addEventListener("submit", async function (event) {
                            event.preventDefault(); // Prevent default form submission
                            const ssid = document.getElementById("ssid").value;
                            const password = document.getElementById("password").value;
                            const responseStatus = document.getElementById("responseStatus");

                            responseStatus.textContent = "Saving...";
                            responseStatus.style.color = "orange";

                            try {
                                // Send credentials as JSON to the /setupWifi endpoint
                                const response = await fetch("/setupWifi", {
                                    method: "POST",
                                    headers: {
                                        "Content-Type": "application/json",
                                    },
                                    body: JSON.stringify({ ssid: ssid, password: password }),
                                });
                                const data = await response.json(); // Parse the JSON response from the server

                                if (response.ok) {
                                    // Check if the HTTP status code is 2xx (success)
                                    responseStatus.textContent =
                                        data.message || "Credentials saved! Connecting to WiFi...";
                                    responseStatus.style.color = "green";
                                    // You might want to automatically redirect the user or provide further instructions here
                                } else {
                                    responseStatus.textContent =
                                        data.error || "Failed to save credentials.";
                                    responseStatus.style.color = "red";
                                }
                            } catch (error) {
                                console.error("Error:", error); // Log any network or fetch errors
                                responseStatus.textContent = "Network error. Try again.";
                                responseStatus.style.color = "red";
                            }
                        });

                    document
                        .getElementById("scanButton")
                        .addEventListener("click", async function () {
                            const networksList = document.getElementById("networksList");
                            const scanStatus = document.getElementById("scanStatus");
                            scanStatus.textContent = "Scanning...";
                            networksList.innerHTML = ""; // Clear previous list

                            try {
                                const response = await fetch("/scanNetworks");
                                const networks = await response.json();

                                if (networks.length === 0) {
                                    scanStatus.textContent = "No networks found.";
                                } else {
                                    scanStatus.textContent = `Found ${networks.length} networks:`;
                                    networks.forEach((net) => {
                                        const li = document.createElement("li");
                                        li.textContent = `${net.ssid} (RSSI: ${net.rssi})`;
                                        li.dataset.ssid = net.ssid; // Store SSID in data attribute
                                        li.addEventListener("click", function () {
                                            document.getElementById("ssid").value = this.dataset.ssid;
                                        });
                                        networksList.appendChild(li);
                                    });
                                }
                            } catch (error) {
                                console.error("Error scanning networks:", error);
                                scanStatus.textContent = "Error scanning networks.";
                            }
                        });

                    // Optional: Trigger a scan on page load
                    document.addEventListener("DOMContentLoaded", function () {
                        document.getElementById("scanButton").click();
                    });
                    passwordContainer.addEventListener("click", (e) => {
                        document.getElementById("password").focus();

                        passwordContainer.classList.add("password-focus");
                    });

                    document.getElementById("password").addEventListener("blur", () => {
                        passwordContainer.classList.remove("password-focus");
                    });

                    showPassword.addEventListener("click", () => {
                        hidePassword.classList.remove("hidden");
                        showPassword.classList.add("hidden");

                        document.getElementById("password").type = "text";
                    });

                    hidePassword.addEventListener("click", () => {
                        showPassword.classList.remove("hidden");
                        hidePassword.classList.add("hidden");

                        document.getElementById("password").type = "password";
                    });
                </script>
            </body>
        </html>
        )rawliteral"); // R"rawliteral(...)rawliteral" allows multi-line string without escaping
               });

    // Define the new /scanNetworks route to perform and return WiFi scan results.
    _server.on("/scanNetworks", HTTP_GET, [this](AsyncWebServerRequest *request)
               { request->send(200, "application/json", getScannedNetworksJson()); });

    // Define the /setupWifi POST route to receive new WiFi credentials.
    _server.on("/setupWifi", HTTP_POST,
               // This handler processes the incoming request body (data), not files.
               // The first lambda is for file uploads, which we don't need here, so it's empty.
               [](AsyncWebServerRequest *request) {}, // No file upload handler
               NULL,                                  // The second NULL is for the upload handler if there were file uploads
               // This lambda processes the received data (JSON payload).
               [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
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

            // Set the flag to indicate that a WiFi switch is requested.
            // The actual mode change will be handled in the main loop.
            _wifiSwitchRequested = true;

            // Send success response FIRST, then set flag for main loop to switch WiFi mode.
            request->send(200, "application/json", "{\"message\":\"WiFi credentials saved! Attempting to connect to WiFi in a moment...\"}"); });

    _server.begin(); // Start the server, making it listen for incoming connections.
    Serial.println("[HotspotWebServer] Web server started on Hotspot mode (port 80).");
}

// Stops the web server.
void HotspotWebServer::end()
{
    _server.end(); // Stops the server instance.
    Serial.println("[HotspotWebServer] Web server stopped.");
}

// Helper function to perform a WiFi scan and return results as a JSON array.
String HotspotWebServer::getScannedNetworksJson()
{
    Serial.println("[HotspotWebServer] Starting WiFi scan...");
    // Scan for WiFi networks. 'true' for hidden networks, 'false' for blocking scan.
    // For a web server, a blocking scan is generally not ideal as it can freeze the server.
    // However, for simplicity here, we use blocking. Consider asynchronous scanning for production.
    int n = WiFi.scanNetworks();
    Serial.printf("[HotspotWebServer] Scan done. Found %d networks.\n", n);

    JsonDocument doc; // Adjust size based on expected number of networks
    JsonArray networksArray = doc.to<JsonArray>();

    for (int i = 0; i < n; ++i)
    {
        JsonObject network = networksArray.add<JsonObject>();
        network["ssid"] = WiFi.SSID(i);
        network["rssi"] = WiFi.RSSI(i); // Signal strength
        network["channel"] = WiFi.channel(i);
        // Add more details like encryption type if needed: WiFi.encryptionType(i)
    }

    String jsonOutput;
    serializeJson(doc, jsonOutput);
    WiFi.scanDelete(); // Clear scan results to free memory
    return jsonOutput;
}

bool HotspotWebServer::isWifiSwitchRequested()
{
    return _wifiSwitchRequested;
}

void HotspotWebServer::resetWifiSwitchRequest()
{
    _wifiSwitchRequested = false;
}
