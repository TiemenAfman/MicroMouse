#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>
#include <PubSubClient.h>  // Ensure to include PubSubClient for MQTT
#include "sendMessage.h"  

extern const int sensorPin;  // Analoge pin waar de sensor op is aangesloten// Pin-definities multiplexing
extern const int S0;
extern const int S1;
extern const int S2;
extern int lastValue;  // Initialize with a value that your sensor will not output

bool wallFront_();
bool wallLeft_();
bool wallRight_();
float wallDistance(int channel);
void checkAndSendMessage(PubSubClient& client, int sensorPin, const char* topic, int& lastValue);

#endif