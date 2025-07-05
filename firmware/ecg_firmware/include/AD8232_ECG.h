// AD8232_ECG.h
// This header file defines the AD8232_ECG class for interacting with the AD8232 ECG sensor.
#ifndef AD8232_ECG_H
#define AD8232_ECG_H

#include <Arduino.h>

/**
 * @brief A class to encapsulate the functionality of the AD8232 ECG sensor.
 *
 * This class provides methods to initialize the sensor, check its connection
 * status (lead-off detection), and read the raw analog ECG signal.
 */
class AD8232_ECG {
public:
    /**
     * @brief Constructor for the AD8232_ECG class.
     * @param outputPin The analog pin connected to the 'OUTPUT' pin of the AD8232.
     * @param loPlusPin The digital pin connected to the 'LO+' (Lead-Off +) pin of the AD8232.
     * @param loMinusPin The digital pin connected to the 'LO-' (Lead-Off -) pin of the AD8232.
     */
    AD8232_ECG(int outputPin, int loPlusPin, int loMinusPin);

    /**
     * @brief Initializes the pins connected to the AD8232 sensor.
     * This method sets the pin modes for the lead-off detection pins.
     */
    void begin();

    /**
     * @brief Checks if the ECG leads are properly connected to the patient.
     * The AD8232 provides active high signals on LO+ and LO- pins when a lead-off
     * condition is detected (i.e., leads are disconnected). This method returns true
     * if both leads are connected (LO+ and LO- are LOW), and false otherwise.
     * @return true if both leads are connected, false if any lead is off.
     */
    bool isSensorConnected();

    /**
     * @brief Reads the raw analog ECG value from the AD8232's OUTPUT pin.
     * This value represents the amplified and filtered ECG signal.
     * The range of the analog reading depends on the ADC resolution (e.g., 0-4095 for ESP32's 12-bit ADC).
     * @return The raw analog value read from the ECG output pin.
     */
    int readECG();

private:
    int _outputPin;  // Pin connected to the OUTPUT of AD8232 (analog input)
    int _loPlusPin;  // Pin connected to LO+ of AD8232 (digital input for lead-off detection)
    int _loMinusPin; // Pin connected to LO- of AD8232 (digital input for lead-off detection)
};

#endif // AD8232_ECG_H