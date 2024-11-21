#include "sensor.h"

// Define global variables
const int sensorPinF = A0;  // Analoge pin waar de sensor op is aangesloten
const int sensorPinL = A1;  // Analoge pin waar de sensor op is aangesloten
const int sensorPinR = A2;  // Analoge pin waar de sensor op is aangesloten

int lastValue = -1;  // Initialize with a value that your sensor will not output

bool wallFront() {
  int sensorValue = analogRead(sensorPinF);  // Lees de waarde van de sensor
  float voltage = sensorValue * (3.3 / 4095.0);  // Converteer de analoge waarde naar spanning
  
  // Omzetten van spanning naar afstand (zie datasheet voor de juiste formule)
  float distance = 12.08 * pow(voltage, -1.058); // Voorbeeldformule, deze kan afwijken
  
  // Bepaal de drempelwaarde voor muurdetectie (bijv. 30 cm)
  float thresholdDistance = 4.5; // Pas dit aan afhankelijk van je vereisten
  
  // Retourneer true als de afstand kleiner is dan de drempelwaarde, anders false
  return (distance < thresholdDistance);
}

bool wallLeft() {
  int sensorValue = analogRead(sensorPinL);  // Lees de waarde van de sensor
  float voltage = sensorValue * (3.3 / 4095.0);  // Converteer de analoge waarde naar spanning
  
  // Omzetten van spanning naar afstand (zie datasheet voor de juiste formule)
  float distance = 12.08 * pow(voltage, -1.058); // Voorbeeldformule, deze kan afwijken
  
  // Bepaal de drempelwaarde voor muurdetectie (bijv. 30 cm)
  float thresholdDistance = 45.0; // Pas dit aan afhankelijk van je vereisten
  
  // Retourneer true als de afstand kleiner is dan de drempelwaarde, anders false
  return (distance > thresholdDistance);
}

bool wallRight() {
  int sensorValue = analogRead(sensorPinR);  // Lees de waarde van de sensor
  float voltage = sensorValue * (3.3 / 4095.0);  // Converteer de analoge waarde naar spanning
  
  // Omzetten van spanning naar afstand (zie datasheet voor de juiste formule)
  float distance = 12.08 * pow(voltage, -1.058); // Voorbeeldformule, deze kan afwijken
  
  // Bepaal de drempelwaarde voor muurdetectie (bijv. 30 cm)
  float thresholdDistance = 45.0; // Pas dit aan afhankelijk van je vereisten
  
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

// Function to read sensor value, check for change, and publish if changed
void checkAndSendMessage(PubSubClient& client, int sensorPin, const char* topic, int& lastValue) {
  int sensorValue = analogRead(sensorPin);  // Read the sensor value

  // Check if the sensor value has changed
  if (sensorValue != lastValue) {
    lastValue = sensorValue;  // Update the last value

    // Publish the new sensor value
    publishValue(client, topic, String(sensorValue));
  }
}
