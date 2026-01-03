#include "comm.h"

// 定义帧头帧尾
#define FRAME_HEAD 0xAA
#define FRAME_TAIL 0x55

// 计算帧总长度（固定字段 + 数据 + CRC + 帧尾）
#define FRAME_FIXED_LEN 3       // head(1) + len(1) + cmd(1)
#define CRC_LEN 2               // CRC16占2字节
#define TAIL_LEN 1              // 帧尾占1字节
#define GET_FRAME_TOTAL_LEN(data_len) (FRAME_FIXED_LEN + data_len + CRC_LEN + TAIL_LEN)

// 接收状态枚举
typedef enum {
    RECV_STATE_WAIT_HEAD,    // 等待帧头
    RECV_STATE_GET_LEN,      // 接收长度字段
    RECV_STATE_GET_CMD,      // 接收命令字段
    RECV_STATE_GET_DATA,     // 接收柔性数据
    RECV_STATE_GET_CRC_H,    // 接收CRC高8位
    RECV_STATE_GET_CRC_L,    // 接收CRC低8位
    RECV_STATE_GET_TAIL      // 接收帧尾
} RecvState;

#pragma pack(1)
// 接收处理器（含柔性数组缓冲区）
typedef struct {
    RecvState state;          // 当前接收状态
    uint8_t data_len;         // 柔性数据长度（由len字段解析）
    uint8_t recv_cnt;         // 已接收字节计数
    uint16_t crc_recv;        // 接收的CRC值
} RecvHandler;

// 带柔性数组的通信帧结构体
typedef struct {
    uint8_t head;          // 帧头 (1字节)
    uint8_t len;           // 长度：命令(1) + 数据(N) (1字节)
    uint8_t cmd;           // 命令码 (1字节)
    uint8_t data[];        // 柔性数组：存储实际数据（0~MAX_DATA_LEN字节）
    // 注：CRC和帧尾不放入结构体，避免柔性数组被隔断；传输时拼接在数据后
} CommFrame;
#pragma pack()

uint8_t packedFrameBuffer[MAX_COMM_DATA_LEN];
uint8_t parsedFrameBuffer[MAX_COMM_DATA_LEN];
RecvHandler parseHandler = {RECV_STATE_WAIT_HEAD, 0, 0, 0};

static const char *COMM_TAG = "COMM";

// CRC16-Modbus算法（输入任意长度字节流）
uint16_t CalculateCrc16(uint8_t *buf, uint16_t len) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= buf[i];
        for (uint8_t j = 0; j < 8; j++) {
            crc = (crc & 0x0001) ? ((crc >> 1) ^ 0xA001) : (crc >> 1);
        }
    }
    return crc;
}

/**
 * @brief  封装并发送带柔性数组的通信帧
 * @param  cmd：命令码
 * @param  data：待发送数据（可为空）
 * @param  data_len：数据长度（0~MAX_DATA_LEN）
 * @retval 0：成功；-1：参数错误
 */
int PackFrame(Comm_Cmd_e cmd, uint8_t *data, uint8_t data_len, uint32_t *frameAddr) {
    // 1. 参数校验
    if (data_len > MAX_COMM_DATA_LEN) return -1;

    // 2. 填充固定字段
    CommFrame *frame = (CommFrame*)packedFrameBuffer;
    frame->head = FRAME_HEAD;
    frame->len = 1 + data_len;  // len = 命令(1) + 数据长度
    frame->cmd = cmd;

    // 3. 填充柔性数组数据（无数据则跳过）
    if (data_len > 0 && data != NULL) {
        memcpy(frame->data, data, data_len);
    }

    // 4. 计算CRC（校验范围：len + cmd + 柔性数据）
    uint8_t crc_src_len = frame->len + 1;  // len(1) + cmd(1) + data(data_len)
    uint8_t *crc_src = &frame->len;        // 从len字段开始作为CRC校验源
    uint16_t crc_val = CalculateCrc16(crc_src, crc_src_len);

    // 5. 拼接CRC和帧尾（放在柔性数组后）
    uint16_t crc_pos = FRAME_FIXED_LEN + data_len;           // CRC起始位置
    packedFrameBuffer[crc_pos] = (crc_val >> 8) & 0xFF;      // CRC高8位
    packedFrameBuffer[crc_pos + 1] = crc_val & 0xFF;         // CRC低8位
    packedFrameBuffer[crc_pos + 2] = FRAME_TAIL;                   // 帧尾

    *frameAddr = (uint32_t)packedFrameBuffer;
    return crc_pos + 3;
}

