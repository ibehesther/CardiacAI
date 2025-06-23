#include "WirelessCommunication.h"

WirelessCommunication::WirelessCommunication() {
    Serial.println("[WirelessCommunication] Initialized");
}

void WirelessCommunication::begin() {
    // Initialize Preferences storage for WiFi credentials
    prefs.begin(WIFI_CREDS, false);

    // Load saved WiFi credentials
    ssid = getSavedSSID();
    password = getSavedPassword();

    // Set initial mode to "off" if not already set
    if (getMode().isEmpty()) {
        setMode("off");
        mode = "off"; // Initialize mode variable
    }
    else {
        mode = getMode();
    }

    Serial.println("[WirelessCommunication] Begin completed");
}

void WirelessCommunication::activateWiFiMode() {

    // Check if credentials exist before attempting to connect
    if (ssid.isEmpty() || password.isEmpty()) {
        Serial.println("No saved WiFi credentials found!");
        return;
    }

    // Disconnect from any previous connections and set WiFi mode to Station
    WiFi.disconnect(true); // true parameter releases IP and cleans up
    WiFi.mode(WIFI_STA);
    delay(100); // Small delay to allow mode change to take effect

    // Attempt to connect to the saved WiFi network
    WiFi.begin(ssid.c_str(), password.c_str());
    // WiFi.begin("Electrify", "Victory111");
    Serial.print("Connecting to WiFi/nSSID:");
    Serial.print(ssid);

    // Connection retry loop
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 40) { // Max 40 retries (4 seconds)
        delay(100); // Wait for 100ms between retries
        Serial.print(".");
        retries++;
    }
    setMode("wifi"); // Update the stored mode to "wifi"
    mode = "wifi"; // Update the in-memory mode variable
    // Check final connection status
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFailed to connect to WiFi");
    }
}

bool WirelessCommunication::isConnected() {
    // Return the current WiFi connection status
    return (WiFi.status() == WL_CONNECTED);
}

bool WirelessCommunication::connectWiFi() {
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
    setMode("wifi");

    // Check final connection status and update mode if successful
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("\nWiFi connected: ");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
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
        mode = "hotspot"; // Update the in-memory mode variable
    } else {
        Serial.println("Failed to start hotspot");
    }
}

void WirelessCommunication::turnOffWireless() {
    WiFi.disconnect(true);
    delay(100);
    setMode("off");
    mode = "off"; // Update the in-memory mode variable
    Serial.println("Wireless turned off");
}

void WirelessCommunication::saveWiFiCredentials(const char *_sssid, const char *_passsword) {
    prefs.begin(WIFI_CREDS, false);
    prefs.putString("ssid", _sssid);
    prefs.putString("password", _passsword);
    prefs.end();

    // Update the in-memory variables immediately
    ssid = String(_sssid);
    password = String(_passsword);

    Serial.println("WiFi credentials saved");
}

String WirelessCommunication::getSavedSSID() {
    prefs.begin(WIFI_CREDS, false);
    String saved_ssid = prefs.getString("ssid", "");
    prefs.end();

    Serial.print("SSID: ");
    Serial.println(saved_ssid);
    return saved_ssid;
}

String WirelessCommunication::getSavedPassword() {
    prefs.begin(WIFI_CREDS, false);
    String saved_password = prefs.getString("password", "");
    prefs.end();

    Serial.print("PASSWORD: ");
    Serial.println(saved_password);
    return saved_password;
}

String WirelessCommunication::getMode() {
    prefs.begin(WIFI_CREDS, false);
    String mmode = prefs.getString("mode", "off");
    prefs.end();
    return mmode;
}

String WirelessCommunication::getLocalMode() {
    return mode; 
}

void WirelessCommunication::setMode(const char *mode) {
    prefs.begin(WIFI_CREDS, false);
    prefs.putString("mode", mode);
    prefs.end();
}