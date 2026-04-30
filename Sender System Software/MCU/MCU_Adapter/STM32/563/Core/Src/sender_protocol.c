#include "sender_protocol.h"

uint16_t sender_proto_crc16_ccitt(const uint8_t* data, uint16_t size)
{
    uint16_t crc = 0xFFFFu;
    uint16_t i;

    if (data == 0)
    {
        return crc;
    }

    for (i = 0; i < size; ++i)
    {
        uint8_t b = data[i];
        uint8_t bit;

        crc ^= (uint16_t)b << 8;
        for (bit = 0; bit < 8; ++bit)
        {
            if ((crc & 0x8000u) != 0u)
            {
                crc = (uint16_t)((crc << 1) ^ 0x1021u);
            }
            else
            {
                crc = (uint16_t)(crc << 1);
            }
        }
    }

    return crc;
}