/**
 * @brief  字节级解析柔性数组帧
 * @param  handler：接收处理器指针
 * @param  byte：当前接收的字节
 * @retval 0：解析中/失败；1：解析成功（返回有效帧）
 */
uint8_t ParseFrame(uint8_t byte, Comm_Cmd_e *pFrameCmd, uint32_t *frameDataAddr) {
    switch (parseHandler.state) {
        case RECV_STATE_WAIT_HEAD:  // 1. 等待帧头
            if (byte == FRAME_HEAD) {
                parseHandler.recv_cnt = 1;  // 已接收帧头
                parseHandler.state = RECV_STATE_GET_LEN;
            }
            break;

        case RECV_STATE_GET_LEN:  // 2. 接收长度字段
            if (byte < 1 || byte > (1 + MAX_COMM_DATA_LEN)) {  // len最小为1（仅命令）
                parseHandler.state = RECV_STATE_WAIT_HEAD;  // 长度非法，重置
                break;
            }
            parseHandler.data_len = byte - 1;  // 数据长度 = len - 命令(1)
            // 填充帧头和长度
            CommFrame *frame = (CommFrame*)parsedFrameBuffer;
            frame->head = FRAME_HEAD;
            frame->len = byte;
            parseHandler.recv_cnt++;
            parseHandler.state = RECV_STATE_GET_CMD;
            break;

        case RECV_STATE_GET_CMD:  // 3. 接收命令字段
            ((CommFrame*)parsedFrameBuffer)->cmd = byte;
            parseHandler.recv_cnt++;
            // 无数据则直接跳转到CRC
            parseHandler.state = (parseHandler.data_len == 0) ? RECV_STATE_GET_CRC_H : RECV_STATE_GET_DATA;
            break;

        case RECV_STATE_GET_DATA:  // 4. 接收柔性数据
            // 填充数据到柔性数组
            uint8_t data_pos = FRAME_FIXED_LEN + (parseHandler.recv_cnt - FRAME_FIXED_LEN);
            parsedFrameBuffer[data_pos] = byte;
            parseHandler.recv_cnt++;
            // 数据接收完毕，跳转到CRC
            if (parseHandler.recv_cnt == (FRAME_FIXED_LEN + parseHandler.data_len)) {
                parseHandler.state = RECV_STATE_GET_CRC_H;
            }
            break;

        case RECV_STATE_GET_CRC_H:  // 5. 接收CRC高8位
            parseHandler.crc_recv = (byte << 8);
            parseHandler.recv_cnt++;
            parseHandler.state = RECV_STATE_GET_CRC_L;
            break;

        case RECV_STATE_GET_CRC_L:  // 6. 接收CRC低8位
            parseHandler.crc_recv |= byte;
            parseHandler.recv_cnt++;
            parseHandler.state = RECV_STATE_GET_TAIL;
            break;

        case RECV_STATE_GET_TAIL:  // 7. 接收帧尾并校验
            if (byte == FRAME_TAIL) {
                CommFrame *frame = (CommFrame*)parsedFrameBuffer;
                // 计算CRC（校验范围：len + cmd + 柔性数据）
                uint8_t crc_src_len = frame->len + 1;
                uint8_t *crc_src = &frame->len;
                uint16_t crc_calc = CalculateCrc16(crc_src, crc_src_len);

                // CRC校验通过
                if (crc_calc == parseHandler.crc_recv) {
                    parseHandler.state = RECV_STATE_WAIT_HEAD;  // 重置状态
                    *pFrameCmd = frame->cmd;
                    *frameDataAddr = (uint32_t)frame->data;
                    return 1;  // 解析成功
                }
            }
            // 帧尾错误或CRC失败，重置
            parseHandler.state = RECV_STATE_WAIT_HEAD;
            break;

        default:
            parseHandler.state = RECV_STATE_WAIT_HEAD;
            break;
    }
    return 0;
}