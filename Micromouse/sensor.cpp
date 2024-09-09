#include "sensor.h"

// Define global variables
const int sensorPin = A0;  // Analoge pin waar de sensor op is aangesloten// Pin-definities multiplexing
const int S0 = D5;
const int S1 = D6;
const int S2 = D7;

int lastValue = -1;  // Initialize with a value that your sensor will not output

bool wallFront_() {
  // Stel multiplexer in
  digitalWrite(S0, LOW);
  digitalWrite(S1, LOW);
  digitalWrite(S2, LOW);
  
  // Wacht even om de multiplexer de tijd te geven om te schakelen
  delay(5);  // Korte vertraging, aanpasbaar
  
  int sensorValue = analogRead(sensorPin);  // Lees de waarde van de sensor
  float voltage = sensorValue * (5.0 / 1023.0);  // Converteer de analoge waarde naar spanning
  
  // Omzetten van spanning naar afstand (zie datasheet voor de juiste formule)
  float distance = 12.08 * pow(voltage, -1.058); // Voorbeeldformule, deze kan afwijken
  
  // Bepaal de drempelwaarde voor muurdetectie (bijv. 30 cm)
  float thresholdDistance = 3.0; // Pas dit aan afhankelijk van je vereisten
  
  // Retourneer true als de afstand kleiner is dan de drempelwaarde, anders false
  return (distance < thresholdDistance);
}

bool wallLeft_() {
  // Stel multiplexer in
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);
  digitalWrite(S2, LOW);
  
  // Wacht even om de multiplexer de tijd te geven om te schakelen
  delay(5);  // Korte vertraging, aanpasbaar
  
  int sensorValue = analogRead(sensorPin);  // Lees de waarde van de sensor
  float voltage = sensorValue * (5.0 / 1023.0);  // Converteer de analoge waarde naar spanning
  
  // Omzetten van spanning naar afstand (zie datasheet voor de juiste formule)
  float distance = 12.08 * pow(voltage, -1.058); // Voorbeeldformule, deze kan afwijken
  
  // Bepaal de drempelwaarde voor muurdetectie (bijv. 30 cm)
  float thresholdDistance = 3.0; // Pas dit aan afhankelijk van je vereisten
  
  // Retourneer true als de afstand kleiner is dan de drempelwaarde, anders false
  return (distance < thresholdDistance);
}

bool wallRight_() {
  // Stel multiplexer in
  digitalWrite(S0, LOW);
  digitalWrite(S1, HIGH);
  digitalWrite(S2, LOW);
  
  // Wacht even om de multiplexer de tijd te geven om te schakelen
  delay(5);  // Korte vertraging, aanpasbaar
  
  int sensorValue = analogRead(sensorPin);  // Lees de waarde van de sensor
  float voltage = sensorValue * (5.0 / 1023.0);  // Converteer de analoge waarde naar spanning
  
  // Omzetten van spanning naar afstand (zie datasheet voor de juiste formule)
  float distance = 12.08 * pow(voltage, -1.058); // Voorbeeldformule, deze kan afwijken
  
  // Bepaal de drempelwaarde voor muurdetectie (bijv. 30 cm)
  float thresholdDistance = 3.0; // Pas dit aan afhankelijk van je vereisten
  
  // Retourneer true als de afstand kleiner is dan de drempelwaarde, anders false
  return (distance < thresholdDistance);
}

float wallDistance(int channel){
  // Stel de selectiepins in op het gewenste kanaal
  digitalWrite(S0, bitRead(channel, 0));  // Zet het minst significante bit (LSB)
  digitalWrite(S1, bitRead(channel, 1));  // Zet het middelste bit
  digitalWrite(S2, bitRead(channel, 2));  // Zet het meest significante bit (MSB)
  
  // Wacht even om de multiplexer de tijd te geven om te schakelen
  delay(5);  // Korte vertraging, aanpasbaar
  
  // Lees de waarde van de geselecteerde sensor
  int sensorValue = analogRead(sensorPin);
  
  // Converteer de analoge waarde naar spanning
  float voltage = sensorValue * (5.0 / 1023.0);
  
  // Omzetten van spanning naar afstand (gebruik de juiste formule voor je specifieke sensor)
  float distance = 12.08 * pow(voltage, -1.058); // Voorbeeldformule, aanpasbaar afhankelijk van de datasheet
  
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
