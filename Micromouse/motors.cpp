#include "motors.h"
#include <AccelStepper.h>
#include "sensor.h"

// Define global variables
const int motorL_step_pin = D8; // Step pin
const int motorL_dir_pin = D7;  // Dir pin 
const int motorR_step_pin = D10; // Step pin
const int motorR_dir_pin = D9;  // Dir pin
const int ENBL_PIN = D4; //disable steppers when high
const int left = 1;
const int right = -1;

// Definieer het type stappenmotor (1 = DRIVER)
AccelStepper stepperL(AccelStepper::DRIVER, motorL_step_pin, motorL_dir_pin); // pin 2 = pulssignaal, pin 3 = richtingssignaal
AccelStepper stepperR(AccelStepper::DRIVER, motorR_step_pin, motorR_dir_pin); // pin 2 = pulssignaal, pin 3 = richtingssignaal

float kp = 10, ki = 0.0, kd = 0.0;
float error, previous_error, integral, derivative, pid_output;
unsigned long motor1_last_step_time = 0;
unsigned long motor2_last_step_time = 0;

unsigned long last_time = millis();

int base_steps_per_second = 1000;  // Basis aantal stappen per seconde voor beide motoren
int motor1_steps_per_second = 0;
int motor2_steps_per_second = 0;

int lastError = 0;  // Variabele om de fout van de vorige cyclus op te slaan

// 1/1 stap (volledige stap): M0 = laag, M1 = laag, M2 = laag
// 1/2 stap: M0 = hoog, M1 = laag, M2 = laag
// 1/4 stap: M0 = laag, M1 = hoog, M2 = laag <------
// 1/8 stap: M0 = hoog, M1 = hoog, M2 = laag  
// 1/16 stap: M0 = laag, M1 = laag, M2 = hoog
// 1/32 stap: M0 = hoog, M1 = hoog, M2 = hoog

const int steps_per_revolution = 200 * 4;   // Stel het aantal stappen per omwenteling van de motor in. 
const float wheel_circumference = 199.8; // Omtrek van het wiel in mm
const float distanceBetweenWheels = 109;  // Afstand tussen de wielen in mm (bv. 106.4 mm)

float FilteredLeft;
float FilteredRight;

String moveForward(float distance_cm) {
  String info = "";
  
  // Bereken het aantal stappen dat nodig is om de gewenste afstand te overbruggen
  int steps_needed = (distance_cm / wheel_circumference) * steps_per_revolution;
  
  // Zet het aantal stappen voor beide motoren
  stepperL.move(steps_needed);
  stepperR.move(steps_needed);

  stepperR.setAcceleration(10000);                   // Acceleratie in stappen per seconde^2
  stepperL.setAcceleration(10000);                   // Acceleratie in stappen per seconde^2

  // Blijf motoren bewegen totdat beide het benodigde aantal stappen hebben genomen
  while (stepperL.distanceToGo() != 0 && stepperR.distanceToGo() != 0) {

    // Lees de sensoren in elke cyclus om de PID-aanpassing te updaten
    float sensor_left = wallDistance(sensorPinL);
    float sensor_right = wallDistance(sensorPinR);

    // if (wallLeft() && !wallRight()){        // wel muur links maar rechts niet
    //   sensor_right = 89;                      // zet rechter sensor op vaste waarde
    // } else if(!wallLeft() && wallRight()){  // wel muur rechts maar links niet
    //   sensor_left = 89;                       // zet linker sensor op vaste waarde
    // }
    // if (wallLeft() || wallRight()){         // als muur links of rechts bereken dan pid
    //   // PID-berekeningen herberekenen tijdens elke iteratie
    //   calculatePID(sensor_left, sensor_right);      
    // } else{                                   // geen muren dan rechtdoor rijden
    //   pid_output = 0;
    // }

    if (wallLeft() && wallRight()){
    calculatePID(sensor_left, sensor_right); 
    }
    else {
    pid_output = 0;
    }
    
    // Bereken het aantal stappen per seconde voor beide motoren
    float motor1_steps_per_second = base_steps_per_second + pid_output;  // Correctie voor linkermotor
    float motor2_steps_per_second = base_steps_per_second - pid_output;  // Correctie voor rechtermotor

    // Zorg ervoor dat de stappen per seconde niet negatief zijn
    motor1_steps_per_second = constrain(motor1_steps_per_second, base_steps_per_second - 600, base_steps_per_second + 1000);
    motor2_steps_per_second = constrain(motor2_steps_per_second, base_steps_per_second - 600, base_steps_per_second + 1000);

    // Update de motoren en kijk of ze verder kunnen stappen
    stepperL.setMaxSpeed(motor1_steps_per_second);  // Maximale snelheid in stappen per seconde
    stepperR.setMaxSpeed(motor2_steps_per_second);  // Maximale snelheid in stappen per seconde

    // Laat de motoren stappen
    stepperL.run();  // Motor bewegen
    stepperR.run();  // Motor bewegen
  }

  info = "Done";

  return info;
}

void calculatePID(float sensor_left, float sensor_right) {
  error = sensor_left - sensor_right;

  // PD-berekeningen
  float p_term = kp * error;
  float derivative = (error - previous_error) / ((millis() - last_time) / 1000.0);  // dt in seconden
  float d_term = kd * derivative;

  // PD-output
  pid_output = p_term + d_term;

  // Sla de huidige fout en tijd op voor de volgende iteratie
  previous_error = error;
  last_time = millis();
}

void turn(int direction){
  stepperL.setCurrentPosition(0);
  stepperR.setCurrentPosition(0);
  stepperR.setAcceleration(10000);                   // Acceleratie in stappen per seconde^2
  stepperL.setAcceleration(10000);                   // Acceleratie in stappen per seconde^2
  stepperL.setMaxSpeed(base_steps_per_second);  // Maximale snelheid in stappen per seconde
  stepperR.setMaxSpeed(base_steps_per_second);  // Maximale snelheid in stappen per seconde

  // Bereken de rotatie-omtrek voor 90 graden draaien
  float rotationCircumference = PI * distanceBetweenWheels;

  // Afstand die elk wiel moet afleggen voor een 90 graden draai
  float distancePerWheel = rotationCircumference / 4;

  // Aantal stappen om de afstand af te leggen
  int stepsToTurn = (distancePerWheel / wheel_circumference) * steps_per_revolution;

  // Laat de linker motor achteruit draaien en de rechter vooruit
  stepperL.move(-stepsToTurn * direction);  // Negatief om linksom te draaien
  stepperR.move(stepsToTurn * direction);

  // Beweeg de motoren tot ze klaar zijn
  while (stepperL.distanceToGo() != 0 && stepperR.distanceToGo() != 0) {
    stepperL.run();
    stepperR.run();
  }
}
