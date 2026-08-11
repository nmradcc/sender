#include "sender_engine.h"

#include <string.h>

#include "main.h"
#include "stm32h5xx_hal.h"

/* ----------------------------------------------------------------------- *
 * DCC output hardware assignment                                           *
 *                                                                          *
 * TIM5 (32-bit GP timer, APB1).  PCLK1 = 250 MHz.                        *
 * PSC = 249 -> timer tick = 1 us.                                         *
 * CH1 in Output Compare Toggle mode drives PA0 (TIM5_CH1 / AF2).         *
 *                                                                          *
 * The DCC waveform is produced by an interrupt-driven half-period FIFO:   *
 *  - Thread pushes half-period values (in us) into g_hp_fifo.             *
 *  - TIM5 CC1 ISR pops one value per event and advances CCR1 by that      *
 *    amount; the output toggles automatically in hardware at the match.   *
 *  - When the FIFO is empty the ISR inserts DCC_IDLE_HP_US (58 us) so     *
 *    the output remains alive as a stream of one-bits.                    *
 * ----------------------------------------------------------------------- */

#define DCC_TIM_INSTANCE    TIM5
#define DCC_TIM_IRQn        TIM5_IRQn
#define DCC_TIM_IRQ_PRIO    5u          /* below USB (0), above HAL tick (15) */
#define DCC_TIM_PSC         249u        /* 250 MHz / 250 = 1 MHz -> 1 us/tick */
#define DCC_GPIO_PORT       GPIOA
#define DCC_GPIO_PIN        GPIO_PIN_0  /* PA0 = TIM5_CH1 (AF2) */
#define DCC_GPIO_AF         GPIO_AF2_TIM5

/* Half-period FIFO (single-producer / single-consumer, power-of-2 size). */
#define DCC_FIFO_SIZE       4096u
#define DCC_FIFO_MASK       (DCC_FIFO_SIZE - 1u)
#define DCC_ADR_EVT_FIFO_SIZE 64u
#define DCC_ADR_EVT_FIFO_MASK (DCC_ADR_EVT_FIFO_SIZE - 1u)
#define DCC_ADR_EVT_SET      1u
#define DCC_ADR_EVT_CLEAR    0u
#define DCC_HPS_PER_BIT      2u
#define DCC_HPS_TO_FIRST_BIT_IN_BYTE (7u * DCC_HPS_PER_BIT)
/*
 * TIM5 CC ISR runs at half-period boundaries. Scheduling clear +2 half-periods
 * after first address-bit start clears at the end of that bit cell.
 */
#define DCC_HPS_TO_ADDR_PULSE_CLEAR DCC_HPS_PER_BIT
#define DCC_HPS_TO_FIRST_ADDR_BIT ((DCC_PREAMBLE_BITS + 1u) * 2u)

static volatile uint16_t g_hp_fifo[DCC_FIFO_SIZE];
static volatile uint32_t g_hp_head;     /* ISR consumer  */
static volatile uint32_t g_hp_tail;     /* thread producer */
static volatile uint32_t g_hp_enqueued_total;
static volatile uint32_t g_hp_dequeued_total;

static volatile uint32_t g_adr_evt_fifo[DCC_ADR_EVT_FIFO_SIZE];
static volatile uint8_t g_adr_evt_addr_fifo[DCC_ADR_EVT_FIFO_SIZE];
static volatile uint8_t g_adr_evt_action_fifo[DCC_ADR_EVT_FIFO_SIZE];
static volatile uint32_t g_adr_evt_head;
static volatile uint32_t g_adr_evt_tail;

static volatile uint32_t g_non_idle_packet_events;
static volatile uint8_t g_last_non_idle_address;

typedef enum dcc_mon_state
{
    DCC_MON_WAIT_PREAMBLE = 0,
    DCC_MON_READ_BYTE,
    DCC_MON_WAIT_DELIM
} dcc_mon_state_t;

