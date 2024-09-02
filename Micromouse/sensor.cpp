#include "sensor.h"

// Define global variables
const int sensorPin = A0;  // Analoge pin waar de sensor op is aangesloten
int lastValue = -1;  // Initialize with a value that your sensor will not output

bool wallFront_() {
  int sensorValue = analogRead(sensorPin);  // Lees de waarde van de sensor
  float voltage = sensorValue * (5.0 / 1023.0);  // Converteer de analoge waarde naar spanning
  
  // Omzetten van spanning naar afstand (zie datasheet voor de juiste formule)
  float distance = 12.08 * pow(voltage, -1.058); // Voorbeeldformule, deze kan afwijken
  
  // Bepaal de drempelwaarde voor muurdetectie (bijv. 30 cm)
  float thresholdDistance = 3.0; // Pas dit aan afhankelijk van je vereisten
  
  // Retourneer true als de afstand kleiner is dan de drempelwaarde, anders false
  return (distance < thresholdDistance);
}

float wallDistance(){
  int sensorValue = analogRead(sensorPin);  // Lees de waarde van de sensor
  float voltage = sensorValue * (5.0 / 1023.0);  // Converteer de analoge waarde naar spanning
  
  // Omzetten van spanning naar afstand (zie datasheet voor de juiste formule)
  float distance = 12.08 * pow(voltage, -1.058); // Voorbeeldformule, deze kan afwijken

  return voltage;
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
