#ifndef SENDER_TRANSPORT_H
#define SENDER_TRANSPORT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void sender_transport_init(void);

bool sender_transport_read_frame(uint8_t* frame, uint16_t capacity,
                                 uint16_t* frame_size);

void sender_transport_write_frame(const uint8_t* frame, uint16_t frame_size);

#ifdef __cplusplus
}
#endif

#endif /* SENDER_TRANSPORT_H */