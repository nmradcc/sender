#include "sender_transport.h"

/*
 * Replace this stub with your STM32 transport implementation.
 * Recommended first target: USB CDC.
 */

void sender_transport_init(void)
{
}

bool sender_transport_read_frame(uint8_t* frame, uint16_t capacity,
                                 uint16_t* frame_size)
{
    (void)frame;
    (void)capacity;
    (void)frame_size;
    return false;
}

void sender_transport_write_frame(const uint8_t* frame, uint16_t frame_size)
{
    (void)frame;
    (void)frame_size;
}
