#include "SENDER_HW_MCU_USB.H"

#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#endif

Sender_hw_mcu_usb::Sender_hw_mcu_usb(const char* port_name, uint32_t baud_rate)
    : m_port_name(port_name ? port_name : ""),
      m_baud_rate(baud_rate),
      m_seq(0),
    m_open(false),
    m_handle(NULL)
{
}

Sender_hw_mcu_usb::~Sender_hw_mcu_usb()
{
    close();
}

bool Sender_hw_mcu_usb::init()
{
    if (m_open)
    {
        return true;
    }

#if defined(_WIN32)
    std::string full_port;
    DCB dcb;
    COMMTIMEOUTS cto;
    std::vector<uint8_t> rsp;
    uint8_t status = SHP_STATUS_INTERNAL_ERROR;

    if (m_port_name.empty())
    {
        return false;
    }

    if (m_port_name.find("\\\\.\\") == 0)
    {
        full_port = m_port_name;
    }
    else
    {
        full_port = "\\\\.\\" + m_port_name;
    }

    m_handle = (void*)CreateFileA(full_port.c_str(),
                                  GENERIC_READ | GENERIC_WRITE,
                                  0,
                                  NULL,
                                  OPEN_EXISTING,
                                  0,
                                  NULL);
    if (m_handle == (void*)INVALID_HANDLE_VALUE)
    {
        m_handle = NULL;
        return false;
    }

    memset(&dcb, 0, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState((HANDLE)m_handle, &dcb))
    {
        close();
        return false;
    }

    dcb.BaudRate = m_baud_rate;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fBinary = TRUE;
    dcb.fParity = FALSE;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;

    if (!SetCommState((HANDLE)m_handle, &dcb))
    {
        close();
        return false;
    }

    memset(&cto, 0, sizeof(cto));
    cto.ReadIntervalTimeout = 10;
    cto.ReadTotalTimeoutMultiplier = 0;
    cto.ReadTotalTimeoutConstant = 20;
    cto.WriteTotalTimeoutMultiplier = 0;
    cto.WriteTotalTimeoutConstant = 100;
    if (!SetCommTimeouts((HANDLE)m_handle, &cto))
    {
        close();
        return false;
    }

    PurgeComm((HANDLE)m_handle,
              PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);

    m_open = true;

    if (!send_command_wait_response(SHP_CMD_GET_INFO, NULL, 0, rsp, status))
    {
        close();
        return false;
    }

    return true;
#else
    return false;
#endif
}

bool Sender_hw_mcu_usb::close()
{
#if defined(_WIN32)
    if (m_handle != NULL)
    {
        CloseHandle((HANDLE)m_handle);
        m_handle = NULL;
    }
#endif
    m_open = false;
    return true;
}

bool Sender_hw_mcu_usb::write_all(const uint8_t* data, size_t size)
{
#if defined(_WIN32)
    size_t off = 0;

    if (!m_open || m_handle == NULL || data == NULL)
    {
        return false;
    }

    while (off < size)
    {
        DWORD written = 0;
        DWORD chunk = (DWORD)(size - off);
        if (!WriteFile((HANDLE)m_handle, data + off, chunk, &written, NULL))
        {
            return false;
        }
        if (written == 0)
        {
            return false;
        }
        off += written;
    }

    return true;
#else
    (void)data;
    (void)size;
    return false;
#endif
}

bool Sender_hw_mcu_usb::read_exact(uint8_t* data, size_t size, uint32_t timeout_ms)
{
#if defined(_WIN32)
    DWORD start_ms;
    size_t off = 0;

    if (!m_open || m_handle == NULL || data == NULL)
    {
        return false;
    }

    start_ms = GetTickCount();
    while (off < size)
    {
        DWORD got = 0;
        if (!ReadFile((HANDLE)m_handle, data + off, (DWORD)(size - off), &got, NULL))
        {
            return false;
        }

        if (got > 0)
        {
            off += got;
            continue;
        }

        if ((GetTickCount() - start_ms) >= timeout_ms)
        {
            return false;
        }
        Sleep(1);
    }

    return true;
#else
    (void)data;
    (void)size;
    (void)timeout_ms;
    return false;
#endif
}

