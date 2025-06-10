// main.cpp
// This file integrates all previously defined classes to create the main logic
// for the portable ECG machine. It handles ECG data reading, wireless communication
// (WiFi STA and Hotspot modes), WebSocket data transmission, RGB LED indication,
// and a local web server for WiFi configuration, all controlled by button presses.

#include <Arduino.h>               // Standard Arduino library
#include "AD8232_ECG.h"            // Include the AD8232_ECG sensor class
#include "WirelessCommunication.h" // Include the WirelessCommunication class
#include "ECGWebSocket.h"    // Include the ECGWebSocketClient class
#include "PeripheralHandler.h"     // Include the PeripheralHandler class for LED and Button
#include "HotspotWebServer.h"      // Include the new HotspotWebServer class

// --- Pin Definitions ---
// IMPORTANT: Adjust these pin numbers to match your actual ESP32 wiring.

// AD8232 ECG Sensor Pins
const int ECG_OUTPUT_PIN = 34; // Analog pin connected to AD8232's OUTPUT (e.g., GPIO34, GPIO35, GPIO32 on ESP32)
const int LO_PLUS_PIN = 33;    // Digital pin connected to AD8232's LO+ (Lead-Off +)
const int LO_MINUS_PIN = 32;   // Digital pin connected to AD8232's LO- (Lead-Off -)

// RGB LED Pins (Common Cathode assumed: HIGH turns segment ON)
const int RGB_RED_PIN = 13;    // Example pin for Red LED (e.g., GPIO13)
const int RGB_GREEN_PIN = 12;  // Example pin for Green LED (e.g., GPIO12)
const int RGB_BLUE_PIN = 14;   // Example pin for Blue LED (e.g., GPIO14)

// Button Pin (configured as INPUT_PULLUP: LOW when pressed)
const int BUTTON_PIN = 27;     // Example pin for a push button (e.g., GPIO27)

// --- WebSocket Server Details ---
// IMPORTANT: Replace with your actual WebSocket server IP, port, and path.
const char* WS_SERVER_IP = "192.168.1.100"; // Example: IP address of your WebSocket server
const uint16_t WS_SERVER_PORT = 8080;         // Example: Port of your WebSocket server
const char* WS_SERVER_PATH = "/ws";          // Example: Path on your WebSocket server

// --- Hotspot Details (used when entering Hotspot mode) ---
const char* HOTSPOT_SSID = "ECG_Hotspot_AP";
const char* HOTSPOT_PASSWORD = "ecg12345"; // Must be at least 8 characters for WPA2

// --- Reconnection Intervals ---
const unsigned long RECONNECT_INTERVAL_MS = 10000; // Attempt reconnect every 10 seconds

// --- Global Class Instances ---
AD8232_ECG ecgSensor(ECG_OUTPUT_PIN, LO_PLUS_PIN, LO_MINUS_PIN);
WirelessCommunication wirelessComm;
ECGWebSocketClient wsClient;
PeripheralHandler ledHandler(RGB_RED_PIN, RGB_GREEN_PIN, RGB_BLUE_PIN, BUTTON_PIN);
// The HotspotWebServer takes a reference to wirelessComm so it can save/change WiFi settings
HotspotWebServer hotspotServer(wirelessComm);

// --- Global variables for non-blocking timing ---
unsigned long lastWiFiReconnectAttempt = 0;
unsigned long lastWsReconnectAttempt = 0;

// --- Global variable to track if the Hotspot server is active ---
bool hotspotServerActive = false;


