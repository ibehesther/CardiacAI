// HotspotWebServer.h
// This header file defines the HotspotWebServer class to handle a local web server
// when the ESP32 is in Hotspot (Access Point) mode. This server will provide a web interface
// for configuring WiFi credentials.

#ifndef HOTSPOT_WEB_SERVER_H
#define HOTSPOT_WEB_SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h> 
#include "WirelessCommunication.h"

class HotspotWebServer;

/**
 * @brief A class to manage an asynchronous web server for configuration purposes
 * when the ESP32 is operating as a WiFi Hotspot.
 *
 * This server provides endpoints to:
 * - Serve a basic HTML page for WiFi credential input.
 * - Get device status and information.
 * - Receive and save new WiFi credentials, then trigger a connection attempt.
 */
class HotspotWebServer {
public:
    /**
     * @brief Constructor for the HotspotWebServer class.
     * @param comm A reference to the WirelessCommunication object, allowing the server
     * to save new WiFi credentials and initiate connection attempts.
     */
    HotspotWebServer(WirelessCommunication& comm);

    /**
     * @brief Starts the web server and defines all its routes (endpoints).
     * This method should be called when the device enters Hotspot mode.
     */
    void begin();

    /**
     * @brief Stops the web server.
     * This method should be called when the device exits Hotspot mode.
     */
    void end();

    /**
     * @brief Checks if a WiFi switch (from Hotspot to Station) has been requested by the web interface.
     * @return true if a switch has been requested, false otherwise.
     */
    bool isWifiSwitchRequested();

    /**
     * @brief Resets the flag indicating a WiFi switch request.
     * This should be called by the main loop after handling the switch.
     */
    void resetWifiSwitchRequest();

private:
    AsyncWebServer _server; // The instance of the asynchronous web server
    WirelessCommunication& _wirelessComm; // Reference to the wireless communication handler
    bool _wifiSwitchRequested; // Flag to indicate if a WiFi mode switch has been requested

    /**
     * @brief Helper function to perform a WiFi scan and return results as a JSON array.
     * @return A String containing scanned networks in JSON format.
     */
    String getScannedNetworksJson();
};

#endif // HOTSPOT_WEB_SERVER_H