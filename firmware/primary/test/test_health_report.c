#include "unity.h"
#include "health_report.h"

void setUp(void) {}
void tearDown(void) {}

void test_roundtrip(void)
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
    TEST_ASSERT_EQUAL_HEX16(ERR_CONTROL_SAT, out.error_flags);
}

void test_corruption_detected(void)
{
    health_frame_t in = { .seq = 1 };
    uint8_t buf[HEALTH_WIRE_SIZE];
    health_frame_t out;
    health_encode(&in, buf, sizeof(buf));
    buf[5] ^= 0xA5;
    TEST_ASSERT_FALSE(health_decode(buf, sizeof(buf), &out));
}

void test_short_buffers_rejected(void)
{
    health_frame_t h = {0};
    uint8_t buf[8];
    TEST_ASSERT_EQUAL_size_t(0, health_encode(&h, buf, sizeof(buf)));
    TEST_ASSERT_FALSE(health_decode(buf, sizeof(buf), &h));
}
