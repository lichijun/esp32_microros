#pragma once

#include <stdint.h>

void micro_ros_task(void *arg);
void publish_int32(int32_t data);
void update_int32(int32_t data);
void update_imu_data(float acc_x, float acc_y, float acc_z, float gyro_x, float gyro_y, float gyro_z);