// PeripheralHandler.cpp
// This file implements the methods defined in the PeripheralHandler class.

#include "PeripheralHandler.h" // Include the corresponding header file

// Initialize the global pointer to null initially
PeripheralHandler* globalPeripheralHandler = nullptr;

// Constructor definition
PeripheralHandler::PeripheralHandler(int redPin, int greenPin, int bluePin, int buttonPin) :
    _redPin(redPin),
    _greenPin(greenPin),
    _bluePin(bluePin),
    _buttonPin(buttonPin),
    _button({.lastPressTime = 0, .nPresses = 0})
{
    // Set the global pointer to this instance.
    // This allows the static ISR function to call a member function of this object.
    globalPeripheralHandler = this;
}

// Initializes pins for LED and button
void PeripheralHandler::begin() {
    pinMode(_redPin, OUTPUT);
    pinMode(_greenPin, OUTPUT);
    pinMode(_bluePin, OUTPUT);
    turnOff();

    pinMode(_buttonPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(_buttonPin), onButtonInterrupt, FALLING);

    Serial.println("[PeripheralHandler] Initialized LED and Button.");
}

void PeripheralHandler::setBlue(int bState) {
    digitalWrite(_bluePin, bState);
    // Serial.println("[LED] Set to Blue (WiFi Connected, WS Connected)");
}

void PeripheralHandler::setGreen(int gState) {
    digitalWrite(_greenPin, gState);
    // Serial.println("[LED] Set to Green (WiFi Connected, WS Disconnected)");
}

void PeripheralHandler::setRed(int rState) {
    digitalWrite(_redPin, rState);
    // Serial.println("[LED] Set to Red (Hotspot Mode)");
}

void PeripheralHandler::turnOff() {
    _setColor(LOW, LOW, LOW);
    // Serial.println("[LED] Turned Off");
}

void PeripheralHandler::toggleGreenSixTimes(unsigned long delayTime = 300) {
  bool originalState = digitalRead(_greenPin);

  for (int i = 0; i < 6; i++) {
    digitalWrite(_greenPin, !digitalRead(_greenPin));
    delay(delayTime);
  }

  // Ensure the pin returns to its original state
  digitalWrite(_greenPin, originalState);
}


// Helper function to set individual LED states
void PeripheralHandler::_setColor(int rState, int gState, int bState) {
    digitalWrite(_redPin, rState);
    digitalWrite(_greenPin, gState);
    digitalWrite(_bluePin, bState);
}

unsigned int PeripheralHandler::getAndResetClickCount() {
    unsigned int clicks = 0;
    if (_button.nPresses > 0) {
        unsigned long currentTime = millis();
        if (currentTime - _button.lastPressTime > MULTI_CLICK_TIMEOUT_MS) {
            // Disable interrupts briefly to safely read and reset volatile variables
            noInterrupts();
            clicks = _button.nPresses;
            _button.nPresses = 0;
            interrupts();

            Serial.printf("[PeripheralHandler] Detected %u clicks.\n", clicks);
            return clicks;
        }
        else {
            // If we are still within the timeout, we return the current count without resetting
            Serial.printf("[PeripheralHandler] Clicks in progress: %u\n", _button.nPresses);
            delay(MULTI_CLICK_TIMEOUT_MS);
            clicks = _button.nPresses; // Capture the current count
            _button.nPresses = 0;
            return clicks;
        }
    }
    return 0;
}


// ISR function - called directly by the hardware interrupt
void IRAM_ATTR onButtonInterrupt() {
    if (globalPeripheralHandler != nullptr) {
        globalPeripheralHandler->_handleButtonPress();
    }
    else {
        Serial.println("[PeripheralHandler] Error: Global instance pointer is null in ISR.");
    }
}

// Private method called by the ISR to handle button press logic
void PeripheralHandler::_handleButtonPress() {
    unsigned long currentTime = millis();
    if (currentTime - _button.lastPressTime > CLICK_DEBOUNCE_MS) {
        _button.lastPressTime = currentTime;
        _button.nPresses = _button.nPresses + 1;
        Serial.printf("[PeripheralHandler] ISR: Press detected, count = %u\n", _button.nPresses);
    }
}