#include "sensor.h"

// Define global variables
const int sensorPinF = A0;  // Analoge pin waar de sensor op is aangesloten
const int sensorPinL = A1;  // Analoge pin waar de sensor op is aangesloten
const int sensorPinR = A2;  // Analoge pin waar de sensor op is aangesloten

bool wallFront() {
  int sensorValue = analogRead(sensorPinF);  // Lees de waarde van de sensor
  float voltage = sensorValue * (3.3 / 4095.0);  // Converteer de analoge waarde naar spanning
  
  // Omzetten van spanning naar afstand (zie datasheet voor de juiste formule)
  float distance = 12.08 * pow(voltage, -1.058); // Voorbeeldformule, deze kan afwijken
  
  // Bepaal de drempelwaarde voor muurdetectie (bijv. 30 cm)
  float thresholdDistance = 20.0; // Pas dit aan afhankelijk van je vereisten
  
  // Retourneer true als de afstand kleiner is dan de drempelwaarde, anders false
  return (distance > thresholdDistance);
}


bool wallLeft() {
  int sensorValue = analogRead(sensorPinL);  // Lees de waarde van de sensor
  float voltage = sensorValue * (3.3 / 4095.0);  // Converteer de analoge waarde naar spanning
  
  // Omzetten van spanning naar afstand (zie datasheet voor de juiste formule)
  float distance = 12.08 * pow(voltage, -1.058); // Voorbeeldformule, deze kan afwijken
  
  // Bepaal de drempelwaarde voor muurdetectie (bijv. 30 cm)
  float thresholdDistance = 5.0; // Pas dit aan afhankelijk van je vereisten
  
  // Retourneer true als de afstand kleiner is dan de drempelwaarde, anders false
  return (distance > thresholdDistance);
}

bool wallRight() {
  int sensorValue = analogRead(sensorPinR);  // Lees de waarde van de sensor
  float voltage = sensorValue * (3.3 / 4095.0);  // Converteer de analoge waarde naar spanning
  
  // Omzetten van spanning naar afstand (zie datasheet voor de juiste formule)
  float distance = 12.08 * pow(voltage, -1.058); // Voorbeeldformule, deze kan afwijken
  
  // Bepaal de drempelwaarde voor muurdetectie (bijv. 30 cm)
  float thresholdDistance = 5.0; // Pas dit aan afhankelijk van je vereisten
  
  // Retourneer true als de afstand kleiner is dan de drempelwaarde, anders false
  return (distance > thresholdDistance);
}

float wallDistance(int AnalogPin){
  
  // Lees de waarde van de geselecteerde sensor
  int sensorValue = analogRead(AnalogPin);
  
  // Converteer de analoge waarde naar spanning
  float voltage = sensorValue * (3.3 / 4095.0);
  
  // Omzetten van spanning naar afstand (gebruik de juiste formule voor je specifieke sensor)
  float distance = 12.08 * pow(voltage, -1.058); // Voorbeeldformule, aanpasbaar afhankelijk van de datasheet
  
  return distance;
}