void setup() {
    Serial.begin(115200); // Initialize serial communication for debugging
    Serial.println("\n--- ECG Machine Booting Up ---");

    // Initialize all peripheral components
    ecgSensor.begin();
    ledHandler.begin(); // Initializes RGB LED and button interrupt

    // Initial wireless setup:
    // Attempt to connect to WiFi using saved credentials (Station mode).
    // If successful, try to connect to the WebSocket server.
    // If WiFi connection fails, activate Hotspot mode.

    Serial.println("Attempting to connect to saved WiFi...");
    wirelessComm.activateWiFiMode(); // This will try to connect to previously saved WiFi

    // Wait for WiFi to connect for a limited time (e.g., 20 seconds) before proceeding.
    // This is a blocking wait during setup to ensure an initial connection attempt.
    unsigned long wifiConnectStartTime = millis();
    while (!wirelessComm.isConnected() && (millis() - wifiConnectStartTime < 20000)) {
        Serial.print(".");
        delay(500); // Small delay to prevent busy-waiting and allow connection to establish
    }

    if (wirelessComm.isConnected()) {
        Serial.println("\nWiFi connected successfully!");
        // If WiFi is connected, try to establish WebSocket connection immediately.
        Serial.println("Attempting WebSocket connection...");
        wsClient.connect(WS_SERVER_IP, WS_SERVER_PORT, WS_SERVER_PATH);
    } else {
        Serial.println("\nFailed to connect to WiFi. Entering Hotspot mode...");
        // If initial WiFi connection fails, activate Hotspot mode for configuration.
        wirelessComm.activateHotspotMode(HOTSPOT_SSID, HOTSPOT_PASSWORD);
        hotspotServer.begin(); // Start the web server immediately in Hotspot mode
        hotspotServerActive = true;
    }

    Serial.println("Setup complete. Starting main loop.");
}

