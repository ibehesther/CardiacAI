#ifndef _NETWORK_HEADER_
#define _NETWORK_HEADER_

#include "main.h"

/**
 * WirelessCommunication class
 * @brief A class to manage wireless communication using WiFi.
 *
 * This class provides methods to activate WiFi mode, create a hotspot,
 * turn off wireless communication, and save/retrieve WiFi credentials.
 * It uses the Preferences library to store credentials in non-volatile memory.
 */
class WirelessCommunication {
private:
    Preferences prefs;
    String ssid;
    String password;

public:
    WirelessCommunication() {
        // Load credentials from Preferences at startup
        ssid = getSavedSSID();
        password = getSavedPassword();
    }

    void activateWiFiMode() {
        if (ssid.isEmpty() || password.isEmpty()) {
            Serial.println("No saved WiFi credentials found!");
            return;
        }

        WiFi.disconnect(true);
        WiFi.mode(WIFI_STA);
        delay(100);
        WiFi.begin(ssid.c_str(), password.c_str());

        Serial.print("Connecting to WiFi");
        int retries = 0;
        while (WiFi.status() != WL_CONNECTED && retries < 40) {
            delay(100);
            Serial.print(".");
            retries++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nWiFi connected");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
            setMode("wifi");
        } else {
            Serial.println("\nFailed to connect to WiFi");
        }
    }

    bool isConnected() {
      return (WiFi.status() == WL_CONNECTED);
    }

    bool connectWiFi() {
        if (WiFi.status() == WL_CONNECTED) {
            return true;
        }

        if (ssid.isEmpty() || password.isEmpty()) {
            Serial.println("No saved WiFi credentials found!");
            return false;
        }

        WiFi.begin(ssid.c_str(), password.c_str());
        Serial.print("Connecting to WiFi");
        int retries = 0;
        while (WiFi.status() != WL_CONNECTED && retries < 50) {
            delay(100);
            Serial.print(".");
            retries++;
        }

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

    void activateHotspotMode(const char* ssid, const char* password = nullptr) {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_AP);
        delay(100);

        bool result;
        if (password && strlen(password) >= 8) {
            result = WiFi.softAP(ssid, password);
        } else {
            result = WiFi.softAP(ssid);
        }

        if (result) {
            Serial.println("Hotspot activated");
            Serial.print("Hotspot IP: ");
            Serial.println(WiFi.softAPIP());
            setMode("hotspot");
        } else {
            Serial.println("Failed to start hotspot");
        }
    }

    void turnOffWireless() {
        WiFi.disconnect(true);
        delay(100);
        setMode("off");
        Serial.println("Wireless turned off");
    }

    void saveWiFiCredentials(const char* _ssid, const char* _password) {
        prefs.begin(WIFI_CREDS, false);
        prefs.putString("ssid", _ssid);
        prefs.putString("password", _password);
        prefs.end();

        ssid = String(_ssid);
        password = String(_password);

        Serial.println("WiFi credentials saved");
    }

    String getSavedSSID() {
        prefs.begin(WIFI_CREDS, true);
        String saved_ssid = prefs.getString("ssid", "");
        prefs.end();
        return saved_ssid;
    }

    String getSavedPassword() {
        prefs.begin(WIFI_CREDS, true);
        String saved_password = prefs.getString("password", "");
        prefs.end();
        return saved_password;
    }

    String getMode() {
        prefs.begin(WIFI_CREDS, true);
        String mode = prefs.getString("mode", "off");
        prefs.end();
        return mode;
    }

private:
    void setMode(const char* mode) {
        prefs.begin(WIFI_CREDS, false);
        prefs.putString("mode", mode);
        prefs.end();
    }
};

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

)rawliteral";

AsyncWebServer server(80);
void startLocalServer()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/html", index_html); });

  server.on("/setupWifi", HTTP_POST, [](AsyncWebServerRequest *request) {},
            NULL, // no file upload handler
            [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
    WirelessCommunication Comm;
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data);

    if (error) {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }

    const char* ssid = doc["ssid"];
    const char* password = doc["password"];
    if (!ssid || !password) {
      request->send(400, "application/json", "{\"error\":\"Missing SSID or password\"}");
      return;
    }
    Comm.saveWiFiCredentials(ssid, password);
    // resetButton.nPresses = 1;
    // resetButton.pressed = true;
    request->send(200, "application/json", "{\"message\":\"WiFi credentials saved\"}");
    
    // Reset ESP here
    });

  server.begin();
  Serial.println("Server started");
}

void stopLocalServer()
{
  server.end();
  Serial.println("Server stopped and WiFi disconnected");
}

#endif // _NETWORK_HEADER_