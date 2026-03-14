#ifndef SENDER_TRANSPORT_H
#define SENDER_TRANSPORT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Transport contract for STM32 project.
 * Implement with USB CDC, UART DMA, or any byte stream.
 */
void sender_transport_init(void);

/*
 * Reads one complete SHP frame into buffer.
 * Returns true when a full frame is available.
 */
bool sender_transport_read_frame(uint8_t* frame, uint16_t capacity,
                                 uint16_t* frame_size);

/* Sends one complete SHP frame. */
void sender_transport_write_frame(const uint8_t* frame, uint16_t frame_size);

#ifdef __cplusplus
}
#endif

#endif /* SENDER_TRANSPORT_H */
