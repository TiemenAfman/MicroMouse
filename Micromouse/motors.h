#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>

extern const int stepPin; // Step pin
extern const int dirPin;  // Dir pin
extern int speed;

bool moveForward_();

#endif