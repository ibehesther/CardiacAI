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
    // Initialize button struct members
    _button({.pressed = false, .lastPressTime = 0, .nPresses = 0})
{
    // Set the global pointer to this instance.
    // This allows the static ISR function to call a member function of this object.
    globalPeripheralHandler = this;
}

// Initializes pins for LED and button
void PeripheralHandler::begin() {
    // Set LED pins as OUTPUTs
    pinMode(_redPin, OUTPUT);
    pinMode(_greenPin, OUTPUT);
    pinMode(_bluePin, OUTPUT);
    turnOff(); // Ensure the LED is off initially

    // Set button pin as INPUT_PULLUP
    pinMode(_buttonPin, INPUT_PULLUP);
    // Attach the interrupt to the button pin.
    // The ISR (onButtonInterrupt) will be triggered on a FALLING edge,
    // which corresponds to the button being pressed when using INPUT_PULLUP.
    attachInterrupt(digitalPinToInterrupt(_buttonPin), onButtonInterrupt, FALLING);

    Serial.println("[PeripheralHandler] Initialized LED and Button.");
}

// Sets LED to blue
void PeripheralHandler::setBlue() {
    _setColor(LOW, LOW, HIGH); // Common cathode: HIGH for ON
    // Serial.println("[LED] Set to Blue (WebSocket Connected)"); // Uncomment for debugging
}

// Sets LED to green
void PeripheralHandler::setGreen() {
    _setColor(LOW, HIGH, LOW); // Common cathode: HIGH for ON
    // Serial.println("[LED] Set to Green (WiFi Connected, WS Disconnected)"); // Uncomment for debugging
}

// Sets LED to red
void PeripheralHandler::setRed() {
    _setColor(HIGH, LOW, LOW); // Common cathode: HIGH for ON
    // Serial.println("[LED] Set to Red (Hotspot Mode)"); // Uncomment for debugging
}

// Turns off the LED
void PeripheralHandler::turnOff() {
    _setColor(LOW, LOW, LOW); // Turn off all segments
    // Serial.println("[LED] Turned Off"); // Uncomment for debugging
}

// Helper function to set individual LED states
void PeripheralHandler::_setColor(int rState, int gState, int bState) {
    digitalWrite(_redPin, rState);
    digitalWrite(_greenPin, gState);
    digitalWrite(_bluePin, bState);
}

// Public method to get and reset the detected click count
unsigned int PeripheralHandler::getAndResetClickCount() {
    // This function implements non-blocking logic to detect single/double clicks.
    // It processes clicks only after a timeout period has passed since the last press,
    // or if a high number of presses (e.g., > 2) is immediately detected.

    // Use a static variable to track the last time clicks were processed,
    // or just rely on _button.nPresses and _button.lastPressTime.

    if (_button.nPresses > 0) { // If there are any presses registered by the ISR
        unsigned long currentTime = millis();
        // Check if the multi-click timeout has passed since the last press
        // OR if a significant number of presses has accumulated quickly (e.g., more than 2, indicating a rapid sequence)
        if (currentTime - _button.lastPressTime > MULTI_CLICK_TIMEOUT_MS) {
            // Disable interrupts briefly to safely read and reset volatile variables
            noInterrupts();
            unsigned int clicks = _button.nPresses;
            _button.nPresses = 0;   // Reset click count
            _button.pressed = false; // Reset pressed flag
            interrupts(); // Re-enable interrupts

            // Serial.printf("[PeripheralHandler] Detected %u clicks.\n", clicks); // Uncomment for debugging
            return clicks; // Return the detected number of clicks
        }
    }
    return 0; // No complete click sequence detected yet
}


// ISR function - called directly by the hardware interrupt
void IRAM_ATTR onButtonInterrupt() {
    // Check if the global instance pointer is valid.
    // This ensures we can safely call the instance's member function.
    if (globalPeripheralHandler != nullptr) {
        globalPeripheralHandler->_handleButtonPress();
    }
}

// Private method called by the ISR to handle button press logic
void PeripheralHandler::_handleButtonPress() {
    unsigned long currentTime = millis();
    // Debounce: Only register a press if enough time (CLICK_DEBOUNCE_MS) has passed
    // since the last registered press. This prevents multiple detections from
    // a single physical button press due to switch bounce.
    if (currentTime - _button.lastPressTime > CLICK_DEBOUNCE_MS) {
        _button.pressed = true;        // Mark that a press occurred
        _button.lastPressTime = currentTime; // Update the time of this press
        // Directly assign instead of ++ to avoid potential compiler warnings with volatile types
        _button.nPresses = _button.nPresses + 1; // Increment the press counter
        // Serial.printf("[PeripheralHandler] ISR: Press detected, count = %u\n", _button.nPresses); // Uncomment for debugging
    }
}