#include "motors.h"

// Define global variables
const int stepPin = D2; // Step pin verbonden met D2
const int dirPin = D3;  // Dir pin verbonden met D3

bool moveForward_(){
      for (int i = 1; i <= 400; i++) {
        // Stappenmotor aansturen
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(speed); // Pas de snelheid aan
        digitalWrite(stepPin, LOW);
        delayMicroseconds(speed); // Pas de snelheid aan
    }

    return true;

}