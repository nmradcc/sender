#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/*
 * Firmware starter stub for the Sender MCU adapter protocol.
 * This file is intentionally minimal and does not target a specific MCU SDK.
 */

#define SHP_SOF_HOST 0xA5u
#define SHP_SOF_MCU  0x5Au
#define SHP_VERSION  0x01u

enum
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

    SHP_CMD_SET_GEN_OUT = 0x20,
    SHP_CMD_GET_GEN_IN = 0x21,

    SHP_CMD_GET_STATUS = 0x30,
    SHP_CMD_GET_STATS = 0x31,

    SHP_CMD_RESET_DEVICE = 0x7F
};

enum
{
    SHP_STATUS_OK = 0,
    SHP_STATUS_BAD_VERSION = 1,
    SHP_STATUS_BAD_COMMAND = 2,
    SHP_STATUS_BAD_LENGTH = 3,
    SHP_STATUS_BAD_CRC = 4,
    SHP_STATUS_BUSY = 5,
    SHP_STATUS_INTERNAL_ERROR = 6
};

typedef struct SenderState
{
    bool running;
    bool underflow;
    uint16_t clk0t_us;
    uint16_t clk0h_us;
    uint16_t clk1t_us;
    uint32_t packets_sent;
    uint32_t bytes_sent;
    uint32_t underruns;
    uint8_t gen_out;
    uint8_t gen_in1;
    uint8_t gen_in2;
} SenderState;

static SenderState g_state;

static uint16_t crc16_ccitt(const uint8_t* data, uint16_t size)
{
    uint16_t crc = 0xFFFFu;
    uint16_t i;

    for (i = 0; i < size; ++i)
    {
        uint8_t b = data[i];
        uint8_t bit;

        crc ^= (uint16_t)b << 8;
        for (bit = 0; bit < 8; ++bit)
        {
            if ((crc & 0x8000u) != 0u)
            {
                crc = (uint16_t)((crc << 1) ^ 0x1021u);
            }
            else
            {
                crc = (uint16_t)(crc << 1);
            }
        }
    }

    return crc;
}

/* Platform hook placeholders */
static bool transport_read_frame(uint8_t* frame, uint16_t* frame_size)
{
    (void)frame;
    (void)frame_size;
    return false;
}

static void transport_write_frame(const uint8_t* frame, uint16_t frame_size)
{
    (void)frame;
    (void)frame_size;
}

static void engine_set_timing(uint16_t clk0t, uint16_t clk0h, uint16_t clk1t)
{
    g_state.clk0t_us = clk0t;
    g_state.clk0h_us = clk0h;
    g_state.clk1t_us = clk1t;
}

static void engine_start(void)
{
    g_state.running = true;
}

static void engine_stop(void)
{
    g_state.running = false;
}

