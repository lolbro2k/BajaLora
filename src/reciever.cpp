// Teensy 4.0 + RFM95W - Receiver for raw CAN-over-LoRa packets
#include <SPI.h>
#include <LoRa.h>

// Must match transmitter wiring
const int LORA_CS   = 6;
const int LORA_RST  = 9;
const int LORA_DIO0 = 20;

// Must match transmitter addressing
const byte LOCAL_ADDRESS = 0xAB;   // This node (receiver)
                               

void onReceive(int packetSize) {
    byte dest    = LoRa.read();
    byte src     = LoRa.read();
    byte canIdLSB = LoRa.read();  // Only the low byte of CAN ID with current transmitter
    int payloadLen = packetSize - 3;

    uint8_t data[8] = {0};
    int copyLen = payloadLen > 8 ? 8 : payloadLen; // CAN frames max 8 bytes
    for (int i = 0; i < copyLen && LoRa.available(); i++) {
        data[i] = (uint8_t)LoRa.read();
    }
    // Drain any excess (shouldn't happen with proper transmitter)
    while (LoRa.available()) (void)LoRa.read();

    // Filter: only process frames addressed to us or broadcast (0xFF)
    if (dest != LOCAL_ADDRESS && dest != 0xFF) {
        return;
    }

    // Print decoded packet
    Serial.print("SRC:0x");
    Serial.print(src, HEX);
    Serial.print(" DEST:0x");
    Serial.print(dest, HEX);
    Serial.print(" CAN_ID:0x");
    if (canIdLSB < 0x10) Serial.print('0');
    Serial.print(canIdLSB, HEX);
    Serial.print(" LEN:");
    Serial.print(copyLen);
    Serial.print(" DATA:");
    for (int i = 0; i < copyLen; i++) {
        Serial.print(' ');
        if (data[i] < 0x10) Serial.print('0');
        Serial.print(data[i], HEX);
    }
    Serial.println();

    // RF diagnostics
    Serial.print("RSSI:");
    Serial.print(LoRa.packetRssi());
    Serial.print(" SNR:");
    Serial.println(LoRa.packetSnr(), 1);

    // Ready for next packet
    LoRa.receive();
}

void setup() {
    Serial.begin(115200);
    unsigned long t0 = millis();
    while (!Serial && (millis() - t0) < 1500) {}

    SPI.begin();
    LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);
    if (!LoRa.begin(915E6)) {
        while (true) {
            // Hard fault indication (blink if needed)
        }
    }
    LoRa.onReceive(onReceive);
    LoRa.receive();

    Serial.println("Receiver ready (raw CAN bridge mode).");
}

void loop() {
    // Nothing needed; onReceive callback handles packets
}