// AD8232_ECG.cpp
// This file implements the methods defined in the AD8232_ECG class.
#include "AD8232_ECG.h"

AD8232_ECG::AD8232_ECG(int outputPin, int loPlusPin, int loMinusPin) {
    _outputPin = outputPin;
    _loPlusPin = loPlusPin;
    _loMinusPin = loMinusPin;
}

void AD8232_ECG::begin() {
    pinMode(_loPlusPin, INPUT);
    pinMode(_loMinusPin, INPUT);
    pinMode(_outputPin, INPUT);
}

bool AD8232_ECG::isSensorConnected() {


    // We want both to be LOW (connected) for the sensor to be considered "connected".
    bool loPlusStatus = digitalRead(_loPlusPin);
    bool loMinusStatus = digitalRead(_loMinusPin);

    // If both are LOW (0), then leads are connected.
    // If either is HIGH (1), then at least one lead is off.
    return (loPlusStatus == LOW && loMinusStatus == LOW);
}

int AD8232_ECG::readECG() {
    return analogRead(_outputPin);
}