static void process_command(uint8_t cmd, const uint8_t* payload,
                            uint16_t payload_len, uint8_t* out_status,
                            uint8_t* out_payload, uint16_t* out_payload_len)
{
    *out_status = SHP_STATUS_OK;
    *out_payload_len = 0;

    switch (cmd)
    {
        case SHP_CMD_GET_INFO:
            out_payload[0] = SHP_VERSION;
            out_payload[1] = 0x01; /* capability bitfield low */
            out_payload[2] = 0x00; /* capability bitfield high */
            out_payload[3] = 0x00; /* max payload low byte placeholder */
            out_payload[4] = 0x04; /* max payload high byte placeholder */
            *out_payload_len = 5;
            break;

        case SHP_CMD_SET_TIMING:
            if (payload_len != 6)
            {
                *out_status = SHP_STATUS_BAD_LENGTH;
                break;
            }
            engine_set_timing((uint16_t)payload[0] | ((uint16_t)payload[1] << 8),
                              (uint16_t)payload[2] | ((uint16_t)payload[3] << 8),
                              (uint16_t)payload[4] | ((uint16_t)payload[5] << 8));
            break;

        case SHP_CMD_START_CLK:
            engine_start();
            break;

        case SHP_CMD_STOP_CLK:
            engine_stop();
            break;

        case SHP_CMD_CLR_UNDERFLOW:
            g_state.underflow = false;
            break;

        case SHP_CMD_SET_SCOPE:
            if (payload_len != 1)
            {
                *out_status = SHP_STATUS_BAD_LENGTH;
                break;
            }
            break;

        case SHP_CMD_SEND_BYTES:
        case SHP_CMD_SEND_PACKET:
        case SHP_CMD_SEND_STRETCHED_BYTE:
        case SHP_CMD_SEND_SPECIAL:
            if (!g_state.running)
            {
                *out_status = SHP_STATUS_BUSY;
                break;
            }
            break;

        case SHP_CMD_SET_GEN_OUT:
            if (payload_len != 1)
            {
                *out_status = SHP_STATUS_BAD_LENGTH;
                break;
            }
            g_state.gen_out = payload[0];
            break;

        case SHP_CMD_GET_GEN_IN:
            out_payload[0] = g_state.gen_in1;
            out_payload[1] = g_state.gen_in2;
            *out_payload_len = 2;
            break;

        case SHP_CMD_GET_STATUS:
            out_payload[0] = g_state.running ? 1u : 0u;
            out_payload[1] = g_state.underflow ? 1u : 0u;
            out_payload[2] = 0;
            out_payload[3] = 0;
            out_payload[4] = 0;
            out_payload[5] = 0;
            out_payload[6] = 0;
            out_payload[7] = 0;
            out_payload[8] = g_state.gen_in1;
            out_payload[9] = g_state.gen_in2;
            *out_payload_len = 10;
            break;

        case SHP_CMD_GET_STATS:
            out_payload[0] = (uint8_t)(g_state.packets_sent & 0xFFu);
            out_payload[1] = (uint8_t)((g_state.packets_sent >> 8) & 0xFFu);
            out_payload[2] = (uint8_t)((g_state.packets_sent >> 16) & 0xFFu);
            out_payload[3] = (uint8_t)((g_state.packets_sent >> 24) & 0xFFu);
            out_payload[4] = (uint8_t)(g_state.bytes_sent & 0xFFu);
            out_payload[5] = (uint8_t)((g_state.bytes_sent >> 8) & 0xFFu);
            out_payload[6] = (uint8_t)((g_state.bytes_sent >> 16) & 0xFFu);
            out_payload[7] = (uint8_t)((g_state.bytes_sent >> 24) & 0xFFu);
            out_payload[8] = (uint8_t)(g_state.underruns & 0xFFu);
            out_payload[9] = (uint8_t)((g_state.underruns >> 8) & 0xFFu);
            out_payload[10] = (uint8_t)((g_state.underruns >> 16) & 0xFFu);
            out_payload[11] = (uint8_t)((g_state.underruns >> 24) & 0xFFu);
            *out_payload_len = 12;
            break;

        case SHP_CMD_RESET_DEVICE:
            memset(&g_state, 0, sizeof(g_state));
            break;

        default:
            *out_status = SHP_STATUS_BAD_COMMAND;
            break;
    }
}

int main(void)
{
    uint8_t rx[1024];
    uint8_t tx[1024];
    uint16_t rx_len;

    memset(&g_state, 0, sizeof(g_state));

    for (;;)
    {
        uint8_t cmd;
        uint8_t seq;
        uint16_t payload_len;
        uint16_t crc_rx;
        uint16_t crc_calc;
        uint8_t status;
        uint16_t rsp_payload_len;

        if (!transport_read_frame(rx, &rx_len))
        {
            continue;
        }

        if (rx_len < 8u || rx[0] != SHP_SOF_HOST)
        {
            continue;
        }

        if (rx[1] != SHP_VERSION)
        {
            continue;
        }

        cmd = rx[2];
        seq = rx[3];
        payload_len = (uint16_t)rx[4] | ((uint16_t)rx[5] << 8);

        if ((uint16_t)(payload_len + 8u) != rx_len)
        {
            continue;
        }

        crc_rx = (uint16_t)rx[6u + payload_len] |
                 ((uint16_t)rx[7u + payload_len] << 8);
        crc_calc = crc16_ccitt(&rx[1], (uint16_t)(5u + payload_len));
        if (crc_rx != crc_calc)
        {
            continue;
        }

        process_command(cmd, &rx[6], payload_len, &status, &tx[7],
                        &rsp_payload_len);

        tx[0] = SHP_SOF_MCU;
        tx[1] = SHP_VERSION;
        tx[2] = cmd;
        tx[3] = seq;
        {
            uint16_t crc;

            tx[4] = status;
            tx[5] = (uint8_t)(rsp_payload_len & 0xFFu);
            tx[6] = (uint8_t)((rsp_payload_len >> 8) & 0xFFu);

            crc = crc16_ccitt(&tx[1], (uint16_t)(6u + rsp_payload_len));
            tx[7u + rsp_payload_len] = (uint8_t)(crc & 0xFFu);
            tx[8u + rsp_payload_len] = (uint8_t)((crc >> 8) & 0xFFu);

            transport_write_frame(tx, (uint16_t)(9u + rsp_payload_len));
        }
    }

    return 0;
}