static dcc_mon_state_t g_mon_state;
static uint8_t g_mon_preamble_ones;
static uint8_t g_mon_byte_accum;
static uint8_t g_mon_bit_count;
static bool g_mon_first_byte_pending;

extern TIM_HandleTypeDef htim5;

/* DCC packet/special constants. */
#define DCC_PREAMBLE_BITS   14u
#define DCC_RESET_BITS      25u
#define DCC_HARD_RESET_BITS 50u
#define DCC_IDLE_HP_US      58u         /* half-period for a DCC "1" bit */
#define DCC_MON_MIN_PREAMBLE_ONES 10u

static void dcc_monitor_reset(void)
{
    g_mon_state = DCC_MON_WAIT_PREAMBLE;
    g_mon_preamble_ones = 0u;
    g_mon_byte_accum = 0u;
    g_mon_bit_count = 0u;
    g_mon_first_byte_pending = false;
}

static void dcc_monitor_note_address(uint8_t address)
{
    if (address != 0xFFu && address != 0x00u)
    {
        g_last_non_idle_address = address;
        g_non_idle_packet_events += 1u;
    }
}

static void dcc_schedule_address_event(uint8_t address, uint32_t hp_index)
{
    uint32_t set_slot;
    uint32_t clear_slot;
    uint32_t next_tail;

    if (address == 0xFFu || address == 0x00u)
    {
        return;
    }

    set_slot = g_adr_evt_tail;
    clear_slot = (set_slot + 1u) & DCC_ADR_EVT_FIFO_MASK;
    next_tail = (clear_slot + 1u) & DCC_ADR_EVT_FIFO_MASK;
    if (next_tail == g_adr_evt_head)
    {
        return;
    }

    g_adr_evt_fifo[set_slot] = hp_index;
    g_adr_evt_addr_fifo[set_slot] = address;
    g_adr_evt_action_fifo[set_slot] = DCC_ADR_EVT_SET;

    g_adr_evt_fifo[clear_slot] = hp_index + DCC_HPS_TO_ADDR_PULSE_CLEAR;
    g_adr_evt_addr_fifo[clear_slot] = 0u;
    g_adr_evt_action_fifo[clear_slot] = DCC_ADR_EVT_CLEAR;

    __DMB();
    g_adr_evt_tail = next_tail;
}

static void dcc_monitor_consume_bit(bool one, uint32_t bit_hp_index)
{
    switch (g_mon_state)
    {
        case DCC_MON_WAIT_PREAMBLE:
            if (one)
            {
                if (g_mon_preamble_ones < 0xFFu)
                {
                    g_mon_preamble_ones += 1u;
                }
            }
            else
            {
                if (g_mon_preamble_ones >= DCC_MON_MIN_PREAMBLE_ONES)
                {
                    g_mon_state = DCC_MON_READ_BYTE;
                    g_mon_first_byte_pending = true;
                    g_mon_bit_count = 0u;
                    g_mon_byte_accum = 0u;
                }
                g_mon_preamble_ones = 0u;
            }
            break;

        case DCC_MON_READ_BYTE:
            g_mon_byte_accum = (uint8_t)((g_mon_byte_accum << 1u) | (one ? 1u : 0u));
            g_mon_bit_count += 1u;
            if (g_mon_bit_count >= 8u)
            {
                if (g_mon_first_byte_pending)
                {
                    if (g_mon_byte_accum != 0xFFu && g_mon_byte_accum != 0x00u)
                    {
                        dcc_schedule_address_event(
                            g_mon_byte_accum,
                            bit_hp_index - DCC_HPS_TO_FIRST_BIT_IN_BYTE);
                    }
                    g_mon_first_byte_pending = false;
                }
                g_mon_state = DCC_MON_WAIT_DELIM;
                g_mon_bit_count = 0u;
                g_mon_byte_accum = 0u;
            }
            break;

        case DCC_MON_WAIT_DELIM:
            if (!one)
            {
                g_mon_state = DCC_MON_READ_BYTE;
                g_mon_bit_count = 0u;
                g_mon_byte_accum = 0u;
            }
            else
            {
                g_mon_state = DCC_MON_WAIT_PREAMBLE;
                g_mon_preamble_ones = 1u;
            }
            break;

        default:
            dcc_monitor_reset();
            break;
    }
}

