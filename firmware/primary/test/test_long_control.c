#include "unity.h"
#include "long_control.h"

static long_ctrl_t c;

void setUp(void)    { long_init(&c, 0.8f, 0.3f, -3.0f, 1.5f, 0.8f); }
void tearDown(void) {}

void test_zero_error_zero_command(void)
{
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.0f, long_step(&c, 10.0f, 10.0f, 0.01f));
}

void test_command_respects_accel_limits(void)
{
    float a = 0.0f;
    for (int i = 0; i < 1000; i++) a = long_step(&c, 30.0f, 0.0f, 0.01f);
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 1.5f, a);
    for (int i = 0; i < 1000; i++) a = long_step(&c, 0.0f, 30.0f, 0.01f);
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, -3.0f, a);
}

void test_jerk_limit_bounds_step_change(void)
{
    float prev = long_step(&c, 20.0f, 0.0f, 0.01f);
    for (int i = 0; i < 200; i++) {
        float a = long_step(&c, 20.0f, 0.0f, 0.01f);
        TEST_ASSERT_TRUE(a - prev <= 0.8f * 0.01f + 1e-6f);
        prev = a;
    }
}

void test_first_command_also_jerk_limited(void)
{
    float a = long_step(&c, 30.0f, 0.0f, 0.01f);
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.008f, a); /* 0 + jerk*dt */
}

void test_invalid_dt_holds_last_command(void)
{
    float a1 = long_step(&c, 10.0f, 0.0f, 0.01f);
    TEST_ASSERT_EQUAL_FLOAT(a1, long_step(&c, 10.0f, 0.0f, 0.0f));
}
