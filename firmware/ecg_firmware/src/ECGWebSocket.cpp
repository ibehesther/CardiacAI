// ECGWebSocketClient.cpp
// This file implements the methods defined in the ECGWebSocketClient class.

#include "ECGWebSocket.h" // Include the corresponding header file

// Use the websockets namespace for convenience, as it's common with this library.
using namespace websockets;

ECGWebSocketClient::ECGWebSocketClient() {
    // Set up the message and event callbacks using lambdas that capture 'this'.
    // This allows the callbacks to call member functions of this class.
    _webSocket.onMessage([this](WebsocketsMessage message) {
        this->onWsMessage(message);
    });

    _webSocket.onEvent([this](WebsocketsEvent event, String data) {
        this->onWsEvent(event, data);
    });
}

bool ECGWebSocketClient::connect(const char *ip, uint16_t port, const char *path) {
    if (_webSocket.available()) { // Check if already connected using the library's method
        Serial.println("[WS] Already connected.");
        return true;
    }

    Serial.printf("[WS] Attempting to connect to ws://%s:%u%s\n", ip, port, path);
    // The connect method directly tries to establish the connection.
    // It returns true if the connection process was started, false if it failed immediately.
    // The actual connection status will be confirmed by onWsEvent.
    return _webSocket.connect(ip, port, path);
}

bool ECGWebSocketClient::sendECGValue(int ecgValue) {
    if (_webSocket.available()) { // Check connection status using the library's method
        String data = String(ecgValue); // Convert integer to String
        _webSocket.send(data);          // Send the data as a text message
        // Serial.printf("[WS] Sent ECG value: %d\n", ecgValue); // Uncomment for debugging
        return true;
    } else {
        Serial.println("[WS] Not connected, cannot send ECG value.");
        return false;
    }
}

bool ECGWebSocketClient::isConnected() {
    // Return the current connection status from the library.
    return _webSocket.available();
}

void ECGWebSocketClient::disconnect() {
    if (_webSocket.available()) {
        _webSocket.close(); // Use close() for explicit disconnection
        Serial.println("[WS] Disconnected explicitly.");
    }
}

void ECGWebSocketClient::loop() {
    // This method must be called frequently (e.g., in Arduino's loop())
    // to process WebSocket events like sending/receiving pings, handling data,
    // and managing the connection state.
    _webSocket.poll();
}

// Private methods for handling WebSocket events
void ECGWebSocketClient::onWsMessage(WebsocketsMessage message) {
    // Handle incoming text messages from the server.
    Serial.print("[WS] Got Message: ");
    Serial.println(message.data());
    // Add any specific logic here if your server sends commands or acknowledgements.
}

void ECGWebSocketClient::onWsEvent(WebsocketsEvent event, String data) {
    // Handle various WebSocket lifecycle events.
    switch (event) {
        case WebsocketsEvent::ConnectionOpened:
            Serial.println("[WS] Connnection Opened");
            // No need to manually set _connected flag, as _webSocket.available() handles it.
            break;
        case WebsocketsEvent::ConnectionClosed:
            Serial.println("[WS] Connnection Closed");
            break;
        case WebsocketsEvent::GotPing:
            Serial.println("[WS] Got a Ping!");
            break;
        case WebsocketsEvent::GotPong:
            Serial.println("[WS] Got a Pong!");
            break;
        default:
            // Handle other event types if necessary
            break;
    }
}
