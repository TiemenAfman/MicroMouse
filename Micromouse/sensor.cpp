#include "sensor.h"
#include <Arduino.h>  // Nodig om Arduino functies zoals analogRead te gebruiken

// Define global variables
const int sensorPinF = A0;  // Analoge pin waar de sensor op is aangesloten
const int sensorPinL = A1;  // Analoge pin waar de sensor op is aangesloten
const int sensorPinR = A2;  // Analoge pin waar de sensor op is aangesloten

int lastValue = -1;  // Initialize with a value that your sensor will not output

bool wallFront_() {
  int sensorValue = analogRead(sensorPinF);  // Lees de waarde van de sensor
  float voltage = sensorValue * (3.3 / 4095.0);  // Converteer de analoge waarde naar spanning
  
  // Omzetten van spanning naar afstand (zie datasheet voor de juiste formule)
  float distance = 12.08 * pow(voltage, -1.058); // Voorbeeldformule, deze kan afwijken
  
  // Bepaal de drempelwaarde voor muurdetectie (bijv. 30 cm)
  float thresholdDistance = 4.5; // Pas dit aan afhankelijk van je vereisten
  
  // Retourneer true als de afstand kleiner is dan de drempelwaarde, anders false
  return (distance < thresholdDistance);
}

bool wallLeft_() {
  int sensorValue = analogRead(sensorPinL);  // Lees de waarde van de sensor
  float voltage = sensorValue * (3.3 / 2095.0);  // Converteer de analoge waarde naar spanning
  
  // Omzetten van spanning naar afstand (zie datasheet voor de juiste formule)
  float distance = 12.08 * pow(voltage, -1.058); // Voorbeeldformule, deze kan afwijken
  
  // Bepaal de drempelwaarde voor muurdetectie (bijv. 30 cm)
  float thresholdDistance = 10.0; // Pas dit aan afhankelijk van je vereisten
  
  // Retourneer true als de afstand kleiner is dan de drempelwaarde, anders false
  return (distance > thresholdDistance);
}

bool wallRight_() {
  int sensorValue = analogRead(sensorPinR);  // Lees de waarde van de sensor
  float voltage = sensorValue * (3.3 / 4095.0);  // Converteer de analoge waarde naar spanning
  
  // Omzetten van spanning naar afstand (zie datasheet voor de juiste formule)
  float distance = 12.08 * pow(voltage, -1.058); // Voorbeeldformule, deze kan afwijken
  
  // Bepaal de drempelwaarde voor muurdetectie (bijv. 30 cm)
  float thresholdDistance = 10.0; // Pas dit aan afhankelijk van je vereisten
  
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

// Constructor: initialiseer de pin en zet de variabelen op 0
Sensor::Sensor(int pin) {
    analogPin = pin;
    readIndex = 0;
    total = 0;
    averageDistance = 0;
    for (int i = 0; i < numReadings; i++) {
        readings[i] = 0;
    }
}

// Functie om de afstand te berekenen en de moving average toe te passen
float Sensor::getFilteredDistance() {
    // Lees de waarde van de geselecteerde sensor
    int sensorValue = analogRead(analogPin);

    // Bereken de afstand op basis van de analoge waarde
    float distance = calculateDistance(sensorValue);

    // Trek de oudste meting af van de totale som
    total = total - readings[readIndex];

    // Vervang de oudste meting met de nieuwe afstandswaarde
    readings[readIndex] = distance;

    // Voeg de nieuwe waarde toe aan de totale som
    total = total + readings[readIndex];

    // Ga naar de volgende index
    readIndex = readIndex + 1;

    // Als we het einde van de array hebben bereikt, begin opnieuw bij index 0
    if (readIndex >= numReadings) {
        readIndex = 0;
    }

    // Bereken het gemiddelde van de laatste metingen
    averageDistance = total / numReadings;

    // Retourneer de gefilterde afstand
    return averageDistance;
}

// Hulpfunctie om de afstand te berekenen op basis van de sensorwaarde
float Sensor::calculateDistance(int sensorValue) {
    // Converteer de analoge waarde naar spanning
    float voltage = sensorValue * (3.3 / 4095.0);

    // Omzetten van spanning naar afstand (gebruik de juiste formule voor je specifieke sensor)
    float distance = 12.08 * pow(voltage, -1.058);  // Voorbeeldformule
    return distance;
}
