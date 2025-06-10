// PeripheralHandler.cpp
// This file implements the methods defined in the PeripheralHandler class.
#include "PeripheralHandler.h"

PeripheralHandler::PeripheralHandler(int redPin, int greenPin, int bluePin) {
    _redPin = redPin;
    _greenPin = greenPin;
    _bluePin = bluePin;
}

void PeripheralHandler::begin() {
    // Set all LED pins as OUTPUT.
    pinMode(_redPin, OUTPUT);
    pinMode(_greenPin, OUTPUT);
    pinMode(_bluePin, OUTPUT);
    turnOff(); // Ensure the LED is off initially
}

void PeripheralHandler::setBlue() {
    // Set Red and Green to LOW, Blue to HIGH for a blue color.
    _setColor(LOW, LOW, HIGH);
    Serial.println("[LED] Set to Blue (WebSocket Connected)");
}

void PeripheralHandler::setGreen() {
    // Set Red and Blue to LOW, Green to HIGH for a green color.
    _setColor(LOW, HIGH, LOW);
    Serial.println("[LED] Set to Green (WiFi Connected, WS Disconnected)");
}

void PeripheralHandler::setRed() {
    // Set Green and Blue to LOW, Red to HIGH for a red color.
    _setColor(HIGH, LOW, LOW);
    Serial.println("[LED] Set to Red (Hotspot Mode)");
}

void PeripheralHandler::turnOff() {
    // Turn off all LED segments.
    _setColor(LOW, LOW, LOW);
    Serial.println("[LED] Turned Off");
}

void PeripheralHandler::_setColor(int rState, int gState, int bState) {
    digitalWrite(_redPin, rState);
    digitalWrite(_greenPin, gState);
    digitalWrite(_bluePin, bState);
}