void loop() {
    // This must be called continuously to process WebSocket events
    wsClient.loop();

    // --- Handle WiFi Switch Request from Hotspot Web Server ---
    // This check is performed first to ensure the web server response is sent
    // before any network mode change is initiated.
    if (hotspotServer.isWifiSwitchRequested()) {
        Serial.println("WiFi switch requested by web interface. Attempting to connect to new WiFi...");
        hotspotServer.resetWifiSwitchRequest(); // Reset the flag immediately
        if (hotspotServerActive) { // If server is active, stop it before switching modes
            hotspotServer.end();
            hotspotServerActive = false;
        }
        wirelessComm.activateWiFiMode(); // Initiate connection to the newly saved WiFi
        // If WiFi connects, immediately try WebSocket.
        if (wirelessComm.isConnected()) {
            wsClient.connect(WS_SERVER_IP, WS_SERVER_PORT, WS_SERVER_PATH);
        }
    }


    // --- Button Click Handling for Mode Transitions ---
    unsigned int clicks = ledHandler.getAndResetClickCount();

    if (clicks == 1) { // Single click: Attempt to switch to WiFi Station mode and connect to WebSocket
        Serial.println("Single click detected! Attempting to activate WiFi mode...");
        if (hotspotServerActive) { // If currently in Hotspot mode, stop the server
            hotspotServer.end();
            hotspotServerActive = false;
        }
        wirelessComm.activateWiFiMode(); // Try to connect to saved WiFi credentials
        // If WiFi connects, immediately try WebSocket.
        if (wirelessComm.isConnected()) {
            wsClient.connect(WS_SERVER_IP, WS_SERVER_PORT, WS_SERVER_PATH);
        }
    } else if (clicks == 2) { // Double click: Force switch to Hotspot mode
        Serial.println("Double click detected! Activating Hotspot mode...");
        wirelessComm.activateHotspotMode(HOTSPOT_SSID, HOTSPOT_PASSWORD); // Activate Hotspot with defined credentials
        wsClient.disconnect(); // Disconnect WebSocket client if switching to Hotspot mode
        if (!hotspotServerActive) { // Only start server if not already active
            hotspotServer.begin(); // Start the web server
            hotspotServerActive = true;
        }
    }

    // --- RGB LED Status Indication ---
    // Update the LED color to reflect the current operational state.
    if (wsClient.isConnected()) {
        ledHandler.setBlue(); // Device is in WiFi mode and connected to WebSocket
    } else if (wirelessComm.isConnected() && wirelessComm.getMode() == "wifi") {
        ledHandler.setGreen(); // Device is in WiFi mode but WebSocket is not connected (or trying)
    } else if (wirelessComm.getMode() == "hotspot") {
        ledHandler.setRed(); // Device is in Hotspot (Access Point) mode
    } else {
        // This state implies wireless is off or failed to connect to any mode.
        // Could be a transient state during connection attempts or after turning off wireless.
        ledHandler.turnOff();
    }

    // --- ECG Data Reading and Transmission ---
    // Only perform ECG operations if the device is NOT in Hotspot mode.
    // In Hotspot mode, the device will be waiting for configuration requests.
    if (wirelessComm.getMode() != "hotspot") {
        // Ensure Hotspot server is off if we are not in Hotspot mode
        if (hotspotServerActive) {
            hotspotServer.end();
            hotspotServerActive = false;
        }

        int ecgValue = ecgSensor.readECG(); // Get the raw ECG reading

        if (ecgSensor.isSensorConnected()) { // Check if ECG leads are properly attached
            if (wsClient.isConnected()) {
                // If WebSocket is connected, send the ECG data.
                wsClient.sendECGValue(ecgValue);
                // Serial.println(ecgValue); // Optional: print to Serial Plotter even when sending over WS for debugging
            } else {
                // If WebSocket is NOT connected but leads are ON, print to serial plotter.
                // This ensures you still see data if WS is down but WiFi is up.
                Serial.println(ecgValue);
                // Serial.println("WS Disconnected, plotting locally."); // Debug message
            }
        } else {
            // If ECG leads are off, print a placeholder (e.g., "0") to the plotter
            // and a message to the Serial Monitor.
            Serial.println("0");
            Serial.println("ECG Leads OFF!"); // Message for Serial Monitor
            delay(500); // Add a small delay when leads are off to prevent overwhelming serial output
        }
        // Small delay to control the ECG sampling rate. Adjust as needed.
        delay(10); // Example: 10ms delay gives approx. 100 samples/second (Hz)
    } else {
        // If in Hotspot mode, we are not reading/sending ECG data.
        // Ensure Hotspot server is active if we are in Hotspot mode
        if (!hotspotServerActive) {
            hotspotServer.begin();
            hotspotServerActive = true;
        }
        // A small delay here prevents a tight loop, allowing other tasks (like the web server) to run.
        delay(100);
    }

    // --- Non-Blocking Reconnection Logic ---
    // This section attempts to reconnect to WiFi or WebSocket if they drop,
    // ensuring resilience for the data transmission path.
    // This only runs if we are NOT in Hotspot mode.

    if (wirelessComm.getMode() != "hotspot") {
        // If the device is currently in "wifi" mode (intended state) but not actually connected to WiFi,
        // and enough time has passed, try to reconnect to WiFi.
        if (!wirelessComm.isConnected() && (millis() - lastWiFiReconnectAttempt > RECONNECT_INTERVAL_MS)) {
            Serial.println("WiFi lost or not connected. Re-attempting WiFi connection...");
            wirelessComm.activateWiFiMode(); // This re-initiates connection to saved WiFi
            lastWiFiReconnectAttempt = millis();
        }

        // If WiFi is connected and we are in "wifi" mode, but WebSocket is not connected,
        // and enough time has passed, try to reconnect to WebSocket.
        if (wirelessComm.isConnected() && !wsClient.isConnected() && (millis() - lastWsReconnectAttempt > RECONNECT_INTERVAL_MS)) {
            Serial.println("WebSocket lost or not connected. Re-attempting WebSocket connection...");
            wsClient.connect(WS_SERVER_IP, WS_SERVER_PORT, WS_SERVER_PATH);
            lastWsReconnectAttempt = millis();
        }
    }
}
