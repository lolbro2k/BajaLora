#ifndef CAN_TYPES_H
#define CAN_TYPES_H

#include <FlexCAN_T4.h>

// A struct to hold the data from a RaceGrade IMU CAN message
struct RaceGradeIMU {
    int16_t lateral_acc;
    int16_t longitudinal_acc;
    int16_t vertical_acc;
    int16_t yaw_rot;
    int16_t pitch_rot;
    int16_t roll_rot;
};

// Function to parse a CAN message and, if it's an IMU message, populate the struct
bool parse_can_message(const CAN_message_t &msg, RaceGradeIMU &imu_data);

#endif // CAN_TYPES_H