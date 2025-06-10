// main.cpp
// This file contains the main Arduino sketch for reading ECG data from the AD8232 sensor
// and plotting it on the Serial Plotter.

#include <Arduino.h>       // Standard Arduino library
#include "AD8232_ECG.h"    // Include the AD8232_ECG sensor class

// Define the pins connected to your AD8232 ECG sensor.
// IMPORTANT: Adjust these pin numbers to match your actual ESP32 wiring.
const int ECG_OUTPUT_PIN = 34; // Analog pin connected to AD8232's OUTPUT (e.g., GPIO34, GPIO35, GPIO32 on ESP32)
const int LO_PLUS_PIN = 33;    // Digital pin connected to AD8232's LO+ (Lead-Off +)
const int LO_MINUS_PIN = 32;   // Digital pin connected to AD8232's LO- (Lead-Off -)

// Create an instance of the AD8232_ECG class
AD8232_ECG ecgSensor(ECG_OUTPUT_PIN, LO_PLUS_PIN, LO_MINUS_PIN);

void setup() {
    // Initialize serial communication at a baud rate suitable for the Serial Plotter.
    // 115200 is a common choice for faster data transmission.
    Serial.begin(115200);
    Serial.println("Initializing ECG Sensor and Serial Plotter...");

    // Initialize the ECG sensor pins.
    ecgSensor.begin();
    Serial.println("ECG Sensor initialized. Place electrodes and open Serial Plotter.");
}

void loop() {
    // Read the raw ECG value from the sensor.
    int ecgValue = ecgSensor.readECG();

    // Check if the leads are connected.
    // This is a crucial step to ensure meaningful readings.
    if (ecgSensor.isSensorConnected()) {
        // Print the ECG value to the Serial Monitor.
        // For the Serial Plotter, simply print the value followed by a newline.
        Serial.println(ecgValue);
    } else {
        // If leads are off, print a message indicating the lead-off condition.
        // For the Serial Plotter, you might print a consistent "0" or a predefined
        // out-of-range value if you still want to see data, or just stop printing data.
        // For this example, we'll print a message and a "0" to keep the plotter active.
        Serial.println("0"); // Or another placeholder value to keep the plot consistent
        Serial.println("ECG Leads OFF!"); // This will show in the Serial Monitor, not plotter
        delay(500); // Add a small delay to avoid overwhelming the serial output
    }

    // Add a small delay to control the sampling rate.
    // The AD8232 typically produces a signal that needs to be sampled at a higher rate
    // (e.g., 250-500Hz for clinical use). Adjust 'delay' for your plotting needs.
    // For basic visualization, a smaller delay (e.g., 5-10ms) is good.
    delay(10); // Adjust this delay for your desired sampling rate
}
