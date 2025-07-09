#include <Arduino.h>               
#include "AD8232_ECG.h"            
#include "ECGWebSocket.h"   
#include "PeripheralHandler.h"    
#include "HotspotWebServer.h"    
#include <DNSServer.h>

// AD8232 ECG Sensor Pins
const int ECG_OUTPUT_PIN = 32;
const int LO_PLUS_PIN = 14;
const int LO_MINUS_PIN = 35;

// RGB LED Pins (Common Cathode assumed: HIGH turns segment ON)
const int RGB_RED_PIN = 18;
const int RGB_GREEN_PIN = 4;
const int RGB_BLUE_PIN = 15;
// Button Pin (configured as INPUT_PULLUP: LOW when pressed)
const int BUTTON_PIN = 19;

// --- WebSocket Server Details ---
const char* WS_SERVER_IP = "api.cardiacai.tech";
const uint16_t WS_SERVER_PORT = 8000;
const char* WS_SERVER_PATH = "/api/ws/device?device_id=cardiacai-123";

const char *domainName = "cardiacai.local"
const byte DNS_PORT = 53;
DNSServer dnsServer;

const char* HOTSPOT_SSID = "CardiacAI";
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


bool startDNS() {
    dnsServer.start(DNS_PORT, "*", WirelessCommunication.getIP());
    Serial.println("DNS server started. Redirecting all requests to " + String(domainName));
    return true;
}

bool stopDNS() {
    dnsServer.stop();
    Serial.println("DNS server stopped.");
    return true;
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n--- ECG Machine Booting Up ---");

    wirelessComm.begin();
    ecgSensor.begin();
    ledHandler.begin();

    clicks = ledHandler.getAndResetClickCount();

    delay(2000);
    ledHandler.setRed(1);
    delay(500);
    ledHandler.setRed(0);
    ledHandler.setGreen(1);
    delay(500);
    ledHandler.setGreen(0);
    ledHandler.setBlue(1);
    delay(500);
    ledHandler.setBlue(0);
    delay(500);
    ledHandler.setRed(1);

    Serial.println("Attempting to connect to saved WiFi...");
    wirelessComm.activateWiFiMode();                          // This will try to connect to previously saved WiFi

    unsigned long wifiConnectStartTime = millis();
    if (wirelessComm.isConnected()) {
        ledHandler.setGreen(1);
        Serial.println("\nWiFi connected successfully!");
        Serial.println("Attempting WebSocket connection...");
        if(wsClient.connect(WS_SERVER_IP, WS_SERVER_PORT, WS_SERVER_PATH)) {
            ledHandler.setBlue(1);
            Serial.println("WebSocket connected successfully!");
        } else {
            ledHandler.setBlue(0);
            Serial.println("Failed to connect to WebSocket server.");
        }
    }
    else {
        ledHandler.setGreen(0);
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
            ledHandler.setBlue(1);
            ledHandler.setGreen(1);
           } else {
            ledHandler.setBlue(0);
            Serial.println("Failed to connect to WebSocket server after WiFi switch.");
            ledHandler.setGreen(1);
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
            ledHandler.setGreen(1); 
            // Attempt to connect to WebSocket server
           if (wsClient.connect(WS_SERVER_IP, WS_SERVER_PORT, WS_SERVER_PATH)) {
            Serial.println("WebSocket connected successfully after WiFi switch!");
            ledHandler.setBlue(1);
           } else {
            Serial.println("Failed to connect to WebSocket server after WiFi switch.");
            ledHandler.setBlue(0);
           }
        }
        else {
            Serial.println("Failed to connect to WiFi. Will keep trying after Some time.");
            ledHandler.setGreen(0);

        }
    } else if (clicks == 2) { // Double click: Force switch to Hotspot mode
        Serial.println("Double click detected! Activating Hotspot mode...");
        wsClient.disconnect();
        wirelessComm.activateHotspotMode(HOTSPOT_SSID, HOTSPOT_PASSWORD);
        ledHandler.setBlue(0);
        ledHandler.toggleGreenSixTimes(300); // Blink green LED 6 times
        if (!hotspotServerActive) {
            hotspotServer.begin();
            hotspotServerActive = true;
        }
    }

    // --- ECG Data Reading and Transmission ---
    // Only perform ECG operations if the device is NOT in Hotspot mode.
    // In Hotspot mode, the device will be waiting for configuration requests.
    String localMode = wirelessComm.getLocalMode();
    if (localMode == "wifi") {
        int ecgValue = ecgSensor.readECG();
        Serial.println(ecgValue);

        // if (ecgSensor.isSensorConnected()) { // Check if ECG leads are properly attached
        wsClient.sendECGValue(ecgValue);
    }
        else {
        ledHandler.toggleGreenSixTimes(300); // Blink green LED 6 times if WiFi is not connected
    }

    if (localMode == "wifi" && !wirelessComm.isConnected() && (millis() - lastWiFiReconnectAttempt > RECONNECT_INTERVAL_MS)) {
        ledHandler.setGreen(0);
        ledHandler.setBlue(0);
        Serial.println("WiFi lost or not connected. Re-attempting WiFi connection...");
        wirelessComm.activateWiFiMode();
        lastWiFiReconnectAttempt = millis();
        if (wirelessComm.isConnected()) {
            ledHandler.setGreen(1);
            Serial.println("Reconnected to WiFi successfully!");
        }
    }
    if (localMode == "wifi" && wirelessComm.isConnected() && !wsClient.isConnected() && (millis() - lastWsReconnectAttempt > RECONNECT_INTERVAL_MS)) {
        Serial.println("WebSocket lost or not connected. Re-attempting WebSocket connection...");
        ledHandler.setGreen(1);
        wsClient.connect(WS_SERVER_IP, WS_SERVER_PORT, WS_SERVER_PATH);
        lastWsReconnectAttempt = millis();
        if (wsClient.isConnected()) {
            ledHandler.setBlue(1);
            Serial.println("Reconnected to WebSocket successfully!");
        } else {
            ledHandler.setBlue(0);
            Serial.println("Failed to reconnect to WebSocket.");
        }
    }
    delay(100);
}
