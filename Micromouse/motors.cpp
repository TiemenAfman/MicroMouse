#include "motors.h"

// Define global variables
const int motorL_step_pin = D7; // Step pin
const int motorL_dir_pin = D8;  // Dir pin 
const int motorR_step_pin = D9; // Step pin
const int motorR_dir_pin = D10;  // Dir pin
const int ENBL_PINL = D6; //disable steppers when high
const int ENBL_PINR = D5; //disable steppers when high

float kp = 1.0, ki = 0.0, kd = 0.0;
float error, previous_error, integral, derivative, pid_output;
unsigned long motor1_last_step_time = 0;
unsigned long motor2_last_step_time = 0;

unsigned long last_time = millis();

int base_steps_per_second = 400;  // Basis aantal stappen per seconde voor beide motoren
int motor1_steps_per_second = 0;
int motor2_steps_per_second = 0;

int lastError = 0;  // Variabele om de fout van de vorige cyclus op te slaan

// Voeg een nieuwe parameter toe aan moveForward_ om de afstand te bepalen
String moveForward_(float distance_cm) {

  String info = "";

  int steps_per_revolution = 200;   // Stel het aantal stappen per omwenteling van de motor in
  float wheel_circumference = 199.8; // Omtrek van het wiel in cm (bijv. 10 cm diameter => 31.4 cm omtrek)
  
  // Bereken het aantal stappen dat nodig is om de gewenste afstand te overbruggen
  int steps_needed = (distance_cm / wheel_circumference) * steps_per_revolution;

  int motor1_steps_taken = 0;
  int motor2_steps_taken = 0;

  float sensor_left = wallDistance(A1);
  float sensor_right = wallDistance(A2);

  sensor_right = 6.35; // test
  // PID-berekeningen
  calculatePID(sensor_left, sensor_right);

  //pid_output = 50; // test

  // Bereken het aantal stappen per seconde voor beide motoren
  motor1_steps_per_second = base_steps_per_second - pid_output;  // Correctie voor linkermotor
  motor2_steps_per_second = base_steps_per_second + pid_output;  // Correctie voor rechtermotor

  // Zorg ervoor dat de stappen per seconde niet negatief zijn
  motor1_steps_per_second = max(motor1_steps_per_second, 0);
  motor2_steps_per_second = max(motor2_steps_per_second, 0);

  // Blijf motoren bewegen totdat beide het benodigde aantal stappen hebben genomen
  while (motor1_steps_taken < steps_needed || motor2_steps_taken < steps_needed) {
    // Update de motoren en kijk of ze verder kunnen stappen
    info = moveMotors(motor1_steps_per_second, motor2_steps_per_second);

    // Controleer of motoren daadwerkelijk stappen hebben gemaakt
    unsigned long current_time = micros();
    
    if (current_time - motor1_last_step_time >= (1000000 / motor1_steps_per_second)) {
      motor1_steps_taken++;
    }

    if (current_time - motor2_last_step_time >= (1000000 / motor2_steps_per_second)) {
      motor2_steps_taken++;
    }
  }

    // test
    // for (int i = 0; i < steps_needed; i++) {
    //   // Stappenmotor aansturen
    //   // digitalWrite(motorL_step_pin, HIGH);
    //   // delayMicroseconds(2000); // Pas de snelheid aan
    //   // digitalWrite(motorL_step_pin, LOW);
    //   // delayMicroseconds(2000); // Pas de snelheid aan
    //   // digitalWrite(motorR_step_pin, HIGH);
    //   // delayMicroseconds(2000); // Pas de snelheid aan
    //   // digitalWrite(motorR_step_pin, LOW);
    //   // delayMicroseconds(2000); // Pas de snelheid aan

    //   stepMotor(motorL_step_pin);
    //   stepMotor(motorR_step_pin);
    // }

  return info;
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

String moveMotors(int motor1_steps_per_second, int motor2_steps_per_second) {

  unsigned long current_time = micros();  // Gebruik micros() voor nauwkeurige timing

  // Bereken de benodigde tijd tussen stappen voor elke motor in microseconden
  unsigned long motor1_step_interval = 1000000 / motor1_steps_per_second;  // Tijd tussen stappen in microseconden
  unsigned long motor2_step_interval = 1000000 / motor2_steps_per_second;  // Tijd tussen stappen in microseconden

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

  String info = "Motor1: " + String(motor1_steps_per_second) + " steps/sec\n";
  info += "Motor2: " + String(motor2_steps_per_second) + " steps/sec\n";
  info += "CurrentTime: " + String(current_time) + " micros\n";
  info += "Motor1 step last time: " + String(motor1_last_step_time) + " lastTime\n";
  info += "Motor2 step last time: " + String(motor2_last_step_time) + " lastTime\n";
  info += "motor1_step_interval: " + String(motor1_step_interval) + " microseconds\n";
  info += "motor2_step_interval: " + String(motor2_step_interval) + " microseconds\n";
  return info;
}

void stepMotor(int stepPin) {
  noInterrupts();  // Zorgt ervoor dat er geen interrupts optreden tijdens de stap
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(100);
  digitalWrite(stepPin, LOW);
  interrupts();  // Zet interrupts weer aan
}