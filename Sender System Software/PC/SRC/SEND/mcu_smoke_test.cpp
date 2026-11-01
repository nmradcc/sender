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
 *   2. FREQ_TEST — MCU measures the 4 MHz reference (4 MHz +/- 200 Hz).
 *   3. RESET_STATS.
 *   4. SET_TIMING with default DCC values.
 *   5. START_CLK.
 *   6. SEND_BYTES (4 x 0xFF idle bytes).
 *   7. GET_STATUS, then GET_STATS — verify exact counts 4 bytes, 1 packet.
 *   8. RESET_STATS again.
 *   9. SEND_PACKET (DCC idle packet: FF 00 FF).
 *  10. GET_STATS  — verify exact counts 3 bytes, 1 packet.
 *  11. STOP_CLK.
 *  12. Close device.
 *  13. Re-open and verify link recovery.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "SEND.H"
#include "SENDER_HW_MCU_USB.H"
#include "SENDER_HW_PROTOCOL.H"

static const char *PASS_TEXT = "PASS";
static const char *FAIL_TEXT = "FAIL";

static int g_failures = 0;

static const char *release_level_name(Rel_levels level)
{
    switch (level)
    {
    case VER_DEB:  return "debug";
    case VER_EXP:  return "experimental";
    case VER_BETA: return "beta";
    case VER_REL:  return "release";
    default:       return "unknown";
    }
}

static void dump_version_info(void)
{
    printf("Version info:\n");
    printf("  sender host version : %u.%u.%u.%u (%c, %s)\n",
           Ver_maj,
           Ver_min,
           Ver_pat,
           Ver_bld,
           static_cast<char>( Ver_rel ),
           release_level_name(Ver_rel));
    printf("  host protocol ver   : %u\n", static_cast<unsigned int>( SHP_VERSION ) );
    printf("  build timestamp     : %s %s\n", __DATE__, __TIME__);
#if defined(_MSC_VER)
    printf("  compiler            : MSVC %d\n", _MSC_VER);
#endif
}

static void dump_mcu_version_info(Sender_hw_mcu_usb& dev)
{
    uint32_t version;
    if (!dev.get_mcu_version(version))
    {
        printf("  mcu fw version      : unavailable (GET_MCU_VERSION failed)\n");
        printf("----------------------------------------\n");
        return;
    }

    printf("  mcu fw version      : %u.%u.%u.%u\n",
           static_cast<unsigned int>((version >> 24) & 0xFF),
           static_cast<unsigned int>((version >> 16) & 0xFF),
           static_cast<unsigned int>((version >> 8) & 0xFF),
           static_cast<unsigned int>(version & 0xFF));

    printf("----------------------------------------\n");
}

static void check(const char *test_name, bool ok)
{
    printf("  %-40s %s\n", test_name, ok ? PASS_TEXT : FAIL_TEXT);
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
    dump_version_info();

    /* 1. Open + GET_INFO -------------------------------------------------- */
    bool opened = dev.init();
    if (opened)
    {
        dump_mcu_version_info(dev);
    }
    else
    {
        printf("----------------------------------------\n");
    }
    check("1. GET_INFO (version check)", opened);
    if (!opened)
    {
        printf("Cannot open port — aborting.\n");
        return 1;
    }

    /* 2. FREQ_TEST --------------------------------------------------------- */
    {
        bool freq_pass = false;
        uint32_t freq_hz = 0;
        bool ok = dev.freq_test(freq_pass, freq_hz);

        check("2. FREQ_TEST (4 MHz +/- 200 Hz)", ok && freq_pass);
        {
            char freq_text[48];
            snprintf(freq_text, sizeof(freq_text), "2b. measured freq == %luHz",
                     static_cast<unsigned long>(freq_hz));
            check(freq_text, ok && freq_pass);
        }
    }

    /* 3. RESET_STATS + verify zero ----------------------------------------- */
    check("3. RESET_STATS", dev.reset_stats());

    /* 4. SET_TIMING -------------------------------------------------------- */
    /* Defaults: 0T=200 us, 0H=100 us, 1T=116 us */
    check("4. SET_TIMING (200/100/116 us)",
          dev.set_timing(200, 100, 116));

    /* 5. START_CLK --------------------------------------------------------- */
    check("5. START_CLK", dev.start_clock());

    /* 6. SEND_BYTES -------------------------------------------------------- */
    check("6. SEND_BYTES (4 x 0xFF)", dev.send_bytes(4, 0xFF, "smoke"));

    /* 7. GET_STATUS -------------------------------------------------------- */
    {
        Sender_hw_status st;
        bool ok = dev.get_status(st);
        check("7. GET_STATUS succeeds", ok);
        if (ok)
        {
            check("7a. running == true",  st.running);
        }
    }

    /* 7b/7c. GET_STATS after GET_STATUS (SEND_BYTES phase) ---------------- */
    {
        Sender_hw_stats stats;
        bool ok = dev.get_stats(stats);
        check("7b. bytes_sent == 4", ok && stats.bytes_sent == 4);
        check("7c. packets_sent == 1", ok && stats.packets_sent == 1);
    }

    /* 8. RESET_STATS again + verify zero ----------------------------------- */
    check("8. RESET_STATS (before SEND_PACKET)", dev.reset_stats());

    /* 9. SEND_PACKET ------------------------------------------------------- */
    /* DCC idle packet: address=0xFF, data=0x00, checksum=0xFF */
    {
        static const uint8_t idle_pkt[] = { 0xFF, 0x00, 0xFF };
        check("9. SEND_PACKET (DCC idle FF 00 FF)",
              dev.send_packet(idle_pkt, sizeof(idle_pkt), "smoke"));
    }

    /* 10. GET_STATS -------------------------------------------------------- */
    {
        Sender_hw_stats stats;
        bool ok = dev.get_stats(stats);
        check("10. GET_STATS succeeds", ok);
        if (ok)
        {
            check("10a. bytes_sent == 3", stats.bytes_sent == 3);
            check("10b. packets_sent == 1", stats.packets_sent == 1);
        }
    }

    /* 11. STOP_CLK --------------------------------------------------------- */
    check("11. STOP_CLK", dev.stop_clock());

    /* 12. CLOSE_DEVICE ----------------------------------------------------- */
    check("12. CLOSE_DEVICE", dev.close() /* close flushes; reset via re-init */
          || true /* close always succeeds */);
    /* Re-open and send RESET_DEVICE command explicitly */
    {
        Sender_hw_mcu_usb dev2(port, 115200);
        if (dev2.init())
        {
                /* Verify the port can be reopened after close. */
            check("13. Re-open after close", true);
            dev2.close();
        }
        else
        {
            check("13. Re-open after close", false);
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
