#ifndef SENDER_PROTOCOL_H
#define SENDER_PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHP_SOF_HOST 0xA5u
#define SHP_SOF_MCU  0x5Au
#define SHP_VERSION  0x01u

enum shp_cmd
{
    SHP_CMD_GET_INFO = 0x01,
    SHP_CMD_SET_TIMING = 0x02,
    SHP_CMD_START_CLK = 0x03,
    SHP_CMD_STOP_CLK = 0x04,
    SHP_CMD_CLR_UNDERFLOW = 0x05,
    SHP_CMD_SET_SCOPE = 0x06,

    SHP_CMD_SEND_BYTES = 0x10,
    SHP_CMD_SEND_PACKET = 0x11,
    SHP_CMD_SEND_STRETCHED_BYTE = 0x12,
    SHP_CMD_SEND_SPECIAL = 0x13,
    SHP_CMD_APPEND_PACKET_CHUNK = 0x14,
    SHP_CMD_COMMIT_PACKET = 0x15,
    SHP_CMD_APPEND_RAW_CHUNK = 0x16,
    SHP_CMD_COMMIT_RAW_BYTES = 0x17,

    SHP_CMD_SET_GEN_OUT = 0x20,
    SHP_CMD_GET_GEN_IN = 0x21,

    SHP_CMD_GET_STATUS = 0x30,
    SHP_CMD_GET_STATS = 0x31,
    SHP_CMD_RESET_STATS = 0x32,

    SHP_CMD_RESET_DEVICE = 0x7F
};

enum shp_special
{
    SHP_SPECIAL_RESET = 0,
    SHP_SPECIAL_HARD_RESET = 1,
    SHP_SPECIAL_IDLE = 2,
    SHP_SPECIAL_WARBLE = 3
};

enum shp_status
{
    SHP_STATUS_OK = 0,
    SHP_STATUS_BAD_VERSION = 1,
    SHP_STATUS_BAD_COMMAND = 2,
    SHP_STATUS_BAD_LENGTH = 3,
    SHP_STATUS_BAD_CRC = 4,
    SHP_STATUS_BUSY = 5,
    SHP_STATUS_INTERNAL_ERROR = 6
};

#define SHP_PACKET_CHUNK_MAX 26u

typedef struct sender_status_payload
{
    bool running;
    bool underflow;
    uint32_t err_flags;
    uint16_t queue_depth;
    uint8_t gen1;
    uint8_t gen2;
} sender_status_payload_t;

typedef struct sender_stats_payload
{
    uint32_t packets_sent;
    uint32_t bytes_sent;
    uint32_t underruns;
} sender_stats_payload_t;

uint16_t sender_proto_crc16_ccitt(const uint8_t* data, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /* SENDER_PROTOCOL_H */