// Teensy 4.0 + Adafruit RFM95W (915 MHz) configuration
#include <SPI.h>
#include <LoRa.h>

// Pin mapping for Teensy 4.0 (adjust if wired differently)
const int LORA_CS   = 10; // NSS
// Teensy 4.0 + Adafruit RFM95W (915 MHz) configuration
#include <SPI.h>
#include <LoRa.h>

// Pin mapping for Teensy 4.0 (adjust if wired differently)
const int LORA_CS   = 6; // NSS
const int LORA_RST  = 9;  // RST
const int LORA_DIO0 = 10;  // DIO0

// Device addressing
byte msgCount      = 0;      // count of outgoing messages
byte localAddress  = 0xAB;   // this node
byte destination   = 0xBA;   // target node

// Mode control
bool sendMode = false;
long sendModeStart = 0;
int  sendModeDuration = 5000; // ms

void sendMessage(String outgoing);
void onReceive(int packetSize);

void setup() {
  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 3000) { /* allow headless start */ }

  Serial.println("LoRa Duplex (Teensy 4.0)");

  SPI.begin();
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(915E6)) {
    Serial.println("LoRa init failed. Check wiring/antenna.");
    while (true) { delay(1000); }
  }

  // Optional radio parameters - tune as needed
  LoRa.setSpreadingFactor(7);         // 6..12
  LoRa.setSignalBandwidth(125E3);     // 125E3 typical
  LoRa.setCodingRate4(5);             // denominator = 4/5
  LoRa.setPreambleLength(8);
  LoRa.setTxPower(17, PA_OUTPUT_PA_BOOST_PIN); // 2..20 (watch current draw)
  // LoRa.enableCrc(); // uncomment if both ends enable CRC

  LoRa.onReceive(onReceive);
  LoRa.receive();

  Serial.println("LoRa init succeeded.");
  Serial.println("Type 245 or 246 in Serial Monitor to simulate commands.");
}

void loop() {
  // Allow manual command injection over USB Serial for quick tests
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line == "245") {
      sendMode = true;
      sendModeStart = millis();
      Serial.println("[Serial CMD] Enter send mode");
    } else if (line == "246") {
      sendMode = false;
      LoRa.receive();
      Serial.println("[Serial CMD] Enter receive mode");
    }
  }

  // Timeout for send mode
  if (sendMode && (millis() - sendModeStart > sendModeDuration)) {
    sendMode = false;
    LoRa.receive();
    Serial.println("Send mode timeout -> receive mode");
  }

  if (sendMode) {
    String message = "Hello from Teensy to 0xBA!";
    sendMessage(message);
    Serial.println("Sent: " + message);
    LoRa.receive();
    sendMode = false; // single send then back
    Serial.println("Back to receive mode");
  }
}

void sendMessage(String outgoing) {
  LoRa.beginPacket();
  LoRa.write(destination);
  LoRa.write(localAddress);
  LoRa.write(msgCount);
  LoRa.write(outgoing.length());
  LoRa.print(outgoing);
  LoRa.endPacket(); // blocking until transmitted
  msgCount++;
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;

  int  recipient      = LoRa.read();
  byte sender         = LoRa.read();
  byte incomingMsgId  = LoRa.read();
  byte incomingLength = LoRa.read();

  String incoming;
  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {
    Serial.println("Length mismatch - discarding");
    return;
  }
  if (recipient != localAddress && recipient != 0xFF) {
    return; // not for us
  }

  Serial.print("From 0x"); Serial.print(sender, HEX);
  Serial.print(" To 0x");  Serial.print(recipient, HEX);
  Serial.print(" ID ");    Serial.print(incomingMsgId);
  Serial.print(" RSSI ");  Serial.print(LoRa.packetRssi());
  Serial.print(" SNR ");   Serial.println(LoRa.packetSnr(), 1);
  Serial.print("Payload: "); Serial.println(incoming);

  if (incoming == "245") {
    sendMode = true;
    sendModeStart = millis();
    Serial.println("Cmd -> send mode");
  } else if (incoming == "246") {
    sendMode = false;
    LoRa.receive();
    Serial.println("Cmd -> receive mode");
  }

  sendMessage("ACK");
  LoRa.receive();
}