/* ======================================================================= */
/* FIFO helpers (SPSC, lock-free)                                          */
/* ======================================================================= */

/* Number of free slots available for the thread to write. */
static uint32_t dcc_fifo_free(void)
{
    uint32_t used = (g_hp_tail - g_hp_head) & DCC_FIFO_MASK;
    return DCC_FIFO_MASK - used;    /* max usable capacity = FIFO_SIZE - 1 */
}

/* Push one half-period value.  Caller must have verified sufficient space. */
static void dcc_push_unchecked(uint16_t hp)
{
    g_hp_fifo[g_hp_tail] = hp;
    __DMB();                            /* store to fifo before updating tail */
    g_hp_tail = (g_hp_tail + 1u) & DCC_FIFO_MASK;
    g_hp_enqueued_total += 1u;
}

/* Push both half-periods of one DCC bit.  Caller ensures >= 2 free slots. */
static void dcc_push_bit_unchecked(const sender_engine_t *eng, bool one)
{
    if (one)
    {
        uint16_t hp = (uint16_t)(eng->clk1t_us / 2u);
        dcc_push_unchecked(hp);
        dcc_push_unchecked(hp);
    }
    else
    {
        dcc_push_unchecked(eng->clk0h_us);
        dcc_push_unchecked((uint16_t)(eng->clk0t_us - eng->clk0h_us));
    }
}

/* Push 8 data bits of byte_val MSB-first.  Caller ensures >= 16 free slots. */
static void dcc_push_byte_unchecked(const sender_engine_t *eng, uint8_t byte_val)
{
    uint8_t mask;
    for (mask = 0x80u; mask != 0u; mask = (uint8_t)(mask >> 1u))
    {
        dcc_push_bit_unchecked(eng, (byte_val & mask) != 0u);
    }
}

/* Push `count` identical bits.  Returns false (no partial update) if no space. */
static bool dcc_push_bits(const sender_engine_t *eng, uint32_t count, bool one)
{
    uint32_t i;
    if (dcc_fifo_free() < count * 2u)
    {
        return false;
    }
    for (i = 0u; i < count; i++)
    {
        dcc_push_bit_unchecked(eng, one);
    }
    return true;
}

/* ======================================================================= */
/* Hardware start / stop (TIM5 init owned by CubeMX)                       */
/* ======================================================================= */

static void dcc_hw_start(void)
{
    /* Ensure BR_ENABLE is high before starting DCC output */
    HAL_GPIO_WritePin(BR_ENABLE_GPIO_Port, BR_ENABLE_Pin, GPIO_PIN_SET);

    /* Ensure PA0 is back on TIM5_CH1 even after a prior STOP set it as GPIO. */
    HAL_TIM_MspPostInit(&htim5);

    __HAL_TIM_SET_COUNTER(&htim5, 0u);
    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, DCC_IDLE_HP_US + 10u);
    __HAL_TIM_CLEAR_FLAG(&htim5, TIM_FLAG_CC1);

    __HAL_TIM_ENABLE_IT(&htim5, TIM_IT_CC1);
    HAL_NVIC_SetPriority(DCC_TIM_IRQn, DCC_TIM_IRQ_PRIO, 0u);
    HAL_NVIC_EnableIRQ(DCC_TIM_IRQn);
    HAL_TIM_OC_Start(&htim5, TIM_CHANNEL_1);
}

