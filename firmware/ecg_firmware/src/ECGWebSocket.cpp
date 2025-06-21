// ECGWebSocketClient.cpp
// This file implements the methods defined in the ECGWebSocketClient class.

#include "ECGWebSocket.h"

using namespace websockets;

ECGWebSocketClient::ECGWebSocketClient() {
    _webSocket.onMessage([this](WebsocketsMessage message) {
        this->onWsMessage(message);
    });

    _webSocket.onEvent([this](WebsocketsEvent event, String data) {
        this->onWsEvent(event, data);
    });
}

bool ECGWebSocketClient::connect(const char *ip, uint16_t port, const char *path) {
    if (_webSocket.available()) { // Check if already connected
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
    if (_webSocket.available()) {                       // Check connection status
        String data = String(ecgValue);                 // Convert integer to String
        _webSocket.send(data);                          // Send the data as a text message
        return true;
    } else {

        /* We should probably try reconnecting here */
        Serial.println("[WS] Not connected, cannot send ECG value.");
        return false;
    }
}

bool ECGWebSocketClient::isConnected() {
    return _webSocket.available();
}

void ECGWebSocketClient::disconnect() {
    if (_webSocket.available()) {
        _webSocket.close();
        Serial.println("[WS] Disconnected explicitly.");
    }
}

void ECGWebSocketClient::loop() {
    // This method must be called frequently in a running loop
    // to process WebSocket events like sending/receiving pings, handling data,
    // and managing the connection state.
    _webSocket.poll();
}

// Private methods for handling WebSocket events
void ECGWebSocketClient::onWsMessage(WebsocketsMessage message) {
    // Handle incoming text messages from the server.
    Serial.print("[WS] Got Message: ");
    Serial.println(message.data());
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
            break;
    }
}
