#ifndef SENDER_HW_MCU_USB_H_DECLARED
#define SENDER_HW_MCU_USB_H_DECLARED

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <vector>

#include "SENDER_HW_IF.H"
#include "SENDER_HW_PROTOCOL.H"

class Sender_hw_mcu_usb : public Sender_hw_if
{
  public:
    Sender_hw_mcu_usb(const char* port_name, uint32_t baud_rate);
    virtual ~Sender_hw_mcu_usb();

    virtual bool init();
    virtual bool close();

    virtual bool set_timing(uint16_t clk0t_us, uint16_t clk0h_us,
                            uint16_t clk1t_us);
    virtual bool start_clock();
    virtual bool stop_clock();
    virtual bool clear_underflow();
    virtual bool set_scope(bool on);

    virtual bool send_bytes(uint16_t repeat_count, uint8_t byte_value,
                            const char* info_tag);
    virtual bool send_packet(const uint8_t* data, size_t size,
                             const char* info_tag);
    virtual bool send_raw_bytes(const uint8_t* data, size_t size,
                  const char* info_tag);
    virtual bool send_stretched_byte(uint16_t clk0t_us, uint16_t clk0h_us,
                                     uint8_t byte_value,
                                     const char* info_tag);
    virtual bool send_special(uint8_t special_id);

    virtual bool set_gen_out(uint8_t value);
    virtual bool get_gen_in(uint8_t& gen1, uint8_t& gen2);

    virtual bool get_status(Sender_hw_status& status);
    virtual bool get_stats(Sender_hw_stats& stats);
    virtual bool reset_stats();

    /* Helpers used by host and test code for frame handling. */
    static uint16_t crc16_ccitt(const uint8_t* data, size_t size);
    static std::vector<uint8_t> build_frame(uint8_t sof, uint8_t cmd,
                                            uint8_t seq,
                                            const uint8_t* payload,
                                            uint16_t payload_len);

  private:
    bool write_all(const uint8_t* data, size_t size);
    bool read_exact(uint8_t* data, size_t size, uint32_t timeout_ms);
    bool send_command_wait_response(uint8_t cmd, const uint8_t* payload,
                                    uint16_t payload_len,
                                    std::vector<uint8_t>& out_payload,
                                    uint8_t& out_status);
    bool send_chunked_commit(const uint8_t* data, size_t size,
                 uint8_t append_cmd, uint8_t commit_cmd);

  private:
    std::string m_port_name;
    uint32_t m_baud_rate;
    uint8_t m_seq;
    bool m_open;
    void* m_handle;
};

#endif /* SENDER_HW_MCU_USB_H_DECLARED */
