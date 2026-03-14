#ifndef SENDER_PARSER_H
#define SENDER_PARSER_H

#include <stdbool.h>
#include <stdint.h>

#include "sender_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sender_request
{
    uint8_t cmd;
    uint8_t seq;
    uint16_t payload_len;
    const uint8_t* payload;
} sender_request_t;

typedef struct sender_response
{
    uint8_t cmd;
    uint8_t seq;
    uint8_t status;
    uint16_t payload_len;
    uint8_t payload[512];
} sender_response_t;

bool sender_parser_parse_request(const uint8_t* frame, uint16_t frame_size,
                                 sender_request_t* out_req);

uint16_t sender_parser_build_response_frame(const sender_response_t* rsp,
                                            uint8_t* out_frame,
                                            uint16_t out_capacity);

#ifdef __cplusplus
}
#endif

#endif /* SENDER_PARSER_H */
