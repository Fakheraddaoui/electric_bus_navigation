/* Test cases TC-5..TC-7 (safety monitor MCU) of the project's seven
 * documented cases. TC-1..TC-4 live in firmware/primary/test/test_cases.c.
 * Mirrors docs/TEST_CASES.md. */
#include "unity.h"
#include "takeover_fsm.h"

static safety_monitor_t sm;

static health_frame_t healthy(uint32_t seq)
{
    health_frame_t h = { .seq = seq, .task_overruns = 0,
                         .cpu_load_pct_x10 = 400, .error_flags = 0 };
    return h;
}

void setUp(void)    { sm_init(&sm, 100, 5, 900); }
void tearDown(void) {}

/* TC-5  Corrupted health frame (flipped byte) -> decode false: monitor
 *       never acts on bad data */
void test_tc5_corrupt_health_frame_rejected(void)
{
    health_frame_t in = healthy(1), out;
    uint8_t buf[HEALTH_WIRE_SIZE];
    health_encode(&in, buf, sizeof(buf));
    buf[5] ^= 0xA5;
    TEST_ASSERT_FALSE(health_decode(buf, sizeof(buf), &out));
}

/* TC-6  Primary silent 200 ms (timeout 100 ms) -> TAKEOVER, monitor has
 *       authority, decel command -2.0 m/s^2 */
void test_tc6_frame_timeout_takeover(void)
{
    health_frame_t h = healthy(1);
    sm_on_frame(&sm, &h, 0);
    TEST_ASSERT_EQUAL(SM_NORMAL, sm_tick(&sm, 50, 12.0f));
    TEST_ASSERT_EQUAL(SM_TAKEOVER, sm_tick(&sm, 200, 12.0f));
    TEST_ASSERT_TRUE(sm_monitor_has_authority(&sm));
    TEST_ASSERT_EQUAL_FLOAT(-2.0f, sm_decel_command(&sm));
}

/* TC-7  Primary reports EtherCAT down -> CONTROLLED_STOP at -1.2 m/s^2
 *       (primary keeps steering); at standstill latches SAFE_STOPPED */
void test_tc7_controlled_stop_to_latched_safe_stop(void)
{
    health_frame_t h1 = healthy(1); h1.error_flags = ERR_ETHERCAT_DOWN;
    sm_on_frame(&sm, &h1, 0);  sm_tick(&sm, 5, 12.0f);
    health_frame_t h2 = healthy(2); h2.error_flags = ERR_ETHERCAT_DOWN;
    sm_on_frame(&sm, &h2, 10);
    TEST_ASSERT_EQUAL(SM_CONTROLLED_STOP, sm_tick(&sm, 15, 12.0f));
    TEST_ASSERT_EQUAL_FLOAT(-1.2f, sm_decel_command(&sm));
    TEST_ASSERT_FALSE(sm_monitor_has_authority(&sm));

    health_frame_t h3 = healthy(3);
    sm_on_frame(&sm, &h3, 20);
    TEST_ASSERT_EQUAL(SM_SAFE_STOPPED, sm_tick(&sm, 25, 0.05f));
    health_frame_t h4 = healthy(4);
    sm_on_frame(&sm, &h4, 30);
    TEST_ASSERT_EQUAL(SM_SAFE_STOPPED, sm_tick(&sm, 35, 0.0f)); /* latched */
}
