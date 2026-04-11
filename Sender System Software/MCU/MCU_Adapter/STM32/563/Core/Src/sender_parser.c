#include "sender_parser.h"

#include <string.h>

bool sender_parser_parse_request(const uint8_t* frame, uint16_t frame_size,
                                 sender_request_t* out_req)
{
    uint16_t payload_len;
    uint16_t crc_rx;
    uint16_t crc_calc;

    if (frame == 0 || out_req == 0)
    {
        return false;
    }

    if (frame_size < 8u)
    {
        return false;
    }
    if (frame[0] != SHP_SOF_HOST || frame[1] != SHP_VERSION)
    {
        return false;
    }

    payload_len = (uint16_t)frame[4] | ((uint16_t)frame[5] << 8);
    if ((uint16_t)(payload_len + 8u) != frame_size)
    {
        return false;
    }

    crc_rx = (uint16_t)frame[6u + payload_len] |
             ((uint16_t)frame[7u + payload_len] << 8);
    crc_calc = sender_proto_crc16_ccitt(&frame[1], (uint16_t)(5u + payload_len));
    if (crc_rx != crc_calc)
    {
        return false;
    }

    out_req->cmd = frame[2];
    out_req->seq = frame[3];
    out_req->payload_len = payload_len;
    out_req->payload = &frame[6];
    return true;
}

uint16_t sender_parser_build_response_frame(const sender_response_t* rsp,
                                            uint8_t* out_frame,
                                            uint16_t out_capacity)
{
    uint16_t total_size;
    uint16_t crc;

    if (rsp == 0 || out_frame == 0)
    {
        return 0;
    }

    total_size = (uint16_t)(9u + rsp->payload_len);
    if (total_size > out_capacity)
    {
        return 0;
    }

    out_frame[0] = SHP_SOF_MCU;
    out_frame[1] = SHP_VERSION;
    out_frame[2] = rsp->cmd;
    out_frame[3] = rsp->seq;
    out_frame[4] = rsp->status;
    out_frame[5] = (uint8_t)(rsp->payload_len & 0xFFu);
    out_frame[6] = (uint8_t)((rsp->payload_len >> 8) & 0xFFu);

    if (rsp->payload_len > 0)
    {
        memcpy(&out_frame[7], rsp->payload, rsp->payload_len);
    }

    crc = sender_proto_crc16_ccitt(&out_frame[1], (uint16_t)(6u + rsp->payload_len));
    out_frame[7u + rsp->payload_len] = (uint8_t)(crc & 0xFFu);
    out_frame[8u + rsp->payload_len] = (uint8_t)((crc >> 8) & 0xFFu);

    return total_size;
}