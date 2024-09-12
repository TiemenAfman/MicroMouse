#include "motors.h"

// Define global variables
const int motorL_step_pin = D2; // Step pin verbonden met D2
const int motorL_dir_pin = D3;  // Dir pin verbonden met D3
const int motorR_step_pin = D4; // Step pin verbonden met D2
const int motorR_dir_pin = D5;  // Dir pin verbonden met D3

float kp = 1.0, ki = 0.0, kd = 0.0;
float error, previous_error, integral, derivative, pid_output;
unsigned long current_time;
unsigned long motor1_last_step_time = 0;
unsigned long motor2_last_step_time = 0;

unsigned long last_time = millis();

int base_steps_per_second = 200;  // Basis aantal stappen per seconde voor beide motoren
int motor1_steps_per_second = 0;
int motor2_steps_per_second = 0;

int lastError = 0;  // Variabele om de fout van de vorige cyclus op te slaan

// Voeg een nieuwe parameter toe aan moveForward_ om de afstand te bepalen
bool moveForward_(float distance_cm) {
  int steps_per_revolution = 200;   // Stel het aantal stappen per omwenteling van de motor in
  float wheel_circumference = 31.4; // Omtrek van het wiel in cm (bijv. 10 cm diameter => 31.4 cm omtrek)
  
  // Bereken het aantal stappen dat nodig is om de gewenste afstand te overbruggen
  int steps_needed = (distance_cm / wheel_circumference) * steps_per_revolution;
  
  int motor1_steps_taken = 0;
  int motor2_steps_taken = 0;

  float sensor_left = wallDistance(A1);
  float sensor_right = wallDistance(A2);

  // PID-berekeningen
  calculatePID(sensor_left, sensor_right);

  // Bereken het aantal stappen per seconde voor beide motoren
  motor1_steps_per_second = base_steps_per_second - pid_output;  // Correctie voor linkermotor
  motor2_steps_per_second = base_steps_per_second + pid_output;  // Correctie voor rechtermotor

  // Zorg ervoor dat de stappen per seconde niet negatief zijn
  motor1_steps_per_second = max(motor1_steps_per_second, 0);
  motor2_steps_per_second = max(motor2_steps_per_second, 0);

  while (motor1_steps_taken < steps_needed && motor2_steps_taken < steps_needed) {
    // Stuur stappen naar beide motoren
    moveMotors(motor1_steps_per_second, motor2_steps_per_second);

    // Update het aantal genomen stappen
    motor1_steps_taken++;
    motor2_steps_taken++;
  }

  return true;
}


void calculatePID(float sensor_left, float sensor_right) {
  error = sensor_left - sensor_right;

  // PID-berekeningen
  float p_term = kp * error;
  integral += error * (millis() - last_time) / 1000.0;  // dt in seconden
  float i_term = ki * integral;
  derivative = (error - previous_error) / ((millis() - last_time) / 1000.0);  // dt in seconden
  float d_term = kd * derivative;

  // PID-output
  pid_output = p_term + i_term + d_term;

  // Sla de huidige fout en tijd op voor de volgende iteratie
  previous_error = error;
  last_time = millis();
}

void moveMotors(int motor1_steps_per_second, int motor2_steps_per_second) {
  current_time = millis();

  // Bereken de benodigde tijd tussen stappen voor elke motor
  unsigned long motor1_step_interval = 1000 / motor1_steps_per_second;  // Tijd tussen stappen in milliseconden
  unsigned long motor2_step_interval = 1000 / motor2_steps_per_second;  // Tijd tussen stappen in milliseconden

  // Motor 1: Als de tijd sinds de laatste stap groter is dan de stapinterval, dan stap
  if (motor1_steps_per_second > 0 && (current_time - motor1_last_step_time >= motor1_step_interval)) {
    stepMotor(motorL_step_pin);
    motor1_last_step_time = current_time;  // Tijd van de laatste stap bijwerken
  }

  // Motor 2: Als de tijd sinds de laatste stap groter is dan de stapinterval, dan stap
  if (motor2_steps_per_second > 0 && (current_time - motor2_last_step_time >= motor2_step_interval)) {
    stepMotor(motorR_step_pin);
    motor2_last_step_time = current_time;  // Tijd van de laatste stap bijwerken
  }
}

void stepMotor(int stepPin) {
  // Zet een enkele stap voor de motor
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(10);  // Korte puls om een stap te zetten
  digitalWrite(stepPin, LOW);
}