#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>
#include "sensor.h"
#include <AccelStepper.h>

extern const int motorL_step_pin; // Step pin
extern const int motorL_dir_pin;  // Dir pin
extern const int motorR_step_pin; // Step pin
extern const int motorR_dir_pin;  // Dir pin
extern const int ENBL_PINL; //disable steppers  when high
extern const int ENBL_PINR; //disable steppers  when high
extern unsigned long last_time;
extern const int left;
extern const int right;

String moveForward(float distance_cm);
void calculatePID(float sensor_left, float sensor_right);
String moveMotors(int motor1_steps_per_second, int motor2_steps_per_second);
void stepMotor(int stepPin);
void turn(int direction);

#endif