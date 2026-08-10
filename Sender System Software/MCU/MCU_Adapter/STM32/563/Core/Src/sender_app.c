#include "sender_app.h"

#include <string.h>

#include "sender_engine.h"
#include "sender_parser.h"
#include "sender_transport.h"

static sender_engine_t g_engine;
static uint8_t g_rx_frame[1024];
static uint8_t g_tx_frame[1024];
static sender_request_t g_request;
static sender_response_t g_response;
static uint8_t g_packet_stage[1024];
static uint16_t g_packet_stage_len;
static uint8_t g_packet_stage_mode;

#define PACKET_STAGE_NONE 0u
#define PACKET_STAGE_DCC_PACKET 1u
#define PACKET_STAGE_RAW_BYTES 2u

static void sender_app_handle_request(const sender_request_t* req,
                                      sender_response_t* rsp)
{
    sender_status_payload_t status;
    sender_stats_payload_t stats;

    rsp->cmd = req->cmd;
    rsp->seq = req->seq;
    rsp->status = SHP_STATUS_OK;
    rsp->payload_len = 0;

    if (req->cmd != SHP_CMD_APPEND_PACKET_CHUNK &&
        req->cmd != SHP_CMD_COMMIT_PACKET &&
        req->cmd != SHP_CMD_APPEND_RAW_CHUNK &&
        req->cmd != SHP_CMD_COMMIT_RAW_BYTES)
    {
        g_packet_stage_len = 0u;
        g_packet_stage_mode = PACKET_STAGE_NONE;
    }

    switch (req->cmd)
    {
        case SHP_CMD_GET_INFO:
            rsp->payload[0] = SHP_VERSION;
            rsp->payload[1] = SHP_MCU_FW_VERSION_MAJOR;
            rsp->payload[2] = SHP_MCU_FW_VERSION_MINOR;
            rsp->payload[3] = SHP_MCU_FW_VERSION_PATCH;
            rsp->payload[4] = 0x02;
            rsp->payload_len = 5;
            break;

        case SHP_CMD_GET_MCU_VERSION:
            if (req->payload_len != 0u)
            {
                rsp->status = SHP_STATUS_BAD_LENGTH;
                break;
            }

            rsp->payload[0] = (uint8_t)(SHP_MCU_FW_VERSION_NUMBER & 0xFFu);
            rsp->payload[1] =
                (uint8_t)((SHP_MCU_FW_VERSION_NUMBER >> 8) & 0xFFu);
            rsp->payload[2] =
                (uint8_t)((SHP_MCU_FW_VERSION_NUMBER >> 16) & 0xFFu);
            rsp->payload[3] =
                (uint8_t)((SHP_MCU_FW_VERSION_NUMBER >> 24) & 0xFFu);
            rsp->payload_len = 4;
            break;

        case SHP_CMD_SET_TIMING:
            if (req->payload_len != 6)
            {
                rsp->status = SHP_STATUS_BAD_LENGTH;
                break;
            }
            rsp->status = sender_engine_set_timing(
                &g_engine,
                (uint16_t)req->payload[0] | ((uint16_t)req->payload[1] << 8),
                (uint16_t)req->payload[2] | ((uint16_t)req->payload[3] << 8),
                (uint16_t)req->payload[4] | ((uint16_t)req->payload[5] << 8));
            break;

        case SHP_CMD_START_CLK:
            rsp->status = sender_engine_start(&g_engine);
            break;

        case SHP_CMD_STOP_CLK:
            rsp->status = sender_engine_stop(&g_engine);
            break;

        case SHP_CMD_CLR_UNDERFLOW:
            rsp->status = sender_engine_clear_underflow(&g_engine);
            break;

        case SHP_CMD_SET_SCOPE:
            if (req->payload_len != 1)
            {
                rsp->status = SHP_STATUS_BAD_LENGTH;
                break;
            }
            rsp->status = sender_engine_set_scope(&g_engine, req->payload[0] != 0);
            break;

        case SHP_CMD_SEND_BYTES:
            if (req->payload_len != 3)
            {
                rsp->status = SHP_STATUS_BAD_LENGTH;
                break;
            }
            rsp->status = sender_engine_send_bytes(
                &g_engine,
                (uint16_t)req->payload[0] | ((uint16_t)req->payload[1] << 8),
                req->payload[2]);
            break;

        case SHP_CMD_SEND_PACKET:
            rsp->status = sender_engine_send_packet(
                &g_engine, req->payload, req->payload_len);
            break;

        case SHP_CMD_APPEND_PACKET_CHUNK:
        case SHP_CMD_APPEND_RAW_CHUNK:
            if (req->payload_len == 0u || req->payload_len > SHP_PACKET_CHUNK_MAX)
            {
                g_packet_stage_len = 0u;
                g_packet_stage_mode = PACKET_STAGE_NONE;
                rsp->status = SHP_STATUS_BAD_LENGTH;
                break;
            }

            if (g_packet_stage_len == 0u)
            {
                g_packet_stage_mode =
                    (req->cmd == SHP_CMD_APPEND_PACKET_CHUNK)
                        ? PACKET_STAGE_DCC_PACKET
                        : PACKET_STAGE_RAW_BYTES;
            }
            else if (((req->cmd == SHP_CMD_APPEND_PACKET_CHUNK) &&
                      (g_packet_stage_mode != PACKET_STAGE_DCC_PACKET)) ||
                     ((req->cmd == SHP_CMD_APPEND_RAW_CHUNK) &&
                      (g_packet_stage_mode != PACKET_STAGE_RAW_BYTES)))
            {
                g_packet_stage_len = 0u;
                g_packet_stage_mode = PACKET_STAGE_NONE;
                rsp->status = SHP_STATUS_BAD_LENGTH;
                break;
            }

            if ((uint32_t)g_packet_stage_len + (uint32_t)req->payload_len >
                (uint32_t)sizeof(g_packet_stage))
            {
                g_packet_stage_len = 0u;
                g_packet_stage_mode = PACKET_STAGE_NONE;
                rsp->status = SHP_STATUS_BAD_LENGTH;
                break;
            }

            memcpy(&g_packet_stage[g_packet_stage_len], req->payload, req->payload_len);
            g_packet_stage_len = (uint16_t)(g_packet_stage_len + req->payload_len);
            rsp->status = SHP_STATUS_OK;
            break;

        case SHP_CMD_COMMIT_PACKET:
            if (req->payload_len != 0u || g_packet_stage_len == 0u ||
                g_packet_stage_mode != PACKET_STAGE_DCC_PACKET)
            {
                g_packet_stage_len = 0u;
                g_packet_stage_mode = PACKET_STAGE_NONE;
                rsp->status = SHP_STATUS_BAD_LENGTH;
                break;
            }

            rsp->status = sender_engine_send_packet(&g_engine, g_packet_stage,
                                                    g_packet_stage_len);
            g_packet_stage_len = 0u;
            g_packet_stage_mode = PACKET_STAGE_NONE;
            break;

        case SHP_CMD_COMMIT_RAW_BYTES:
            if (req->payload_len != 0u || g_packet_stage_len == 0u ||
                g_packet_stage_mode != PACKET_STAGE_RAW_BYTES)
            {
                g_packet_stage_len = 0u;
                g_packet_stage_mode = PACKET_STAGE_NONE;
                rsp->status = SHP_STATUS_BAD_LENGTH;
                break;
            }

            rsp->status = sender_engine_send_raw_bytes(&g_engine,
                                                       g_packet_stage,
                                                       g_packet_stage_len);
            g_packet_stage_len = 0u;
            g_packet_stage_mode = PACKET_STAGE_NONE;
            break;

        case SHP_CMD_SEND_STRETCHED_BYTE:
            if (req->payload_len != 5)
            {
                rsp->status = SHP_STATUS_BAD_LENGTH;
                break;
            }
            rsp->status = sender_engine_send_stretched_byte(
                &g_engine,
                (uint16_t)req->payload[0] | ((uint16_t)req->payload[1] << 8),
                (uint16_t)req->payload[2] | ((uint16_t)req->payload[3] << 8),
                req->payload[4]);
            break;

        case SHP_CMD_SEND_SPECIAL:
            if (req->payload_len != 1)
            {
                rsp->status = SHP_STATUS_BAD_LENGTH;
                break;
            }
            rsp->status = sender_engine_send_special(&g_engine, req->payload[0]);
            break;

        case SHP_CMD_SET_GEN_OUT:
            if (req->payload_len != 1)
            {
                rsp->status = SHP_STATUS_BAD_LENGTH;
                break;
            }
            rsp->status = sender_engine_set_gen_out(&g_engine, req->payload[0]);
            break;

        case SHP_CMD_GET_GEN_IN:
            sender_engine_get_gen_in(&g_engine, &rsp->payload[0], &rsp->payload[1]);
            rsp->payload_len = 2;
            break;

        case SHP_CMD_GET_STATUS:
            sender_engine_get_status(&g_engine, &status);
            rsp->payload[0] = status.running ? 1u : 0u;
            rsp->payload[1] = status.underflow ? 1u : 0u;
            rsp->payload[2] = (uint8_t)(status.err_flags & 0xFFu);
            rsp->payload[3] = (uint8_t)((status.err_flags >> 8) & 0xFFu);
            rsp->payload[4] = (uint8_t)((status.err_flags >> 16) & 0xFFu);
            rsp->payload[5] = (uint8_t)((status.err_flags >> 24) & 0xFFu);
            rsp->payload[6] = (uint8_t)(status.queue_depth & 0xFFu);
            rsp->payload[7] = (uint8_t)((status.queue_depth >> 8) & 0xFFu);
            rsp->payload[8] = status.gen1;
            rsp->payload[9] = status.gen2;
            rsp->payload[10] = (uint8_t)(status.non_idle_event_count & 0xFFu);
            rsp->payload[11] =
                (uint8_t)((status.non_idle_event_count >> 8) & 0xFFu);
            rsp->payload[12] =
                (uint8_t)((status.non_idle_event_count >> 16) & 0xFFu);
            rsp->payload[13] =
                (uint8_t)((status.non_idle_event_count >> 24) & 0xFFu);
            rsp->payload[14] = status.last_non_idle_address;
            rsp->payload_len = 15;
            break;

        case SHP_CMD_GET_STATS:
            sender_engine_get_stats(&g_engine, &stats);
            rsp->payload[0] = (uint8_t)(stats.packets_sent & 0xFFu);
            rsp->payload[1] = (uint8_t)((stats.packets_sent >> 8) & 0xFFu);
            rsp->payload[2] = (uint8_t)((stats.packets_sent >> 16) & 0xFFu);
            rsp->payload[3] = (uint8_t)((stats.packets_sent >> 24) & 0xFFu);
            rsp->payload[4] = (uint8_t)(stats.bytes_sent & 0xFFu);
            rsp->payload[5] = (uint8_t)((stats.bytes_sent >> 8) & 0xFFu);
            rsp->payload[6] = (uint8_t)((stats.bytes_sent >> 16) & 0xFFu);
            rsp->payload[7] = (uint8_t)((stats.bytes_sent >> 24) & 0xFFu);
            rsp->payload[8] = (uint8_t)(stats.underruns & 0xFFu);
            rsp->payload[9] = (uint8_t)((stats.underruns >> 8) & 0xFFu);
            rsp->payload[10] = (uint8_t)((stats.underruns >> 16) & 0xFFu);
            rsp->payload[11] = (uint8_t)((stats.underruns >> 24) & 0xFFu);
            rsp->payload_len = 12;
            break;

        case SHP_CMD_RESET_STATS:
            if (req->payload_len != 0u)
            {
                rsp->status = SHP_STATUS_BAD_LENGTH;
                break;
            }
            sender_engine_reset_stats(&g_engine);
            break;

        case SHP_CMD_RESET_DEVICE:
            sender_engine_reset(&g_engine);
            break;

        default:
            rsp->status = SHP_STATUS_BAD_COMMAND;
            break;
    }
}

void sender_app_init(void)
{
    sender_transport_init();
    sender_engine_init(&g_engine);
    g_packet_stage_len = 0u;
    g_packet_stage_mode = PACKET_STAGE_NONE;
}

void sender_app_poll(void)
{
    uint16_t rx_size = 0;
    uint16_t tx_size;

    if (!sender_transport_read_frame(g_rx_frame, (uint16_t)sizeof(g_rx_frame),
                                     &rx_size))
    {
        return;
    }

    if (!sender_parser_parse_request(g_rx_frame, rx_size, &g_request))
    {
        return;
    }

    memset(&g_response, 0, sizeof(g_response));
    sender_app_handle_request(&g_request, &g_response);

    tx_size = sender_parser_build_response_frame(
        &g_response, g_tx_frame, (uint16_t)sizeof(g_tx_frame));
    if (tx_size > 0)
    {
        sender_transport_write_frame(g_tx_frame, tx_size);
    }
}

void sender_app_get_non_idle_packet_event(uint32_t *event_count,
                                          uint8_t *last_address)
{
    sender_engine_get_non_idle_packet_event(event_count, last_address);
}