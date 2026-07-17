/* Test cases TC-1..TC-4 (primary MCU) of the project's seven documented
 * cases. TC-5..TC-7 live in firmware/safety_monitor/test/test_cases.c.
 * Mirrors docs/TEST_CASES.md. */
#include "unity.h"
#include "long_control.h"
#include "health_report.h"

void setUp(void) {}
void tearDown(void) {}

/* TC-1  long_step(): zero speed error -> 0.0 m/s^2 command */
void test_tc1_zero_error_zero_accel(void)
{
    long_ctrl_t c;
    long_init(&c, 0.8f, 0.3f, -3.0f, 1.5f, 0.8f);
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.0f, long_step(&c, 10.0f, 10.0f, 0.01f));
}

/* TC-2  Jerk limit 0.8 m/s^3: first 10 ms step toward a 30 m/s error is
 *       capped at exactly 0.008 m/s^2 */
void test_tc2_first_step_jerk_limited(void)
{
    long_ctrl_t c;
    long_init(&c, 0.8f, 0.3f, -3.0f, 1.5f, 0.8f);
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.008f, long_step(&c, 30.0f, 0.0f, 0.01f));
}

/* TC-3  Sustained demand saturates at accel_max = 1.5 m/s^2 (comfort cap) */
void test_tc3_accel_saturation(void)
{
    long_ctrl_t c;
    long_init(&c, 0.8f, 0.3f, -3.0f, 1.5f, 0.8f);
    float a = 0.0f;
    for (int i = 0; i < 1000; i++) a = long_step(&c, 30.0f, 0.0f, 0.01f);
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 1.5f, a);
}

/* TC-4  health_encode/decode: seq 12345, accel -1500 mm/s^2 survive the
 *       18-byte CRC'd frame bit-exact */
void test_tc4_health_frame_roundtrip(void)
{
    health_frame_t in = { .seq = 12345, .task_overruns = 2,
                          .cpu_load_pct_x10 = 653, .error_flags = ERR_CONTROL_SAT,
                          .accel_cmd_mm_s2 = -1500, .steer_cmd_centideg = 250 };
    uint8_t buf[HEALTH_WIRE_SIZE];
    health_frame_t out;
    TEST_ASSERT_EQUAL_size_t(HEALTH_WIRE_SIZE, health_encode(&in, buf, sizeof(buf)));
    TEST_ASSERT_TRUE(health_decode(buf, sizeof(buf), &out));
    TEST_ASSERT_EQUAL_UINT32(12345, out.seq);
    TEST_ASSERT_EQUAL_INT16(-1500, out.accel_cmd_mm_s2);
}
