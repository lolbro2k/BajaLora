#ifndef LORA_CONFIG_H
#define LORA_CONFIG_H

#include <LoRa.h>

// Define the pins for your LoRa module
#define LORA_CS_PIN    9
#define LORA_RESET_PIN 6
#define LORA_IRQ_PIN   2 // You'll need to connect the LoRa module's IRQ pin

void setup_lora();
void send_lora_message(const String& message);
void on_receive(int packetSize); // Callback function for when a LoRa message is received

#endif // LORA_CONFIG_H