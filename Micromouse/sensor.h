#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>
#include <PubSubClient.h>  // Ensure to include PubSubClient for MQTT
#include "sendMessage.h"  

extern const int sensorPinF;  // Analoge pin waar de sensor op is aangesloten
extern const int sensorPinL;  // Analoge pin waar de sensor op is aangesloten
extern const int sensorPinR;  // Analoge pin waar de sensor op is aangesloten
extern int lastValue;  // Initialize with a value that your sensor will not output

bool wallFront_();
bool wallLeft_();
bool wallRight_();
float wallDistance(int channel);
void checkAndSendMessage(PubSubClient& client, int sensorPin, const char* topic, int& lastValue);

const int numReadings = 10;  // Aantal metingen voor de moving average

class Sensor {
public:
    // Constructor om de pin te initialiseren
    Sensor(int pin);

    // Functie om de gefilterde afstand te berekenen
    float getFilteredDistance();

private:
    int analogPin;                 // De analoge pin voor de sensor
    float readings[numReadings];    // Array voor afstandsmetingen
    int readIndex;                  // Index voor de huidige meting
    float total;                    // Totale som van de metingen
    float averageDistance;          // Gemiddelde afstand

    // Hulpfunctie om een nieuwe afstand te berekenen
    float calculateDistance(int sensorValue);
};

#endif