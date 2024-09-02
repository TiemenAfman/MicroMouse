#include "sendMessage.h"

// Map to keep track of last values published for each topic
std::map<String, String> lastPublishedValues;

// Function to publish a string value to a specific MQTT topic
void publishValue(PubSubClient& client, const char* topic, String value) {
  String topicStr = String(topic);
  
  // Check if the value has changed
  if (lastPublishedValues[topicStr] != value) {
    // Update the last published value for this topic
    lastPublishedValues[topicStr] = value;

    // Print the message to the Serial Monitor (for debugging purposes)
    Serial.print("Publishing message: ");
    Serial.println(value);
    
    // Publish the message to the specified MQTT topic
    client.publish(topic, value.c_str());
  }
}
