// WirelessCommunication.cpp
// This file implements the methods defined in the WirelessCommunication class.
#include "WirelessCommunication.h"

WirelessCommunication::WirelessCommunication() {
    // Load credentials from Preferences at startup
    // This makes the latest saved credentials available immediately upon class instantiation.
    ssid = getSavedSSID();
    password = getSavedPassword();
}

void WirelessCommunication::activateWiFiMode() {
    // Check if credentials exist before attempting to connect
    if (ssid.isEmpty() || password.isEmpty()) {
        Serial.println("No saved WiFi credentials found!");
        return; // Exit if no credentials to connect with
    }

    // Disconnect from any previous connections and set WiFi mode to Station
    WiFi.disconnect(true); // true parameter releases IP and cleans up
    WiFi.mode(WIFI_STA);
    delay(100); // Small delay to allow mode change to take effect

    // Attempt to connect to the saved WiFi network
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.print("Connecting to WiFi");

    // Connection retry loop
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 40) { // Max 40 retries (4 seconds)
        delay(100); // Wait for 100ms between retries
        Serial.print(".");
        retries++;
    }

    // Check final connection status
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        setMode("wifi"); // Update the stored mode to "wifi"
    } else {
        Serial.println("\nFailed to connect to WiFi");
        // No need to set mode to "off" explicitly here, as the previous mode persists if connection fails.
        // It might be useful to explicitly handle this if a "failed_wifi" mode is desired.
    }
}

bool WirelessCommunication::isConnected() {
    // Return the current WiFi connection status
    return (WiFi.status() == WL_CONNECTED);
}

bool WirelessCommunication::connectWiFi() {
    // If already connected, no need to reconnect
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }

    // Ensure credentials exist before attempting connection
    if (ssid.isEmpty() || password.isEmpty()) {
        Serial.println("No saved WiFi credentials found!");
        return false;
    }

    // Attempt to connect to the saved WiFi network
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.print("Connecting to WiFi");

    // Connection retry loop (longer timeout than activateWiFiMode for re-connection attempts)
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 50) { // Max 50 retries (5 seconds)
        delay(100);
        Serial.print(".");
        retries++;
    }

    // Check final connection status and update mode if successful
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("\nWiFi connected: ");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        setMode("wifi");
        return true;
    } else {
        Serial.println("\nFailed to connect to WiFi");
        return false;
    }
}

void WirelessCommunication::activateHotspotMode(const char *ssid_ap, const char *password_ap) {
    // Disconnect any existing connections and set mode to Access Point
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);
    delay(100);

    bool result;
    // Start SoftAP with or without a password based on input
    if (password_ap && strlen(password_ap) >= 8) { // Password must be at least 8 characters for WPA2
        result = WiFi.softAP(ssid_ap, password_ap);
    } else {
        result = WiFi.softAP(ssid_ap); // Open access point if no password or too short
    }

    // Report hotspot activation status
    if (result) {
        Serial.println("Hotspot activated");
        Serial.print("Hotspot IP: ");
        Serial.println(WiFi.softAPIP());
        setMode("hotspot"); // Update the stored mode to "hotspot"
    } else {
        Serial.println("Failed to start hotspot");
    }
}

void WirelessCommunication::turnOffWireless() {
    // Disconnect and turn off WiFi completely
    WiFi.disconnect(true);
    delay(100); // Give time for disconnect to process
    setMode("off"); // Update the stored mode to "off"
    Serial.println("Wireless turned off");
}

void WirelessCommunication::saveWiFiCredentials(const char *_ssid, const char *_password) {
    // Open Preferences in read-write mode and save SSID and password
    prefs.begin(WIFI_CREDS, false); // false means read-write mode
    prefs.putString("ssid", _ssid);
    prefs.putString("password", _password);
    prefs.end(); // Close Preferences

    // Update the in-memory variables immediately
    ssid = String(_ssid);
    password = String(_password);

    Serial.println("WiFi credentials saved");
}

String WirelessCommunication::getSavedSSID() {
    // Open Preferences in read-only mode and retrieve SSID
    prefs.begin(WIFI_CREDS, true); // true means read-only mode
    String saved_ssid = prefs.getString("ssid", ""); // Default to empty string if not found
    prefs.end();
    return saved_ssid;
}

String WirelessCommunication::getSavedPassword() {
    // Open Preferences in read-only mode and retrieve password
    prefs.begin(WIFI_CREDS, true);
    String saved_password = prefs.getString("password", "");
    prefs.end();
    return saved_password;
}

String WirelessCommunication::getMode() {
    // Open Preferences in read-only mode and retrieve the current mode
    prefs.begin(WIFI_CREDS, true);
    String mode = prefs.getString("mode", "off"); // Default to "off" if not found
    prefs.end();
    return mode;
}

void WirelessCommunication::setMode(const char *mode) {
    // Open Preferences in read-write mode and save the current wireless mode
    prefs.begin(WIFI_CREDS, false);
    prefs.putString("mode", mode);
    prefs.end();
}