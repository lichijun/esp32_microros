#ifndef __COMM_H__
#define __COMM_H__

#include <stdint.h>
#include <string.h>

#define MAX_COMM_DATA_LEN 256

// enum must less than 256
typedef enum
{
    COMM_CMD_TEST = 0,
    COMM_CMD_IMU = 1,
} Comm_Cmd_e;

#pragma pack(1)
typedef struct
{
    uint8_t u8Num;
    uint16_t u16Num;
    float fNum;
} Comm_Data_Test_st;

typedef struct
{
    float fAccX;
    float fAccY;
    float fAccZ;
    float fGyroX;
    float fGyroY;
    float fGyroZ;
} Comm_Data_Imu_st;

#pragma pack()


uint16_t CalculateCrc16(uint8_t *buf, uint16_t len);
int PackFrame(Comm_Cmd_e cmd, uint8_t *data, uint8_t data_len, uint32_t *frameAddr);
uint8_t ParseFrame(uint8_t byte, Comm_Cmd_e *pFrameCmd, uint32_t *frameDataAddr);

#endif