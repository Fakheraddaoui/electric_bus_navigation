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

void test_stays_normal_with_healthy_frames(void)
{
    for (uint32_t t = 0; t < 1000; t += 10) {
        health_frame_t h = healthy(t / 10 + 1);
        sm_on_frame(&sm, &h, t);
        TEST_ASSERT_EQUAL(SM_NORMAL, sm_tick(&sm, t, 12.0f));
    }
    TEST_ASSERT_FALSE(sm_monitor_has_authority(&sm));
}

void test_frame_timeout_triggers_takeover(void)
{
    health_frame_t h = healthy(1);
    sm_on_frame(&sm, &h, 0);
    sm_tick(&sm, 50, 12.0f);
    TEST_ASSERT_EQUAL(SM_TAKEOVER, sm_tick(&sm, 200, 12.0f));
    TEST_ASSERT_TRUE(sm_monitor_has_authority(&sm));
    TEST_ASSERT_EQUAL_FLOAT(-2.0f, sm_decel_command(&sm));
}

void test_sequence_stall_triggers_takeover(void)
{
    for (uint32_t t = 0; t < 100; t += 10) {
        health_frame_t h = healthy(7);   /* seq frozen */
        sm_on_frame(&sm, &h, t);
        sm_tick(&sm, t, 12.0f);
    }
    TEST_ASSERT_EQUAL(SM_TAKEOVER, sm.state);
}

void test_overruns_cause_controlled_stop_not_takeover(void)
{
    health_frame_t h = healthy(1);
    h.task_overruns = 10;
    sm_on_frame(&sm, &h, 0);
    sm_tick(&sm, 5, 12.0f);              /* NORMAL -> DEGRADED */
    health_frame_t h2 = healthy(2);
    h2.task_overruns = 11;
    sm_on_frame(&sm, &h2, 10);
    TEST_ASSERT_EQUAL(SM_CONTROLLED_STOP, sm_tick(&sm, 15, 12.0f));
    TEST_ASSERT_EQUAL_FLOAT(-1.2f, sm_decel_command(&sm));
    TEST_ASSERT_FALSE(sm_monitor_has_authority(&sm)); /* primary still steers */
}

void test_stop_completes_into_safe_stopped_latch(void)
{
    health_frame_t h = healthy(1);
    h.error_flags = ERR_ETHERCAT_DOWN;
    sm_on_frame(&sm, &h, 0);
    sm_tick(&sm, 5, 12.0f);
    health_frame_t h2 = healthy(2);
    h2.error_flags = ERR_ETHERCAT_DOWN;
    sm_on_frame(&sm, &h2, 10);
    sm_tick(&sm, 15, 12.0f);
    TEST_ASSERT_EQUAL(SM_CONTROLLED_STOP, sm.state);

    health_frame_t h3 = healthy(3);
    sm_on_frame(&sm, &h3, 20);
    TEST_ASSERT_EQUAL(SM_SAFE_STOPPED, sm_tick(&sm, 25, 0.05f));
    /* Latched even if everything looks fine afterwards */
    health_frame_t h4 = healthy(4);
    sm_on_frame(&sm, &h4, 30);
    TEST_ASSERT_EQUAL(SM_SAFE_STOPPED, sm_tick(&sm, 35, 0.0f));
}

void test_cpu_overload_degrades(void)
{
    health_frame_t h = healthy(1);
    h.cpu_load_pct_x10 = 950;
    sm_on_frame(&sm, &h, 0);
    TEST_ASSERT_EQUAL(SM_DEGRADED, sm_tick(&sm, 5, 12.0f));
}
