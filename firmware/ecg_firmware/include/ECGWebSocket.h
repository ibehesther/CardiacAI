// ECGWebSocketClient.h
// This header file defines the ECGWebSocketClient class for transmitting ECG data via WebSockets
// using the ArduinoWebsockets library.

#ifndef ECG_WEBSOCKET_CLIENT_H
#define ECG_WEBSOCKET_CLIENT_H

#include <Arduino.h>           // For basic Arduino functions like Serial.println
#include <ArduinoWebsockets.h> // For WebSocket client functionality using ArduinoWebsockets

// Use the websockets namespace for convenience, as it's common with this library.
using namespace websockets;

/**
 * @brief A class to handle WebSocket communication for sending ECG integer data
 * using the ArduinoWebsockets library.
 *
 * This class establishes and maintains a WebSocket connection to a specified server
 * and provides a method to send integer values, typically raw ECG readings.
 */
class ECGWebSocketClient {
public:
    /**
     * @brief Constructor for the ECGWebSocketClient class.
     * Initializes the WebSocket client instance and sets up event handlers.
     */
    ECGWebSocketClient();

    /**
     * @brief Attempts to connect to a WebSocket server.
     * @param ip The IP address or hostname of the WebSocket server.
     * @param port The port number of the WebSocket server.
     * @param path The path on the WebSocket server (e.g., "/ws" or "/").
     * @return true if the connection attempt was initiated, false otherwise.
     */
    bool connect(const char *ip, uint16_t port, const char *path = "/");

    /**
     * @brief Sends an integer ECG value over the WebSocket connection.
     * The integer is converted to a string before transmission.
     * @param ecgValue The integer value representing the ECG reading.
     * @return true if the data was successfully sent, false if not connected.
     */
    bool sendECGValue(int ecgValue);

    /**
     * @brief Checks if the WebSocket client is currently connected to the server.
     * This uses the client's available() method from ArduinoWebsockets.
     * @return true if connected, false otherwise.
     */
    bool isConnected();

    /**
     * @brief Disconnects the WebSocket client from the server.
     */
    void disconnect();

    /**
     * @brief Must be called regularly in the Arduino loop() function.
     * This function processes incoming WebSocket events and maintains the connection.
     * It internally calls websocketsClient.poll().
     */
    void loop();

private:
    WebsocketsClient _webSocket; // The WebSocket client instance from ArduinoWebsockets

    /**
     * @brief Internal handler for incoming WebSocket messages.
     * @param message The received WebSocket message.
     */
    void onWsMessage(WebsocketsMessage message);

    /**
     * @brief Internal handler for various WebSocket events (connection, disconnection, ping, pong).
     * @param event The type of WebSocket event.
     * @param data Additional data associated with the event (e.g., error message).
     */
    void onWsEvent(WebsocketsEvent event, String data);
};

#endif // ECG_WEBSOCKET_CLIENT_H