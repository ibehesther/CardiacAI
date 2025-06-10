// PeripheralHandler.h
// This header file defines the PeripheralHandler class for controlling an RGB LED.

#ifndef PERIPHERAL_HANDLER_H
#define PERIPHERAL_HANDLER_H

#include <Arduino.h> // Required for basic Arduino functions like pinMode, digitalWrite

/**
 * @brief A class to control an RGB LED based on different operational modes.
 *
 * This class provides methods to initialize the RGB LED pins and set specific
 * colors (blue, green, red) to indicate the device's connection status.
 * It assumes a common cathode RGB LED, where HIGH turns the LED segment ON.
 * If you have a common anode LED, you will need to invert the HIGH/LOW logic
 * in the _setColor method.
 */
class PeripheralHandler {
public:
    /**
     * @brief Constructor for the PeripheralHandler class.
     * @param redPin The digital pin connected to the Red segment of the RGB LED.
     * @param greenPin The digital pin connected to the Green segment of the RGB LED.
     * @param bluePin The digital pin connected to the Blue segment of the RGB LED.
     */
    PeripheralHandler(int redPin, int greenPin, int bluePin);

    /**
     * @brief Initializes the RGB LED pins.
     * This method sets the pin modes for the LED pins to OUTPUT.
     */
    void begin();

    /**
     * @brief Sets the RGB LED to blue, indicating WebSocket connection.
     */
    void setBlue();

    /**
     * @brief Sets the RGB LED to green, indicating WiFi client mode but no WebSocket connection.
     */
    void setGreen();

    /**
     * @brief Sets the RGB LED to red, indicating Hotspot mode.
     */
    void setRed(); // This will serve as the "third color"

    /**
     * @brief Turns off all segments of the RGB LED.
     */
    void turnOff();

private:
    int _redPin;   // Pin for the Red LED segment
    int _greenPin; // Pin for the Green LED segment
    int _bluePin;  // Pin for the Blue LED segment

    /**
     * @brief Helper function to set the state of the RGB LED segments.
     * Assumes common cathode: HIGH for ON, LOW for OFF.
     * If common anode, invert logic (LOW for ON, HIGH for OFF).
     * @param rState Digital state for Red (HIGH/LOW).
     * @param gState Digital state for Green (HIGH/LOW).
     * @param bState Digital state for Blue (HIGH/LOW).
     */
    void _setColor(int rState, int gState, int bState);
};

#endif // PERIPHERAL_HANDLER_H