static void dcc_hw_stop(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_TIM_DISABLE_IT(&htim5, TIM_IT_CC1);
    HAL_NVIC_DisableIRQ(DCC_TIM_IRQn);
    HAL_TIM_OC_Stop(&htim5, TIM_CHANNEL_1);

    /* Reconfigure PA0 as GPIO and drive it low */
    gpio.Pin   = DCC_GPIO_PIN;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DCC_GPIO_PORT, &gpio);
    HAL_GPIO_WritePin(DCC_GPIO_PORT, DCC_GPIO_PIN, GPIO_PIN_RESET);
    /* Ensure BR_ENABLE is low after stopping DCC output */
    HAL_GPIO_WritePin(BR_ENABLE_GPIO_Port, BR_ENABLE_Pin, GPIO_PIN_RESET);
}

/* ======================================================================= */
/* TIM5 CC1 interrupt handler                                              */
/* Called from TIM5_IRQHandler in stm32h5xx_it.c.                         */
/* ======================================================================= */

void sender_engine_tim_irq_handler(void)
{
    uint32_t head;
    uint32_t tail;
    uint16_t hp;
    uint32_t trigger_hp_index;
    uint8_t trigger_address;
    uint8_t trigger_action;

    if (__HAL_TIM_GET_FLAG(&htim5, TIM_FLAG_CC1) == 0)
    {
        return;
    }
    __HAL_TIM_CLEAR_FLAG(&htim5, TIM_FLAG_CC1);

    head = g_hp_head;
    tail = g_hp_tail;

    if (head != tail)
    {
        hp = g_hp_fifo[head];
        __DMB();                        /* read fifo slot before advancing head */
        g_hp_head = (head + 1u) & DCC_FIFO_MASK;
        g_hp_dequeued_total += 1u;

        while (g_adr_evt_head != g_adr_evt_tail)
        {
            trigger_hp_index = g_adr_evt_fifo[g_adr_evt_head];
            if ((int32_t)((g_hp_dequeued_total - 1u) - trigger_hp_index) < 0)
            {
                break;
            }

            trigger_address = g_adr_evt_addr_fifo[g_adr_evt_head];
            trigger_action = g_adr_evt_action_fifo[g_adr_evt_head];
            g_adr_evt_head = (g_adr_evt_head + 1u) & DCC_ADR_EVT_FIFO_MASK;
            if (trigger_action == DCC_ADR_EVT_SET)
            {
                HAL_GPIO_WritePin(SCOPE_ADR_GPIO_Port, SCOPE_ADR_Pin, GPIO_PIN_SET);
                dcc_monitor_note_address(trigger_address);
            }
            else
            {
                HAL_GPIO_WritePin(SCOPE_ADR_GPIO_Port, SCOPE_ADR_Pin, GPIO_PIN_RESET);
            }
        }
    }
    else
    {
        /* FIFO empty: insert a "ones" half-period to keep output alive */
        hp = DCC_IDLE_HP_US;
    }

    /* Advance the compare value; the timer toggles the output at the match. */
    TIM5->CCR1 = TIM5->CCR1 + (uint32_t)hp;
}

/* ======================================================================= */
/* Public engine API                                                       */
/* ======================================================================= */

void sender_engine_init(sender_engine_t *eng)
{
    if (eng == 0)
    {
        return;
    }

    memset(eng, 0, sizeof(*eng));
    eng->clk0t_us = 200u;
    eng->clk0h_us = 100u;
    eng->clk1t_us = 116u;
    eng->scope_on = false;

    g_hp_head = 0u;
    g_hp_tail = 0u;
    g_hp_enqueued_total = 0u;
    g_hp_dequeued_total = 0u;
    g_adr_evt_head = 0u;
    g_adr_evt_tail = 0u;
    g_non_idle_packet_events = 0u;
    g_last_non_idle_address = 0xFFu;
    dcc_monitor_reset();

    /* Ensure the external scope trigger GPIO is in the OFF state. */
    HAL_GPIO_WritePin(SCOPE_GPIO_Port, SCOPE_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SCOPE_ADR_GPIO_Port, SCOPE_ADR_Pin, GPIO_PIN_RESET);
}