bool Sender_hw_mcu_usb::set_timing(uint16_t clk0t_us, uint16_t clk0h_us,
                                   uint16_t clk1t_us)
{
    uint8_t payload[6];
    payload[0] = (uint8_t)(clk0t_us & 0xFF);
    payload[1] = (uint8_t)((clk0t_us >> 8) & 0xFF);
    payload[2] = (uint8_t)(clk0h_us & 0xFF);
    payload[3] = (uint8_t)((clk0h_us >> 8) & 0xFF);
    payload[4] = (uint8_t)(clk1t_us & 0xFF);
    payload[5] = (uint8_t)((clk1t_us >> 8) & 0xFF);

    std::vector<uint8_t> out;
    uint8_t status = SHP_STATUS_INTERNAL_ERROR;
    return send_command_wait_response(SHP_CMD_SET_TIMING, payload,
                                      (uint16_t)sizeof(payload), out, status);
}

bool Sender_hw_mcu_usb::start_clock()
{
    std::vector<uint8_t> out;
    uint8_t status = SHP_STATUS_INTERNAL_ERROR;
    return send_command_wait_response(SHP_CMD_START_CLK, NULL, 0, out, status);
}

bool Sender_hw_mcu_usb::stop_clock()
{
    std::vector<uint8_t> out;
    uint8_t status = SHP_STATUS_INTERNAL_ERROR;
    return send_command_wait_response(SHP_CMD_STOP_CLK, NULL, 0, out, status);
}

bool Sender_hw_mcu_usb::clear_underflow()
{
    std::vector<uint8_t> out;
    uint8_t status = SHP_STATUS_INTERNAL_ERROR;
    return send_command_wait_response(SHP_CMD_CLR_UNDERFLOW, NULL, 0, out,
                                      status);
}

bool Sender_hw_mcu_usb::set_scope(bool on)
{
    uint8_t payload = on ? 1 : 0;
    std::vector<uint8_t> out;
    uint8_t status = SHP_STATUS_INTERNAL_ERROR;
    return send_command_wait_response(SHP_CMD_SET_SCOPE, &payload, 1, out,
                                      status);
}

bool Sender_hw_mcu_usb::send_bytes(uint16_t repeat_count, uint8_t byte_value,
                                   const char* info_tag)
{
    (void)info_tag;

    uint8_t payload[3];
    payload[0] = (uint8_t)(repeat_count & 0xFF);
    payload[1] = (uint8_t)((repeat_count >> 8) & 0xFF);
    payload[2] = byte_value;

    std::vector<uint8_t> out;
    uint8_t status = SHP_STATUS_INTERNAL_ERROR;
    return send_command_wait_response(SHP_CMD_SEND_BYTES, payload,
                                      (uint16_t)sizeof(payload), out, status);
}

bool Sender_hw_mcu_usb::send_packet(const uint8_t* data, size_t size,
                                    const char* info_tag)
{
    (void)info_tag;

    return send_chunked_commit(data, size, SHP_CMD_APPEND_PACKET_CHUNK,
                               SHP_CMD_COMMIT_PACKET);
}

bool Sender_hw_mcu_usb::send_raw_bytes(const uint8_t* data, size_t size,
                                       const char* info_tag)
{
    (void)info_tag;

    return send_chunked_commit(data, size, SHP_CMD_APPEND_RAW_CHUNK,
                               SHP_CMD_COMMIT_RAW_BYTES);
}

bool Sender_hw_mcu_usb::send_chunked_commit(const uint8_t* data, size_t size,
                                            uint8_t append_cmd,
                                            uint8_t commit_cmd)
{
    const uint32_t busy_wait_timeout_ms = 5000;

    if (data == NULL || size == 0 || size > 0xFFFF)
    {
        return false;
    }

    std::vector<uint8_t> out;
    uint8_t status = SHP_STATUS_INTERNAL_ERROR;
    size_t off = 0;

    while (off < size)
    {
        uint16_t chunk_len = (uint16_t)((size - off) > SHP_PACKET_CHUNK_MAX
                                            ? SHP_PACKET_CHUNK_MAX
                                            : (size - off));
        status = SHP_STATUS_INTERNAL_ERROR;
        if (!send_command_wait_response(append_cmd,
                                        data + off,
                                        chunk_len,
                                        out,
                                        status))
        {
            return false;
        }

        off += chunk_len;
    }

    status = SHP_STATUS_INTERNAL_ERROR;
    if (send_command_wait_response(commit_cmd,
                                   NULL,
                                   0,
                                   out,
                                   status))
    {
        return true;
    }

    if (status == SHP_STATUS_BUSY)
    {
#if defined(_WIN32)
        uint32_t start_ms = GetTickCount();
        while ((GetTickCount() - start_ms) < busy_wait_timeout_ms)
#else
        for (uint32_t i = 0; i < busy_wait_timeout_ms; ++i)
#endif
        {
            Sender_hw_status hw_status;
            if (get_status(hw_status) && hw_status.queue_depth == 0)
            {
                return true;
            }
#if defined(_WIN32)
            Sleep(1);
#endif
        }
    }

    return false;
}

