#include "sender_engine.h"

#include <string.h>

void sender_engine_init(sender_engine_t* eng)
{
    if (eng == 0)
    {
        return;
    }

    memset(eng, 0, sizeof(*eng));
    eng->clk0t_us = 200;
    eng->clk0h_us = 100;
    eng->clk1t_us = 116;
}

void sender_engine_reset(sender_engine_t* eng)
{
    sender_engine_init(eng);
}

uint8_t sender_engine_set_timing(sender_engine_t* eng, uint16_t clk0t_us,
                                 uint16_t clk0h_us, uint16_t clk1t_us)
{
    if (eng == 0)
    {
        return SHP_STATUS_INTERNAL_ERROR;
    }

    eng->clk0t_us = clk0t_us;
    eng->clk0h_us = clk0h_us;
    eng->clk1t_us = clk1t_us;
    return SHP_STATUS_OK;
}

uint8_t sender_engine_start(sender_engine_t* eng)
{
    if (eng == 0)
    {
        return SHP_STATUS_INTERNAL_ERROR;
    }

    eng->running = true;
    return SHP_STATUS_OK;
}

uint8_t sender_engine_stop(sender_engine_t* eng)
{
    if (eng == 0)
    {
        return SHP_STATUS_INTERNAL_ERROR;
    }

    eng->running = false;
    return SHP_STATUS_OK;
}

uint8_t sender_engine_clear_underflow(sender_engine_t* eng)
{
    if (eng == 0)
    {
        return SHP_STATUS_INTERNAL_ERROR;
    }

    eng->underflow = false;
    return SHP_STATUS_OK;
}

uint8_t sender_engine_set_scope(sender_engine_t* eng, bool scope_on)
{
    if (eng == 0)
    {
        return SHP_STATUS_INTERNAL_ERROR;
    }

    eng->scope_on = scope_on;
    return SHP_STATUS_OK;
}

uint8_t sender_engine_send_bytes(sender_engine_t* eng, uint16_t repeat_count,
                                 uint8_t byte_value)
{
    (void)byte_value;

    if (eng == 0)
    {
        return SHP_STATUS_INTERNAL_ERROR;
    }
    if (!eng->running)
    {
        return SHP_STATUS_BUSY;
    }

    eng->bytes_sent += repeat_count;
    if (repeat_count > 0)
    {
        eng->packets_sent += 1;
    }
    return SHP_STATUS_OK;
}

uint8_t sender_engine_send_packet(sender_engine_t* eng, const uint8_t* data,
                                  uint16_t size)
{
    (void)data;

    if (eng == 0)
    {
        return SHP_STATUS_INTERNAL_ERROR;
    }
    if (!eng->running)
    {
        return SHP_STATUS_BUSY;
    }

    eng->bytes_sent += size;
    if (size > 0)
    {
        eng->packets_sent += 1;
    }
    return SHP_STATUS_OK;
}

uint8_t sender_engine_send_stretched_byte(sender_engine_t* eng,
                                          uint16_t clk0t_us,
                                          uint16_t clk0h_us,
                                          uint8_t byte_value)
{
    if (eng == 0)
    {
        return SHP_STATUS_INTERNAL_ERROR;
    }

    (void)sender_engine_set_timing(eng, clk0t_us, clk0h_us, eng->clk1t_us);
    return sender_engine_send_bytes(eng, 1, byte_value);
}

uint8_t sender_engine_send_special(sender_engine_t* eng, uint8_t special_id)
{
    if (eng == 0)
    {
        return SHP_STATUS_INTERNAL_ERROR;
    }
    if (!eng->running)
    {
        return SHP_STATUS_BUSY;
    }

    switch (special_id)
    {
        case SHP_SPECIAL_RESET:
        case SHP_SPECIAL_HARD_RESET:
        case SHP_SPECIAL_IDLE:
            eng->packets_sent += 1;
            eng->bytes_sent += 5;
            return SHP_STATUS_OK;

        case SHP_SPECIAL_WARBLE:
            eng->packets_sent += 2;
            eng->bytes_sent += 4096;
            return SHP_STATUS_OK;

        default:
            return SHP_STATUS_BAD_LENGTH;
    }
}

uint8_t sender_engine_set_gen_out(sender_engine_t* eng, uint8_t value)
{
    if (eng == 0)
    {
        return SHP_STATUS_INTERNAL_ERROR;
    }

    eng->gen_out = value;
    return SHP_STATUS_OK;
}

void sender_engine_get_gen_in(const sender_engine_t* eng, uint8_t* gen1,
                              uint8_t* gen2)
{
    if (eng == 0 || gen1 == 0 || gen2 == 0)
    {
        return;
    }

    *gen1 = eng->gen_in1;
    *gen2 = eng->gen_in2;
}

void sender_engine_get_status(const sender_engine_t* eng,
                              sender_status_payload_t* out)
{
    if (eng == 0 || out == 0)
    {
        return;
    }

    out->running = eng->running;
    out->underflow = eng->underflow;
    out->err_flags = eng->err_flags;
    out->queue_depth = 0;
    out->gen1 = eng->gen_in1;
    out->gen2 = eng->gen_in2;
}

void sender_engine_get_stats(const sender_engine_t* eng,
                             sender_stats_payload_t* out)
{
    if (eng == 0 || out == 0)
    {
        return;
    }

    out->packets_sent = eng->packets_sent;
    out->bytes_sent = eng->bytes_sent;
    out->underruns = eng->underruns;
}
