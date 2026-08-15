#ifndef SENDER_HW_IF_H_DECLARED
#define SENDER_HW_IF_H_DECLARED

#include <stddef.h>
#include <stdint.h>

struct Sender_hw_status
{
    bool running;
    bool underflow;
    uint32_t err_flags;
    uint16_t queue_depth;
    uint8_t gen1;
    uint8_t gen2;
  uint32_t non_idle_event_count;
  uint8_t last_non_idle_address;

    Sender_hw_status()
        : running(false), underflow(false), err_flags(0), queue_depth(0),
      gen1(0), gen2(0), non_idle_event_count(0),
      last_non_idle_address(0xFF)
    {
    }
};

struct Sender_hw_stats
{
    uint32_t packets_sent;
    uint32_t bytes_sent;
    uint32_t underruns;

    Sender_hw_stats() : packets_sent(0), bytes_sent(0), underruns(0)
    {
    }
};

class Sender_hw_if
{
  public:
    virtual ~Sender_hw_if() {}

    virtual bool init() = 0;
    virtual bool close() = 0;

    virtual bool set_timing(uint16_t clk0t_us, uint16_t clk0h_us,
                            uint16_t clk1t_us) = 0;
    virtual bool start_clock() = 0;
    virtual bool stop_clock() = 0;
    virtual bool clear_underflow() = 0;
    virtual bool set_scope(bool on) = 0;

    virtual bool send_bytes(uint16_t repeat_count, uint8_t byte_value,
                            const char* info_tag) = 0;
    virtual bool send_packet(const uint8_t* data, size_t size,
                             const char* info_tag) = 0;
    virtual bool send_raw_bytes(const uint8_t* data, size_t size,
                  const char* info_tag) = 0;
    virtual bool send_stretched_byte(uint16_t clk0t_us, uint16_t clk0h_us,
                                     uint8_t byte_value,
                                     const char* info_tag) = 0;
    virtual bool send_special(uint8_t special_id) = 0;

    virtual bool set_gen_out(uint8_t value) = 0;
    virtual bool get_gen_in(uint8_t& gen1, uint8_t& gen2) = 0;

    virtual bool get_status(Sender_hw_status& status) = 0;
    virtual bool get_stats(Sender_hw_stats& stats) = 0;
    virtual bool reset_stats() = 0;
    virtual bool reset_device() = 0;
};

#endif /* SENDER_HW_IF_H_DECLARED */
