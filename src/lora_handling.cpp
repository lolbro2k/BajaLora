#include "lora_config.h"

void setup_lora() {
    LoRa.setPins(LORA_CS_PIN, LORA_RESET_PIN, LORA_IRQ_PIN);
    if (!LoRa.begin(915E6)) { // Set the frequency to 915 MHz
        Serial.println("Starting LoRa failed!");
        while (1);
    }
    LoRa.onReceive(on_receive); // Register the receive callback
    LoRa.receive(); // Put the radio into receive mode
}

void send_lora_message(const String& message) {
    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();
    LoRa.receive(); // Go back into receive mode
}

void on_receive(int packetSize) {
    if (packetSize == 0) return;

    String incoming = "";
    while (LoRa.available()) {
        incoming += (char)LoRa.read();
    }

    Serial.print("LoRa Received: ");
    Serial.println(incoming);
}