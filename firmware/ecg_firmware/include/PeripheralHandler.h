// PeripheralHandler.h
// This header file defines the PeripheralHandler class for controlling an RGB LED and a button.

#ifndef PERIPHERAL_HANDLER_H
#define PERIPHERAL_HANDLER_H

#include <Arduino.h>

#define CLICK_DEBOUNCE_MS 180 // Minimum time in ms between two distinct press detections

#define MULTI_CLICK_TIMEOUT_MS 1000 // Time in ms after a press to wait for more presses

class PeripheralHandler;

// Global pointer to the PeripheralHandler instance.
// This is necessary because attachInterrupt requires a C-style function pointer for the ISR,
// which cannot directly be a non-static member function. The ISR will use this pointer
// to access the actual PeripheralHandler instance's members.
extern PeripheralHandler* globalPeripheralHandler;

/**
 * @brief Interrupt Service Routine (ISR) for the button.
 * This function is called every time the button state changes (falling edge).
 * It updates the button press state and counts presses, handling debouncing.
 *
 * NOTE: IRAM_ATTR is applied only to the definition in the .cpp file, not here,
 * to avoid compilation warnings related to conflicting section attributes.
 */
void onButtonInterrupt();


/**
 * @brief A class to control an RGB LED based on different operational modes
 * and handle button input for mode transitions.
 *
 * This class provides methods to initialize the RGB LED pins and set specific
 * colors (blue, green, red) to indicate the device's connection status.
 * It also manages a button, detecting single and double clicks using an ISR
 * and providing a non-blocking method to retrieve click counts.
 * It assumes a common cathode RGB LED, where HIGH turns the LED segment ON,
 * and an INPUT_PULLUP button, where a falling edge indicates a press.
 */
class PeripheralHandler {
public:
    /**
     * @brief Constructor for the PeripheralHandler class.
     * @param redPin The digital pin connected to the Red segment of the RGB LED.
     * @param greenPin The digital pin connected to the Green segment of the RGB LED.
     * @param bluePin The digital pin connected to the Blue segment of the RGB LED.
     * @param buttonPin The digital pin connected to the button (configured as INPUT_PULLUP).
     */
    PeripheralHandler(int redPin, int greenPin, int bluePin, int buttonPin);

    /**
     * @brief Initializes the RGB LED and button pins.
     * This method sets the pin modes and attaches the interrupt to the button pin.
     */
    void begin();

    /**
     * @brief Sets the RGB LED to blue, indicating WebSocket connection.
     */
    void setBlue(int bState);

    /**
     * @brief Sets the RGB LED to green, indicating WiFi client mode but no WebSocket connection.
     */
    void setGreen(int gState);

    /**
     * @brief Sets the RGB LED to red, indicating Hotspot mode.
     */
    void setRed(int rState); // This will serve as the "third color"

    /**
     * @brief Turns off all segments of the RGB LED.
     */
    void turnOff();

    void toggleGreenSixTimes(unsigned long delayTime);

    /**
     * @brief Checks for button clicks and returns the number of clicks detected.
     * This function should be called regularly in the main loop to process button presses.
     * It handles debouncing and aggregation of rapid clicks into a single/double click count
     * based on the `MULTI_CLICK_TIMEOUT_MS`.
     * @return The number of clicks detected (e.g., 1 for single, 2 for double), or 0 if no clicks.
     */
    unsigned int getAndResetClickCount();

    /**
     * @brief Public method called by the ISR to handle a button press.
     * It applies debouncing and increments the press counter.
     * This method is public so the global ISR can access it.
     */
    void _handleButtonPress();

private:
    int _redPin;   // Pin for the Red LED segment
    int _greenPin; // Pin for the Green LED segment
    int _bluePin;  // Pin for the Blue LED segment
    int _buttonPin; // Pin for the button

    /**
     * @brief Structure to hold button state and click count.
     * Members are volatile because they are modified within an ISR.
     */
    struct Button {
        volatile unsigned long lastPressTime; // Time of the last detected press
        volatile unsigned int nPresses;     // Number of rapid presses within the timeout window
    } _button;

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