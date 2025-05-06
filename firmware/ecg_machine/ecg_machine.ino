#include "main.h"

const int ecgPin = 34;
const int loPlus = 33;
const int loMinus = 32;

// Sinewave simulating ecg scan
float angle = 0.0;         // For sine wave simulation
const float step = 0.1;    // Angle step (radians)
const float frequency = 1; // Hz (roughly 1 cycle per second)
const int amplitude = 1500;
const int offset = 2000;   // Center of wave in the Serial Plotter

void setup() {
  Serial.begin(115200);
  // pinMode(loPlus, INPUT);
  // pinMode(loMinus, INPUT);
}

void loop() {
  // if (digitalRead(loPlus) == HIGH || digitalRead(loMinus) == HIGH) {
  //   Serial.println("0");
  // } else {
  //   int ecgValue = analogRead(ecgPin);
  //   Serial.println(ecgValue);
  // }

  // Mocked sinewave simulation
  float sineValue = sin(angle);
  int simulatedECG = int(sineValue * amplitude + offset);
  Serial.println(simulatedECG);

  angle += step;
  if (angle >= 2 * PI) angle -= 2 * PI;

  delay(10);
}
