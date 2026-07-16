#include "long_control.h"

static float clampf(float v, float lo, float hi)
{ return v < lo ? lo : (v > hi ? hi : v); }

void long_init(long_ctrl_t *c, float kp, float ki,
               float accel_min, float accel_max, float jerk_limit)
{
    c->kp = kp; c->ki = ki;
    c->accel_min = accel_min; c->accel_max = accel_max;
    c->jerk_limit = jerk_limit;
    long_reset(c);
}

void long_reset(long_ctrl_t *c)
{
    c->integral = 0.0f;
    c->last_accel = 0.0f;
}

float long_step(long_ctrl_t *c, float v_target, float v_actual, float dt)
{
    if (dt <= 0.0f) return c->last_accel;

    const float err = v_target - v_actual;
    c->integral = clampf(c->integral + err * dt, -2.0f, 2.0f);

    float a = clampf(c->kp * err + c->ki * c->integral, c->accel_min, c->accel_max);

    /* Jerk limiting for passenger comfort (it's a bus) */
    const float max_da = c->jerk_limit * dt;
    a = clampf(a, c->last_accel - max_da, c->last_accel + max_da);

    c->last_accel = a;
    return a;
}
