#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

extern const int sensorPinF;  // Analoge pin waar de sensor op is aangesloten
extern const int sensorPinL;  // Analoge pin waar de sensor op is aangesloten
extern const int sensorPinR;  // Analoge pin waar de sensor op is aangesloten
extern int lastValue;  // Initialize with a value that your sensor will not output

bool wallFront();
bool wallLeft();
bool wallRight();
float wallDistance(int channel);

#endif