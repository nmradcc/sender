#ifndef SENDER_APP_H
#define SENDER_APP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void sender_app_init(void);
void sender_app_poll(void);
void sender_app_get_non_idle_packet_event(uint32_t* event_count,
										  uint8_t* last_address);

#ifdef __cplusplus
}
#endif

#endif /* SENDER_APP_H */