bool Sender_hw_mcu_usb::send_stretched_byte(uint16_t clk0t_us,
                                            uint16_t clk0h_us,
                                            uint8_t byte_value,
                                            const char* info_tag)
{
    (void)info_tag;

    uint8_t payload[5];
    payload[0] = (uint8_t)(clk0t_us & 0xFF);
    payload[1] = (uint8_t)((clk0t_us >> 8) & 0xFF);
    payload[2] = (uint8_t)(clk0h_us & 0xFF);
    payload[3] = (uint8_t)((clk0h_us >> 8) & 0xFF);
    payload[4] = byte_value;

    std::vector<uint8_t> out;
    uint8_t status = SHP_STATUS_INTERNAL_ERROR;
    return send_command_wait_response(SHP_CMD_SEND_STRETCHED_BYTE, payload,
                                      (uint16_t)sizeof(payload), out, status);
}

bool Sender_hw_mcu_usb::send_special(uint8_t special_id)
{
    std::vector<uint8_t> out;
    uint8_t status = SHP_STATUS_INTERNAL_ERROR;
    return send_command_wait_response(SHP_CMD_SEND_SPECIAL, &special_id, 1, out,
                                      status);
}

bool Sender_hw_mcu_usb::set_gen_out(uint8_t value)
{
    std::vector<uint8_t> out;
    uint8_t status = SHP_STATUS_INTERNAL_ERROR;
    return send_command_wait_response(SHP_CMD_SET_GEN_OUT, &value, 1, out,
                                      status);
}

bool Sender_hw_mcu_usb::get_gen_in(uint8_t& gen1, uint8_t& gen2)
{
    std::vector<uint8_t> out;
    uint8_t status = SHP_STATUS_INTERNAL_ERROR;

    if (!send_command_wait_response(SHP_CMD_GET_GEN_IN, NULL, 0, out, status))
    {
        return false;
    }

    if (out.size() < 2)
    {
        return false;
    }

    gen1 = out[0];
    gen2 = out[1];

//    gen1 ^= 0x01; /* Invert gen1 bit 0 to match legacy hardware behavior. */
//    gen2 ^= 0x01; /* Invert gen2 bit 0 to match legacy hardware behavior. */
    
    return true;
}

bool Sender_hw_mcu_usb::get_status(Sender_hw_status& status)
{
    std::vector<uint8_t> out;
    uint8_t rsp_status = SHP_STATUS_INTERNAL_ERROR;

    if (!send_command_wait_response(SHP_CMD_GET_STATUS, NULL, 0, out,
                                    rsp_status))
    {
        return false;
    }

    if (out.size() < 10)
    {
        return false;
    }

    status.running = (out[0] != 0);
    status.underflow = (out[1] != 0);
    status.err_flags = ((uint32_t)out[2]) | ((uint32_t)out[3] << 8) |
                       ((uint32_t)out[4] << 16) | ((uint32_t)out[5] << 24);
    status.queue_depth = (uint16_t)out[6] | ((uint16_t)out[7] << 8);
    status.gen1 = out[8];
    status.gen2 = out[9];

    return true;
}

bool Sender_hw_mcu_usb::get_stats(Sender_hw_stats& stats)
{
    std::vector<uint8_t> out;
    uint8_t rsp_status = SHP_STATUS_INTERNAL_ERROR;

    if (!send_command_wait_response(SHP_CMD_GET_STATS, NULL, 0, out,
                                    rsp_status))
    {
        return false;
    }

    if (out.size() < 12)
    {
        return false;
    }

    stats.packets_sent = ((uint32_t)out[0]) | ((uint32_t)out[1] << 8) |
                         ((uint32_t)out[2] << 16) | ((uint32_t)out[3] << 24);
    stats.bytes_sent = ((uint32_t)out[4]) | ((uint32_t)out[5] << 8) |
                       ((uint32_t)out[6] << 16) | ((uint32_t)out[7] << 24);
    stats.underruns = ((uint32_t)out[8]) | ((uint32_t)out[9] << 8) |
                      ((uint32_t)out[10] << 16) | ((uint32_t)out[11] << 24);

    return true;
}

bool Sender_hw_mcu_usb::reset_stats()
{
    std::vector<uint8_t> out;
    uint8_t rsp_status = SHP_STATUS_INTERNAL_ERROR;
    return send_command_wait_response(SHP_CMD_RESET_STATS, NULL, 0, out,
                                      rsp_status);
}

