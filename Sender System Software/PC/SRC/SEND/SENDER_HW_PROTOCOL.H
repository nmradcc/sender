#ifndef SENDER_HW_PROTOCOL_H_DECLARED
#define SENDER_HW_PROTOCOL_H_DECLARED

#include <stdint.h>

/*
 * MCU adapter protocol constants shared by host and firmware.
 */

static const uint8_t SHP_SOF_HOST = 0xA5;
static const uint8_t SHP_SOF_MCU = 0x5A;
static const uint8_t SHP_VERSION = 0x01;

/* Command IDs */
enum Shp_cmd
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
    SHP_CMD_COMMIT_RAW_STRETCHED_BYTE = 0x18,
    SHP_CMD_COMMIT_RAW_TIMED_BITS = 0x19,

    SHP_CMD_SET_GEN_OUT = 0x20,
    SHP_CMD_GET_GEN_IN = 0x21,

    SHP_CMD_GET_STATUS = 0x30,
    SHP_CMD_GET_STATS = 0x31,
    SHP_CMD_RESET_STATS = 0x32,
    SHP_CMD_GET_MCU_VERSION = 0x33,

    SHP_CMD_RESET_DEVICE = 0x7F
};

/* SHP_CMD_SEND_SPECIAL payload IDs */
enum Shp_special
{
    SHP_SPECIAL_RESET = 0,
    SHP_SPECIAL_HARD_RESET = 1,
    SHP_SPECIAL_IDLE = 2,
    SHP_SPECIAL_WARBLE = 3
};

/* Status codes in MCU responses */
enum Shp_status
{
    SHP_STATUS_OK = 0,
    SHP_STATUS_BAD_VERSION = 1,
    SHP_STATUS_BAD_COMMAND = 2,
    SHP_STATUS_BAD_LENGTH = 3,
    SHP_STATUS_BAD_CRC = 4,
    SHP_STATUS_BUSY = 5,
    SHP_STATUS_INTERNAL_ERROR = 6
};

/* Host default payload size for chunked packet append commands. */
static const uint16_t SHP_PACKET_CHUNK_MAX = 26;

/*
 * On-wire frame format:
 * SOF(1), Version(1), Command(1), Sequence(1), Length(2), Payload(N), CRC16(2)
 * CRC16-CCITT is calculated over Version..Payload (SOF excluded).
 */

#endif /* SENDER_HW_PROTOCOL_H_DECLARED */
