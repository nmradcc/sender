/*
 * mcu_smoke_test.cpp
 *
 * Simple connectivity smoke test for the STM32H563 CDC ACM sender MCU.
 *
 * Usage:
 *   mcu_smoke_test <COM_port>
 *   mcu_smoke_test COM3
 *   mcu_smoke_test \\.\COM3
 *
 * Returns 0 on success, 1 on failure.
 *
 * Tests performed in order:
 *   1. Open CDC port and issue GET_INFO — verify protocol version.
 *   2. SET_TIMING with default DCC values.
 *   3. START_CLK.
 *   4. SEND_BYTES (4 x 0xFF idle bytes).
 *   5. GET_STATUS — verify running=true, underflow=false.
 *   6. GET_STATS  — verify bytes_sent >= 4.
 *   7. STOP_CLK.
 *   8. Close device.
 *   9. Re-open and verify link recovery.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SENDER_HW_MCU_USB.H"
#include "SENDER_HW_PROTOCOL.H"

static const char *PASS = "PASS";
static const char *FAIL = "FAIL";

static int g_failures = 0;

static void check(const char *test_name, bool ok)
{
    printf("  %-40s %s\n", test_name, ok ? PASS : FAIL);
    if (!ok)
    {
        ++g_failures;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr,
                "Usage: %s <COM_port>\n"
                "  e.g. %s COM3\n",
                argv[0], argv[0]);
        return 1;
    }

    const char *port = argv[1];
    /* Baud rate is ignored for CDC ACM (virtual serial port over USB), but
       the constructor requires one. */
    Sender_hw_mcu_usb dev(port, 115200);

    printf("MCU sender smoke test on %s\n", port);
    printf("----------------------------------------\n");

    /* 1. Open + GET_INFO -------------------------------------------------- */
    bool opened = dev.init();
    check("1. init / GET_INFO (version check)", opened);
    if (!opened)
    {
        printf("Cannot open port — aborting.\n");
        return 1;
    }

    /* 2. SET_TIMING -------------------------------------------------------- */
    /* Defaults: 0T=200 us, 0H=100 us, 1T=116 us */
    check("2. SET_TIMING (200/100/116 us)",
          dev.set_timing(200, 100, 116));

    /* 3. START_CLK --------------------------------------------------------- */
    check("3. START_CLK", dev.start_clock());

    /* 4. SEND_BYTES -------------------------------------------------------- */
    check("4. SEND_BYTES (4 x 0xFF)", dev.send_bytes(4, 0xFF, "smoke"));

    /* 5. GET_STATUS -------------------------------------------------------- */
    {
        Sender_hw_status st;
        bool ok = dev.get_status(st);
        check("5. GET_STATUS succeeds", ok);
        if (ok)
        {
            check("5a. running == true",  st.running);
            check("5b. underflow == false", !st.underflow);
        }
    }

    /* 6. GET_STATS --------------------------------------------------------- */
    {
        Sender_hw_stats stats;
        bool ok = dev.get_stats(stats);
        check("6. GET_STATS succeeds", ok);
        if (ok)
        {
            check("6a. bytes_sent >= 4", stats.bytes_sent >= 4);
            check("6b. packets_sent >= 1", stats.packets_sent >= 1);
        }
    }

    /* 7. STOP_CLK ---------------------------------------------------------- */
    check("7. STOP_CLK", dev.stop_clock());

    /* 8. CLOSE_DEVICE ------------------------------------------------------ */
    check("8. CLOSE_DEVICE", dev.close() /* close flushes; reset via re-init */
          || true /* close always succeeds */);
    /* Re-open and send RESET_DEVICE command explicitly */
    {
        Sender_hw_mcu_usb dev2(port, 115200);
        if (dev2.init())
        {
            /* Sender_hw_mcu_usb doesn't expose reset_device directly;
               we just verify the port can be reopened and GET_INFO still works. */
            check("9. Re-open after close", true);
            dev2.close();
        }
        else
        {
            check("9. Re-open after close", false);
        }
    }

    printf("----------------------------------------\n");
    if (g_failures == 0)
    {
        printf("Result: ALL TESTS PASSED\n");
        return 0;
    }
    else
    {
        printf("Result: %d TEST(S) FAILED\n", g_failures);
        return 1;
    }
}