bool Sender_hw_mcu_usb::get_info(std::vector<uint8_t>& info_payload)
{
    uint8_t rsp_status = SHP_STATUS_INTERNAL_ERROR;
    info_payload.clear();
    return send_command_wait_response(SHP_CMD_GET_INFO, NULL, 0, info_payload,
                                      rsp_status);
}

uint16_t Sender_hw_mcu_usb::crc16_ccitt(const uint8_t* data, size_t size)
{
    uint16_t crc = 0xFFFF;
    size_t i;

    if (data == NULL)
    {
        return crc;
    }

    for (i = 0; i < size; ++i)
    {
        uint8_t b = data[i];
        int bit;

        crc ^= (uint16_t)b << 8;
        for (bit = 0; bit < 8; ++bit)
        {
            if ((crc & 0x8000U) != 0U)
            {
                crc = (uint16_t)((crc << 1) ^ 0x1021U);
            }
            else
            {
                crc = (uint16_t)(crc << 1);
            }
        }
    }

    return crc;
}

std::vector<uint8_t> Sender_hw_mcu_usb::build_frame(uint8_t sof, uint8_t cmd,
                                                     uint8_t seq,
                                                     const uint8_t* payload,
                                                     uint16_t payload_len)
{
    std::vector<uint8_t> out;
    const size_t fixed_len = 1 + 1 + 1 + 1 + 2 + 2;
    size_t payload_size = (size_t)payload_len;

    out.reserve(fixed_len + payload_size);
    out.push_back(sof);
    out.push_back(SHP_VERSION);
    out.push_back(cmd);
    out.push_back(seq);
    out.push_back((uint8_t)(payload_len & 0xFF));
    out.push_back((uint8_t)((payload_len >> 8) & 0xFF));

    if (payload != NULL && payload_len > 0)
    {
        out.insert(out.end(), payload, payload + payload_len);
    }

    /* CRC excludes SOF by protocol definition. */
    {
        uint16_t crc = crc16_ccitt(&out[1], out.size() - 1);
        out.push_back((uint8_t)(crc & 0xFF));
        out.push_back((uint8_t)((crc >> 8) & 0xFF));
    }

    return out;
}

bool Sender_hw_mcu_usb::send_command_wait_response(uint8_t cmd,
                                                    const uint8_t* payload,
                                                    uint16_t payload_len,
                                                    std::vector<uint8_t>& out_payload,
                                                    uint8_t& out_status)
{
    std::vector<uint8_t> tx;
    uint8_t hdr[7];
    uint16_t rsp_len;
    std::vector<uint8_t> tail;
    uint16_t crc_rx;
    uint16_t crc_calc;
    std::vector<uint8_t> crc_buf;
    uint8_t seq;

    out_payload.clear();
    out_status = SHP_STATUS_INTERNAL_ERROR;

    if (!m_open)
    {
        return false;
    }

    seq = ++m_seq;
    tx = build_frame(SHP_SOF_HOST, cmd, seq, payload, payload_len);

    if (!write_all(&tx[0], tx.size()))
    {
        return false;
    }

    if (!read_exact(hdr, sizeof(hdr), 1000))
    {
        return false;
    }

    if (hdr[0] != SHP_SOF_MCU || hdr[1] != SHP_VERSION)
    {
        return false;
    }
    if (hdr[2] != cmd || hdr[3] != seq)
    {
        return false;
    }

    out_status = hdr[4];
    rsp_len = (uint16_t)hdr[5] | ((uint16_t)hdr[6] << 8);

    tail.resize((size_t)rsp_len + 2);
    if (!read_exact(&tail[0], tail.size(), 1000))
    {
        return false;
    }

    crc_rx = (uint16_t)tail[rsp_len] | ((uint16_t)tail[rsp_len + 1] << 8);

    crc_buf.reserve(6 + rsp_len);
    crc_buf.push_back(hdr[1]);
    crc_buf.push_back(hdr[2]);
    crc_buf.push_back(hdr[3]);
    crc_buf.push_back(hdr[4]);
    crc_buf.push_back(hdr[5]);
    crc_buf.push_back(hdr[6]);
    if (rsp_len > 0)
    {
        crc_buf.insert(crc_buf.end(), tail.begin(), tail.begin() + rsp_len);
    }

    crc_calc = crc16_ccitt(&crc_buf[0], crc_buf.size());
    if (crc_calc != crc_rx)
    {
        return false;
    }

    if (rsp_len > 0)
    {
        out_payload.assign(tail.begin(), tail.begin() + rsp_len);
    }

    return out_status == SHP_STATUS_OK;
}
