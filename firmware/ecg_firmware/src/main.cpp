#include <Arduino.h>               
#include "AD8232_ECG.h"            
#include "ECGWebSocket.h"   
#include "PeripheralHandler.h"    
#include "HotspotWebServer.h"    


// AD8232 ECG Sensor Pins
const int ECG_OUTPUT_PIN = 26;
const int LO_PLUS_PIN = 14;
const int LO_MINUS_PIN = 33;

// RGB LED Pins (Common Cathode assumed: HIGH turns segment ON)
const int RGB_RED_PIN = 18;
const int RGB_GREEN_PIN = 4;
const int RGB_BLUE_PIN = 15;
// Button Pin (configured as INPUT_PULLUP: LOW when pressed)
const int BUTTON_PIN = 19;

// --- WebSocket Server Details ---
const char* WS_SERVER_IP = "192.168.0.149";
const uint16_t WS_SERVER_PORT = 8000;
const char* WS_SERVER_PATH = "/api/ws/device?device_id=cardiacai-123";

// --- HotSpot AP Details ---
const char* HOTSPOT_SSID = "ECG_Hotspot_AP";
const char* HOTSPOT_PASSWORD = "ecg12345";

// Attempt reconnect every 10 seconds
const unsigned long RECONNECT_INTERVAL_MS = 10000;

unsigned long lastWiFiReconnectAttempt = 0;
unsigned long lastWsReconnectAttempt = 0;
bool hotspotServerActive = false;
unsigned int clicks;

AD8232_ECG ecgSensor(ECG_OUTPUT_PIN, LO_PLUS_PIN, LO_MINUS_PIN);
WirelessCommunication wirelessComm;
ECGWebSocketClient wsClient;
PeripheralHandler ledHandler(RGB_RED_PIN, RGB_GREEN_PIN, RGB_BLUE_PIN, BUTTON_PIN);
HotspotWebServer hotspotServer(wirelessComm);


void setup() {
    Serial.begin(115200);
    Serial.println("\n--- ECG Machine Booting Up ---");

    wirelessComm.begin();
    ecgSensor.begin();
    ledHandler.begin();

    clicks = ledHandler.getAndResetClickCount();

    delay(2000);
    ledHandler.setBlue();
    delay(500);
    ledHandler.setGreen();
    delay(500);
    ledHandler.setRed();
    delay(500);
    ledHandler.turnOff();

    Serial.println("Attempting to connect to saved WiFi...");
    wirelessComm.activateWiFiMode();                          // This will try to connect to previously saved WiFi

    unsigned long wifiConnectStartTime = millis();
    if (wirelessComm.isConnected()) {
        ledHandler.setGreen();
        Serial.println("\nWiFi connected successfully!");
        Serial.println("Attempting WebSocket connection...");
        if(wsClient.connect(WS_SERVER_IP, WS_SERVER_PORT, WS_SERVER_PATH)) {
            ledHandler.setBlue();
            Serial.println("WebSocket connected successfully!");
        } else {
            Serial.println("Failed to connect to WebSocket server.");
        }
    }
    else {
        ledHandler.turnOff();
        Serial.println("\nFailed to connect to WiFi. Will keep trying...");
    }
    // } else {
    //     Serial.println("\nFailed to connect to WiFi. Entering Hotspot mode...");
    //     wirelessComm.activateHotspotMode(HOTSPOT_SSID, HOTSPOT_PASSWORD);
    //     hotspotServer.begin();
    //     hotspotServerActive = true;
    //     ledHandler.setRed();
    // }

    Serial.println("Setup complete. Starting main loop.");
}

void loop() {
    // Refresh connection
    wsClient.loop();

    // --- Handle WiFi Switch Request from Hotspot Web Server ---
    if (hotspotServer.isWifiSwitchRequested()) {
        Serial.println("WiFi switch requested by web interface. Attempting to connect to new WiFi...");
        hotspotServer.resetWifiSwitchRequest();
        if (hotspotServerActive) {
            hotspotServer.end();
            hotspotServerActive = false;
        }
        wirelessComm.activateWiFiMode();
        if (wirelessComm.isConnected()) {
           if (wsClient.connect(WS_SERVER_IP, WS_SERVER_PORT, WS_SERVER_PATH)) {
            Serial.println("WebSocket connected successfully after WiFi switch!");
            ledHandler.setBlue();
           } else {
            Serial.println("Failed to connect to WebSocket server after WiFi switch.");
            ledHandler.setGreen();
           };
        }
    }


    clicks = ledHandler.getAndResetClickCount();
    if (clicks == 1) { // Single click: Attempt to switch to WiFi Station mode and connect to WebSocket
        Serial.println("Single click detected! Attempting to activate WiFi mode...");
        if (hotspotServerActive) {
            hotspotServer.end();
            hotspotServerActive = false;
        }
        wirelessComm.activateWiFiMode();
        if (wirelessComm.isConnected()) {
           if (wsClient.connect(WS_SERVER_IP, WS_SERVER_PORT, WS_SERVER_PATH)) {
            Serial.println("WebSocket connected successfully after WiFi switch!");
            ledHandler.setBlue();
           } else {
            Serial.println("Failed to connect to WebSocket server after WiFi switch.");
            ledHandler.setGreen();
           }
        }
        else {
            Serial.println("Failed to connect to WiFi. Will keep trying after Some time.");
            ledHandler.turnOff();

        }
    } else if (clicks == 2) { // Double click: Force switch to Hotspot mode
        Serial.println("Double click detected! Activating Hotspot mode...");
        wsClient.disconnect();
        wirelessComm.activateHotspotMode(HOTSPOT_SSID, HOTSPOT_PASSWORD);
        if (!hotspotServerActive) {
            hotspotServer.begin();
            hotspotServerActive = true;
        }
        ledHandler.setRed();
        
    }

    // --- ECG Data Reading and Transmission ---
    // Only perform ECG operations if the device is NOT in Hotspot mode.
    // In Hotspot mode, the device will be waiting for configuration requests.
    String localMode = wirelessComm.getLocalMode();
    if (localMode == "wifi") {
        int ecgValue = ecgSensor.readECG();
        Serial.print("ECG Value: ");
        Serial.println(ecgValue);

        if (ecgSensor.isSensorConnected()) { // Check if ECG leads are properly attached
            wsClient.sendECGValue(ecgValue);
        } else {
            Serial.println("ECG Leads OFF!");
        }
    }

    if (localMode != "hotspot" && !wirelessComm.isConnected() && (millis() - lastWiFiReconnectAttempt > RECONNECT_INTERVAL_MS)) {
        Serial.println("WiFi lost or not connected. Re-attempting WiFi connection...");
        wirelessComm.activateWiFiMode();
        lastWiFiReconnectAttempt = millis();
        if (wirelessComm.isConnected()) {
            ledHandler.setGreen();
            Serial.println("Reconnected to WiFi successfully!");
        }
    }
    if (localMode != "hotspot" && wirelessComm.isConnected() && !wsClient.isConnected() && (millis() - lastWsReconnectAttempt > RECONNECT_INTERVAL_MS)) {
        Serial.println("WebSocket lost or not connected. Re-attempting WebSocket connection...");
        wsClient.connect(WS_SERVER_IP, WS_SERVER_PORT, WS_SERVER_PATH);
        lastWsReconnectAttempt = millis();
        if (wsClient.isConnected()) {
            ledHandler.setBlue();
            Serial.println("Reconnected to WebSocket successfully!");
        } else {
            ledHandler.setGreen();
            Serial.println("Failed to reconnect to WebSocket.");
        }
    }
    delay(1000);
}