void sender_engine_reset(sender_engine_t *eng)
{
    bool was_running;

    if (eng == 0)
    {
        return;
    }

    was_running = eng->running;
    if (was_running)
    {
        dcc_hw_stop();
    }

    sender_engine_init(eng);

    if (was_running)
    {
        dcc_hw_start();
        eng->running = true;
    }
}

uint8_t sender_engine_set_timing(sender_engine_t *eng, uint16_t clk0t_us,
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

uint8_t sender_engine_start(sender_engine_t *eng)
{
    if (eng == 0)
    {
        return SHP_STATUS_INTERNAL_ERROR;
    }
    if (eng->running)
    {
        return SHP_STATUS_OK;
    }

    g_hp_head     = 0u;
    g_hp_tail     = 0u;
    g_hp_enqueued_total = 0u;
    g_hp_dequeued_total = 0u;
    g_adr_evt_head = 0u;
    g_adr_evt_tail = 0u;
    eng->underflow = false;
    dcc_hw_start();
    eng->running  = true;
    return SHP_STATUS_OK;
}

uint8_t sender_engine_stop(sender_engine_t *eng)
{
    if (eng == 0)
    {
        return SHP_STATUS_INTERNAL_ERROR;
    }
    if (!eng->running)
    {
        return SHP_STATUS_OK;
    }

    dcc_hw_stop();
    eng->running = false;
    return SHP_STATUS_OK;
}

uint8_t sender_engine_clear_underflow(sender_engine_t *eng)
{
    if (eng == 0)
    {
        return SHP_STATUS_INTERNAL_ERROR;
    }

    eng->underflow = false;
    return SHP_STATUS_OK;
}

uint8_t sender_engine_set_scope(sender_engine_t *eng, bool scope_on)
{
    if (eng == 0)
    {
        return SHP_STATUS_INTERNAL_ERROR;
    }

    eng->scope_on = scope_on;
    HAL_GPIO_WritePin(SCOPE_GPIO_Port, SCOPE_Pin,
                      scope_on ? GPIO_PIN_SET : GPIO_PIN_RESET);
    return SHP_STATUS_OK;
}

uint8_t sender_engine_send_bytes(sender_engine_t *eng, uint16_t repeat_count,
                                 uint8_t byte_value)
{
    uint16_t i;

    if (eng == 0)
    {
        return SHP_STATUS_INTERNAL_ERROR;
    }
    if (!eng->running)
    {
        return SHP_STATUS_BUSY;
    }
    if (repeat_count == 0u)
    {
        return SHP_STATUS_OK;
    }
    /* 8 bits per byte -> 16 half-periods per byte */
    if (dcc_fifo_free() < (uint32_t)repeat_count * 16u)
    {
        return SHP_STATUS_BUSY;
    }

    for (i = 0u; i < repeat_count; i++)
    {
        dcc_push_byte_unchecked(eng, byte_value);
    }

    eng->bytes_sent   += repeat_count;
    eng->packets_sent += 1u;
    return SHP_STATUS_OK;
}

uint8_t sender_engine_send_packet(sender_engine_t *eng, const uint8_t *data,
                                  uint16_t size)
{
    /*
     * DCC packet framing:
     *   preamble (>=14 ones) | start(0) | byte1 | sep(0) | byte2 | ... | end(1)
     * Half-periods needed = (14 + 1 + size*9) * 2
     */
    uint32_t hps_needed;
    uint32_t packet_hp_start;
    uint16_t i;

    if (eng == 0)
    {
        return SHP_STATUS_INTERNAL_ERROR;
    }
    if (!eng->running)
    {
        return SHP_STATUS_BUSY;
    }
    if (data == 0 || size == 0u)
    {
        return SHP_STATUS_OK;
    }

    hps_needed = (DCC_PREAMBLE_BITS + 1u + (uint32_t)size * 9u) * 2u;
    if (dcc_fifo_free() < hps_needed)
    {
        return SHP_STATUS_BUSY;
    }

    packet_hp_start = g_hp_enqueued_total;
    dcc_schedule_address_event(data[0], packet_hp_start + DCC_HPS_TO_FIRST_ADDR_BIT);

    /* Preamble */
    for (i = 0u; i < DCC_PREAMBLE_BITS; i++)
    {
        dcc_push_bit_unchecked(eng, true);
    }

    /* start(0) + byte + sep(0) + byte + ... */
    for (i = 0u; i < size; i++)
    {
        dcc_push_bit_unchecked(eng, false);     /* start / inter-byte separator */
        dcc_push_byte_unchecked(eng, data[i]);
    }

    /* End bit */
    dcc_push_bit_unchecked(eng, true);

    eng->bytes_sent   += size;
    eng->packets_sent += 1u;
    return SHP_STATUS_OK;
}

uint8_t sender_engine_send_raw_bytes(sender_engine_t *eng, const uint8_t *data,
                                     uint16_t size)
{
    uint32_t hps_needed;
    uint32_t bit_hp_index;
    uint16_t i;
    uint8_t mask;
    bool one;

    if (eng == 0)
    {
        return SHP_STATUS_INTERNAL_ERROR;
    }
    if (!eng->running)
    {
        return SHP_STATUS_BUSY;
    }
    if (data == 0 || size == 0u)
    {
        return SHP_STATUS_OK;
    }

    /* Raw path pushes bytes exactly as provided: 16 half-periods per byte. */
    hps_needed = (uint32_t)size * 16u;
    if (dcc_fifo_free() < hps_needed)
    {
        return SHP_STATUS_BUSY;
    }

    for (i = 0u; i < size; ++i)
    {
        for (mask = 0x80u; mask != 0u; mask = (uint8_t)(mask >> 1u))
        {
            one = ((data[i] & mask) != 0u);
            bit_hp_index = g_hp_enqueued_total;
            dcc_monitor_consume_bit(one, bit_hp_index);
            dcc_push_bit_unchecked(eng, one);
        }
    }

    eng->bytes_sent   += size;
    eng->packets_sent += 1u;
    return SHP_STATUS_OK;
}

uint8_t sender_engine_send_stretched_byte(sender_engine_t *eng,
                                          uint16_t clk0t_us,
                                          uint16_t clk0h_us,
                                          uint8_t byte_value)
{
    uint16_t save_t;
    uint16_t save_h;

    if (eng == 0)
    {
        return SHP_STATUS_INTERNAL_ERROR;
    }
    if (!eng->running)
    {
        return SHP_STATUS_BUSY;
    }
    if (dcc_fifo_free() < 16u)
    {
        return SHP_STATUS_BUSY;
    }

    /* Temporarily substitute the caller's zero-bit timing for this byte. */
    save_t = eng->clk0t_us;
    save_h = eng->clk0h_us;
    eng->clk0t_us = clk0t_us;
    eng->clk0h_us = clk0h_us;

    dcc_push_byte_unchecked(eng, byte_value);

    eng->clk0t_us = save_t;
    eng->clk0h_us = save_h;

    eng->bytes_sent += 1u;
    return SHP_STATUS_OK;
}

uint8_t sender_engine_send_special(sender_engine_t *eng, uint8_t special_id)
{
    static const uint8_t idle_pkt[3]   = { 0xFFu, 0x00u, 0xFFu };
    static const uint8_t warble_pkt[3] = { 0x55u, 0xAAu, 0xFFu };
    uint32_t hps_needed;

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
            hps_needed = (DCC_PREAMBLE_BITS + DCC_RESET_BITS) * 2u;
            if (dcc_fifo_free() < hps_needed)
            {
                return SHP_STATUS_BUSY;
            }
            dcc_push_bits(eng, DCC_PREAMBLE_BITS, true);
            dcc_push_bits(eng, DCC_RESET_BITS, false);
            eng->packets_sent += 1u;
            eng->bytes_sent   += 4u;
            return SHP_STATUS_OK;

        case SHP_SPECIAL_HARD_RESET:
            hps_needed = (DCC_PREAMBLE_BITS + DCC_HARD_RESET_BITS) * 2u;
            if (dcc_fifo_free() < hps_needed)
            {
                return SHP_STATUS_BUSY;
            }
            dcc_push_bits(eng, DCC_PREAMBLE_BITS, true);
            dcc_push_bits(eng, DCC_HARD_RESET_BITS, false);
            eng->packets_sent += 1u;
            eng->bytes_sent   += 8u;
            return SHP_STATUS_OK;

        case SHP_SPECIAL_IDLE:
            /* DCC idle packet: address=0xFF, data=0x00, error=0xFF */
            return sender_engine_send_packet(eng, idle_pkt, 3u);

        case SHP_SPECIAL_WARBLE:
            /* Service-mode entry: alternating pattern */
            return sender_engine_send_packet(eng, warble_pkt, 3u);

        default:
            return SHP_STATUS_BAD_LENGTH;
    }
}

