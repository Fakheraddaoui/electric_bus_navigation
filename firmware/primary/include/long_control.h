#ifndef LONG_CONTROL_H
#define LONG_CONTROL_H
/* Longitudinal controller: speed tracking with jerk-limited accel command. */
#include <stdbool.h>

typedef struct {
    float kp, ki;
    float integral;
    float accel_min, accel_max;   /* m/s^2 */
    float jerk_limit;             /* m/s^3 */
    float last_accel;
} long_ctrl_t;

void  long_init(long_ctrl_t *c, float kp, float ki,
                float accel_min, float accel_max, float jerk_limit);
float long_step(long_ctrl_t *c, float v_target, float v_actual, float dt);
void  long_reset(long_ctrl_t *c);

#endif
