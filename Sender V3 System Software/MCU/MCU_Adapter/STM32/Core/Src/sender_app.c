#include "sender_app.h"

#include <string.h>

#include "sender_engine.h"
#include "sender_parser.h"
#include "sender_transport.h"

static sender_engine_t g_engine;

static void sender_app_handle_request(const sender_request_t* req,
                                      sender_response_t* rsp)
{
    sender_status_payload_t status;
    sender_stats_payload_t stats;

    rsp->cmd = req->cmd;
    rsp->seq = req->seq;
    rsp->status = SHP_STATUS_OK;
    rsp->payload_len = 0;

    switch (req->cmd)
    {
        case SHP_CMD_GET_INFO:
            rsp->payload[0] = SHP_VERSION;
            rsp->payload[1] = 0x01;
            rsp->payload[2] = 0x00;
            rsp->payload[3] = 0x00;
            rsp->payload[4] = 0x02;
            rsp->payload_len = 5;
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
            rsp->payload_len = 10;
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
}

void sender_app_poll(void)
{
    uint8_t rx[1024];
    uint16_t rx_size = 0;
    sender_request_t req;
    sender_response_t rsp;
    uint8_t tx[1024];
    uint16_t tx_size;

    if (!sender_transport_read_frame(rx, (uint16_t)sizeof(rx), &rx_size))
    {
        return;
    }

    if (!sender_parser_parse_request(rx, rx_size, &req))
    {
        return;
    }

    memset(&rsp, 0, sizeof(rsp));
    sender_app_handle_request(&req, &rsp);

    tx_size = sender_parser_build_response_frame(&rsp, tx, (uint16_t)sizeof(tx));
    if (tx_size > 0)
    {
        sender_transport_write_frame(tx, tx_size);
    }
}
