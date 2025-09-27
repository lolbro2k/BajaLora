#include <Arduino.h>
#include <FlexCAN_T4.h>
#include "can_types.h"
#include "lora_config.h"

// Create a FlexCAN_T4 object for CAN2
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;

// Variables for non-blocking timing
unsigned long lastLedToggle = 0;
const unsigned long LED_INTERVAL = 500;

// Circular buffer for CAN messages
const int CAN_BUFFER_SIZE = 32;
volatile CAN_message_t canBuffer[CAN_BUFFER_SIZE];
volatile int bufferHead = 0;
volatile int bufferTail = 0;
volatile int bufferCount = 0;

// CAN interrupt handler with circular buffer
void canSniff(const CAN_message_t &msg) {
    // Only add message if buffer isn't full
    if (bufferCount < CAN_BUFFER_SIZE) {
        // Copy message fields individually (FlexCAN_T4 doesn't have copy constructor)
        canBuffer[bufferHead].id = msg.id;
        canBuffer[bufferHead].len = msg.len;
        // Note: flags field may not be available in all FlexCAN_T4 versions
        // canBuffer[bufferHead].flags = msg.flags;
        for (int i = 0; i < 8; i++) {
            canBuffer[bufferHead].buf[i] = msg.buf[i];
        }
        bufferHead = (bufferHead + 1) % CAN_BUFFER_SIZE;
        bufferCount++;
    }
    // If buffer is full, oldest messages are automatically overwritten
}

// Function to safely get a message from the buffer
bool getCanMessage(CAN_message_t &msg) {
    noInterrupts();
    if (bufferCount > 0) {
        // Copy message fields individually
        msg.id = canBuffer[bufferTail].id;
        msg.len = canBuffer[bufferTail].len;
        // Note: flags field may not be available in all FlexCAN_T4 versions
        // msg.flags = canBuffer[bufferTail].flags;
        for (int i = 0; i < 8; i++) {
            msg.buf[i] = canBuffer[bufferTail].buf[i];
        }
        bufferTail = (bufferTail + 1) % CAN_BUFFER_SIZE;
        bufferCount--;
        interrupts();
        return true;
    }
    interrupts();
    return false;
}

void setup() {
    Serial.begin(115200);
    while (!Serial); // Wait for the serial port to connect

    // Setup CAN bus
    can2.begin();
    can2.setBaudRate(1000000); // 1 Mbps
    can2.onReceive(canSniff); // Set interrupt handler for CAN messages

    // Setup LoRa
    setup_lora();

    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    // Process CAN messages from buffer
    CAN_message_t msg;
    while (getCanMessage(msg)) {
        // Process the CAN message
        RaceGradeIMU imu_data;
        if (parse_can_message(msg, imu_data)) {
            // We have new IMU data, let's send it over LoRa
            String message = "IMU: lat=" + String(imu_data.lateral_acc) +
                             ", lon=" + String(imu_data.longitudinal_acc) +
                             ", vert=" + String(imu_data.vertical_acc);
            send_lora_message(message);
        }
    }

    // Non-blocking LED toggle for system health indication
    if (millis() - lastLedToggle >= LED_INTERVAL) {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        lastLedToggle = millis();
    }
    
    // Give other tasks a chance to run
    yield();
}