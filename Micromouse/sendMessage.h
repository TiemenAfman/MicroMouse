#ifndef SEND_MESSAGE_H
#define SEND_MESSAGE_H

#include <PubSubClient.h>
#include <map>

// Declaration of the publishValue function
void publishValue(PubSubClient& client, const char* topic, String value);

#endif // SEND_MESSAGE_H
