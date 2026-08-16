#ifndef SENDER_ENGINE_H
#define SENDER_ENGINE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "sender_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sender_engine
{
    bool running;
    bool underflow;
    bool scope_on;

    uint16_t clk0t_us;
    uint16_t clk0h_us;
    uint16_t clk1t_us;

    uint8_t gen_out;
    uint8_t gen_in1;
    uint8_t gen_in2;

    uint32_t packets_sent;
    uint32_t bytes_sent;
    uint32_t underruns;
    uint32_t err_flags;
} sender_engine_t;

void sender_engine_init(sender_engine_t* eng);
void sender_engine_reset(sender_engine_t* eng);

uint8_t sender_engine_set_timing(sender_engine_t* eng, uint16_t clk0t_us,
                                 uint16_t clk0h_us, uint16_t clk1t_us);
uint8_t sender_engine_start(sender_engine_t* eng);
uint8_t sender_engine_stop(sender_engine_t* eng);
uint8_t sender_engine_clear_underflow(sender_engine_t* eng);
uint8_t sender_engine_set_scope(sender_engine_t* eng, bool scope_on);

uint8_t sender_engine_send_bytes(sender_engine_t* eng, uint16_t repeat_count,
                                 uint8_t byte_value);
uint8_t sender_engine_send_packet(sender_engine_t* eng, const uint8_t* data,
                                  uint16_t size);
uint8_t sender_engine_send_raw_bytes(sender_engine_t* eng, const uint8_t* data,
                                     uint16_t size);
uint8_t sender_engine_send_raw_bytes_stretched(sender_engine_t* eng,
                                               const uint8_t* data,
                                               uint16_t size,
                                               uint16_t stretch_byte_index,
                                               uint16_t clk0t_us,
                                               uint16_t clk0h_us);
uint8_t sender_engine_send_raw_bytes_timed(sender_engine_t* eng,
                                           const uint8_t* data, uint16_t size,
                                           uint16_t bit_index1, uint16_t clk0t1_us,
                                           uint16_t clk0h1_us, uint16_t bit_index2,
                                           uint16_t clk0t2_us, uint16_t clk0h2_us);
uint8_t sender_engine_send_stretched_byte(sender_engine_t* eng,
                                          uint16_t clk0t_us,
                                          uint16_t clk0h_us,
                                          uint8_t byte_value);
uint8_t sender_engine_send_special(sender_engine_t* eng, uint8_t special_id);

uint8_t sender_engine_set_gen_out(sender_engine_t* eng, uint8_t value);
void sender_engine_get_gen_in(sender_engine_t* eng, uint8_t* gen1,
                              uint8_t* gen2);

void sender_engine_reset_stats(sender_engine_t* eng);
void sender_engine_get_status(const sender_engine_t* eng,
                              sender_status_payload_t* out);
void sender_engine_get_stats(const sender_engine_t* eng,
                             sender_stats_payload_t* out);

void sender_engine_get_non_idle_packet_event(uint32_t* event_count,
                                             uint8_t* last_address);

/* Called from TIM2_IRQHandler — do not call directly. */
void sender_engine_tim_irq_handler(void);

#ifdef __cplusplus
}
#endif

#endif /* SENDER_ENGINE_H */