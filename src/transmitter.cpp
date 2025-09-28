#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <FlexCAN_T4.h>

const int LORA_CS   = 6;  // Chip Select (NSS)
const int LORA_RST  = 9;  // Reset pin
const int LORA_DIO0 = 20; // DIO0 interrupt pin

const long LORA_FREQUENCY = 915E6; // 915 MHz (US band)
const int LORA_TX_POWER = 20;      // Transmission power (5-20 dBm)

const byte LOCAL_ADDRESS = 0x0e; // transmitter's address
const byte DESTINATION   = 0xAB; // Receiver's address

const uint32_t CAN_BAUD_RATE = 1000000; // 1 Mbps

FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;

/**
 * @brief Sends a LoRa packet containing raw byte data.
 *
 * @param canID The ID of the CAN message.
 * @param dataBuffer A pointer to the byte array (CAN data payload).
 * @param dataLength The number of bytes in the dataBuffer.
 */
void sendLoRaCanMessage(uint32_t canID, const uint8_t* dataBuffer, uint8_t dataLength) {
    LoRa.beginPacket();
    // --- Header ---
    LoRa.write(DESTINATION);    // Destination address
    LoRa.write(LOCAL_ADDRESS);  // Source address
    LoRa.write(canID);
    LoRa.write(dataBuffer, dataLength);
    LoRa.endPacket(); // Send the packet
}

/**
 * @brief Initializes the LoRa module with predefined settings.
 */
bool initializeLoRa() {
    LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);

    if (!LoRa.begin(LORA_FREQUENCY)) {
        return false;
    }
    LoRa.setTxPower(LORA_TX_POWER);
    return true;
}

/**
 * @brief Initializes the CAN controller.
 */
void initializeCAN() {
    can2.begin();
    can2.setBaudRate(CAN_BAUD_RATE);
    can2.setMaxMB(16);
    can2.enableFIFO();
    can2.mailboxStatus();
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 2000); // Wait for Serial Monitor
    initializeCAN();
    initializeLoRa(); 
}

void loop() {
    can2.events(); 
    CAN_message_t incomingMessage;
    if (can2.read(incomingMessage)) {
        switch (incomingMessage.id) {
            case 0x490:
                sendLoRaCanMessage(incomingMessage.id, incomingMessage.buf, incomingMessage.len);
            case 0x480:
                sendLoRaCanMessage(incomingMessage.id, incomingMessage.buf, incomingMessage.len);
            default:
                break;
        }
    }
}