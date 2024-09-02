#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>
#include <PubSubClient.h>  // Ensure to include PubSubClient for MQTT
#include "sendMessage.h"  

extern const int sensorPin;  // Analoge pin waar de sensor op is aangesloten
extern int lastValue;  // Initialize with a value that your sensor will not output

bool wallFront_();
float wallDistance();
void checkAndSendMessage(PubSubClient& client, int sensorPin, const char* topic, int& lastValue);

#endif