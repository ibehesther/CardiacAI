// WirelessCommunication.h
// This header file defines the WirelessCommunication class for managing WiFi and Hotspot connections.

#ifndef WIRELESS_COMMUNICATION_H
#define WIRELESS_COMMUNICATION_H

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

// Namespace for Preferences storage.
#define WIFI_CREDS "wifi_creds"

/**
 * @brief A class to handle wireless communication (WiFi Station and Access Point modes)
 * and persist WiFi credentials using ESP32 Preferences (NVS).
 */
class WirelessCommunication {
public:
    /**
     * @brief Constructor for the WirelessCommunication class.
     * Initializes the class and loads saved WiFi credentials from NVS.
     */
    WirelessCommunication();

    /**
     * @brief Initializes the wireless communication system.
     * This method should be called in setup() to prepare the WiFi module.
     */
    void begin();

    /**
     * @brief Activates WiFi Station (client) mode and attempts to connect to a saved network.
     * If no credentials are saved, it prints a message and returns.
     * It will retry connection multiple times.
     */
    void activateWiFiMode();

    /**
     * @brief Checks if the ESP32 is currently connected to a WiFi network.
     * @return true if connected to WiFi, false otherwise.
     */
    bool isConnected();

    /**
     * @brief Attempts to connect to a previously saved WiFi network.
     * This method can be called to re-establish a connection if lost,
     * or to connect after saving new credentials.
     * @return true if successfully connected, false otherwise.
     */
    bool connectWiFi();

    /**
     * @brief Activates WiFi Access Point (hotspot) mode.
     * @param ssid The SSID (name) of the hotspot.
     * @param password (Optional) The password for the hotspot. Must be at least 8 characters if provided.
     */
    void activateHotspotMode(const char *ssid, const char *password = nullptr);

    /**
     * @brief Turns off all wireless communication (WiFi Station and Access Point).
     */
    void turnOffWireless();

    /**
     * @brief Saves the provided WiFi SSID and password to non-volatile storage (NVS).
     * These credentials will be used for subsequent WiFi connections.
     * @param _ssid The WiFi network SSID to save.
     * @param _password The WiFi network password to save.
     */
    void saveWiFiCredentials(const char *_ssid, const char *_password);

    /**
     * @brief Retrieves the saved WiFi SSID from NVS.
     * @return The saved SSID as a String, or an empty String if none is saved.
     */
    String getSavedSSID();

    /**
     * @brief Retrieves the saved WiFi password from NVS.
     * @return The saved password as a String, or an empty String if none is saved.
     */
    String getSavedPassword();

    /**
     * @brief Retrieves the currently set wireless mode from NVS (e.g., "wifi", "hotspot", "off").
     * @return The current mode as a String.
     */
    String getMode();

    String getLocalMode();

    IPAddress getIP();

private:
    Preferences prefs; // Instance of the Preferences library for NVS access
    String ssid;       // Stored WiFi SSID
    String password;   // Stored WiFi password
    String mode;       // Current wireless mode (e.g., "wifi", "hotspot", "off")

    /**
     * @brief Saves the current wireless mode (e.g., "wifi", "hotspot", "off") to NVS.
     * @param mode A C-style string representing the mode.
     */
    void setMode(const char *mode);
};

#endif // WIRELESS_COMMUNICATION_H