uint8_t sender_engine_set_gen_out(sender_engine_t *eng, uint8_t value)
{
    if (eng == 0)
    {
        return SHP_STATUS_INTERNAL_ERROR;
    }

    eng->gen_out = value;
    return SHP_STATUS_OK;
}

void sender_engine_get_gen_in(sender_engine_t *eng, uint8_t *gen1,
                              uint8_t *gen2)
{
    bool in0_status;
    bool in1_status;

    if (eng == 0 || gen1 == 0 || gen2 == 0)
    {
        return;
    }
 
    in0_status = (HAL_GPIO_ReadPin(IN0_GPIO_Port, IN0_Pin) == GPIO_PIN_SET);
    in1_status = (HAL_GPIO_ReadPin(IN1_GPIO_Port, IN1_Pin) == GPIO_PIN_SET);
    eng->gen_in1 = (uint8_t)((eng->gen_in1 & (uint8_t)~0x01u) |
                             (in0_status ? 0x01u : 0x00u));
    eng->gen_in2 = (uint8_t)((eng->gen_in2 & (uint8_t)~0x01u) |
                             (in1_status ? 0x01u : 0x00u));

    *gen1 = eng->gen_in1;
    *gen2 = eng->gen_in2;
}

void sender_engine_reset_stats(sender_engine_t *eng)
{
    if (eng == 0)
    {
        return;
    }

    eng->packets_sent = 0u;
    eng->bytes_sent = 0u;
    eng->underruns = 0u;
}

void sender_engine_get_status(const sender_engine_t *eng,
                              sender_status_payload_t *out)
{
    if (eng == 0 || out == 0)
    {
        return;
    }

    out->running    = eng->running;
    out->underflow  = eng->underflow;
    out->err_flags  = eng->err_flags;
    out->queue_depth = (uint16_t)((g_hp_tail - g_hp_head) & DCC_FIFO_MASK);
    out->gen1 = eng->gen_in1;
    out->gen2 = eng->gen_in2;
    out->non_idle_event_count = g_non_idle_packet_events;
    out->last_non_idle_address = g_last_non_idle_address;
}

void sender_engine_get_stats(const sender_engine_t *eng,
                             sender_stats_payload_t *out)
{
    if (eng == 0 || out == 0)
    {
        return;
    }

    out->packets_sent = eng->packets_sent;
    out->bytes_sent   = eng->bytes_sent;
    out->underruns    = eng->underruns;
}

void sender_engine_get_non_idle_packet_event(uint32_t *event_count,
                                             uint8_t *last_address)
{
    if (event_count == 0 || last_address == 0)
    {
        return;
    }

    *event_count = g_non_idle_packet_events;
    *last_address = g_last_non_idle_address;
}