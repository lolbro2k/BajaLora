#include "can_types.h"

bool parse_can_message(const CAN_message_t &msg, RaceGradeIMU &imu_data) {
    // The RaceGrade IMU sends data on IDs 1136 and 1137
    if (msg.id == 1136 || msg.id == 1137) {
        // The data is sent as a series of 16-bit integers in big-endian format.
        // We need to swap the byte order because the Teensy is little-endian.
        imu_data.lateral_acc = (msg.buf[0] << 8) | msg.buf[1];
        imu_data.longitudinal_acc = (msg.buf[2] << 8) | msg.buf[3];
        imu_data.vertical_acc = (msg.buf[4] << 8) | msg.buf[5];
        // The rotation data would be in the next CAN frame (ID 1137)
        // This example only shows one frame, but you could extend it to handle both.
        return true;
    }
    return false;
}