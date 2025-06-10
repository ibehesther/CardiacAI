// AD8232_ECG.cpp
// This file implements the methods defined in the AD8232_ECG class.
#include "AD8232_ECG.h"

AD8232_ECG::AD8232_ECG(int outputPin, int loPlusPin, int loMinusPin) {
    _outputPin = outputPin;
    _loPlusPin = loPlusPin;
    _loMinusPin = loMinusPin;
}

void AD8232_ECG::begin() {
    // Set the lead-off detection pins as input.
    // These pins will be HIGH when a lead-off condition is detected.
    pinMode(_loPlusPin, INPUT);
    pinMode(_loMinusPin, INPUT);
    // The output pin is an analog input, so no explicit pinMode is needed for analogRead,
    // but defining it as INPUT is good practice for clarity.
    pinMode(_outputPin, INPUT);
}

bool AD8232_ECG::isSensorConnected() {
    // Read the status of the lead-off detection pins.
    // A HIGH value on either LO+ or LO- indicates a disconnected lead.
    // We want both to be LOW (connected) for the sensor to be considered "connected".
    bool loPlusStatus = digitalRead(_loPlusPin);
    bool loMinusStatus = digitalRead(_loMinusPin);

    // If both are LOW (0), then leads are connected.
    // If either is HIGH (1), then at least one lead is off.
    return (loPlusStatus == LOW && loMinusStatus == LOW);
}

int AD8232_ECG::readECG() {
    // Read the analog value from the ECG output pin.
    // This value will vary with the heart's electrical activity.
    return analogRead(_outputPin);
}
