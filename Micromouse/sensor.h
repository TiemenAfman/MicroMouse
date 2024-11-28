#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>
//#include <PubSubClient.h>  // Ensure to include PubSubClient for MQTT
//#include "sendMessage.h"  

extern const int sensorPinF;  // Analoge pin waar de sensor op is aangesloten
extern const int sensorPinL;  // Analoge pin waar de sensor op is aangesloten
extern const int sensorPinR;  // Analoge pin waar de sensor op is aangesloten
extern int lastValue;  // Initialize with a value that your sensor will not output

bool wallFront();
bool wallLeft();
bool wallRight();
float wallDistance(int channel);
//void checkAndSendMessage(PubSubClient& client, int sensorPin, const char* topic, int& lastValue);

#endif