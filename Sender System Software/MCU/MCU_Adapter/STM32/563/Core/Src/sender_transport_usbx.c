#include "sender_transport.h"

#include <string.h>

#include "sender_protocol.h"
#include "ux_device_cdc_acm.h"

#define SENDER_USBX_RX_BUFFER_SIZE 2048u
#define SENDER_USBX_READ_CHUNK_SIZE 64u

static uint8_t g_rx_stream[SENDER_USBX_RX_BUFFER_SIZE];
static uint16_t g_rx_stream_length;

static void sender_transport_drop_leading_bytes(uint16_t count)
{
    if (count >= g_rx_stream_length)
    {
        g_rx_stream_length = 0;
        return;
    }

    memmove(g_rx_stream, &g_rx_stream[count], g_rx_stream_length - count);
    g_rx_stream_length = (uint16_t)(g_rx_stream_length - count);
}

static void sender_transport_resync(void)
{
    while (g_rx_stream_length > 0u && g_rx_stream[0] != SHP_SOF_HOST)
    {
        sender_transport_drop_leading_bytes(1u);
    }
}

static bool sender_transport_try_extract_frame(uint8_t *frame, uint16_t capacity,
                                               uint16_t *frame_size)
{
    uint16_t payload_length;
    uint16_t total_length;

    sender_transport_resync();
    if (g_rx_stream_length < 6u)
    {
        return false;
    }

    payload_length = (uint16_t)g_rx_stream[4] | ((uint16_t)g_rx_stream[5] << 8);
    total_length = (uint16_t)(payload_length + 8u);
    if (total_length > SENDER_USBX_RX_BUFFER_SIZE)
    {
        sender_transport_drop_leading_bytes(1u);
        return false;
    }
    if (g_rx_stream_length < total_length)
    {
        return false;
    }
    if (capacity < total_length)
    {
        g_rx_stream_length = 0;
        return false;
    }

    memcpy(frame, g_rx_stream, total_length);
    *frame_size = total_length;
    sender_transport_drop_leading_bytes(total_length);
    return true;
}

static void sender_transport_append_bytes(const uint8_t *data, uint16_t length)
{
    if (length == 0u)
    {
        return;
    }

    if ((uint32_t)g_rx_stream_length + (uint32_t)length >
        (uint32_t)SENDER_USBX_RX_BUFFER_SIZE)
    {
        g_rx_stream_length = 0;
        if (length > SENDER_USBX_RX_BUFFER_SIZE)
        {
            data = &data[length - SENDER_USBX_RX_BUFFER_SIZE];
            length = SENDER_USBX_RX_BUFFER_SIZE;
        }
    }

    memcpy(&g_rx_stream[g_rx_stream_length], data, length);
    g_rx_stream_length = (uint16_t)(g_rx_stream_length + length);
}

void sender_transport_init(void)
{
    g_rx_stream_length = 0;
}

bool sender_transport_read_frame(uint8_t *frame, uint16_t capacity,
                                 uint16_t *frame_size)
{
    UX_SLAVE_CLASS_CDC_ACM *cdc_acm;
    uint8_t read_buffer[SENDER_USBX_READ_CHUNK_SIZE];
    ULONG actual_length;
    UINT status;

    if (frame == 0 || frame_size == 0)
    {
        return false;
    }

    *frame_size = 0u;
    if (sender_transport_try_extract_frame(frame, capacity, frame_size))
    {
        return true;
    }

    cdc_acm = USBD_CDC_ACM_GetInstance();
    if (cdc_acm == UX_NULL)
    {
        return false;
    }

    for (;;)
    {
        actual_length = 0u;
        status = ux_device_class_cdc_acm_read(cdc_acm, read_buffer,
                                              SENDER_USBX_READ_CHUNK_SIZE,
                                              &actual_length);
        if (status != UX_SUCCESS)
        {
            g_rx_stream_length = 0;
            return false;
        }
        if (actual_length == 0u)
        {
            return false;
        }

        sender_transport_append_bytes(read_buffer, (uint16_t)actual_length);
        if (sender_transport_try_extract_frame(frame, capacity, frame_size))
        {
            return true;
        }
    }
}

void sender_transport_write_frame(const uint8_t *frame, uint16_t frame_size)
{
    UX_SLAVE_CLASS_CDC_ACM *cdc_acm;
    const uint8_t *cursor;
    ULONG actual_length;
    ULONG remaining;
    UINT status;

    if (frame == 0 || frame_size == 0u)
    {
        return;
    }

    cdc_acm = USBD_CDC_ACM_GetInstance();
    if (cdc_acm == UX_NULL)
    {
        return;
    }

    cursor = frame;
    remaining = frame_size;
    while (remaining > 0u)
    {
        actual_length = 0u;
        status = ux_device_class_cdc_acm_write(cdc_acm, (UCHAR *)cursor,
                                               remaining, &actual_length);
        if (status != UX_SUCCESS || actual_length == 0u)
        {
            return;
        }

        cursor += actual_length;
        remaining -= actual_length;
    }
}